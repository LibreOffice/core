/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_COMPHELPER_DOCUMENTCONSTANTS_HXX
#define INCLUDED_COMPHELPER_DOCUMENTCONSTANTS_HXX

#include <rtl/ustring.hxx>
#include <o3tl/typed_flags_set.hxx>

// formats of SO6/7
#define MIMETYPE_VND_SUN_XML_WRITER_ASCII               "application/vnd.sun.xml.writer"
#define MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII           "application/vnd.sun.xml.writer.web"
#define MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII        "application/vnd.sun.xml.writer.global"
#define MIMETYPE_VND_SUN_XML_DRAW_ASCII                 "application/vnd.sun.xml.draw"
#define MIMETYPE_VND_SUN_XML_IMPRESS_ASCII              "application/vnd.sun.xml.impress"
#define MIMETYPE_VND_SUN_XML_CALC_ASCII                 "application/vnd.sun.xml.calc"
#define MIMETYPE_VND_SUN_XML_CHART_ASCII                "application/vnd.sun.xml.chart"
#define MIMETYPE_VND_SUN_XML_MATH_ASCII                 "application/vnd.sun.xml.math"
#define MIMETYPE_VND_SUN_XML_BASE_ASCII                 "application/vnd.sun.xml.base"

// template formats of SO6/7
#define MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE_ASCII      "application/vnd.sun.xml.writer.template"
#define MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE_ASCII        "application/vnd.sun.xml.draw.template"
#define MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE_ASCII     "application/vnd.sun.xml.impress.template"
#define MIMETYPE_VND_SUN_XML_CALC_TEMPLATE_ASCII        "application/vnd.sun.xml.calc.template"

// formats of SO8
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII          "application/vnd.oasis.opendocument.text"
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII      "application/vnd.oasis.opendocument.text-web"
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII   "application/vnd.oasis.opendocument.text-master"
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII       "application/vnd.oasis.opendocument.graphics"
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII  "application/vnd.oasis.opendocument.presentation"
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII   "application/vnd.oasis.opendocument.spreadsheet"
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII         "application/vnd.oasis.opendocument.chart"
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII       "application/vnd.oasis.opendocument.formula"
#define MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII      "application/vnd.oasis.opendocument.base"
#define MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII        "application/vnd.sun.xml.report"
#define MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII  "application/vnd.sun.xml.report.chart"

// template formats of SO8
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII          "application/vnd.oasis.opendocument.text-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII   "application/vnd.oasis.opendocument.text-master-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII       "application/vnd.oasis.opendocument.graphics-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII  "application/vnd.oasis.opendocument.presentation-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII   "application/vnd.oasis.opendocument.spreadsheet-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII         "application/vnd.oasis.opendocument.chart-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII       "application/vnd.oasis.opendocument.formula-template"


// ODF versions
#define ODFVER_010_TEXT "1.0"
#define ODFVER_011_TEXT "1.1"
#define ODFVER_012_TEXT "1.2"

// filter flags
// TODO/LATER: The flags should be part of the UNO specification

// Note that these flag bits have parallel names in
// filter/source/config/cache/constant.hxx. Some of the bits are
// missing from there, and some have different names. But the meaning
// is presumably the same, and the values are the same.

// http://www.mail-archive.com/dev@openoffice.org/msg05047.html says:

// I can just sum up what comes into my mind, hope I don't miss one:

// Import                  - should be self explaining
// Export                  - should be self explaining
// Template                - deprecated
// TemplatePath            - filter for a documenttemplate
// Own                     - one of the OOo file formats
// Alien                   - no zip container based format
// Preferred               - preferred filter for a particular type
// Asynchron               - deprecated, only HTML-filter isn't synchron
// 3rdPartyFilter          - implemented as a UNO component
// Default                 - default filter for this document type
// NotInFileDialog         - should be self explaining
// NotInChooser            - as above

// (The 3rdPartyFilter flag is here called StarONE)

// At some point (4.0?) we should drop the duplicate set of names over
// in filter, and rename the obscure ones to describe their meaning
// using terms that are understandable.

enum class SfxFilterFlags
{
    IMPORT            = 0x00000001L,
    EXPORT            = 0x00000002L,
    TEMPLATE          = 0x00000004L,
    INTERNAL          = 0x00000008L,
    TEMPLATEPATH      = 0x00000010L,
    OWN               = 0x00000020L,
    ALIEN             = 0x00000040L,
    USESOPTIONS       = 0x00000080L,

    DEFAULT           = 0x00000100L,
    EXECUTABLE        = 0x00000200L,
    SUPPORTSSELECTION = 0x00000400L,
    NOTINFILEDLG      = 0x00001000L,
    OPENREADONLY      = 0x00010000L,
    MUSTINSTALL       = 0x00020000L,
    CONSULTSERVICE    = 0x00040000L,

    STARONEFILTER     = 0x00080000L,
    PACKED            = 0x00100000L,

    ENCRYPTION        = 0x01000000L,
    PASSWORDTOMODIFY  = 0x02000000L,

    PREFERED          = 0x10000000L,

    STARTPRESENTATION = 0x20000000L,

    NONE              = 0
};
namespace o3tl
{
    template<> struct typed_flags<SfxFilterFlags> : is_typed_flags<SfxFilterFlags, 0x331f17ffL> {};
}

#define SFX_FILTER_NOTINSTALLED (SfxFilterFlags::MUSTINSTALL | SfxFilterFlags::CONSULTSERVICE)

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
