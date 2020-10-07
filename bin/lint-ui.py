#!/usr/bin/env python3
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#
# Takes a LibreOffice .ui file and provides linting tips for maintaining
# a consistent look for dialogs

import sys
# Force python XML parser not faster C accelerators
# because we can't hook the C implementation
sys.modules['_elementtree'] = None
import xml.etree.ElementTree as ET
import re

DEFAULT_WARNING_STR = 'Lint assertion failed'

POSSIBLE_TOP_LEVEL_WIDGETS = ['GtkDialog', 'GtkMessageDialog', 'GtkBox', 'GtkFrame', 'GtkGrid',
    'GtkAssistant', 'GtkToolbar', 'GtkNotebook', 'GtkPopover', 'GtkWindow', 'GtkPaned', 'GtkScrolledWindow']
IGNORED_TOP_LEVEL_WIDGETS = ['GtkAdjustment', 'GtkImage', 'GtkListStore', 'GtkSizeGroup', 'GtkMenu', 'GtkTextBuffer', 'GtkTreeStore']
BORDER_WIDTH = '6'
BUTTON_BOX_SPACING = '12'
ALIGNMENT_TOP_PADDING = '6'
#https://developer.gnome.org/hig-book/3.0/windows-alert.html.en#alert-spacing
MESSAGE_BOX_SPACING = '24'
MESSAGE_BORDER_WIDTH = '12'

IGNORED_WORDS = ['the', 'of', 'to', 'for', 'a', 'and', 'as', 'from', 'on', 'into', 'by', 'at', 'or', 'do', 'in', 'when', 'no']

# Hook the XML parser and add line number attributes
class LineNumberingParser(ET.XMLParser):
    def _start(self, *args, **kwargs):
        # Here we assume the default XML parser which is expat
        # and copy its element position attributes into output Elements
        element = super(self.__class__, self)._start(*args, **kwargs)
        element._start_line_number = self.parser.CurrentLineNumber
        element._start_column_number = self.parser.CurrentColumnNumber
        element._start_byte_index = self.parser.CurrentByteIndex
        return element

    def _end(self, *args, **kwargs):
        element = super(self.__class__, self)._end(*args, **kwargs)
        element._end_line_number = self.parser.CurrentLineNumber
        element._end_column_number = self.parser.CurrentColumnNumber
        element._end_byte_index = self.parser.CurrentByteIndex
        return element


def lint_assert(predicate, warning=DEFAULT_WARNING_STR, node=None):
    if not predicate:
        if not(node is None):
            print(sys.argv[1] + ":" + str(node._start_line_number) + ": " + warning)
        else:
            print(sys.argv[1] + ": " + warning)

def check_top_level_widget(element):
    # check widget type
    widget_type = element.attrib['class']
    if not(widget_type in POSSIBLE_TOP_LEVEL_WIDGETS):
        return

    # check border_width property
    border_width_properties = element.findall("property[@name='border_width']")
    # TODO reenable when we are ready to fix
    #if len(border_width_properties) < 1:
    #    lint_assert(False, "No border_width set on top level widget. Should probably be " + BORDER_WIDTH)
    #if len(border_width_properties) == 1:
    #    border_width = border_width_properties[0]
    #    if widget_type == "GtkMessageDialog":
    #        lint_assert(border_width.text == MESSAGE_BORDER_WIDTH,
    #                    "Top level 'border_width' property should be " + MESSAGE_BORDER_WIDTH, border_width)
    #    else:
    #        lint_assert(border_width.text == BORDER_WIDTH,
    #                    "Top level 'border_width' property should be " + BORDER_WIDTH, border_width)

    # check that any widget which has 'has-default' also has 'can-default'
    for widget in element.findall('.//object'):
        if not widget.attrib['class']:
            continue
        widget_type = widget.attrib['class']
        has_defaults = widget.findall("./property[@name='has_default']")
        if len(has_defaults) > 0 and has_defaults[0].text == "True":
            can_defaults = widget.findall("./property[@name='can_default']")
            lint_assert(len(can_defaults)>0 and can_defaults[0].text == "True",
                "has_default without can_default in " + widget_type + " with id = '" + widget.attrib['id'] + "'", widget)

def check_button_box_spacing(element):
    spacing = element.findall("property[@name='spacing']")
    lint_assert(len(spacing) > 0 and spacing[0].text == BUTTON_BOX_SPACING,
                "Button box 'spacing' should be " + BUTTON_BOX_SPACING,
                element)

def check_message_box_spacing(element):
    spacing = element.findall("property[@name='spacing']")
    lint_assert(len(spacing) > 0 and spacing[0].text == MESSAGE_BOX_SPACING,
                "Message box 'spacing' should be " + MESSAGE_BOX_SPACING,
                element)

def check_radio_buttons(root):
    radios = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkRadioButton']
    for radio in radios:
        radio_underlines = radio.findall("./property[@name='use_underline']")
        assert len(radio_underlines) <= 1
        if len(radio_underlines) < 1:
            lint_assert(False, "No use_underline in GtkRadioButton with id = '" + radio.attrib['id'] + "'", radio)

def check_adjustments(root):
    adjustments = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkAdjustment']
    for adjustment in adjustments:
        uppers = adjustment.findall("./property[@name='upper']")
        assert len(uppers) <= 1
        if len(uppers) < 1:
            lint_assert(False, "No upper in GtkAdjustment with id = '" + adjustment.attrib['id'] + "'", adjustment)

