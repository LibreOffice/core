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
from textx.metamodel import metamodel_from_file

def parse_args():
    """
    This function parses the command-line arguments
    to get the input and output file details
    """
    parser = argparse.ArgumentParser(description = "Generate a UI test file from log")
    parser.add_argument("input_address", type = str, help = "The log file address")
    parser.add_argument("output_address", type = str, help = "The test file address")
    args = parser.parse_args()
    return args

class ul_Compiler:
    prev_command=""
    variables=[]
    objects = dict()
    current_app=""
    def __init__(self , input_address , output_address):
        self.ui_dsl_mm = metamodel_from_file('ui_logger_dsl_grammar.tx')
        self.output_stream=self.initiate_test_generation(output_address)
        self.input_address=input_address

    def get_log_file(self , input_address):
        try:
            # load the program
            content = self.ui_dsl_mm.model_from_file(input_address)
        except IOError as err:
            print("IO error: {0}".format(err))
            print("Use " + os.path.basename(sys.argv[0]) + " -h to get usage instructions")
            sys.exit(1)

        return content

    def initiate_test_generation(self,output_address):
        try:
            f = open(output_address,"w")
        except IOError as err:
            print("IO error: {0}".format(err))
            print("Use " + os.path.basename(sys.argv[0]) + " -h to get usage instructions")
            sys.exit(1)
        line="# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-\n\n" + \
        "from uitest.framework import UITestCase\n" + \
        "from libreoffice.uno.propertyvalue import mkPropertyValues\n" + \
        "import importlib\n\n" + \
        "class TestClass(UITestCase):\n" + \
        "\tdef test_function(self):\n"

        self.variables.append(line)

        return f

    def compile(self):
        self.ui_dsl_mm.register_obj_processors({
            'UNOCommand': self.handle_uno,
            'StarterCommand': self.handle_start,
            'CloseDialog': self.handle_Dialog,
            'OpenModelessDialog': self.handle_Dialog,
            'OpenModalDialog':self.handle_Dialog,
            'ButtonUIObject':self.handle_button,
            'CheckBoxUIObject':self.handle_check_box,
            'TabControlUIObject':self.handle_tab,
            'ComboBoxUIObject':self.handle_Combo_box,
            'RadioButtonUIObject':self.handle_Radio_button,
            'ListBoxUIObject':self.handle_List_box,
            'SpinFieldUIObject':self.handle_spin_field,
            'EditUIObject':self.handle_Edit_uiObject,
            'writer_Type_command':self.handle_writer_type,
            'writer_Select_command':self.handle_writer_select,
            'writer_GOTO_command':self.handle_wirter_goto,
            'calc_Select_cell':self.handle_calc_select,
            'calc_switch_sheet':self.handle_calc_switch_sheet,
            'calc_Type_command':self.handle_calc_Type_command,
            'calc_AutoFill_filter':self.handle_calc_AutoFill_filter,
            'impress_Type_command':self.handle_impress_Type_command,
            'math_element_selector':self.handle_math_element_selector,
            'math_Type_command':self.handle_math_Type_command,
            'setZoom_command':self.handle_setZoom_command
            })

        self.log_lines=self.get_log_file(self.input_address)

    def init_app(self):
        if self.current_app in self.objects:
            self.objects[self.current_app]+=1
        else:
            self.objects[self.current_app]=1
            line="\t\t"+self.current_app+" = MainWindow.getChild(\""+self.current_app+"\")\n"
            self.variables.append(line)

    def init_Object(self,Id_of_Object,Obj_parent):

        if Id_of_Object in self.objects:
            self.objects[Id_of_Object]+=1
        else:
            self.objects[Id_of_Object]=1
            line="\t\t"+Id_of_Object+" = "+Obj_parent+\
                ".getChild(\""+Id_of_Object+"\")\n"
            self.variables.append(line)

    def handle_uno(self, UNOCommand):
        if(UNOCommand.prameters==None):
            line = "\t\tself.xUITest.executeCommand(\"" + \
                UNOCommand.uno_command_name +"\")\n"
        else:
            paramaters=""
            for p in UNOCommand.prameters.parameter_data :
                paramaters = paramaters + "\"" + p.key + "\" : " + str(p.value) + " ,"
            paramaters = paramaters[:-1]

            line = "\t\tself.xUITest.executeCommandWithParameters(\"" + \
                UNOCommand.uno_command_name +"\", mkPropertyValues({"+ paramaters +"}) )\n"

        self.variables.append(line)
        self.prev_command=UNOCommand

    def handle_start(self, StarterCommand):
        line="\t\tMainDoc = self.ui_test.create_doc_in_start_center(\""+\
            StarterCommand.program_name+"\")\n"
        self.variables.append(line)

        line="\t\tMainWindow = self.xUITest.getTopFocusWindow()\n"
        self.variables.append(line)
        app={"writer":"writer_edit","calc":"grid_window","impress":"impress_win"\
            ,"math":"math_edit"}
        self.current_app=app[StarterCommand.program_name]
        self.prev_command=StarterCommand

    def handle_Dialog(self, DialogCommand):

        if (DialogCommand.__class__.__name__ == "OpenModalDialog"):
            old_line = self.variables.pop()

            if (self.prev_command.__class__.__name__ == "UNOCommand"):
                key_word=self.prev_command.uno_command_name[-6:]
            else:
                key_word=old_line[-9:-3]

            if ( key_word == "Dialog"):
                old_line="\t\tself.ui_test.execute_dialog_through_command(\""+\
                    self.prev_command.uno_command_name+"\")\n"
            self.variables.append(old_line)
            line = "\t\t" + DialogCommand.dialog_name + " = self.xUITest.getTopFocusWindow()\n"

        elif (DialogCommand.__class__.__name__ == "OpenModelessDialog"):
            old_line = self.variables.pop()
            if (self.prev_command.__class__.__name__ == "UNOCommand"):
                key_word=self.prev_command.uno_command_name[-6:]
            else:
                key_word=old_line[-9:-3]

            if ( key_word == "Dialog"):
                old_line="\t\tself.ui_test.execute_modeless_dialog_through_command(\""+\
                    self.prev_command.uno_command_name+"\")\n"
            self.variables.append(old_line)
            line = "\t\t" + DialogCommand.dialog_name + "  = self.xUITest.getTopFocusWindow()\n"

        elif (DialogCommand.__class__.__name__ == "CloseDialog"):
            if (self.prev_command.__class__.__name__ == "ButtonUIObject"):
                old_line = self.variables.pop()
                line="\t\tself.ui_test.close_dialog_through_button("+\
                    self.prev_command.ui_button+")\n"

        self.variables.append(line)
        self.prev_command=DialogCommand

    def handle_button(self, ButtonUIObject):

        self.init_Object(ButtonUIObject.ui_button,ButtonUIObject.parent_id)

        line="\t\t"+ButtonUIObject.ui_button+".executeAction(\"CLICK\",tuple())\n"
        self.variables.append(line)
        self.prev_command=ButtonUIObject

    def handle_check_box(self, CheckBoxUIObject):

        self.init_Object(CheckBoxUIObject.Check_box_id,CheckBoxUIObject.parent_id)

        line="\t\t"+CheckBoxUIObject.Check_box_id+".executeAction(\"CLICK\",tuple())\n"
        self.variables.append(line)
        self.prev_command=CheckBoxUIObject

    def handle_tab(self, TabControlUIObject):

        self.init_Object(TabControlUIObject.tab_id,TabControlUIObject.parent_id)

        line="\t\t"+TabControlUIObject.tab_id+\
            ".executeAction(\"SELECT\", mkPropertyValues({\"POS\": \""+\
            str(TabControlUIObject.tab_page_number)+"\"}))\n"
        self.variables.append(line)
        self.prev_command=TabControlUIObject

    def handle_Combo_box(self, ComboBoxUIObject):

        self.init_Object(ComboBoxUIObject.Combo_box_id,ComboBoxUIObject.parent_id)

        line="\t\t"+ComboBoxUIObject.Combo_box_id+\
            ".executeAction(\"SELECT\", mkPropertyValues({\"POS\": \""+\
            str(ComboBoxUIObject.item_num)+"\"}))\n"
        self.variables.append(line)
        self.prev_command=ComboBoxUIObject

    def handle_Radio_button(self,RadioButtonUIObject):

        self.init_Object(RadioButtonUIObject.Radio_button_id,RadioButtonUIObject.parent_id)

        line="\t\t"+RadioButtonUIObject.Radio_button_id+".executeAction(\"CLICK\",tuple())\n"
        self.variables.append(line)
        self.prev_command=RadioButtonUIObject

    def handle_List_box(self, ListBoxUIObject):

        self.init_Object(ListBoxUIObject.list_id,ListBoxUIObject.parent_id)

        line="\t\t"+ListBoxUIObject.list_id+\
            ".executeAction(\"SELECT\", mkPropertyValues({\"POS\": \""+\
            str(ListBoxUIObject.POS)+"\"}))\n"
        self.variables.append(line)
        self.prev_command=ListBoxUIObject

    def handle_spin_field(self,SpinFieldUIObject):

        self.init_Object(SpinFieldUIObject.Spin_id,SpinFieldUIObject.parent_id)

        if(SpinFieldUIObject.change=="Increase"):
            line="\t\t"+SpinFieldUIObject.Spin_id+".executeAction(\"UP\",tuple())\n"
        elif(SpinFieldUIObject.change=="Decrease"):
            line="\t\t"+SpinFieldUIObject.Spin_id+".executeAction(\"DOWN\",tuple())\n"
        self.variables.append(line)
        self.prev_command=SpinFieldUIObject

    def handle_Edit_uiObject(self,EditUIObject):

        self.init_Object(EditUIObject.action.edit_button,EditUIObject.parent_id)

        if(EditUIObject.action.__class__.__name__ =="Type_action"):

            if(EditUIObject.action.what_to_type.__class__.__name__=="char"):
                line="\t\t"+EditUIObject.action.edit_button+\
                ".executeAction(\"TYPE\", mkPropertyValues({\"TEXT\": \""+\
                EditUIObject.action.what_to_type.input_char+"\"}))\n"

            elif(EditUIObject.action.what_to_type.__class__.__name__=="KeyCode"):
                line="\t\t"+EditUIObject.action.edit_button+\
                ".executeAction(\"TYPE\", mkPropertyValues({\"KEYCODE\":"+\
                EditUIObject.action.what_to_type.input_key_code+"\"}))\n"
            self.variables.append(line)

        if(EditUIObject.action.__class__.__name__ =="SELECT"):

            line="\t\t"+EditUIObject.action.edit_button+\
                ".executeAction(\"SELECT\", mkPropertyValues({\"from\": \""+\
                str(EditUIObject.action.from_pos )+"\", \"TO\": \""+\
                str(EditUIObject.action.to_pos)+"\"}))\n"
            self.variables.append(line)

        if(EditUIObject.action.__class__.__name__ =="Clear"):

            line="\t\t"+EditUIObject.action.edit_button+\
                ".executeAction(\"CLEAR\",tuple())\n"
            self.variables.append(line)
        self.prev_command=EditUIObject

    def handle_writer_type (self,writer_Type_command):

        self.init_app()

        if(writer_Type_command.what_to_type.__class__.__name__=="char"):
            line="\t\twriter_edit.executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"TEXT\": \""+\
            writer_Type_command.what_to_type.input_char+"\"}))\n"
        elif(writer_Type_command.what_to_type.__class__.__name__=="KeyCode"):
            line="\t\twriter_edit.executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"KEYCODE\": \""+\
            writer_Type_command.what_to_type.input_key_code+"\"}))\n"
        self.variables.append(line)
        self.prev_command=writer_Type_command

    def handle_writer_select (self,writer_Select_command):

        self.init_app()

        line="\t\twriter_edit.executeAction(\"SELECT\", mkPropertyValues({\"END_POS\": \""+\
            str(writer_Select_command.from_pos)+"\", \"START_POS\": \""+\
                str(writer_Select_command.to_pos)+"\"}))\n"
        self.variables.append(line)
        self.prev_command=writer_Select_command

    def handle_wirter_goto (self,writer_GOTO_command):

        self.init_app()

        line="\t\twriter_edit.executeAction(\"GOTO\", mkPropertyValues({\"PAGE\": \""+\
            str(writer_GOTO_command.page_num)+"\"}))\n"
        self.variables.append(line)
        self.prev_command=writer_GOTO_command

    def handle_calc_select (self,calc_Select_cell):

        self.init_app()

        if(calc_Select_cell.select_op.__class__.__name__=="range_of_cells"):
            line="\t\t"+self.current_app+".executeAction(\"SELECT\", mkPropertyValues({\"RANGE\": \""+\
            calc_Select_cell.select_op.input_range+"\"}))\n"

        elif(calc_Select_cell.select_op.__class__.__name__=="one_cell"):
            line="\t\t"+self.current_app+".executeAction(\"SELECT\", mkPropertyValues({\"CELL\": \""+\
            calc_Select_cell.select_op.input_cell+"\"}))\n"

        self.variables.append(line)
        self.prev_command=calc_Select_cell

    def handle_calc_switch_sheet (self,calc_switch_sheet):

        self.init_app()

        line="\t\t"+self.current_app+".executeAction(\"SELECT\", mkPropertyValues({\"TABLE\": \""+\
        str(calc_switch_sheet.sheet_num)+"\"}))\n"

        self.variables.append(line)
        self.prev_command=calc_switch_sheet

    def handle_calc_Type_command (self,calc_Type_command):

        self.init_app()

        if(calc_Type_command.what_to_type.__class__.__name__=="char"):
            line="\t\t"+self.current_app+".executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"TEXT\": \""+\
            calc_Type_command.what_to_type.input_char+"\"}))\n"
        elif(calc_Type_command.what_to_type.__class__.__name__=="KeyCode"):
            line="\t\t"+self.current_app+".executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"KEYCODE\": \""+\
            calc_Type_command.what_to_type.input_key_code+"\"}))\n"

        self.variables.append(line)
        self.prev_command=calc_Type_command

    def handle_calc_AutoFill_filter (self,calc_AutoFill_filter):

        self.init_app()

        line="\t\t"+self.current_app+".executeAction(\"LAUNCH\", mkPropertyValues"+\
            "({\"AUTOFILTER\": \"\", \"COL\": \""+\
            str(calc_AutoFill_filter.col_num)+"\""+\
            ", \"ROW\": \""+str(calc_AutoFill_filter.row_num)\
            +"\"}))\n"

        self.variables.append(line)
        self.prev_command=calc_AutoFill_filter

    def handle_impress_Type_command (self,impress_Type_command):

        self.init_app()

        if(impress_Type_command.what_to_type.__class__.__name__=="char"):
            line="\t\t"+self.current_app+".executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"TEXT\": \""+\
            impress_Type_command.what_to_type.input_char+"\"}))\n"
        elif(impress_Type_command.what_to_type.__class__.__name__=="KeyCode"):
            line="\t\t"+self.current_app+".executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"KEYCODE\": \""+\
            impress_Type_command.what_to_type.input_key_code+"\"}))\n"

        self.variables.append(line)
        self.prev_command=impress_Type_command

    def handle_math_Type_command (self,math_Type_command):

        self.init_app()
        if(math_Type_command.what_to_type.__class__.__name__=="char"):
            line="\t\t"+self.current_app+".executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"TEXT\": \""+\
            math_Type_command.what_to_type.input_char+"\"}))\n"
        elif(math_Type_command.what_to_type.__class__.__name__=="KeyCode"):
            line="\t\t"+self.current_app+".executeAction(\"TYPE\", mkPropertyValues"+\
            "({\"KEYCODE\": \""+\
            math_Type_command.what_to_type.input_key_code+"\"}))\n"

        self.variables.append(line)
        self.prev_command=math_Type_command

    def handle_math_element_selector (self,math_element_selector):

        line="\t\t"+str(math_element_selector.element_no)+" = element_selector.getChild(\""+\
            str(math_element_selector.element_no)+"\")\n"
        self.variables.append(line)
        line="\t\t"+str(math_element_selector.element_no)+".executeAction(\"SELECT\",tuple())\n"
        self.variables.append(line)
        self.prev_command=math_element_selector

    def handle_setZoom_command (self,setZoom_command):

        if self.current_app in self.objects:
            self.objects[self.current_app]+=1
        else:
            self.objects[self.current_app]=1
            line="\t\t"+self.current_app+" = MainWindow.getChild(\""+self.current_app+"\")\n"
            self.variables.append(line)

        line="\t\t"+self.current_app+".executeAction(\"SET\", mkPropertyValues({\"ZOOM\": \""+\
            str(setZoom_command.zoom_value)+"\"}))\n"
        self.variables.append(line)
        self.prev_command=setZoom_command

    def Generate_UI_test(self):
        line="\t\tself.ui_test.close_doc()"
        self.variables.append(line)

        for line in self.variables:
            self.output_stream.write(str(line))

    def __del__(self):
        self.output_stream.close()

def main():
    args = parse_args()
    ui_logger = ul_Compiler(args.input_address,args.output_address)
    ui_logger.compile()
    for statement in (ui_logger.log_lines.commands):
        print(statement)
    ui_logger.Generate_UI_test()
    del ui_logger

if __name__ == '__main__':
    main()