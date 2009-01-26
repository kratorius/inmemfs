#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/errors.h"
#include "../src/shell.h"
#include "../src/node.h"

START_TEST (test_node_creation)
{
	struct node *node;

	node = node_create("File node", N_FILE);
	fail_if (strcmp(node->name, "File node") != 0,
			"Node name is not set correctly");
	fail_if (node->type != N_FILE,
			"Node type is not set correctly");
	node_delete(node);

	node = node_create("Directory node", N_DIRECTORY);
	fail_if (strcmp(node->name, "Directory node") != 0,
			"Node name is not set correctly");
	fail_if (node->type != N_DIRECTORY,
			"Node type is not set correctly");
	node_delete(node);
}
END_TEST

START_TEST (test_node_child_addition)
{
	struct node *father, *children1, *children2;

	father = node_create("Test node", N_DIRECTORY);
	children1 = node_create("Children 1", N_FILE);
	children2 = node_create("Children 2", N_FILE);

	fail_unless (node_add_child(father, children1) != E_FILE_CHILD,
			"Can't add a children to a father node");

	fail_if (node_add_child(children1, children2) != E_FILE_CHILD,
			"Added a file node to a directory node");

	fail_unless (node_get_children_no(father) == 1,
			"Children number returned doesn't match real children number");

	node_delete(father);
}
END_TEST

START_TEST (test_node_list_creation)
{
	struct node_list *nl = NULL;

	nl = node_list_create();

	fail_if (nl == NULL,
			"Node list hasn't been created");
}
END_TEST

START_TEST (test_node_list_add_siblings)
{
	struct node_list *nl, *tmpnl;
	struct node *n1, *n2, *n3;
	int i = 0;

	nl = node_list_create();

	n1 = node_create("Node 1", N_FILE);
	n2 = node_create("Node 2", N_FILE);
	n3 = node_create("Node 3", N_FILE);

	fail_unless (node_list_add_sibling(nl, n1) == n1,
			"Returned object mismatch from expected object");
	fail_unless (node_list_add_sibling(nl, n2) == n2,
			"Returned object mismatch from expected object");
	fail_unless (node_list_add_sibling(nl, n3) == n3,
			"Returned object mismatch from expected object");

	tmpnl = nl;
	while (tmpnl->next != NULL) {
		tmpnl = tmpnl->next;
		i++;
	}

	fail_unless(i == 3,
			"Unexpected number of siblings");
}
END_TEST

START_TEST (test_shell_invalid_command)
{
	int ret = shell_parse_line("Invalid command");
	fail_unless (ret == E_CMD_NOT_FOUND,
			"Invalid command produces unexpected output");
}
END_TEST

START_TEST (test_shell_invalid_syntax)
{
	int ret;

	ret = shell_parse_line("createroot");
	fail_unless (ret == E_INVALID_SYNTAX,
			"Invalid syntax is not recognized on createroot");

	ret = shell_parse_line("setroot a");
	fail_unless (ret == E_INVALID_SYNTAX,
			"Invalid syntax is not recognized on setroot");
}
END_TEST

START_TEST (test_shell_root_limits)
{
	int i, ret;

	for(i = 0; i < MAX_ROOT_NODES; i++)
		shell_parse_line("createroot xxx");

	ret = shell_parse_line("createroot over_limit");

	/* revert changes back */
	for(i = 0; i < MAX_ROOT_NODES; i++)
		shell_parse_line("deleteroot 1");

	fail_unless(ret == E_CANNOT_PROCEED,
			"Resource limits are not hitted");
}
END_TEST

START_TEST (test_shell_no_root_selected)
{
	int ret = shell_parse_line("mkdir testdir");
	fail_unless(ret == E_NO_ROOT,
			"A root node is found even if none is selected");
}
END_TEST

Suite *
inmemfs_suite(void) {
	Suite *s = suite_create("Master");
	TCase *tc_core = tcase_create("Tree tests");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_node_creation);
	tcase_add_test(tc_core, test_node_child_addition);
	tcase_add_test(tc_core, test_node_list_creation);
	tcase_add_test(tc_core, test_node_list_add_siblings);

	tcase_add_test(tc_core, test_shell_invalid_command);
	tcase_add_test(tc_core, test_shell_invalid_syntax);
	tcase_add_test(tc_core, test_shell_root_limits);
	tcase_add_test(tc_core, test_shell_no_root_selected);

	return s;
}

int
main(int argc, char **argv) {
	int number_failed;

	Suite *s = inmemfs_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_VERBOSE);

	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}