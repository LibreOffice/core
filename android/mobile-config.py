#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# this tool rips out configuration pieces that are not useful for
# a mobile viewer / editing application without a full UI.
#
# ideally the postprocess/ makefile would cope with this but its
# already over-complicated by rampant conditionals.

import sys
import xml.etree.ElementTree as ET

main_xcd_discard = [
    'org.openoffice.Office/TableWizard', # huge

    'org.openoffice.Office.DataAccess/Drivers', # no database
    'org.openoffice.Office/Addons', # no addons

    # no conventional UI; reverse sorted by size
    'org.openoffice.Office.UI/GenericCommands',
    'org.openoffice.Office.UI/DrawImpressCommands',
    'org.openoffice.Office.UI/Sidebar',
    'org.openoffice.Office.UI/ChartCommands',
    'org.openoffice.Office.UI/DbuCommands',
    'org.openoffice.Office.UI/Controller',
    'org.openoffice.Office.UI/StartModuleCommands',
    'org.openoffice.Office.UI/BasicIDEWindowState',
    'org.openoffice.Office.UI/GenericCategories',
    'org.openoffice.Office.UI/ChartWindowState',
    'org.openoffice.Office.UI/BaseWindowState',
    'org.openoffice.Office.UI/BasicIDECommands',
    'org.openoffice.Office.UI/BibliographyCommands',
    'org.openoffice.Office.UI/DbQueryWindowState',
    'org.openoffice.Office.UI/DbRelationWindowState',
    'org.openoffice.Office.UI/DbTableWindowState',
    'org.openoffice.Office.UI/DbTableDataWindowState',
    'org.openoffice.Office.UI/DbBrowserWindowState',
    'org.openoffice.Office.UI/WindowContentFactories',
    'org.openoffice.Office.UI/StartModuleWindowState',
    'org.openoffice.Office.UI/GlobalSettings',
    'org.openoffice.Office.UI/BibliographyWindowState',
    'org.openoffice.Office.UI/Category',
    ]

if __name__ == '__main__':
    tree = ET.parse(sys.argv[1])
    root = tree.getroot()

    total = 0
    for child in root:
        total += len(ET.tostring(child))

    saved = 0
    to_remove = []

    for child in root:
        section = child.attrib['{http://openoffice.org/2001/registry}name']
        package = child.attrib['{http://openoffice.org/2001/registry}package']
        size = len(ET.tostring(child));
        key = '%s/%s' % (package, section)
        if key in main_xcd_discard:
            print('removed %s - saving %d' % (key, size))
            saved = saved + size
            to_remove.append(child)

    for child in to_remove:
        root.remove(child)

    print("saved %d of %d bytes: %2.f%%" % (saved, total, saved*100.0/total))

    # Don't do pointless Word -> Writer and similar conversions when we have no UI.
    nsDict = {
        "component-schema": "{http://openoffice.org/2001/registry}component-schema",
        "component-data": "{http://openoffice.org/2001/registry}component-data",
        "name": "{http://openoffice.org/2001/registry}name",
    }
    microsoftImport = '%(component-schema)s[@%(name)s="Common"]/component/group[@%(name)s="Filter"]/group[@%(name)s="Microsoft"]/group[@%(name)s="Import"]/prop' % nsDict
    props = root.findall(microsoftImport)
    for prop in props:
        prop.findall("value")[0].text = "false"

    # Disable View -> Text Boundaries
    for prop in root.findall('%(component-schema)s[@%(name)s="UI"]/templates/group[@%(name)s="ColorScheme"]/group[@%(name)s="DocBoundaries"]/prop' % nsDict):
        for value in prop.findall("value"):
            value.text = "false"

    # Disable Table -> Table Boundaries
    for prop in root.findall('%(component-schema)s[@%(name)s="UI"]/templates/group[@%(name)s="ColorScheme"]/group[@%(name)s="TableBoundaries"]/prop' % nsDict):
        for value in prop.findall("value"):
            value.text = "false"

    # Disable follow link with Ctrl+Click, use Click only for mobile app.
    for prop in root.findall('%(component-schema)s[@%(name)s="Common"]/component/group[@%(name)s="Security"]/group[@%(name)s="Scripting"]/prop[@%(name)s="HyperlinksWithCtrlClick"]' % nsDict):
        for value in prop.findall("value"):
            value.text = "false"

    # Disable Impress View -> Slide Pane
    for prop in root.findall('%(component-data)s[@%(name)s="Impress"]/node[@%(name)s="MultiPaneGUI"]/node[@%(name)s="SlideSorterBar"]/node[@%(name)s="Visible"]/prop[@%(name)s="ImpressView"]' % nsDict):
        for value in prop.findall("value"):
            value.text = "false"

    # The namespace prefixes xs and oor are present in attribute *values*, and namespace
    # declarations for them are needed, even if no actual elements or attributes with these
    # namespace prefixes are present. Fun.
    root.set('xmlns:xs', 'http://www.w3.org/2001/XMLSchema')
    root.set('xmlns:oor', 'http://openoffice.org/2001/registry')

    tree.write(sys.argv[2], 'UTF-8', True)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
