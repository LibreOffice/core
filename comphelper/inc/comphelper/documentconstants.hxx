/*************************************************************************
 *
 *  $RCSfile: documentconstants.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-26 21:43:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COMPHELPER_DOCUMENTCONSTANTS_HXX
#define _COMPHELPER_DOCUMENTCONSTANTS_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

// formats of SO6/7
#define MIMETYPE_VND_SUN_XML_WRITER_ASCII               "application/vnd.sun.xml.writer"
#define MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII           "application/vnd.sun.xml.writer.web"
#define MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII        "application/vnd.sun.xml.writer.global"
#define MIMETYPE_VND_SUN_XML_DRAW_ASCII                 "application/vnd.sun.xml.draw"
#define MIMETYPE_VND_SUN_XML_IMPRESS_ASCII              "application/vnd.sun.xml.impress"
#define MIMETYPE_VND_SUN_XML_CALC_ASCII                 "application/vnd.sun.xml.calc"
#define MIMETYPE_VND_SUN_XML_CHART_ASCII                "application/vnd.sun.xml.chart"
#define MIMETYPE_VND_SUN_XML_MATH_ASCII                 "application/vnd.sun.xml.math"

#define MIMETYPE_VND_SUN_XML_WRITER                     ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_WRITER_ASCII)
#define MIMETYPE_VND_SUN_XML_WRITER_WEB                 ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII)
#define MIMETYPE_VND_SUN_XML_WRITER_GLOBAL              ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII)
#define MIMETYPE_VND_SUN_XML_DRAW                       ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_DRAW_ASCII)
#define MIMETYPE_VND_SUN_XML_IMPRESS                    ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_IMPRESS_ASCII)
#define MIMETYPE_VND_SUN_XML_CALC                       ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_CALC_ASCII)
#define MIMETYPE_VND_SUN_XML_CHART                      ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_CHART_ASCII)
#define MIMETYPE_VND_SUN_XML_MATH                       ::rtl::OUString::createFromAscii(MIMETYPE_VND_SUN_XML_MATH_ASCII)

// formats of SO8
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII          "application/vnd.oasis.opendocument.text"
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII      "application/vnd.oasis.opendocument.text-web"
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII   "application/vnd.oasis.opendocument.text-global"
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII       "application/vnd.oasis.opendocument.drawing"
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII  "application/vnd.oasis.opendocument.presentation"
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII   "application/vnd.oasis.opendocument.spreadsheet"
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII         "application/vnd.oasis.opendocument.chart"
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII       "application/vnd.oasis.opendocument.formula"
#define MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII      "application/vnd.oasis.opendocument.database"

#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT                ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB            ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL         ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_DRAWING             ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION        ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET         ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_CHART               ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_FORMULA             ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII)
#define MIMETYPE_OASIS_OPENDOCUMENT_DATABASE            ::rtl::OUString::createFromAscii(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII)

#endif

