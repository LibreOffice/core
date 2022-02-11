#!/usr/bin/python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
# sadly need lxml because the python one doesn't preserve namespace prefixes
# and type-detection looks for the string "office:document"
from lxml import etree as ET
#import xml.etree.ElementTree as ET

def get_used_p_styles(root):
    elementnames = [
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}p",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}h",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}alphabetical-index-entry-template",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}bibliography-entry-template",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}illustration-index-entry-template",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-source-style",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}object-index-entry-template",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}table-index-entry-template",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}table-of-content-entry-template",
        ".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}user-index-entry-template",
    ]

    # document content
    ps = sum([root.findall(e) for e in elementnames], [])
    usedpstyles = set()
    usedcondstyles = set()
    for p in ps:
        usedpstyles.add(p.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name"))
        if p.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}cond-style-name"):
            usedcondstyles.add(p.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}cond-style-name"))
        if p.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}class-names"):
            for style in p.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}class-names").split(" "):
                usedpstyles.add(style)
    for shape in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:draw:1.0}text-style-name]"):
        usedpstyles.add(shape.get("{urn:oasis:names:tc:opendocument:xmlns:draw:1.0}text-style-name"))
    for tabletemplate in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:table:1.0}paragraph-style-name]"):
        usedpstyles.add(tabletemplate.get("{urn:oasis:names:tc:opendocument:xmlns:table:1.0}paragraph-style-name"))
    for page in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}register-truth-ref-style-name]"):
        usedpstyles.add(page.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}register-truth-ref-style-name"))
    for form in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:form:1.0}text-style-name]"):
        usedpstyles.add(form.get("{urn:oasis:names:tc:opendocument:xmlns:form:1.0}text-style-name"))
    # conditional styles
    for condstyle in usedcondstyles:
        for map_ in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}family='paragraph'][@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name='" + condstyle + "']/{urn:oasis:names:tc:opendocument:xmlns:style:1.0}map"):
            usedpstyles.add(map_.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}apply-style-name"))
    # other styles
    for notesconfig in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}default-style-name]"):
        usedpstyles.add(notesconfig.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}default-style-name"))
    return usedpstyles

def add_parent_styles(usedstyles, styles):
    size = -1
    while size != len(usedstyles):
        size = len(usedstyles)
        for style in styles:
            if style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedstyles:
                if style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}parent-style-name"):
                    usedstyles.add(style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}parent-style-name"))
                # only for paragraph styles and master-pages
                if style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}next-style-name"):
                    usedstyles.add(style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}next-style-name"))

