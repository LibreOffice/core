#!/usr/bin/env python3

# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import xml.etree.ElementTree as ET
import os
from prompt_toolkit import prompt
import xml.dom.minidom

#-------------------------- generating the xml functions --------------------------

def generate_menu_fragment(addon_name, merge_name, merge_point, merge_command, merge_fallback, merge_context, menu_items):
    return ''.join(x for x in [
        '<node oor:name="OfficeMenuBar">',
        f'<node oor:name="{addon_name}" oor:op="replace" xmlns:oor="http://openoffice.org/2001/registry">',
        f'<node oor:name="{merge_name}" oor:op="replace">',
        f'<prop oor:name="MergePoint"><value>{merge_point}</value></prop>' if merge_point else '',
        f'<prop oor:name="MergeCommand"><value>{merge_command}</value></prop>' if merge_command else '',
        f'<prop oor:name="MergeFallback"><value>{merge_fallback}</value></prop>' if merge_fallback else '',
        f'<prop oor:name="Context" oor:type="xs:string"><value>{merge_context}</value></prop>' if merge_context else '',
        '<node oor:name="MenuItems">',
        menu_items,
        '</node>',
        '</node>',
        '</node>',
        '</node>'
    ] if x)

def generate_menu_item_fragment(item_name, title, url, image_path, target, submenus=None):
    submenu_fragment = None
    if submenus:
        submenu_items = ''.join(submenus)
        submenu_fragment = f'<node oor:name="Submenus">{submenu_items}</node>'

    return ''.join(x for x in [
        f'<node oor:name="{item_name}" oor:op="replace">',
        f'<prop oor:name="Title"><value xml:lang="en-US">{title}</value></prop>' if title else '',
        f'<prop oor:name="URL"><value>{url}</value></prop>' if url else '',
        f'<prop oor:name="ImageIdentifier"><value>{image_path}</value></prop>' if image_path else '',
        f'<prop oor:name="Target" oor:type="xs:string"><value>{target}</value></prop>' if target else '',
        submenu_fragment or '',
        '</node>'
    ] if x)

def generate_submenu_item_fragment(submenu_name, title, url, image_path, target):
    return ''.join(x for x in [
        f'<node oor:name="{submenu_name}" oor:op="replace">',
        f'<prop oor:name="Title"><value xml:lang="en-US">{title}</value></prop>' if title else '',
        f'<prop oor:name="URL"><value>{url}</value></prop>' if url else '',
        f'<prop oor:name="ImageIdentifier"><value>{image_path}</value></prop>' if image_path else '',
        f'<prop oor:name="Target" oor:type="xs:string"><value>{target}</value></prop>' if target else '',
        '</node>'
    ] if x)

def generate_toolbar_fragment(addon_name, merge_name, merge_toolbar, merge_point, merge_command, merge_fallback, merge_context, toolbar_items):
    return ''.join(x for x in [
        '<node oor:name="OfficeToolBar">',
        f'<node oor:name="{addon_name}" oor:op="replace" xmlns:oor="http://openoffice.org/2001/registry">',
        f'<node oor:name="{merge_name}" oor:op="replace">',
        f'<prop oor:name="MergeToolBar"><value>{merge_toolbar}</value></prop>' if merge_toolbar else '',
        f'<prop oor:name="MergePoint"><value>{merge_point}</value></prop>' if merge_point else '',
        f'<prop oor:name="MergeCommand"><value>{merge_command}</value></prop>' if merge_command else '',
        f'<prop oor:name="MergeFallback"><value>{merge_fallback}</value></prop>' if merge_fallback else '',
        f'<prop oor:name="Context" oor:type="xs:string"><value>{merge_context}</value></prop>' if merge_context else '',
        '<node oor:name="ToolBarItems">',
        toolbar_items,
        '</node>',
        '</node>',
        '</node>',
        '</node>'
    ] if x)

def generate_toolbar_item_fragment(item_name, title, url, image_path, target, separator_position=None):
    return ''.join(x for x in [
        f'<node oor:name="{item_name}" oor:op="replace" xmlns:oor="http://openoffice.org/2001/registry">',
        f'<prop oor:name="Title" oor:type="xs:string"><value xml:lang="en-US">{title}</value></prop>' if title else '',
        f'<prop oor:name="URL" oor:type="xs:string"><value>{url}</value></prop>' if url else '',
        f'<prop oor:name="ImageIdentifier" oor:type="xs:string"><value>{image_path}</value></prop>' if image_path else '',
        f'<prop oor:name="Target" oor:type="xs:string"><value>{target}</value></prop>' if target else '',
        f'<prop oor:name="SeparatorPosition"><value>{separator_position}</value></prop>' if separator_position else '',
        '</node>'
    ] if x)

