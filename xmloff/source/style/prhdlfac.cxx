/*************************************************************************
 *
 *  $RCSfile: prhdlfac.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-19 14:25:17 $
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


#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <prhdlfac.hxx>
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include "xmltypes.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#include "xmlbahdl.hxx"
#endif

#ifndef _XMLOFF_NAMEDBOOLPROPERTYHANDLER_HXX
#include "NamedBoolPropertyHdl.hxx"
#endif

#ifndef _XMLOFF_PROPERTYHANDLER_CROSSEDOUTTYPES_HXX
#include "cdouthdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_CASEMAPTYPES_HXX
#include "csmaphdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FONTTYPES_HXX
#include "fonthdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_KERNINGTYPES_HXX
#include "kernihdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_POSTURETYPES_HXX
#include <postuhdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_SHADOWTYPES_HXX
#include "shadwhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_SHADOWEDTYPES_HXX
#include "shdwdhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_UNDERLINETYPES_HXX
#include "undlihdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FONTWEIGHTTYPES_HXX
#include "weighhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_FMTSPLITTYPES_HXX
#include "splithdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_BREAKTYPES_HXX
#include "breakhdl.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_ADJUSTTYPES_HXX
#include <adjushdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_ESCAPETYPES_HXX
#include <escphdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_CHARHEIGHTTYPES_HXX
#include <chrhghdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_CHARLOCALETYPES_HXX
#include <chrlohdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_LINESPACETYPES_HXX
#include <lspachdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_BORDERTYPES_HXX
#include <bordrhdl.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLER_TABSTOPTYPES_HXX
#include <tabsthdl.hxx>
#endif

///////////////////////////////////////////////////////////////////////////
//
// Dtor
//
XMLPropertyHandlerFactory::~XMLPropertyHandlerFactory()
{
    for( CacheMap::iterator pPos = maHandlerCache.begin(); pPos != maHandlerCache.end(); pPos++ )
        delete pPos->second;
}

///////////////////////////////////////////////////////////////////////////
//
// Interface
//
const XMLPropertyHandler* XMLPropertyHandlerFactory::GetPropertyHandler( sal_Int32 nType ) const
{
    DBG_ASSERT( (nType & ~((sal_uInt32)MID_FLAG_MASK)) == 0,
                "GetPropertyHandler called with flags in type" );
    return GetBasicHandler( nType );
}

///////////////////////////////////////////////////////////////////////////
//
// Helper-methods to create and cache PropertyHandler
//
XMLPropertyHandler* XMLPropertyHandlerFactory::GetHdlCache( sal_Int32 nType ) const
{
    XMLPropertyHandler* pRet = NULL;

    if( maHandlerCache.find( nType ) != maHandlerCache.end() )
        pRet = maHandlerCache.find( nType )->second;

    return pRet;
}

void XMLPropertyHandlerFactory::PutHdlCache( sal_Int32 nType, const XMLPropertyHandler* pHdl ) const
{
    // Don't be wondered about the following construct. The  sense is to be able to provide a const-
    // method as class-interface.
    ((XMLPropertyHandlerFactory*)this)->maHandlerCache[ nType ] = (XMLPropertyHandler*)pHdl;
}

const XMLPropertyHandler* XMLPropertyHandlerFactory::GetBasicHandler( sal_Int32 nType ) const
{
    XMLPropertyHandler* pPropHdl = GetHdlCache( nType );

    if( !pPropHdl )
    {
        switch( nType )
        {
            case XML_TYPE_BOOL :
                pPropHdl = new XMLBoolPropHdl;
                break;
            case XML_TYPE_MEASURE :
                pPropHdl = new XMLMeasurePropHdl( 4 );
                break;
            case XML_TYPE_MEASURE8 :
                pPropHdl = new XMLMeasurePropHdl( 1 );
                break;
            case XML_TYPE_MEASURE16:
                pPropHdl = new XMLMeasurePropHdl( 2 );
                break;
            case XML_TYPE_PERCENT :
                pPropHdl = new XMLPercentPropHdl( 4 );
                break;
            case XML_TYPE_PERCENT8 :
                pPropHdl = new XMLPercentPropHdl( 1 );
                break;
            case XML_TYPE_PERCENT16 :
                pPropHdl = new XMLPercentPropHdl( 2 );
                break;
            case XML_TYPE_STRING :
                pPropHdl = new XMLStringPropHdl;
                break;
            case XML_TYPE_COLOR :
                pPropHdl = new XMLColorPropHdl;
                break;
            case XML_TYPE_NUMBER :
                pPropHdl = new XMLNumberPropHdl( 4 );
                break;
            case XML_TYPE_NUMBER8 :
                pPropHdl = new XMLNumberPropHdl( 1 );
                break;
            case XML_TYPE_NUMBER16:
                pPropHdl = new XMLNumberPropHdl( 2 );
                break;
            case XML_TYPE_NUMBER_NONE :
                pPropHdl = new XMLNumberNonePropHdl;
                break;
            case XML_TYPE_DOUBLE :
                pPropHdl = new XMLDoublePropHdl;
                break;
            case XML_TYPE_NBOOL :
                pPropHdl = new XMLNBoolPropHdl;
                break;
            case XML_TYPE_COLORTRANSPARENT :
                pPropHdl = new XMLColorTransparentPropHdl;
                break;
            case XML_TYPE_ISTRANSPARENT :
                pPropHdl = new XMLIsTransparentPropHdl;
                break;
            case XML_TYPE_BUILDIN_CMP_ONLY :
                pPropHdl = new XMLCompareOnlyPropHdl;
                break;

            case XML_TYPE_TEXT_CROSSEDOUT:
                pPropHdl = new XMLCrossedOutPropHdl ;
                break;
            case XML_TYPE_TEXT_BOOLCROSSEDOUT:
                {
                    ::rtl::OUString aStrTrueString( RTL_CONSTASCII_USTRINGPARAM( sXML_crossedout_single ) );
                    ::rtl::OUString aStrFalseString( RTL_CONSTASCII_USTRINGPARAM( sXML_crossedout_none ) );
                    pPropHdl = new XMLNamedBoolPropertyHdl( aStrTrueString, aStrFalseString );
                }
                break;
            case XML_TYPE_TEXT_ESCAPEMENT:
                pPropHdl = new XMLEscapementPropHdl;
                break;
            case XML_TYPE_TEXT_ESCAPEMENT_HEIGHT:
                pPropHdl = new XMLEscapementHeightPropHdl;
                break;
            case XML_TYPE_TEXT_CASEMAP:
                pPropHdl = new XMLCaseMapPropHdl;
                break;
            case XML_TYPE_TEXT_CASEMAP_VAR:
                pPropHdl = new XMLCaseMapVariantHdl;
                break;
            case XML_TYPE_TEXT_FONTFAMILYNAME:
                pPropHdl = new XMLFontFamilyNamePropHdl;
                break;
            case XML_TYPE_TEXT_FONTFAMILY:
                pPropHdl = new XMLFontFamilyPropHdl;
                break;
            case XML_TYPE_TEXT_FONTENCODING:
                pPropHdl = new XMLFontEncodingPropHdl;
                break;
            case XML_TYPE_TEXT_FONTPITCH:
                pPropHdl = new XMLFontPitchPropHdl;
                break;
            case XML_TYPE_TEXT_KERNING:
                pPropHdl = new XMLKerningPropHdl;
                break;
            case XML_TYPE_TEXT_POSTURE:
                pPropHdl = new XMLPosturePropHdl;
                break;
            case XML_TYPE_TEXT_SHADOWED:
                pPropHdl = new XMLShadowedPropHdl;
                break;
            case XML_TYPE_TEXT_UNDERLINE:
                pPropHdl = new XMLUnderlinePropHdl;
                break;
            case XML_TYPE_TEXT_WEIGHT:
                pPropHdl = new XMLFontWeightPropHdl;
                break;
            case XML_TYPE_TEXT_SPLIT:
                pPropHdl = new XMLFmtSplitPropHdl;
                break;
            case XML_TYPE_TEXT_BREAKBEFORE:
                pPropHdl = new XMLFmtBreakBeforePropHdl;
                break;
            case XML_TYPE_TEXT_BREAKAFTER:
                pPropHdl = new XMLFmtBreakAfterPropHdl;
                break;
            case XML_TYPE_TEXT_SHADOW:
                pPropHdl = new XMLShadowPropHdl;
                break;
            case XML_TYPE_TEXT_ADJUST:
                pPropHdl = new XMLParaAdjustPropHdl;
                break;
            case XML_TYPE_TEXT_ADJUSTLAST:
                pPropHdl = new XMLLastLineAdjustPropHdl;
                break;
            case XML_TYPE_CHAR_HEIGHT:
                pPropHdl = new XMLCharHeightHdl;
                break;
            case XML_TYPE_CHAR_HEIGHT_PROP:
                pPropHdl = new XMLCharHeightPropHdl;
                break;
            case XML_TYPE_CHAR_LANGUAGE:
                pPropHdl = new XMLCharLanguageHdl;
                break;
            case XML_TYPE_CHAR_COUNTRY:
                pPropHdl = new XMLCharCountryHdl;
                break;
            case XML_TYPE_LINE_SPACE_FIXED:
                pPropHdl = new XMLLineHeightHdl;
                break;
            case XML_TYPE_LINE_SPACE_MINIMUM:
                pPropHdl = new XMLLineHeightAtLeastHdl;
                break;
            case XML_TYPE_LINE_SPACE_DISTANCE:
                pPropHdl = new XMLLineSpacingHdl;
                break;
            case XML_TYPE_BORDER_WIDTH:
                pPropHdl = new XMLBorderWidthHdl;
                break;
            case XML_TYPE_BORDER:
                pPropHdl = new XMLBorderHdl;
                break;
            case XML_TYPE_TEXT_TABSTOP:
                pPropHdl = new XMLTabStopPropHdl;
                break;
        }

        if( pPropHdl )
            PutHdlCache( nType, pPropHdl );
    }

    return pPropHdl;
}