def check_menu_buttons(root):
    buttons = [element for element in root.findall('.//object') if element.attrib['class'] == "GtkMenuButton"]
    for button in buttons:
        labels = button.findall("./property[@name='label']")
        images = button.findall("./property[@name='image']")
        assert(len(labels) <= 1)
        if len(labels) < 1 and len(images) < 1:
            if sys.argv[1] == "vcl/uiconfig/ui/combobox.ui": and button.attrib['id'] == "overlaybutton":
                pass
            else:
                lint_assert(False, "No label in GtkMenuButton with id = '" + button.attrib['id'] + "'", button)

def check_check_buttons(root):
    radios = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkCheckButton']
    for radio in radios:
        radio_underlines = radio.findall("./property[@name='use_underline']")
        assert len(radio_underlines) <= 1
        if len(radio_underlines) < 1:
            lint_assert(False, "No use_underline in GtkCheckButton with id = '" + radio.attrib['id'] + "'", radio)

def check_frames(root):
    frames = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkFrame']
    for frame in frames:
        frame_alignments = frame.findall("./child/object[@class='GtkAlignment']")
        assert len(frame_alignments) <= 1
        if len(frame_alignments) < 1:
            lint_assert(False, "No GtkAlignment in GtkFrame with id = '" + frame.attrib['id'] + "'", frame)
        if len(frame_alignments) == 1:
            alignment = frame_alignments[0]
            check_alignment_top_padding(alignment)

def check_alignment_top_padding(alignment):
    top_padding_properties = alignment.findall("./property[@name='top_padding']")
    assert len(top_padding_properties) <= 1
    # TODO reenable when we are ready to fix
    # if len(top_padding_properties) < 1:
    #     lint_assert(False, "No GtkAlignment 'top_padding' set. Should probably be " + ALIGNMENT_TOP_PADDING, alignment)
    #if len(top_padding_properties) == 1:
    #    top_padding = top_padding_properties[0]
    #    lint_assert(top_padding.text == ALIGNMENT_TOP_PADDING,
    #                "GtkAlignment 'top_padding' should be " + ALIGNMENT_TOP_PADDING, alignment)

def check_title_labels(root):
    labels = root.findall(".//child[@type='label']")
    for label in labels:
        title = label.find(".//property[@name='label']")
        if title is None:
            continue
        words = re.split(r'[^a-zA-Z0-9:_-]', title.text)
        first = True
        for word in words:
            if len(word) and word[0].islower() and (word not in IGNORED_WORDS) and not first:
                context = title.attrib['context']
                # exclude a couple of whole sentences
                if sys.argv[1] == "cui/uiconfig/ui/optpathspage.ui" and context == "optpathspage|label1":
                    pass
                elif sys.argv[1] == "dbaccess/uiconfig/ui/password.ui" and context == "password|label1":
                    pass
                elif sys.argv[1] == "sc/uiconfig/scalc/ui/datastreams.ui" and context == "datastreams|label4":
                    pass
                elif sys.argv[1] == "sc/uiconfig/scalc/ui/scgeneralpage.ui" and context == "scgeneralpage|label6":
                    pass
                elif sys.argv[1] == "sfx2/uiconfig/ui/documentfontspage.ui" and context == "documentfontspage|fontScriptFrameLabel":
                    pass
                elif sys.argv[1] == "sw/uiconfig/swriter/ui/testmailsettings.ui" and context == "testmailsettings|label8":
                    pass
                elif sys.argv[1] == "sw/uiconfig/swriter/ui/optcomparison.ui" and context == "optcomparison|setting":
                    pass
                elif sys.argv[1] == "sw/uiconfig/swriter/ui/optcompatpage.ui" and context == "optcompatpage|label11":
                    pass
                elif sys.argv[1] == "sw/uiconfig/swriter/ui/optcaptionpage.ui" and context == "optcaptionpage|label1":
                    pass
                elif sys.argv[1] == "sw/uiconfig/swriter/ui/mmresultemaildialog.ui" and context == "mmresultemaildialog|attachft":
                    pass
                elif sys.argv[1] == "sw/uiconfig/swriter/ui/mailmerge.ui" and context == "mailmerge|singledocument":
                    pass
                elif sys.argv[1] == "cui/uiconfig/ui/acorexceptpage.ui" and context == "acorexceptpage|label2":
                    pass
                elif sys.argv[1] == "dbaccess/uiconfig/ui/dbwizmysqlintropage.ui" and context == "dbwizmysqlintropage|label1":
                    pass
                else:
                    lint_assert(False, "The word '" + word + "' should be capitalized", label)
            first = False

def main():
    tree = ET.parse(sys.argv[1], parser=LineNumberingParser())
    root = tree.getroot()

    if sys.argv[1] != "libreofficekit/qa/gtktiledviewer/gtv.ui":
        lint_assert('domain' in root.attrib, "interface needs to specify translation domain")

    top_level_widgets = [element for element in root.findall('object') if element.attrib['class'] not in IGNORED_TOP_LEVEL_WIDGETS]
    # eg. one file contains only a Menu, which we don't check
    if len(top_level_widgets) == 0:
        return

    for top_level_widget in top_level_widgets:
        check_top_level_widget(top_level_widget)

    # TODO - only do this if we have a GtkDialog?
    # check button box spacing
    button_box = top_level_widget.findall("./child/object[@id='dialog-vbox1']")
    if len(button_box) > 0:
        element = button_box[0]
        # TODO reenable when we are ready to fix
        #check_button_box_spacing(element)

    message_box = top_level_widget.findall("./child/object[@id='messagedialog-vbox']")
    if len(message_box) > 0:
        element = message_box[0]
        # TODO reenable when we are ready to fix
        #check_message_box_spacing(element)

    check_frames(root)

    # TODO reenable when we are ready to fix
    #check_radio_buttons(root)

    check_menu_buttons(root)

    # TODO reenable when we are ready to fix
    #check_check_buttons(root)

    check_title_labels(root)

if __name__ == "__main__":
    main()
