/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formenums.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:09:23 $
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

#ifndef _XMLOFF_FORMENUMS_HXX_
#define _XMLOFF_FORMENUMS_HXX_

#ifndef _XMLOFF_XMLEMENT_HXX
#include "xmlement.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OEnumMapper
    //=====================================================================
    class OEnumMapper
    {
    public:
        enum EnumProperties
        {
            epSubmitEncoding = 0,
            epSubmitMethod,
            epCommandType,
            epNavigationType,
            epTabCyle,
            epButtonType,
            epListSourceType,
            epCheckState,
            epTextAlign,
            epBorderWidth,
            epFontEmphasis,
            epFontRelief,
            epListLinkageType,
            epOrientation,
            epVisualEffect,
            epImagePosition,
            epImageAlign,

            KNOWN_ENUM_PROPERTIES
        };

    private:
        static const SvXMLEnumMapEntry* s_pEnumMap[KNOWN_ENUM_PROPERTIES];

    public:
        static const SvXMLEnumMapEntry* getEnumMap(EnumProperties _eProperty);
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMENUMS_HXX_

