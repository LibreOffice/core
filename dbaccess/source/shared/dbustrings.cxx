/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbustrings.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 13:58:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "dbustrings.hrc"

namespace dbaui
{

#include "stringconstants.cxx"

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
}