def remove_unused_styles(root, usedstyles, styles, name):
    for style in styles:
        print(style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
        if not(style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedstyles):
            print("removing unused " + name + " " + style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
            # it is really dumb that there is no parent pointer in dom
            try:
                root.find(".//{urn:oasis:names:tc:opendocument:xmlns:office:1.0}automatic-styles").remove(style)
            except ValueError:
                root.find(".//{urn:oasis:names:tc:opendocument:xmlns:office:1.0}styles").remove(style)

def collect_all_attribute(usedstyles, attribute):
    for element in root.findall(".//*[@" + attribute + "]"):
        usedstyles.add(element.get(attribute))

def remove_unused(root):
    # 1) find all elements that may reference page styles - this gets rid of some paragaraphs
    usedpstyles = get_used_p_styles(root)
    print(usedpstyles)
    usedtstyles = set()
    tables = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:table:1.0}table")
    print(tables)
    for table in tables:
        usedtstyles.add(table.get("{urn:oasis:names:tc:opendocument:xmlns:table:1.0}style-name"))
    pstyles = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}family='paragraph']")
    tstyles = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}family='table']")
    usedmasterpages = {"Standard"} # assume this is the default on page 1
    # only automatic styles may have page breaks in LO, so no need to chase parents or nexts
    for pstyle in pstyles:
        print(pstyle.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
        if pstyle.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedpstyles:
            usedmasterpages.add(pstyle.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}master-page-name"))
    for tstyle in tstyles:
        if tstyle.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedtstyles:
            usedmasterpages.add(tstyle.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}master-page-name"))
    for node in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}master-page-name]"):
        usedmasterpages.add(node.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}master-page-name"))
    for node in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:draw:1.0}master-page-name]"):
        usedmasterpages.add(node.get("{urn:oasis:names:tc:opendocument:xmlns:draw:1.0}master-page-name"))
    print(usedmasterpages)
    # iterate parent/next until no more masterpage is added
    size = -1
    while size != len(usedmasterpages):
        size = len(usedmasterpages)
        for mp in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}master-page"):
            if mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedmasterpages:
                if mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}parent-style-name"):
                    usedmasterpages.add(mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}parent-style-name"))
                if mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}next-style-name"):
                    usedmasterpages.add(mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}next-style-name"))
    # remove unused masterpages
    for mp in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}master-page"):
        if not(mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedmasterpages):
            print("removing unused master page " + mp.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
            # there is no way to get the parent element???
            root.find(".//{urn:oasis:names:tc:opendocument:xmlns:office:1.0}master-styles").remove(mp)

    # 2) remove unused paragraph styles
    usedpstyles = get_used_p_styles(root)

    add_parent_styles(usedpstyles, pstyles)
    remove_unused_styles(root, usedpstyles, pstyles, "paragraph style")

    # 3) unused list styles - keep referenced from still used paragraph styles
    usedliststyles = set()
    for style in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}list-style-name]"):
        usedliststyles.add(style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}list-style-name)"))
    for list_ in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}list[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name]"):
        usedliststyles.add(list_.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name"))
    for listitem in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}list-item[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-override]"):
        usedliststyles.add(listitem.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-override"))
    for numpara in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}numbered-paragraph[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name]"):
        usedliststyles.add(list_.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name"))
    # ignore ones that are children of style:graphic-properties, those must be handled as the containing style
    # there is no inheritance for these
    liststyles = root.findall("./*/{urn:oasis:names:tc:opendocument:xmlns:text:1.0}list-style")
    remove_unused_styles(root, usedliststyles, liststyles, "list style")

    # 4) unused text styles
    usedtextstyles = set()
    usedsectionstyles = set()
    usedrubystyles = set()

    sections = {
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}alphabetical-index",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}bibliography",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}illustration-index",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-title",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}object-index",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}section",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}table-of-content",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}table-index",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}user-index",
    }
    texts = {
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}a",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-bibliography",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-chapter",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-link-end",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-link-start",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-page-number",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-span",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-tab-stop",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-entry-text",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}index-title-template",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}linenumbering-configuration",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}list-level-style-number",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}list-level-style-bullet",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}outline-level-style",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}ruby-text",
        "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}span",
    }
    for element in root.findall(".//*[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name]"):
        style = element.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}style-name")
        if element.tag == "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}ruby":
            usedrubystyles.add(style)
        elif element.tag in sections:
            usedsectionstyles.add(style)
        elif element.tag in texts:
            usedtextstyles.add(style)

    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style-name")
    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}leader-text-style")
    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}text-line-through-text-style")
    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}visited-style-name")
    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}main-entry-style-name")
    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}citation-style-name")
    collect_all_attribute(usedtextstyles, "{urn:oasis:names:tc:opendocument:xmlns:text:1.0}citation-body-style-name")
    for span in root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:text:1.0}span[@{urn:oasis:names:tc:opendocument:xmlns:text:1.0}class-names]"):
        for style in span.get("{urn:oasis:names:tc:opendocument:xmlns:text:1.0}class-names").split(" "):
            usedtextstyles.add(style)
    textstyles = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}family='text']")
    add_parent_styles(usedtextstyles, textstyles)
    remove_unused_styles(root, usedtextstyles, textstyles, "text style")

    # 5) unused ruby styles - can't have parents?
    rubystyles = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}family='ruby']")
    remove_unused_styles(root, usedrubystyles, rubystyles, "ruby style")

    # 6) unused section styles - can't have parents?
    sectionstyles = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style[@{urn:oasis:names:tc:opendocument:xmlns:style:1.0}family='section']")
    remove_unused_styles(root, usedsectionstyles, sectionstyles, "section style")

    # TODO 6 other styles

    # 13) unused font-face-decls
    usedfonts = set()
    collect_all_attribute(usedfonts, "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}font-name")
    collect_all_attribute(usedfonts, "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}font-name-asian")
    collect_all_attribute(usedfonts, "{urn:oasis:names:tc:opendocument:xmlns:style:1.0}font-name-complex")
    fonts = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}font-face")
    for font in fonts:
        if not(font.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name") in usedfonts):
            print("removing unused font-face " + font.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
            root.find(".//{urn:oasis:names:tc:opendocument:xmlns:office:1.0}font-face-decls").remove(font)

    # 14) remove rsid attributes
    styles = root.findall(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}style")
    for style in styles:
        tp = style.find(".//{urn:oasis:names:tc:opendocument:xmlns:style:1.0}text-properties")
        if tp is not None:
            if "{http://openoffice.org/2009/office}rsid" in tp.attrib:
                print("removing rsid from " + style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
                del tp.attrib["{http://openoffice.org/2009/office}rsid"]
            if "{http://openoffice.org/2009/office}paragraph-rsid" in tp.attrib:
                print("removing paragraph-rsid from " + style.get("{urn:oasis:names:tc:opendocument:xmlns:style:1.0}name"))
                del tp.attrib["{http://openoffice.org/2009/office}paragraph-rsid"]

    # remove office:settings
    root.remove(root.find(".//{urn:oasis:names:tc:opendocument:xmlns:office:1.0}settings"))

    # scripts are almost never needed
    root.remove(root.find(".//{urn:oasis:names:tc:opendocument:xmlns:office:1.0}scripts"))

    # TODO: replace embedded image with some tiny one
    # TODO: perhaps replace text with xxx (optionally)?

if __name__ == "__main__":
    infile = sys.argv[1]
    outfile = sys.argv[2]

    dom = ET.parse(infile)
    root = dom.getroot()

    remove_unused(root)

    # write output
    dom.write(outfile, encoding='utf-8', xml_declaration=True)

    """
    TODO
    chart:style-name
    -> chart
    db:style-name
    -> table-column, table
    db:default-row-style-name
    -> table-row
    db:default-cell-style-name
    -> cell
    draw:style-name
    -> graphic
    -> drawing-page (only draw:page, presentation:notes, style:handout-master, syle:master-page)
    presentation:style-name
    -> presentation
    style:data-style-name
    -> data style
    presentation:presentation-page-layout-name
    -> presentation-page-layout
    style:page-layout-name
    -> "page layout style" ?
    style:percentage-data-style-name
    -> data style
    table:default-cell-style-name
    -> cell

    draw:class-names
    -> graphic
    presentation:class-names
    -> presentation
    draw:stroke-dash-names
    -> draw:stroke-dash

    draw:fill-gradient-name
    -> gradient
    draw:fill-hatch-name
    -> hatch
    draw:fill-image-name
    -> bitmap
    draw:opacity-name
    -> gradient
    draw:stroke-dash
    -> draw:stroke-dash
    draw:marker-start
    draw:marker-end
    """

# vim: set shiftwidth=4 softtabstop=4 expandtab:
