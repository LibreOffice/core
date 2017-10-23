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

#include "backhdl.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry<style::GraphicLocation> const pXML_BrushHorizontalPos[] =
{
    { XML_LEFT,             style::GraphicLocation_LEFT_MIDDLE   },
    { XML_RIGHT,            style::GraphicLocation_RIGHT_MIDDLE },
    { XML_TOKEN_INVALID,    (style::GraphicLocation)0       }
};

SvXMLEnumMapEntry<style::GraphicLocation> const pXML_BrushVerticalPos[] =
{
    { XML_TOP,              style::GraphicLocation_MIDDLE_TOP   },
    { XML_BOTTOM,           style::GraphicLocation_MIDDLE_BOTTOM    },
    { XML_TOKEN_INVALID,    (style::GraphicLocation)0       }
};


// class XMLBackGraphicPositionPropHdl


XMLBackGraphicPositionPropHdl::~XMLBackGraphicPositionPropHdl()
{
    // Nothing to do
}

bool XMLBackGraphicPositionPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    style::GraphicLocation ePos = style::GraphicLocation_NONE, eTmp;
    style::GraphicLocation nTmpGraphicLocation;
    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    OUString aToken;
    bool bHori = false, bVert = false;

    while( bRet && aTokenEnum.getNextToken( aToken ) )
    {
        if( bHori && bVert )
        {
            bRet = false;
        }
        else if( -1 != aToken.indexOf( '%' ) )
        {
            sal_Int32 nPrc = 50;
            if (::sax::Converter::convertPercent( nPrc, aToken ))
            {
                if( !bHori )
                {
                    ePos = nPrc < 25 ? style::GraphicLocation_LEFT_TOP :
                                       (nPrc < 75 ? style::GraphicLocation_MIDDLE_MIDDLE :
                                                    style::GraphicLocation_RIGHT_BOTTOM);
                    bHori = true;
                }
                else
                {
                    eTmp = nPrc < 25 ? style::GraphicLocation_LEFT_TOP:
                                       (nPrc < 75 ? style::GraphicLocation_LEFT_MIDDLE :
                                                    style::GraphicLocation_LEFT_BOTTOM);
                    MergeXMLVertPos( ePos, eTmp );
                    bVert = true;
                }
            }
            else
            {
                // wrong percentage
                bRet = false;
            }
        }
        else if( IsXMLToken( aToken, XML_CENTER ) )
        {
            if( bHori )
                MergeXMLVertPos( ePos, style::GraphicLocation_MIDDLE_MIDDLE );
            else if ( bVert )
                MergeXMLHoriPos( ePos, style::GraphicLocation_MIDDLE_MIDDLE );
            else
                ePos = style::GraphicLocation_MIDDLE_MIDDLE;
        }
        else if( SvXMLUnitConverter::convertEnum( nTmpGraphicLocation, aToken, pXML_BrushHorizontalPos ) )
        {
            if( bVert )
                MergeXMLHoriPos( ePos, nTmpGraphicLocation );
            else if( !bHori )
                ePos = nTmpGraphicLocation;
            else
                bRet = false;

            bHori = true;
        }
        else if( SvXMLUnitConverter::convertEnum( nTmpGraphicLocation, aToken, pXML_BrushVerticalPos ) )
        {
            if( bHori )
                MergeXMLVertPos( ePos, nTmpGraphicLocation );
            else if( !bVert )
                ePos = nTmpGraphicLocation;
            else
                bRet = false;
            bVert = true;
        }
        else
        {
            bRet = false;
        }
    }

    bRet &= style::GraphicLocation_NONE != ePos;
    if( bRet )
        rValue <<= (style::GraphicLocation)(sal_uInt16)ePos;

    return bRet;
}

bool XMLBackGraphicPositionPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = true;
    OUStringBuffer aOut;

    style::GraphicLocation eLocation;
    if( !( rValue >>= eLocation ) )
    {
        sal_Int32 nValue = 0;
        if( rValue >>= nValue )
            eLocation = (style::GraphicLocation)nValue;
        else
            bRet = false;
    }

    if( bRet )
    {
        bRet = false;

        switch( eLocation )
        {
        case style::GraphicLocation_LEFT_TOP:
        case style::GraphicLocation_MIDDLE_TOP:
        case style::GraphicLocation_RIGHT_TOP:
            aOut.append( GetXMLToken(XML_TOP) );
            bRet = true;
            break;
        case style::GraphicLocation_LEFT_MIDDLE:
        case style::GraphicLocation_MIDDLE_MIDDLE:
        case style::GraphicLocation_RIGHT_MIDDLE:
            aOut.append( GetXMLToken(XML_CENTER) );
            bRet = true;
            break;
        case style::GraphicLocation_LEFT_BOTTOM:
        case style::GraphicLocation_MIDDLE_BOTTOM:
        case style::GraphicLocation_RIGHT_BOTTOM:
            aOut.append( GetXMLToken(XML_BOTTOM) );
            bRet = true;
            break;
        default:
            break;
        }

        if( bRet )
        {
            aOut.append( ' ' );

            switch( eLocation )
            {
            case style::GraphicLocation_LEFT_TOP:
            case style::GraphicLocation_LEFT_BOTTOM:
            case style::GraphicLocation_LEFT_MIDDLE:
                aOut.append( GetXMLToken(XML_LEFT) );
                break;
            case style::GraphicLocation_MIDDLE_TOP:
            case style::GraphicLocation_MIDDLE_MIDDLE:
            case style::GraphicLocation_MIDDLE_BOTTOM:
                aOut.append( GetXMLToken(XML_CENTER) );
                break;
            case style::GraphicLocation_RIGHT_MIDDLE:
            case style::GraphicLocation_RIGHT_TOP:
            case style::GraphicLocation_RIGHT_BOTTOM:
                aOut.append( GetXMLToken(XML_RIGHT) );
                break;
            default:
                break;
            }
        }
    }

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

void XMLBackGraphicPositionPropHdl::MergeXMLVertPos( style::GraphicLocation& ePos, style::GraphicLocation eVert )
{
    switch( ePos )
    {
    case style::GraphicLocation_LEFT_TOP:
    case style::GraphicLocation_LEFT_MIDDLE:
    case style::GraphicLocation_LEFT_BOTTOM:
        ePos = style::GraphicLocation_MIDDLE_TOP==eVert ?
               style::GraphicLocation_LEFT_TOP :
              (style::GraphicLocation_MIDDLE_MIDDLE==eVert ?
               style::GraphicLocation_LEFT_MIDDLE :
               style::GraphicLocation_LEFT_BOTTOM);
        break;

    case style::GraphicLocation_MIDDLE_TOP:
    case style::GraphicLocation_MIDDLE_MIDDLE:
    case style::GraphicLocation_MIDDLE_BOTTOM:
        ePos = eVert;
        break;

    case style::GraphicLocation_RIGHT_TOP:
    case style::GraphicLocation_RIGHT_MIDDLE:
    case style::GraphicLocation_RIGHT_BOTTOM:
        ePos = style::GraphicLocation_MIDDLE_TOP==eVert ?
               style::GraphicLocation_RIGHT_TOP :
              (style::GraphicLocation_MIDDLE_MIDDLE==eVert ?
               style::GraphicLocation_RIGHT_MIDDLE :
               style::GraphicLocation_RIGHT_BOTTOM);
        break;
    default:
        break;
    }
}

void XMLBackGraphicPositionPropHdl::MergeXMLHoriPos( style::GraphicLocation& ePos, style::GraphicLocation eHori )
{
    SAL_WARN_IF( !(style::GraphicLocation_LEFT_MIDDLE==eHori || style::GraphicLocation_MIDDLE_MIDDLE==eHori || style::GraphicLocation_RIGHT_MIDDLE==eHori), "xmloff",
                "lcl_frmitems_MergeXMLHoriPos: vertical pos must be middle" );

    switch( ePos )
    {
    case style::GraphicLocation_LEFT_TOP:
    case style::GraphicLocation_MIDDLE_TOP:
    case style::GraphicLocation_RIGHT_TOP:
        ePos = style::GraphicLocation_LEFT_MIDDLE==eHori ?
               style::GraphicLocation_LEFT_TOP :
              (style::GraphicLocation_MIDDLE_MIDDLE==eHori ?
               style::GraphicLocation_MIDDLE_TOP :
               style::GraphicLocation_RIGHT_TOP);
        break;

    case style::GraphicLocation_LEFT_MIDDLE:
    case style::GraphicLocation_MIDDLE_MIDDLE:
    case style::GraphicLocation_RIGHT_MIDDLE:
        ePos = eHori;
        break;

    case style::GraphicLocation_LEFT_BOTTOM:
    case style::GraphicLocation_MIDDLE_BOTTOM:
    case style::GraphicLocation_RIGHT_BOTTOM:
        ePos = style::GraphicLocation_LEFT_MIDDLE==eHori ?
               style::GraphicLocation_LEFT_BOTTOM :
              (style::GraphicLocation_MIDDLE_MIDDLE==eHori ?
               style::GraphicLocation_MIDDLE_BOTTOM :
               style::GraphicLocation_RIGHT_BOTTOM);
        break;
    default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
