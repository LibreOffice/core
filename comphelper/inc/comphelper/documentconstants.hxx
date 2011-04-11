/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _COMPHELPER_DOCUMENTCONSTANTS_HXX
#define _COMPHELPER_DOCUMENTCONSTANTS_HXX

#include <rtl/ustring.hxx>

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

#define MIMETYPE_VND_SUN_XML_WRITER                     ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_WRITER_ASCII ) )
#define MIMETYPE_VND_SUN_XML_WRITER_WEB                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII ) )
#define MIMETYPE_VND_SUN_XML_WRITER_GLOBAL              ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII ) )
#define MIMETYPE_VND_SUN_XML_DRAW                       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_DRAW_ASCII ) )
#define MIMETYPE_VND_SUN_XML_IMPRESS                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_IMPRESS_ASCII ) )
#define MIMETYPE_VND_SUN_XML_CALC                       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_CALC_ASCII ) )
#define MIMETYPE_VND_SUN_XML_CHART                      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_CHART_ASCII ) )
#define MIMETYPE_VND_SUN_XML_MATH                       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_MATH_ASCII ) )
#define MIMETYPE_VND_SUN_XML_BASE                       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_BASE_ASCII ) )

// template formats of SO6/7
#define MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE_ASCII               "application/vnd.sun.xml.writer.template"
#define MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE_ASCII                 "application/vnd.sun.xml.draw.template"
#define MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE_ASCII              "application/vnd.sun.xml.impress.template"
#define MIMETYPE_VND_SUN_XML_CALC_TEMPLATE_ASCII                 "application/vnd.sun.xml.calc.template"

#define MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE                     ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_WRITER_ASCII ) )
#define MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE                       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_DRAW_ASCII ) )
#define MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_IMPRESS_ASCII ) )
#define MIMETYPE_VND_SUN_XML_CALC_TEMPLATE                       ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_VND_SUN_XML_CALC_ASCII ) )

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

#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART               ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_DATABASE            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_REPORT              ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII ) )

// template formats of SO8
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII          "application/vnd.oasis.opendocument.text-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII       "application/vnd.oasis.opendocument.graphics-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII  "application/vnd.oasis.opendocument.presentation-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII   "application/vnd.oasis.opendocument.spreadsheet-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII         "application/vnd.oasis.opendocument.chart-template"
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII       "application/vnd.oasis.opendocument.formula-template"

#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE         ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE               ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII ) )
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE             ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII ) )

// ODF versions
#define ODFVER_010_TEXT_ASCII   "1.0"
#define ODFVER_011_TEXT_ASCII   "1.1"
#define ODFVER_012_TEXT_ASCII   "1.2"

#define ODFVER_010_TEXT ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ODFVER_010_TEXT_ASCII ) )
#define ODFVER_011_TEXT ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ODFVER_011_TEXT_ASCII ) )
#define ODFVER_012_TEXT ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ODFVER_012_TEXT_ASCII ) )
#endif

// filter flags
// TODO/LATER: The flags should be part of the UNO specification
#define SFX_FILTER_IMPORT            0x00000001L
#define SFX_FILTER_EXPORT            0x00000002L
#define SFX_FILTER_TEMPLATE          0x00000004L
#define SFX_FILTER_INTERNAL          0x00000008L
#define SFX_FILTER_TEMPLATEPATH      0x00000010L
#define SFX_FILTER_OWN               0x00000020L
#define SFX_FILTER_ALIEN             0x00000040L
#define SFX_FILTER_USESOPTIONS       0x00000080L

#define SFX_FILTER_DEFAULT           0x00000100L
#define SFX_FILTER_EXECUTABLE        0x00000200L
#define SFX_FILTER_SUPPORTSSELECTION 0x00000400L
#define SFX_FILTER_MAPTOAPPPLUG      0x00000800L
#define SFX_FILTER_NOTINFILEDLG      0x00001000L
#define SFX_FILTER_NOTINCHOOSER      0x00002000L
#define SFX_FILTER_ASYNC             0x00004000L
#define SFX_FILTER_CREATOR           0x00008000L
#define SFX_FILTER_OPENREADONLY      0x00010000L
#define SFX_FILTER_MUSTINSTALL       0x00020000L
#define SFX_FILTER_CONSULTSERVICE    0x00040000L

#define SFX_FILTER_STARONEFILTER     0x00080000L
#define SFX_FILTER_PACKED            0x00100000L

#define SFX_FILTER_BROWSERPREFERED   0x00400000L

#define SFX_FILTER_ENCRYPTION        0x01000000L
#define SFX_FILTER_PASSWORDTOMODIFY  0x02000000L

#define SFX_FILTER_PREFERED          0x10000000L

#define SFX_FILTER_VERSION_NONE      0
#define SFX_FILTER_NOTINSTALLED      SFX_FILTER_MUSTINSTALL | SFX_FILTER_CONSULTSERVICE

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */