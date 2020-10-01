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
import re

DEFAULT_WARNING_STR = 'Lint assertion failed'

POSSIBLE_TOP_LEVEL_WIDGETS = ['GtkDialog', 'GtkMessageDialog', 'GtkBox', 'GtkFrame', 'GtkGrid', 'GtkAssistant']
IGNORED_TOP_LEVEL_WIDGETS = ['GtkAdjustment', 'GtkImage', 'GtkListStore', 'GtkSizeGroup', 'GtkMenu', 'GtkTextBuffer', 'GtkTreeStore']
BORDER_WIDTH = '6'
BUTTON_BOX_SPACING = '12'
ALIGNMENT_TOP_PADDING = '6'
#https://developer.gnome.org/hig-book/3.0/windows-alert.html.en#alert-spacing
MESSAGE_BOX_SPACING = '24'
MESSAGE_BORDER_WIDTH = '12'

IGNORED_WORDS = ['the', 'of', 'to', 'for', 'a', 'and', 'as', 'from', 'on', 'into', 'by', 'at', 'or', 'do', 'in', 'when']

def lint_assert(predicate, warning=DEFAULT_WARNING_STR):
    if not predicate:
        print("    * " + warning)

def check_top_level_widget(element):
    # check widget type
    widget_type = element.attrib['class']
    lint_assert(widget_type in POSSIBLE_TOP_LEVEL_WIDGETS,
                "Top level widget should be 'GtkDialog', 'GtkFrame', 'GtkBox', or 'GtkGrid', but is " + widget_type)

    # check border_width property
    border_width_properties = element.findall("property[@name='border_width']")
    # This one fires so often I don't think it's useful
    #if len(border_width_properties) < 1:
    #    lint_assert(False, "No border_width set on top level widget. Should probably be " + BORDER_WIDTH)
    if len(border_width_properties) == 1:
        border_width = border_width_properties[0]
        if widget_type == "GtkMessageDialog":
            lint_assert(border_width.text == MESSAGE_BORDER_WIDTH,
                        "Top level 'border_width' property should be " + MESSAGE_BORDER_WIDTH)
        else:
            lint_assert(border_width.text == BORDER_WIDTH,
                        "Top level 'border_width' property should be " + BORDER_WIDTH)

def check_button_box_spacing(element):
    spacing = element.findall("property[@name='spacing']")
    lint_assert(len(spacing) > 0 and spacing[0].text == BUTTON_BOX_SPACING,
                "Button box 'spacing' should be " + BUTTON_BOX_SPACING)

def check_message_box_spacing(element):
    spacing = element.findall("property[@name='spacing']")
    lint_assert(len(spacing) > 0 and spacing[0].text == MESSAGE_BOX_SPACING,
                "Button box 'spacing' should be " + MESSAGE_BOX_SPACING)

def check_radio_buttons(root):
    radios = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkRadioButton']
    for radio in radios:
        radio_underlines = radio.findall("./property[@name='use_underline']")
        assert len(radio_underlines) <= 1
        if len(radio_underlines) < 1:
            lint_assert(False, "No use_underline in GtkRadioButton with id = '" + radio.attrib['id'] + "'")

def check_check_buttons(root):
    radios = [element for element in root.findall('.//object') if element.attrib['class'] == 'GtkCheckButton']
    for radio in radios:
        radio_underlines = radio.findall("./property[@name='use_underline']")
        assert len(radio_underlines) <= 1
        if len(radio_underlines) < 1:
            lint_assert(False, "No use_underline in GtkCheckButton with id = '" + radio.attrib['id'] + "'")


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

def check_title_labels(root):
    labels = root.findall(".//child[@type='label']")
    titles = [label.find(".//property[@name='label']") for label in labels]
    for title in titles:
        if title is None:
            continue
        words = re.split(r'[^a-zA-Z0-9:_-]', title.text)
        first = True
        for word in words:
            if len(word) and word[0].islower() and (word not in IGNORED_WORDS or first):
                lint_assert(False, "The word '" + word + "' should be capitalized")
            first = False

def main():
    print(" == " + sys.argv[1] + " ==")
    tree = ET.parse(sys.argv[1])
    root = tree.getroot()

    lint_assert('domain' in root.attrib, "interface needs to specific translation domain")

    top_level_widgets = [element for element in root.findall('object') if element.attrib['class'] not in IGNORED_TOP_LEVEL_WIDGETS]
    lint_assert( len(top_level_widgets) <= 1, "should be only one top-level widget for us to analyze, found " + str(len(top_level_widgets)))
    if len(top_level_widgets) > 1:
        return
    # eg. one file contains only a Menu, which we don't check
    if len(top_level_widgets) == 0:
        return

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

    check_radio_buttons(root)

    check_check_buttons(root)

    check_title_labels(root)

if __name__ == "__main__":
    main()
