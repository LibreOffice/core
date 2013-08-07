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
import xml.etree.ElementTree as ET

DEFAULT_WARNING_STR = 'Lint assertion failed'

POSSIBLE_TOP_LEVEL_WIDGETS = ['GtkDialog', 'GtkMessageDialog', 'GtkBox', 'GtkFrame', 'GtkGrid']
IGNORED_TOP_LEVEL_WIDGETS = ['GtkAdjustment', 'GtkImage', 'GtkListStore', 'GtkSizeGroup', 'GtkMenu', 'GtkTextBuffer']
BORDER_WIDTH = '6'
BUTTON_BOX_SPACING = '12'
ALIGNMENT_TOP_PADDING = '6'
#https://developer.gnome.org/hig-book/3.0/windows-alert.html.en#alert-spacing
MESSAGE_BOX_SPACING = '24'
MESSAGE_BORDER_WIDTH = '12'

def lint_assert(predicate, warning=DEFAULT_WARNING_STR):
    if not predicate:
        print("    * " + warning)

def check_top_level_widget(element):
    # check widget type
    widget_type = element.attrib['class']
    lint_assert(widget_type in POSSIBLE_TOP_LEVEL_WIDGETS,
                "Top level widget should be 'GtkDialog', 'GtkFrame', 'GtkBox', or 'GtkGrid'")

    # check border_width property
    border_width_properties = element.findall("property[@name='border_width']")
    if len(border_width_properties) < 1:
        lint_assert(False, "No border_width set on top level widget. Should probably be " + BORDER_WIDTH)
    if len(border_width_properties) == 1:
        border_width = border_width_properties[0]
        if widget_type == "GtkMessageDialog":
            lint_assert(border_width.text == MESSAGE_BORDER_WIDTH,
                        "Top level 'border_width' property should be " + MESSAGE_BORDER_WIDTH)
        else:
            lint_assert(border_width.text == BORDER_WIDTH,
                        "Top level 'border_width' property should be " + BORDER_WIDTH)

def check_button_box_spacing(element):
    spacing = element.findall("property[@name='spacing']")[0]
    lint_assert(spacing.text == BUTTON_BOX_SPACING,
                "Button box 'spacing' should be " + BUTTON_BOX_SPACING)

def check_message_box_spacing(element):
    spacing = element.findall("property[@name='spacing']")[0]
    lint_assert(spacing.text == MESSAGE_BOX_SPACING,
                "Button box 'spacing' should be " + MESSAGE_BOX_SPACING)

def check_frames(root):
    frames = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkFrame']
    for frame in frames:
        frame_alignments = frame.findall("./child/object[@class='GtkAlignment']")
        assert len(frame_alignments) <= 1
        if len(frame_alignments) < 1:
            lint_assert(False, "No GtkAlignment in GtkFrame with id = '" + frame.attrib['id'] + "'")
        if len(frame_alignments) == 1:
            alignment = frame_alignments[0]
            check_alignment_top_padding(alignment)

def check_alignment_top_padding(alignment):
    top_padding_properties = alignment.findall("./property[@name='top_padding']")
    assert len(top_padding_properties) <= 1
    if len(top_padding_properties) < 1:
        lint_assert(False, "No GtkAlignment 'top_padding' set. Should probably be " + ALIGNMENT_TOP_PADDING)
    if len(top_padding_properties) == 1:
        top_padding = top_padding_properties[0]
        lint_assert(top_padding.text == ALIGNMENT_TOP_PADDING,
                    "GtkAlignment 'top_padding' should be " + ALIGNMENT_TOP_PADDING)

def main():
    print(" == " + sys.argv[1] + " ==")
    tree = ET.parse(sys.argv[1])
    root = tree.getroot()

    top_level_widgets = [element for element in root.findall('object') if element.attrib['class'] not in IGNORED_TOP_LEVEL_WIDGETS]
    assert len(top_level_widgets) == 1

    top_level_widget = top_level_widgets[0]
    check_top_level_widget(top_level_widget)

    # TODO - only do this if we have a GtkDialog?
    # check button box spacing
    button_box = top_level_widget.findall("./child/object[@id='dialog-vbox1']")
    if len(button_box) > 0:
        element = button_box[0]
        check_button_box_spacing(element)

    message_box = top_level_widget.findall("./child/object[@id='messagedialog-vbox']")
    if len(message_box) > 0:
        element = message_box[0]
        check_message_box_spacing(element)

    check_frames(root)

if __name__ == "__main__":
    main()
