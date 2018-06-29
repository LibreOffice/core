# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os
import sys

def usage():
    message = "usage: {program} inputfile outputfile"

    print(message.format(program = os.path.basename(sys.argv[0])))

def parse_line(line):
    """
    This function parses a line from log file
    and returns the parsed values as a python dictionary
    """
    if (line == "" or line.startswith("Action on element")):
        return
    dict = {}
    if "{" in line:
        start_index_of_parameters = line.find("{")
        end_index_of_parameters = line.find("}") + 1
        parameters = line[start_index_of_parameters:end_index_of_parameters]
        dict["parameters"] = parameters
        line = line[:start_index_of_parameters-1]
    word_list = line.split()
    dict["keyword"] = word_list[0]

    for index in range(1,len(word_list)):
        key, val = word_list[index].split(":",1)
        dict[key] = val
    return dict

def parse_args(argv):
    """
    This function parses the command-line arguments
    to get the input and output file details
    """
    if len(argv) != 3:
        usage()
        sys.exit(1)
    else:
        input_address = argv[1]
        output_address = argv[2]

    return input_address, output_address

def get_log_file(input_address):
    try:
        with open(input_address) as f:
            content = f.readlines()
    except IOError as err:
        print("IO error: {0}".format(err))
        usage()
        sys.exit(1)

    content = [x.strip() for x in content]
    return content

def initiate_test_generation(address):
    try:
        f = open(address,"w")
    except IOError as err:
        print("IO error: {0}".format(err))
        usage()
        sys.exit(1)
    initial_text = \
    "from uitest.framework import UITestCase\n" + \
    "import importlib\n\n" + \
    "class TestClass(UITestCase):\n" + \
    "    def test_function(self):\n"
    f.write(initial_text)
    return f

def get_coupling_type(line1, line2):
    """
    This function checks if two consecutive lines of log file
    refer to the same event
    """
    action_dict1 = parse_line(line1)
    action_dict2 = parse_line(line2)

    if action_dict1["keyword"] == "CommandSent" and \
        action_dict2["keyword"] == "ModalDialogExecuted":
        return "COMMAND_MODAL_COUPLE"

    elif action_dict1["keyword"] == "CommandSent" and \
        action_dict2["keyword"] == "ModelessDialogConstructed":
        return "COMMAND_MODELESS_COUPLE"

    elif action_dict1["keyword"] == "ButtonUIObject" and \
        action_dict2["keyword"] == "DialogClosed":
        return "BUTTON_DIALOGCLOSE_COUPLE"

    elif "parameters" in action_dict1 and \
        "KEYCODE" in action_dict1["parameters"] and \
        action_dict2["keyword"] == "CommandSent":
        return "REDUNDANT_COUPLE"

    return "NOT_A_COUPLE"

def get_test_line_from_one_log_line(log_line):
    action_dict = parse_line(log_line)
    test_line = "        "
    if action_dict["keyword"].endswith("UIObject"):
        parent = action_dict["Parent"]
        if (parent != ""):
            test_line += \
            action_dict["Id"] + " = " + parent + ".getChild(\"" + \
            action_dict["Id"] + "\")\n        " + \
            action_dict["Id"] + ".executeAction(\"" + \
            action_dict["Action"] + "\""
            if "parameters" in action_dict:
                test_line +=  ", mkPropertyValues(" + \
                action_dict["parameters"] + "))\n"
            else:
                test_line += ",tuple())\n"
            return test_line
    elif action_dict["keyword"] == "CommandSent":
        test_line += "self.xUITest.executeCommand(\"" + \
        action_dict["Name"] + "\")\n"
        return test_line

    return ""

def get_test_line_from_two_log_lines(log_line1,log_line2):
    coupling_type = get_coupling_type(log_line1, log_line2)
    action_dict1 = parse_line(log_line1)
    action_dict2 = parse_line(log_line2)
    test_line = "        "
    if coupling_type == "COMMAND_MODAL_COUPLE":
        test_line += \
        "self.ui_test.execute_dialog_through_command(\"" + \
        action_dict1["Name"] + "\")\n        " + \
        action_dict2["Id"] + " = self.xUITest.getTopFocusWindow()\n"
    elif coupling_type == "COMMAND_MODELESS_COUPLE":
        test_line += \
        "self.ui_test.execute_modeless_dialog_through_command(\"" + \
        action_dict1["Name"] + "\")\n        " + \
        action_dict2["Id"] + " = self.xUITest.getTopFocusWindow()\n"
    elif coupling_type == "BUTTON_DIALOGCLOSE_COUPLE":
        test_line += \
        action_dict1["Id"] + " = " + action_dict1["Parent"] + ".getChild(\"" + \
        action_dict1["Id"] + "\")\n        self.ui_test.close_dialog_through_button(" + \
        action_dict1["Id"] + ")\n"
    return test_line

def main():
    input_address, output_address = parse_args(sys.argv)
    log_lines = get_log_file(input_address)
    output_stream = initiate_test_generation(output_address)
    line_number = 0
    while line_number < len(log_lines):
        if line_number == len(log_lines)-1 or \
            get_coupling_type(log_lines[line_number],log_lines[line_number + 1]) == "NOT_A_COUPLE":
            test_line = get_test_line_from_one_log_line(log_lines[line_number])
            output_stream.write(test_line)
            line_number += 1
        elif get_coupling_type(log_lines[line_number],log_lines[line_number + 1]) == "REDUNDANT_COUPLE":
            line_number += 1
        else:
            test_line = get_test_line_from_two_log_lines(log_lines[line_number],log_lines[line_number + 1])
            output_stream.write(test_line)
            line_number += 2
    output_stream.close()

if __name__ == '__main__':
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab: