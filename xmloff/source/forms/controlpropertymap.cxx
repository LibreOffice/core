/*************************************************************************
 *
 *  $RCSfile: controlpropertymap.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-02 05:58:40 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif
#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif
#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif

#ifndef _XMLOFF_FORMS_STRINGS_HXX_
#include "strings.hxx"
#endif
#include <string.h>

//.........................................................................
namespace xmloff
{
//.........................................................................

    XMLPropertyMapEntry aControlStyleProperties[] =
    {
        { PROPERTY_BACKGROUNDCOLOR, XML_NAMESPACE_FO, sXML_background_color,    XML_TYPE_COLOR, 0 },
        { PROPERTY_ALIGN,   XML_NAMESPACE_STYLE,    sXML_text_align,            XML_TYPE_TEXT_ALIGN, 0 },
        { PROPERTY_BORDER,  XML_NAMESPACE_FO,       sXML_border,                XML_TYPE_CONTROL_BORDER, 0 },
        { "FontCharWidth",  XML_NAMESPACE_STYLE,    sXML_font_char_width,       XML_TYPE_NUMBER16, 0 },
        { "FontCharset",    XML_NAMESPACE_STYLE,    sXML_font_charset,          XML_TYPE_TEXT_FONTENCODING, 0 },
        { "FontFamily",     XML_NAMESPACE_STYLE,    sXML_font_family_generic,   XML_TYPE_TEXT_FONTFAMILY, 0 },
        { "FontHeight",     XML_NAMESPACE_FO,       sXML_font_size,             XML_TYPE_CHAR_HEIGHT, 0 },
        { "FontKerning",    XML_NAMESPACE_STYLE,    sXML_letter_kerning,        XML_TYPE_BOOL, 0 },
        { "FontName",       XML_NAMESPACE_STYLE,    sXML_font_name,             XML_TYPE_STRING, 0 },
        { "FontOrientation",XML_NAMESPACE_STYLE,    sXML_rotation_angle,        XML_TYPE_ROTATION_ANGLE, 0 },
        { "FontPitch",      XML_NAMESPACE_STYLE,    sXML_font_pitch,            XML_TYPE_TEXT_FONTPITCH, 0 },
        { "FontSlant",      XML_NAMESPACE_FO,       sXML_font_style,            XML_TYPE_TEXT_POSTURE, 0 },
        { "FontStrikeout",  XML_NAMESPACE_STYLE,    sXML_text_crossing_out,     XML_TYPE_TEXT_CROSSEDOUT, 0 },
        { "FontStyleName",  XML_NAMESPACE_STYLE,    sXML_font_style_name,       XML_TYPE_STRING, 0 },
        // "FontType" - ignored - UnoControls don't use a FontType
        { "FontUnderline",  XML_NAMESPACE_STYLE,    sXML_text_underline,        XML_TYPE_TEXT_UNDERLINE, 0 },
        { "FontWeight",     XML_NAMESPACE_FO,       sXML_font_weight,           XML_TYPE_TEXT_WEIGHT, 0 },
        { "FontWidth",      XML_NAMESPACE_STYLE,    sXML_font_width,            XML_TYPE_FONT_WIDTH, 0 },
        { "FontWordLineMode",XML_NAMESPACE_FO,      sXML_score_spaces,          XML_TYPE_NBOOL, 0 },
        { "TextColor",      XML_NAMESPACE_FO,       sXML_color,                 XML_TYPE_COLOR, 0 },
        { 0, 0, 0, 0 }
    };

    //=====================================================================
    //---------------------------------------------------------------------
    struct XMLPropertyMapEntryLess
    {
        sal_Bool operator()(const XMLPropertyMapEntry& _rLeft, const XMLPropertyMapEntry& _rRight)
        {
            return strcmp(_rLeft.msApiName, _rRight.msApiName) < 0;
        }
    };

    //---------------------------------------------------------------------
    void implSortMap(XMLPropertyMapEntry* _pMap)
    {
        // determine the last element
        for (XMLPropertyMapEntry* pEnd = _pMap; pEnd->msApiName; ++pEnd)
            ;
        ::std::sort(_pMap, pEnd, XMLPropertyMapEntryLess());
    }

    //---------------------------------------------------------------------
    void initializePropertyMaps()
    {
        static sal_Bool bSorted = sal_False;
        if (!bSorted)
        {
            implSortMap(aControlStyleProperties);
            bSorted = sal_True;
        }
    }

//.........................................................................
}   // namespace xmloff
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.5  2001/02/01 17:41:18  mh
 *  chg: header
 *
 *  Revision 1.4  2001/01/18 13:51:30  fs
 *  the background-color is part of the fo (not style) namespace
 *
 *  Revision 1.3  2000/12/19 12:13:57  fs
 *  some changes ... now the exported styles are XSL conform
 *
 *  Revision 1.1  2000/12/18 15:15:32  fs
 *  initial checkin - property maps
 *
 *
 *  Revision 1.0 14.12.00 10:09:14  fs
 ************************************************************************/

