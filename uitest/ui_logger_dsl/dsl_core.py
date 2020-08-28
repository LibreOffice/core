#!/usr/bin/env python3
# This file is part of the LibreOffice UI_logger project.
#
# This file contain the implementation of the Compiler
# for the new logger grammar
#
# ul stands for Ui_Logger

import os
import sys
import argparse
import keyword

try:
    from textx.metamodel import metamodel_from_file
except ImportError:
    print("textx is a required package.")
    print('Please install the package for example with "pip3 install --user textx"')
    sys.exit(1)

tab = "    "
double_tab = "        "


def parse_args():
    """
    This function parses the command-line arguments
    to get the input and output file details
    """
    parser = argparse.ArgumentParser(description="Generate a UI test file from log")
    parser.add_argument("input_address", type=str, help="The log file address")
    parser.add_argument("output_address", type=str, help="The test file address")
    args = parser.parse_args()
    return args


class ul_Compiler:
    prev_command = ""
    variables = []
    objects = dict()
    current_app = ""
    parent_hierarchy_count = 0
    last_parent = []
    flag_for_QuerySaveDialog = False
    math_element_selector_initializer= False;

    def __init__(self, input_address, output_address):
        self.ui_dsl_mm = metamodel_from_file("ui_logger_dsl_grammar.tx")
        self.output_stream = self.initiate_test_generation(output_address)
        self.input_address = input_address

    def get_log_file(self, input_address):
        try:
            # load the program
            content = self.ui_dsl_mm.model_from_file(input_address)
        except IOError as err:
            print("IO error: {0}".format(err))
            print(
                "Use " + os.path.basename(sys.argv[0]) + " -h to get usage instructions"
            )
            sys.exit(1)

        return content

    def initiate_test_generation(self, output_address):
        self.last_parent.append("MainWindow")
        try:
            f = open(output_address, "w")
        except IOError as err:
            print("IO error: {0}".format(err))
            print(
                "Use " + os.path.basename(sys.argv[0]) + " -h to get usage instructions"
            )
            sys.exit(1)
        line = (
            "# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-\n\n"
            + "from uitest.framework import UITestCase\n"
            + "from libreoffice.uno.propertyvalue import mkPropertyValues\n"
            + "from uitest.uihelper.common import get_state_as_dict\n"
            + "import importlib\n\n"
            + "class TestClass(UITestCase):\n"
            + tab
            + "def test_function(self):\n"
        )

        self.variables.append(line)

        return f

    def compile(self):
        self.ui_dsl_mm.register_obj_processors(
            {
                "UNOCommand": self.handle_uno,
                "StarterCommand": self.handle_start,
                "CloseDialog": self.handle_Dialog,
                "OpenModelessDialog": self.handle_Dialog,
                "OpenModalDialog": self.handle_Dialog,
                "ButtonUIObject": self.handle_button,
                "CheckBoxUIObject": self.handle_check_box,
                "TabControlUIObject": self.handle_tab,
                "ComboBoxUIObject": self.handle_Combo_box,
                "RadioButtonUIObject": self.handle_Radio_button,
                "ListBoxUIObject": self.handle_List_box,
                "SpinFieldUIObject": self.handle_spin_field,
                "EditUIObject": self.handle_Edit_uiObject,
                "ToolBoxUIObject": self.handle_ToolBox_uiObject,
                "ValueSetUIObject": self.handle_ValueSet_uiObject,
                "writer_Type_command": self.handle_writer_type,
                "writer_Select_command": self.handle_writer_select,
                "writer_GOTO_command": self.handle_writer_goto,
                "calc_Select_cell": self.handle_calc_select,
                "calc_switch_sheet": self.handle_calc_switch_sheet,
                "calc_Type_command": self.handle_calc_Type_command,
                "calc_AutoFill_filter": self.handle_calc_AutoFill_filter,
                "calc_SelectMenu_filter": self.handle_calc_SelectMenu_filter,
                "calc_Open_Comment": self.handle_calc_Open_Comment,
                "calc_Close_Comment": self.handle_calc_Close_Comment,
                "impress_Type_command": self.handle_impress_Type_command,
                "math_element_selector": self.handle_math_element_selector,
                "math_Type_command": self.handle_math_Type_command,
                "setZoom_command": self.handle_setZoom_command,
                "draw_Type_command": self.handle_draw_Type_command,
                "SideBar": self.handle_SideBar,
                "writer_Comment_leave":self.handle_writer_Comment_leave,
                "writer_Comment_show":self.handle_writer_Comment_show,
                "writer_Comment_hide":self.handle_writer_Comment_hide,
                "writer_Comment_delete":self.handle_writer_Comment_delete,
                "writer_Comment_setresolved":self.handle_writer_Comment_setresolved,
                "writer_Copy_Text": self.do_nothing,
                "writer_Cut_Text": self.do_nothing,
                "writer_Paste_Text": self.do_nothing,
                "writer_Insert_BreakPage": self.do_nothing,
                "writer_Create_table": self.do_nothing,
                "calc_Remove_Content": self.do_nothing,
                "calc_Delete_Cells": self.do_nothing,
                "calc_insert_cells": self.do_nothing,
                "calc_Cut_Cells": self.do_nothing,
                "calc_Copy_Cells": self.do_nothing,
                "calc_Merge_Cells": self.do_nothing,
                "calc_UNMerge_Cells": self.do_nothing,
                "calc_Rename_Sheet": self.do_nothing,
                "calc_Insert_sheet": self.do_nothing,
                "impress_Insert_Slide": self.do_nothing,
                "impress_Delete_Page": self.do_nothing,
                "impress_Duplicate_Slide": self.do_nothing,
                "impress_Rename_Slide": self.do_nothing,
                "draw_Insert_Page": self.do_nothing,
                "draw_Delete_Page": self.do_nothing,
                "draw_Rename_Page": self.do_nothing,
            }
        )

        self.log_lines = self.get_log_file(self.input_address)

    def init_app(self):
        if self.current_app in self.objects:
            self.objects[self.current_app] += 1
        else:
            self.objects[self.current_app] = 1
            line = (
                double_tab
                + self.current_app
                + ' = MainWindow.getChild("'
                + self.current_app
                + '")\n'
            )
            self.variables.append(line)

    def init_Object(self, Id_of_Object, name_of_child, Obj_parent):

        if Id_of_Object in self.objects:
            self.objects[Id_of_Object] += 1
        else:
            self.objects[Id_of_Object] = 1
            line = (
                double_tab
                + Id_of_Object
                + " = "
                + Obj_parent
                + '.getChild("'
                + name_of_child
                + '")\n'
            )

            self.variables.append(line)

    def write_line_without_parameters(self, Action_holder, Action, Action_type):
        line = (
            double_tab
            + Action_holder
            + '.executeAction("'
            + Action
            + '",'
            + Action_type
            + "())\n"
        )
        self.variables.append(line)

    def write_line_with_one_parameters(
        self, Action_holder, Action, Parameter_name, parameter_value
    ):
        line = (
            double_tab
            + Action_holder
            + '.executeAction("'
            + Action
            + '", mkPropertyValues({"'
            + Parameter_name
            + '": "'
            + str(parameter_value)
            + '"}))\n'
        )
        self.variables.append(line)

    def write_line_with_two_parameters(
        self,
        Action_holder,
        Action,
        Parameter_name_1,
        parameter_value_1,
        Parameter_name_2,
        parameter_value_2,
    ):

        line = (
            double_tab
            + Action_holder
            + '.executeAction("'
            + Action
            + '", mkPropertyValues({"'
            + Parameter_name_1
            + '": "'
            + str(parameter_value_1)
            + '", "'
            + Parameter_name_2
            + '": "'
            + str(parameter_value_2)
            + '"}))\n'
        )
        self.variables.append(line)

    def handle_uno(self, UNOCommand):
        if UNOCommand.parameters == None:
            line = (
                double_tab
                + 'self.xUITest.executeCommand("'
                + UNOCommand.uno_command_name
                + '")\n'
            )
        else:
            parameters = ""
            for p in UNOCommand.parameters.parameter_data:
                parameters = parameters + '"' + p.key + '" : ' + str(p.value) + " ,"
            parameters = parameters[:-1]

            line = (
                double_tab
                + 'self.xUITest.executeCommandWithParameters("'
                + UNOCommand.uno_command_name
                + '", mkPropertyValues({'
                + parameters
                + "}) )\n"
            )

        self.variables.append(line)
        self.prev_command = UNOCommand

    def handle_start(self, StarterCommand):
        line = (
            double_tab
            + 'MainDoc = self.ui_test.create_doc_in_start_center("'
            + StarterCommand.program_name
            + '")\n'
        )
        self.variables.append(line)

        line = double_tab + "MainWindow = self.xUITest.getTopFocusWindow()\n"
        self.variables.append(line)
        app = {
            "writer": "writer_edit",
            "calc": "grid_window",
            "impress": "impress_win",
            "math": "math_edit",
            "draw": "draw_win",
        }
        self.current_app = app[StarterCommand.program_name]
        self.prev_command = StarterCommand

    def handle_SideBar(self, SideBar):

        line = '        self.xUITest.executeCommand(".uno:Sidebar")\n'
        self.variables.append(line)

        self.write_line_with_one_parameters(
            "MainWindow", "SIDEBAR", "PANEL", SideBar.name
        )

        self.prev_command = SideBar

    def handle_Dialog(self, DialogCommand):

        if DialogCommand.__class__.__name__ == "OpenModalDialog":

            if DialogCommand.dialog_name != "QuerySaveDialog":
                # This part is just to ignore saving the Save dialog while closing the app

                old_line = self.variables.pop()
                if self.prev_command.__class__.__name__ == "UNOCommand":
                    key_word = self.prev_command.uno_command_name[-6:]
                else:
                    key_word = old_line[-9:-3]

                if key_word == "Dialog":
                    old_line = (
                        double_tab
                        + 'self.ui_test.execute_dialog_through_command("'
                        + self.prev_command.uno_command_name
                        + '")\n'
                    )
                self.variables.append(old_line)
                line = (
                    double_tab
                    + DialogCommand.dialog_name
                    + " = self.xUITest.getTopFocusWindow()\n"
                )
                self.variables.append(line)
                self.last_parent.append(DialogCommand.dialog_name)
                self.parent_hierarchy_count = self.parent_hierarchy_count + 1

            else:
                self.flag_for_QuerySaveDialog = True

        elif DialogCommand.__class__.__name__ == "OpenModelessDialog":
            old_line = self.variables.pop()
            if self.prev_command.__class__.__name__ == "UNOCommand":
                key_word = self.prev_command.uno_command_name[-6:]
            else:
                key_word = old_line[-9:-3]

            if key_word == "Dialog":
                old_line = (
                    double_tab
                    + 'self.ui_test.execute_modeless_dialog_through_command("'
                    + self.prev_command.uno_command_name
                    + '")\n'
                )
            self.variables.append(old_line)
            line = (
                double_tab
                + DialogCommand.dialog_name
                + "  = self.xUITest.getTopFocusWindow()\n"
            )
            self.variables.append(line)
            self.last_parent.append(DialogCommand.dialog_name)
            self.parent_hierarchy_count = self.parent_hierarchy_count + 1

        elif DialogCommand.__class__.__name__ == "CloseDialog":

            if not (self.flag_for_QuerySaveDialog):
                # This part is just to ignore saving the Save dialog while closing the app

                if self.prev_command.__class__.__name__ == "ButtonUIObject":
                    old_line = self.variables.pop()
                    line = ""
                    if keyword.iskeyword(self.prev_command.ui_button):
                        line = (
                            double_tab
                            + "self.ui_test.close_dialog_through_button(x"
                            + self.prev_command.ui_button
                            + ")\n"
                        )
                    else:
                        line = (
                            double_tab
                            + "self.ui_test.close_dialog_through_button("
                            + self.prev_command.ui_button
                            + ")\n"
                        )
                    self.variables.append(line)
                self.last_parent.pop()
                self.parent_hierarchy_count = self.parent_hierarchy_count - 1
            else:
                self.flag_for_QuerySaveDialog = False

        # This is to solve the problem of re-using the same id again in different Dialogs

        self.objects.clear()

        self.prev_command = DialogCommand

    def handle_button(self, ButtonUIObject):

        if ButtonUIObject.parent_id != "QuerySaveDialog":
            # This part is just to ignore saving the Save dialog while closing the app

            name_of_obj = ""
            if keyword.iskeyword(ButtonUIObject.ui_button):
                name_of_obj = "x" + ButtonUIObject.ui_button
            else:
                name_of_obj = ButtonUIObject.ui_button

            if ButtonUIObject.parent_id == "":
                self.init_Object(
                    name_of_obj,
                    ButtonUIObject.ui_button,
                    self.last_parent[self.parent_hierarchy_count],
                )
            else:
                self.init_Object(
                    name_of_obj, ButtonUIObject.ui_button, ButtonUIObject.parent_id
                )

            self.write_line_without_parameters(name_of_obj, "CLICK", "tuple")

            self.prev_command = ButtonUIObject

    def handle_check_box(self, CheckBoxUIObject):

        name_of_obj = ""
        if keyword.iskeyword(CheckBoxUIObject.Check_box_id):
            name_of_obj = "x" + CheckBoxUIObject.Check_box_id
        else:
            name_of_obj = CheckBoxUIObject.Check_box_id

        if CheckBoxUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                CheckBoxUIObject.Check_box_id,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj, CheckBoxUIObject.Check_box_id, CheckBoxUIObject.parent_id
            )

        self.write_line_without_parameters(name_of_obj, "CLICK", "tuple")

        self.prev_command = CheckBoxUIObject

    def handle_tab(self, TabControlUIObject):

        name_of_obj = ""
        if keyword.iskeyword(TabControlUIObject.tab_id):
            name_of_obj = "x" + TabControlUIObject.tab_id
        else:
            name_of_obj = TabControlUIObject.tab_id

        if TabControlUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                TabControlUIObject.tab_id,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj, TabControlUIObject.tab_id, TabControlUIObject.parent_id
            )

        self.write_line_with_one_parameters(
            name_of_obj, "SELECT", "POS", TabControlUIObject.tab_page_number
        )

        self.prev_command = TabControlUIObject

    def handle_Combo_box(self, ComboBoxUIObject):

        name_of_obj = ""
        if keyword.iskeyword(ComboBoxUIObject.Combo_box_id):
            name_of_obj = "x" + ComboBoxUIObject.Combo_box_id
        else:
            name_of_obj = ComboBoxUIObject.Combo_box_id

        if ComboBoxUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                ComboBoxUIObject.Combo_box_id,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj, ComboBoxUIObject.Combo_box_id, ComboBoxUIObject.parent_id
            )

        self.write_line_with_one_parameters(
            name_of_obj, "SELECT", "POS", ComboBoxUIObject.item_num
        )

        self.prev_command = ComboBoxUIObject

    def handle_Radio_button(self, RadioButtonUIObject):

        name_of_obj = ""
        if keyword.iskeyword(RadioButtonUIObject.Radio_button_id):
            name_of_obj = "x" + RadioButtonUIObject.Radio_button_id
        else:
            name_of_obj = RadioButtonUIObject.Radio_button_id

        if RadioButtonUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                RadioButtonUIObject.Radio_button_id,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj,
                RadioButtonUIObject.Radio_button_id,
                RadioButtonUIObject.parent_id,
            )

        self.write_line_without_parameters(name_of_obj, "CLICK", "tuple")

        self.prev_command = RadioButtonUIObject

    def handle_List_box(self, ListBoxUIObject):

        name_of_obj = ""
        if keyword.iskeyword(ListBoxUIObject.list_id):
            name_of_obj = "x" + ListBoxUIObject.list_id
        else:
            name_of_obj = ListBoxUIObject.list_id

        if ListBoxUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                ListBoxUIObject.list_id,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj, ListBoxUIObject.list_id, ListBoxUIObject.parent_id
            )

        self.write_line_with_one_parameters(
            name_of_obj, "SELECT", "POS", ListBoxUIObject.POS
        )

        self.prev_command = ListBoxUIObject

    def handle_spin_field(self, SpinFieldUIObject):

        name_of_obj = ""
        if keyword.iskeyword(SpinFieldUIObject.Spin_id):
            name_of_obj = "x" + SpinFieldUIObject.Spin_id
        else:
            name_of_obj = SpinFieldUIObject.Spin_id

        if SpinFieldUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                SpinFieldUIObject.Spin_id,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj, SpinFieldUIObject.Spin_id, SpinFieldUIObject.parent_id
            )

        if SpinFieldUIObject.change == "Increase":
            self.write_line_without_parameters(name_of_obj, "UP", "tuple")
        elif SpinFieldUIObject.change == "Decrease":
            self.write_line_without_parameters(name_of_obj, "DOWN", "tuple")
        self.prev_command = SpinFieldUIObject

    def handle_Edit_uiObject(self, EditUIObject):

        name_of_obj = ""
        if keyword.iskeyword(EditUIObject.action.edit_button):
            name_of_obj = "x" + EditUIObject.action.edit_button
        else:
            name_of_obj = EditUIObject.action.edit_button

        if EditUIObject.parent_id == "":
            self.init_Object(
                name_of_obj,
                EditUIObject.action.edit_button,
                self.last_parent[self.parent_hierarchy_count],
            )
        else:
            self.init_Object(
                name_of_obj, EditUIObject.action.edit_button, EditUIObject.parent_id
            )

        if EditUIObject.action.__class__.__name__ == "Type_action":

            if EditUIObject.action.what_to_type.__class__.__name__ == "char":
                self.write_line_with_one_parameters(
                    name_of_obj,
                    "TYPE",
                    "TEXT",
                    EditUIObject.action.what_to_type.input_char,
                )

            elif EditUIObject.action.what_to_type.__class__.__name__ == "KeyCode":
                self.write_line_with_one_parameters(
                    name_of_obj,
                    "TYPE",
                    "KEYCODE",
                    EditUIObject.action.what_to_type.input_key_code,
                )

        if EditUIObject.action.__class__.__name__ == "SELECT":

            self.write_line_with_two_parameters(
                name_of_obj,
                "SELECT",
                "FROM",
                EditUIObject.action.from_pos,
                "TO",
                EditUIObject.action.to_pos,
            )

        if EditUIObject.action.__class__.__name__ == "Clear":

            self.write_line_without_parameters(name_of_obj, "CLEAR", "tuple")

        self.prev_command = EditUIObject

    def handle_ToolBox_uiObject(self, ToolBoxUIObject):
        name_of_obj = ""
        if keyword.iskeyword(ToolBoxUIObject.toolbox_id):
            name_of_obj = "x" + ToolBoxUIObject.toolbox_id
        else:
            name_of_obj = ToolBoxUIObject.toolbox_id

        self.init_Object(
            name_of_obj,
            ToolBoxUIObject.toolbox_id,
            self.last_parent[self.parent_hierarchy_count],
        )

        self.write_line_with_one_parameters(
            name_of_obj, "CLICK", "POS", ToolBoxUIObject.POS
        )

        self.prev_command = ToolBoxUIObject

    def handle_ValueSet_uiObject(self, ValueSetUIObject):

        name_of_obj = ""
        if keyword.iskeyword(ValueSetUIObject.value_set_id):
            name_of_obj = "x" + ValueSetUIObject.value_set_id
        else:
            name_of_obj = ValueSetUIObject.value_set_id

        parent_txt = ValueSetUIObject.parent_id.split("/")
        parent = parent_txt[len(parent_txt)-2]
        if( parent.upper() != self.last_parent[self.parent_hierarchy_count].upper()):
            self.init_Object(
                parent,
                parent,
                self.last_parent[self.parent_hierarchy_count],
            )

            self.init_Object(
                name_of_obj, ValueSetUIObject.value_set_id, parent
            )

        else:
            self.init_Object(
                name_of_obj, ValueSetUIObject.value_set_id, self.last_parent[self.parent_hierarchy_count]
            )

        self.write_line_with_one_parameters(
            name_of_obj, "CHOOSE", "POS", ValueSetUIObject.POS
        )

        self.prev_command = ValueSetUIObject

    def handle_writer_type(self, writer_Type_command):

        self.init_app()

        if writer_Type_command.what_to_type.__class__.__name__ == "char":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "TEXT",
                writer_Type_command.what_to_type.input_char,
            )

        elif writer_Type_command.what_to_type.__class__.__name__ == "KeyCode":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "KEYCODE",
                writer_Type_command.what_to_type.input_key_code,
            )

        self.prev_command = writer_Type_command

    def handle_writer_select(self, writer_Select_command):

        self.init_app()

        self.write_line_with_two_parameters(
            self.current_app,
            "SELECT",
            "END_POS",
            writer_Select_command.from_pos,
            "START_POS",
            writer_Select_command.to_pos,
        )

        self.prev_command = writer_Select_command

    def handle_writer_goto(self, writer_GOTO_command):

        self.init_app()

        self.write_line_with_one_parameters(
            self.current_app, "GOTO", "PAGE", writer_GOTO_command.page_num
        )

        self.prev_command = writer_GOTO_command

    def handle_calc_select(self, calc_Select_cell):

        self.init_app()

        if calc_Select_cell.select_op.__class__.__name__ == "range_of_cells":
            self.write_line_with_one_parameters(
                self.current_app,
                "SELECT",
                "RANGE",
                calc_Select_cell.select_op.input_range,
            )

        elif calc_Select_cell.select_op.__class__.__name__ == "one_cell":
            self.write_line_with_one_parameters(
                self.current_app,
                "SELECT",
                "CELL",
                calc_Select_cell.select_op.input_cell,
            )

        self.prev_command = calc_Select_cell

    def handle_calc_switch_sheet(self, calc_switch_sheet):

        self.init_app()

        self.write_line_with_one_parameters(
            self.current_app, "SELECT", "TABLE", calc_switch_sheet.sheet_num
        )

        self.prev_command = calc_switch_sheet

    def handle_calc_Type_command(self, calc_Type_command):

        self.init_app()

        if calc_Type_command.what_to_type.__class__.__name__ == "char":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "TEXT",
                calc_Type_command.what_to_type.input_char,
            )

        elif calc_Type_command.what_to_type.__class__.__name__ == "KeyCode":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "KEYCODE",
                calc_Type_command.what_to_type.input_key_code,
            )

        self.prev_command = calc_Type_command

    def handle_calc_AutoFill_filter(self, calc_AutoFill_filter):

        self.init_app()

        line = (
            double_tab
            + self.current_app
            + '.executeAction("LAUNCH", mkPropertyValues'
            + '({"AUTOFILTER": "", "COL": "'
            + str(calc_AutoFill_filter.col_num)
            + '"'
            + ', "ROW": "'
            + str(calc_AutoFill_filter.row_num)
            + '"}))\n'
        )

        self.variables.append(line)
        self.prev_command = calc_AutoFill_filter

    def handle_calc_Open_Comment(self, calc_Open_Comment):

        line = (
            double_tab
            + self.current_app
            + '.executeAction("COMMENT", mkPropertyValues'
            + '({"OPEN": " "}))\n'
        )

        self.variables.append(line)

        self.prev_command = calc_Open_Comment

    def handle_calc_Close_Comment(self, calc_Close_Comment):

        line = (
            double_tab
            + self.current_app
            + '.executeAction("COMMENT", mkPropertyValues'
            + '({"CLOSE": " "}))\n'
        )

        self.variables.append(line)

        self.prev_command = calc_Close_Comment

    def handle_calc_SelectMenu_filter(self, calc_SelectMenu_filter):

        self.init_app()

        line = (
            double_tab
            + self.current_app
            + '.executeAction("LAUNCH", mkPropertyValues'
            + '({"SELECTMENU": "", "COL": "'
            + str(calc_SelectMenu_filter.col_num)
            + '"'
            + ', "ROW": "'
            + str(calc_SelectMenu_filter.row_num)
            + '"}))\n'
        )

        self.variables.append(line)
        self.prev_command = calc_SelectMenu_filter

    def handle_impress_Type_command(self, impress_Type_command):

        self.init_app()

        if impress_Type_command.what_to_type.__class__.__name__ == "char":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "TEXT",
                impress_Type_command.what_to_type.input_char,
            )

        elif impress_Type_command.what_to_type.__class__.__name__ == "KeyCode":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "KEYCODE",
                impress_Type_command.what_to_type.input_key_code,
            )

        self.prev_command = impress_Type_command

    def handle_math_Type_command(self, math_Type_command):

        self.init_app()
        if math_Type_command.what_to_type.__class__.__name__ == "char":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "TEXT",
                math_Type_command.what_to_type.input_char,
            )

        elif math_Type_command.what_to_type.__class__.__name__ == "KeyCode":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "KEYCODE",
                math_Type_command.what_to_type.input_key_code,
            )

        self.prev_command = math_Type_command

    def handle_draw_Type_command(self, draw_Type_command):

        self.init_app()
        if draw_Type_command.what_to_type.__class__.__name__ == "char":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "TEXT",
                draw_Type_command.what_to_type.input_char,
            )

        elif draw_Type_command.what_to_type.__class__.__name__ == "KeyCode":
            self.write_line_with_one_parameters(
                self.current_app,
                "TYPE",
                "KEYCODE",
                draw_Type_command.what_to_type.input_key_code,
            )

        self.prev_command = draw_Type_command

    def handle_math_element_selector(self, math_element_selector):

        if( self.math_element_selector_initializer == False ):
            # This part to initialize the element selector in math application
            self.math_element_selector_initializer = True
            line = (
                double_tab
                + "element_selector"
                + ' = MainWindow.getChild("'
                + "element_selector"
                + '")\n'
            )
            self.variables.append(line)

        # this put a prefix of char 'x' to avoid variable with name equal to number only
        element_name="x"+str(math_element_selector.element_no)

        self.init_Object(element_name,str(math_element_selector.element_no),"element_selector")

        self.write_line_without_parameters(
            str(element_name), "SELECT", "tuple"
        )

        self.prev_command = math_element_selector

    def handle_writer_Comment_leave(self,writer_Comment_leave):

        self.init_app()

        self.init_Object(
            writer_Comment_leave.comment_id, writer_Comment_leave.comment_id, "MainWindow"
            )

        self.write_line_with_one_parameters(
            writer_Comment_leave.comment_id, "LEAVE", "", ""
        )

        self.prev_command = writer_Comment_leave

    def handle_writer_Comment_show(self,writer_Comment_show):

        self.init_app()

        self.init_Object(
            writer_Comment_show.comment_id, writer_Comment_show.comment_id, "MainWindow"
            )

        self.write_line_with_one_parameters(
            writer_Comment_show.comment_id, "SHOW", "", ""
        )

        self.prev_command = writer_Comment_show

    def handle_writer_Comment_hide(self,writer_Comment_hide):

        self.init_app()

        self.init_Object(
            writer_Comment_hide.comment_id, writer_Comment_hide.comment_id, "MainWindow"
            )

        self.write_line_with_one_parameters(
            writer_Comment_hide.comment_id, "HIDE", "", ""
        )

        self.prev_command = writer_Comment_hide

    def handle_writer_Comment_delete(self,writer_Comment_delete):

        self.init_app()

        self.init_Object(
            writer_Comment_delete.comment_id, writer_Comment_delete.comment_id, "MainWindow"
            )

        self.write_line_with_one_parameters(
            writer_Comment_delete.comment_id, "DELETE", "", ""
        )

        self.prev_command = writer_Comment_delete

    def handle_writer_Comment_setresolved(self,writer_Comment_setresolved):

        self.init_app()

        self.init_Object(
            writer_Comment_setresolved.comment_id, writer_Comment_setresolved.comment_id, "MainWindow"
            )

        self.write_line_with_one_parameters(
            writer_Comment_setresolved.comment_id, "RESOLVE", "", ""
        )

        self.prev_command = writer_Comment_setresolved

    def handle_setZoom_command(self, setZoom_command):

        self.init_app()

        self.write_line_with_one_parameters(
            self.current_app, "SET", "ZOOM", setZoom_command.zoom_value
        )

        self.prev_command = setZoom_command

    def Generate_UI_test(self):
        line = double_tab + "self.ui_test.close_doc()"
        self.variables.append(line)

        line = "\n\n# vim: set shiftwidth=4 softtabstop=4 expandtab:"
        self.variables.append(line)

        for line in self.variables:
            self.output_stream.write(str(line))

    def do_nothing(self, Command):
        line = "to be added in the future"

    def __del__(self):
        self.output_stream.close()


def main():
    args = parse_args()
    ui_logger = ul_Compiler(args.input_address, args.output_address)
    ui_logger.compile()
    for statement in ui_logger.log_lines.commands:
        print(statement)
    ui_logger.Generate_UI_test()
    del ui_logger


if __name__ == "__main__":
    main()
