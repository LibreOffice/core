/*************************************************************************
 *
 *  $RCSfile: dbustrings.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:27:04 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#include "dbustrings.hrc"

namespace dbaui
{

#include "stringconstants.cxx"

    //============================================================
    //= URLs
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_QUERYDESIGN, ".component:DB/QueryDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_TABLEDESIGN, ".component:DB/TableDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_FORMGRIDVIEW, ".component:DB/FormGridView");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_DATASOURCEBROWSER, ".component:DB/DataSourceBrowser");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_RELATIONDESIGN, ".component:DB/RelationDesign");
    IMPLEMENT_CONSTASCII_USTRING(URL_COMPONENT_APPLICATION, ".component:DB/Application");

    //============================================================
    //= service names
    //============================================================

    IMPLEMENT_CONSTASCII_USTRING(SERVICE_SDB_APPLICATIONCONTROLLER, "org.openoffice.comp.dbu.OApplicationController");
    IMPLEMENT_CONSTASCII_USTRING(SERVICE_SDB_DIRECTSQLDIALOG,       "org.openoffice.comp.dbu.DirectSqlDialog");

    //============================================================
    //= other
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(FRAME_NAME_QUERY_PREVIEW,          "QueryPreview");
    IMPLEMENT_CONSTASCII_USTRING( PARAM_INDEPENDENT_SQL_COMMAND,    "IndependentSQLCommand");
    IMPLEMENT_CONSTASCII_USTRING(SERVICE_CONTROLDEFAULT,            "com.sun.star.comp.dbu.OColumnControl");

    //============================================================
    //= properties
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TRISTATE,         "TriState");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ENABLEOUTERJOIN,  "EnableOuterJoinEscape");

    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_TABSTOP, "TabStop");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_DEFAULTCONTROL, "DefaultControl");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_ENABLED, "Enabled");
    IMPLEMENT_CONSTASCII_USTRING(PROPERTY_COLUMN, "Column");
}