def generate_images_fragment(image_small, image_big, image_small_hc, image_big_hc):
    return ''.join(x for x in [
        '<node oor:name="Images" oor:op="replace">',
        f'<prop oor:name="ImageSmall"><value>{image_small}</value></prop>' if image_small else '',
        f'<prop oor:name="ImageBig"><value>{image_big}</value></prop>' if image_big else '',
        f'<prop oor:name="ImageSmallHC"><value>{image_small_hc}</value></prop>' if image_small_hc else '',
        f'<prop oor:name="ImageBigHC"><value>{image_big_hc}</value></prop>' if image_big_hc else '',
        '</node>'
    ] if x)

def generate_addon_menu_fragment(addon_menu_name, title, url, image_path, target):
    return ''.join(x for x in [
        f'<node oor:name="{addon_menu_name}" oor:op="replace">',
        f'<prop oor:name="Title"><value xml:lang="en-US">{title}</value></prop>' if title else '',
        f'<prop oor:name="URL"><value>{url}</value></prop>' if url else '',
        f'<prop oor:name="ImageIdentifier"><value>{image_path}</value></prop>' if image_path else '',
        f'<prop oor:name="Target" oor:type="xs:string"><value>{target}</value></prop>' if target else '',
        '</node>'
    ] if x)

def generate_help_menu_fragment(help_menu_name, title, url, image_path, target):
    return ''.join(x for x in [
        f'<node oor:name="{help_menu_name}" oor:op="replace">',
        f'<prop oor:name="Title"><value xml:lang="en-US">{title}</value></prop>' if title else '',
        f'<prop oor:name="URL"><value>{url}</value></prop>' if url else '',
        f'<prop oor:name="ImageIdentifier"><value>{image_path}</value></prop>' if image_path else '',
        f'<prop oor:name="Target" oor:type="xs:string"><value>{target}</value></prop>' if target else '',
        '</node>'
    ] if x)

def stitching_xcu(addon_name, merge_type, merge_name, merge_point, merge_command, merge_fallback, items, merge_toolbar, merge_context):
    if merge_type == 'menu':
        menu_items = ''.join(items)
        merge_fragment = generate_menu_fragment(addon_name, merge_name, merge_point, merge_command, merge_fallback, merge_context, menu_items=menu_items)
    elif merge_type == 'toolbar':
        toolbar_items = ''.join(items)
        merge_fragment = generate_toolbar_fragment(addon_name, merge_name, merge_toolbar, merge_point, merge_command, merge_fallback, merge_context, toolbar_items=toolbar_items)
    else:
        merge_fragment = ''

    return merge_fragment

def generate_xcu(merge_fragment, output_dir, addon_menu_fragment, help_menu_fragment, images_fragment):
    # Determine the output directory
    if not output_dir:
        while True:
            choice = input("\n\nSave Addons.xcu file to desktop or specify a path to custom directory? (d/c): ").lower()
            if choice in ("d", "desktop"):
                output_dir = os.path.join(os.path.expanduser('~'), 'Desktop')
                break
            elif choice == "c":
                custom_dir = input("\nEnter custom directory path: ")
                output_dir = validate_directory(custom_dir)
                if output_dir:
                    break
            else:
                print("\nInvalid choice. Please choose 'd' or 'c'.")

    final_fragment = f"""<?xml version='1.0' encoding='UTF-8'?>
<oor:component-data oor:package="org.openoffice.Office" oor:name="Addons"
xmlns:oor="http://openoffice.org/2001/registry" xmlns:xs="http://www.w3.org/2001/XMLSchema">
<node oor:name="AddonUI">
    {addon_menu_fragment}
    {merge_fragment}
    {help_menu_fragment}
    {images_fragment}
</node>
</oor:component-data>
"""
    file_path = os.path.join(output_dir, 'Addons.xcu')
    with open(file_path, 'w', encoding='utf-8') as file:
        dom = xml.dom.minidom.parseString(final_fragment)
        pretty_xml_as_string = dom.toprettyxml()
        # Post-process to remove unnecessary newlines
        pretty_xml_as_string = os.linesep.join([s for s in pretty_xml_as_string.splitlines() if s.strip()])
        file.write(pretty_xml_as_string)

#-------------------------- prompt message functions --------------------------

def prompt_merge_context():
    print("\nSelect the application module context for the merge instruction:")
    print("1. Writer")
    print("2. Spreadsheet")
    print("3. Presentation")
    print("4. Draw")
    print("5. Formula")
    print("6. Chart")
    print("7. Bibliography")
    choice = input("Enter your choice (1-7): ").strip()
    if choice == '1':
        return "com.sun.star.text.TextDocument"
    elif choice == '2':
        return "com.sun.star.sheet.SpreadsheetDocument"
    elif choice == '3':
        return "com.sun.star.presentation.PresentationDocument"
    elif choice == '4':
        return "com.sun.star.drawing.DrawingDocument"
    elif choice == '5':
        return "com.sun.star.formula.FormulaProperties"
    elif choice == '6':
        return "com.sun.star.chart.ChartDocument"
    elif choice == '7':
        return "com.sun.star.frame.Bibliography"
    else:
        print("Invalid choice. Please select again.")
        return prompt_merge_context()

