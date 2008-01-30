/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgstrings.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:39:40 $
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
#include "cfgstrings.hrc"

namespace dbacfg
{
#include "stringconstants.cxx"

    //============================================================
    //= configuration key names
    //============================================================
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_CONNECTURL, "URL");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_USER, "User");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_TABLEFILTER, "TableFilter");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_TABLETYEFILTER, "TableTypeFilter");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_LOGINTIMEOUT, "LoginTimeout");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_PASSWORDREQUIRED, "IsPasswordRequired");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_SUPPRESSVERSIONCL, "SuppressVersionColumns");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DBLINK_BOOKMARKS, "Bookmarks");

    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_QRYDESCR_COMMAND, "Command");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_QRYDESCR_ESCAPE_PROCESSING, "EscapeProcessing");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_QRYDESCR_UPDATE_TABLENAME, "UpdateTableName");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_QRYDESCR_UPDATE_SCHEMANAME, "UpdateSchemaName");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_QRYDESCR_UPDATE_CATALOGNAME, "UpdateCatalogName");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_QRYDESCR_COLUMNS, "Columns");

    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_SETTINGS, "Settings");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FILTER, "Filter");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_APPLYFILTER, "ApplyFilter");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_ORDER, "Order");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_ROW_HEIGHT, "RowHeight");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_LAYOUTINFORMATION, "LayoutInformation");

    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_TEXTCOLOR, "TextColor");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_NAME, "Font/Name");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_HEIGHT, "Font/Height");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_WIDTH, "Font/Width");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_STYLENAME, "Font/StyleName");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_FAMILY, "Font/Family");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_CHARSET, "Font/CharSet");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_PITCH, "Font/Pitch");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH, "Font/CharacterWidth");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_WEIGHT, "Font/Weight");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_SLANT, "Font/Slant");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_UNDERLINE, "Font/UnderLine");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_STRIKEOUT, "Font/Strikeout");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_ORIENTATION, "Font/Orientation");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_KERNING, "Font/Kerning");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_WORDLINEMODE, "Font/WordLineMode");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_TYPE, "Font/Type");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_UNDERLINECOLOR, "TextLineColor");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_CHARACTEREMPHASIS, "CharacterEmphasis");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_DEFSET_FONT_CHARACTERRELIEF, "CharacterRelief");

    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_ALIGNMENT, "Align");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_WIDTH, "Width");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_NUMBERFORMAT, "FormatKey");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_RELPOSITION, "Position");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_HIDDEN, "Hidden");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_HELPTEXT, "HelpText");
    IMPLEMENT_CONSTASCII_USTRING(CONFIGKEY_COLUMN_CONTROLDEFAULT, "ControlDefault");
}

