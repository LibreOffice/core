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
    variables=[]
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
            })

        self.log_lines=self.get_log_file(self.input_address)

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

    def handle_start(self, StarterCommand):
        line="\t\tMainDoc = self.ui_test.create_doc_in_start_center(\""+\
            StarterCommand.program_name+"\")\n"
        self.variables.append(line)

        line="\t\tMainWindow = self.xUITest.getTopFocusWindow()\n"
        self.variables.append(line)

    def handle_Dialog(self, DialogCommand):

        if (DialogCommand.__class__.__name__ == "OpenModalDialog"):
            old_line = self.variables.pop()

            key_word=old_line[-9:-3]
            if ( key_word == "Dialog"):
                old_line="\t\tself.ui_test.execute_dialog_through_command(\""+\
                    old_line[31:-3]+"\")\n"
            self.variables.append(old_line)
            line = "\t\t" + DialogCommand.dialog_name + " = self.xUITest.getTopFocusWindow()\n"

        elif (DialogCommand.__class__.__name__ == "OpenModelessDialog"):
            old_line = self.variables.pop()

            key_word=old_line[-9:-3]
            if ( key_word == "Dialog"):
                old_line="\t\tself.ui_test.execute_modeless_dialog_through_command(\""+\
                    old_line[31:-3]+"\")\n"
            self.variables.append(old_line)
            line = "\t\t" + DialogCommand.dialog_name + "  = self.xUITest.getTopFocusWindow()\n"

        elif (DialogCommand.__class__.__name__ == "CloseDialog"):
            line="\t\tto be implemented after ui Objects\n"

        self.variables.append(line)

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