def prompt_images():
    print("\nPlease provide paths to the [ .bmp ] images (type 'skip' to skip providing an image):")
    image_small = input("Path to small image: ").strip()
    if image_small.lower() == 'skip':
        image_small = None
    else:
        image_small = validate_image_path(image_small)

    image_big = input("Path to big image: ").strip()
    if image_big.lower() == 'skip':
        image_big = None
    else:
        image_big = validate_image_path(image_big)

    image_small_hc = input("Path to small high-contrast image: ").strip()
    if image_small_hc.lower() == 'skip':
        image_small_hc = None
    else:
        image_small_hc = validate_image_path(image_small_hc)

    image_big_hc = input("Path to big high-contrast image: ").strip()
    if image_big_hc.lower() == 'skip':
        image_big_hc = None
    else:
        image_big_hc = validate_image_path(image_big_hc)

    return image_small, image_big, image_small_hc, image_big_hc

def prompt_addon_name():
    addon_name = prompt("\nEnter addon name (extID name, e.g., com.<companyname>.<company-namespace>.<extension-namespace>): ").strip()
    return addon_name

def prompt_menu_title(menu_type):
    title = prompt(f"\nEnter {menu_type} item title (use '~' before the accelerator key, e.g., '~File'): ").strip()
    return title

def prompt_button_separator():
    print("\nDo you want to add a button separator?")
    print("1. Before the first button")
    print("2. Between the buttons")
    print("3. No separator")
    choice = input("Enter your choice (1-3): ").strip()
    if choice == '1':
        return "before_first"
    elif choice == '2':
        return "between_buttons"
    elif choice == '3':
        return "none"
    else:
        print("Invalid choice. Please select again.")
        return prompt_button_separator()

def prompt_image_path(merge_type):
    while True:
        image_path = prompt(f"\nEnter path to {merge_type} item image (optional, enter 'skip' to skip): ").strip().lower()
        if image_path == 'skip':
            return None
        image_path = validate_image_path(image_path)
        if image_path:
            return image_path
        print("Invalid file type. Please try again or enter 'skip' to skip.")

#-------------------------- validating provided path --------------------------

def validate_directory(directory_path):
    if not directory_path:
        return None  # No output directory provided
    try:
        if not os.path.isdir(directory_path):
            raise ValueError("\nInvalid directory: Path is not a directory")
        # Additional validations can be added here if needed
    except ValueError as e:
        print(f"Error: {e}")
        return None
    return directory_path

def validate_image_path(image_path):
    if not image_path:
        return None  # No image path provided
    try:
        if not os.path.exists(image_path):
            raise ValueError("\nInvalid path: Path does not exist")
        _, ext = os.path.splitext(image_path)
        if ext.lower() != '.bmp':
            raise ValueError("\nInvalid file type: Only .bmp files are allowed")
        # Additional validations can be added here if needed
    except ValueError as e:
        print(f"Error: {e}")
        return None
    return image_path

#-------------------------- MAIN --------------------------

