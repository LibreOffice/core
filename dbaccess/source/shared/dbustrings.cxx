/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbustrings.cxx,v $
 * $Revision: 1.18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "dbustrings.hrc"

namespace dbaui
{

#include "stringconstants.inc"

    //============================================================
    //= URLs
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_QUERYDESIGN,         ".component:DB/QueryDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_VIEWDESIGN,          ".component:DB/ViewDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_TABLEDESIGN,         ".component:DB/TableDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_FORMGRIDVIEW,        ".component:DB/FormGridView");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_DATASOURCEBROWSER,   ".component:DB/DataSourceBrowser");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_RELATIONDESIGN,      ".component:DB/RelationDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_APPLICATION,         ".component:DB/Application");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_REPORTDESIGN,        ".component:DB/ReportDesign");

    //============================================================
    //= service names
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(SERVICE_SDB_DIRECTSQLDIALOG,       "org.openoffice.comp.dbu.DirectSqlDialog");

    //============================================================
    //= other
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(FRAME_NAME_QUERY_PREVIEW,          "QueryPreview");
    IMPLEMENT_CONSTASCII_USTRING(SERVICE_CONTROLDEFAULT,            "com.sun.star.comp.dbu.OColumnControl");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_QUERYDESIGNVIEW,          "QueryDesignView");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_GRAPHICAL_DESIGN,         "GraphicalDesign");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_HELP_URL,                 "HelpURL");

    //============================================================
    //= properties
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TRISTATE,         "TriState");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ENABLEOUTERJOIN,  "EnableOuterJoinEscape");

    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TABSTOP, "TabStop");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_DEFAULTCONTROL, "DefaultControl");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ENABLED, "Enabled");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_MOUSE_WHEEL_BEHAVIOR, "MouseWheelBehavior");
}