def main():
    print("\nWelcome to the LibreOffice Addons Configuration Tool!\n")
    merge_fragment = ''
    addon_name = prompt_addon_name()
    addon_menu_fragment = ''
    help_menu_fragment = ''
    images_fragment = ''

    # Prompt for addon menu item
    choice = prompt("\nDo you want to create the 'AddonMenu' item (Tools > Add-ons)? (y/n): ").strip().lower()
    if choice == 'y':
        addon_menu_name = prompt("\nEnter addon menu item name (Tools > Add-ons): ").strip()
        addon_menu_title = prompt_menu_title("addon menu")
        addon_menu_url = prompt("\nEnter addon menu item URL: ").strip()
        addon_menu_target = prompt("\nEnter addon menu item target (_self, _default, _top, _parent, or _blank): ").strip()
        addon_menu_image_path = prompt("\nEnter path to addon menu item image (optional): ").strip()
        addon_menu_image_path = validate_image_path(addon_menu_image_path)
        addon_menu_fragment = generate_addon_menu_fragment(addon_menu_name, addon_menu_title, addon_menu_url, addon_menu_image_path, addon_menu_target)

    # Prompt for help menu item
    choice = prompt("\nDo you want to create the 'OfficeHelp' item (Help menu)? (y/n): ").strip().lower()
    if choice == 'y':
        help_menu_name = prompt("\nEnter help menu item name (Help menu): ").strip()
        help_menu_title = prompt_menu_title("help menu")
        help_menu_url = prompt("\nEnter help menu item URL: ").strip()
        help_menu_target = prompt("\nEnter help menu item target (_self, _default, _top, _parent, or _blank): ").strip()
        help_menu_image_path = prompt("\nEnter path to help menu item image (optional): ").strip()
        help_menu_image_path = validate_image_path(help_menu_image_path)
        help_menu_fragment = generate_help_menu_fragment(help_menu_name, help_menu_title, help_menu_url, help_menu_image_path, help_menu_target)

    # Prompt for images
    choice = prompt("\nImages attribute defines the icons that appear next to the text in the menu and toolbar items.\nDo you want to provide images? (y/n): ").strip().lower()
    if choice == 'y':
        image_small, image_big, image_small_hc, image_big_hc = prompt_images()
        images_fragment = generate_images_fragment(image_small, image_big, image_small_hc, image_big_hc)

    while True:  # Outer loop for gathering input for each merge
        items = []  # Initialize items list for each merge type
        merge_type = prompt("\n\nEnter merge type (menu or toolbar): ").strip()

        while merge_type not in ['menu', 'toolbar']:
            merge_type = prompt("\nInvalid input. \nEnter merge type (menu or toolbar): ").strip()

        # Gather input for merge
        if merge_type == 'menu':
            merge_name = prompt("\nEnter merge name (m<No> name for menu): \nFor example:\nFor a menu merge operation, you could enter something like 'm1' or 'm_print_options': ").strip()
        else:
            merge_name = prompt("\nEnter merge name (label[] for toolbar): \nFor a toolbar merge operation, you could enter something like [PrintButton] or [FormatToolbar]: ").strip()

        merge_point = prompt("\nEnter merge point: ").strip()
        merge_command = prompt("\nEnter merge command (possible merge commands are: AddAfter, AddBefore, Replace, Remove):  ").strip()
        merge_fallback = prompt("\nEnter merge fallback (possible merge fallbacks are: AddAfter, AddBefore, Replace, Remove):  ").strip()
        merge_toolbar = None
        output_dir = None
        merge_context = prompt_merge_context()

        while True:  # Inner loop for gathering input for items
            item_name = prompt(f"\nEnter {merge_type} item name (or 'done' to finish): ").strip().lower()
            if item_name.lower() == 'done':
                break

            title = prompt_menu_title(f"{merge_type} item")
            url = prompt(f"\nEnter {merge_type} item URL (can be a macro path or dispatch command): ").strip()
            target = prompt(f"\nEnter {merge_type} item target (_self, _default, _top, _parent, or _blank): ").strip()
            image_path = prompt_image_path(merge_type)

            # Prompt for submenus
            submenus = []
            add_submenu = prompt(f"\nAdd submenu for this {merge_type} item? (y/n): ").strip().lower()
            if add_submenu == 'y':
                while True:
                    submenu_name = prompt("\nEnter submenu name (or 'done' to finish): ").strip().lower()
                    if submenu_name.lower() == 'done':
                        break
                    submenu_title = prompt_menu_title("submenu")
                    submenu_url = prompt("\nEnter submenu URL: ").strip()
                    submenu_target = prompt("\nEnter submenu target (_self, _default, _top, _parent, or _blank): ").strip()
                    submenu_image_path = prompt("\nEnter path to submenu image (optional): ").strip()
                    submenu_image_path = validate_image_path(submenu_image_path)
                    submenus.append(generate_submenu_item_fragment(submenu_name, submenu_title, submenu_url, submenu_image_path, submenu_target))

            # Generate item fragment based on merge type
            if merge_type == 'menu':
                items.append(generate_menu_item_fragment(item_name, title, url, image_path, target, submenus))
            elif merge_type == 'toolbar':
                merge_toolbar = prompt("\nEnter the Merge ToolBar name\n(specifies the appearance of the floating toolbar reached via View > Toolbars > Add-on) : ").strip()
                separator_position = prompt_button_separator()
                items.append(generate_toolbar_item_fragment(item_name, title, url, image_path, target, separator_position))

        merge_fragment += stitching_xcu(addon_name, merge_type, merge_name, merge_point, merge_command, merge_fallback, items, merge_toolbar, merge_context)

        keep_merging = prompt("\n\n\nContinue to modify Menu or Toolbar? ( y or n ): ").strip().lower()
        if keep_merging == 'n':
            # Generate .xcu file
            generate_xcu(merge_fragment, output_dir, addon_menu_fragment, help_menu_fragment, images_fragment)
            break
    print("Thank you for using the Addons.xcu file generation. Have a Nice Day!")

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
