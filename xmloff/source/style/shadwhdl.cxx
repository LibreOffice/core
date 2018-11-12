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

#include "shadwhdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>


#include <com/sun/star/table/ShadowFormat.hpp>
#include <o3tl/safeint.hxx>
#include <tools/color.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;


// class XMLMeasurePropHdl


XMLShadowPropHdl::~XMLShadowPropHdl()
{
    // nothing to do
}

bool XMLShadowPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;
    table::ShadowFormat aShadow;
    aShadow.Location = table::ShadowLocation_BOTTOM_RIGHT;

    bool bColorFound = false;
    bool bOffsetFound = false;
    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    Color aColor( 128,128, 128 );
    OUString aToken;

    while( aTokenEnum.getNextToken( aToken ) )
    {
        if( IsXMLToken( aToken, XML_NONE ) )
        {
            aShadow.Location = table::ShadowLocation_NONE;
            bRet = true;
            break;
        }
        else if( !bColorFound && aToken.startsWith("#") )
        {
            bRet = ::sax::Converter::convertColor( aColor, aToken );
            if( !bRet )
                return false;
            bColorFound = true;
        }
        else if( !bOffsetFound )
        {
            sal_Int32 nX = 0, nY = 0;

            bRet = rUnitConverter.convertMeasureToCore( nX, aToken );
            if( bRet && aTokenEnum.getNextToken( aToken ) )
                bRet = rUnitConverter.convertMeasureToCore( nY, aToken );

            if( bRet )
            {
                if( nX < 0 )
                {
                    if( nY < 0 )
                        aShadow.Location = table::ShadowLocation_TOP_LEFT;
                    else
                        aShadow.Location = table::ShadowLocation_BOTTOM_LEFT;
                }
                else
                {
                    if( nY < 0 )
                        aShadow.Location = table::ShadowLocation_TOP_RIGHT;
                    else
                        aShadow.Location = table::ShadowLocation_BOTTOM_RIGHT;
                }

                if (nX < 0)
                    nX = o3tl::saturating_toggle_sign(nX);
                if (nY < 0)
                    nY = o3tl::saturating_toggle_sign(nY);

                sal_Int32 nWidth;
                bRet = !o3tl::checked_add(nX, nY, nWidth);
                if (bRet)
                    aShadow.ShadowWidth = sal::static_int_cast<sal_Int16>(nWidth >> 1);
            }
        }
    }

    if( bRet && ( bColorFound || bOffsetFound ) )
    {
        aShadow.IsTransparent = aColor.GetTransparency() > 0;
        aShadow.Color = sal_Int32(aColor);
        bRet = true;
    }

    rValue <<= aShadow;

    return bRet;
}

bool XMLShadowPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;
    OUStringBuffer aOut;
    table::ShadowFormat aShadow;

    if( rValue >>= aShadow )
    {
        sal_Int32 nX = 1, nY = 1;

        switch( aShadow.Location )
        {
            case table::ShadowLocation_TOP_LEFT:
                nX = -1;
                nY = -1;
                break;
            case table::ShadowLocation_TOP_RIGHT:
                nY = -1;
                break;
            case table::ShadowLocation_BOTTOM_LEFT:
                nX = -1;
                break;
            case table::ShadowLocation_BOTTOM_RIGHT:
                break;
            case table::ShadowLocation_NONE:
            default:
                rStrExpValue = GetXMLToken(XML_NONE);
                return true;
        }

        nX *= aShadow.ShadowWidth;
        nY *= aShadow.ShadowWidth;

        ::sax::Converter::convertColor( aOut, aShadow.Color );

        aOut.append( ' ' );
        rUnitConverter.convertMeasureToXML( aOut, nX );
        aOut.append( ' ' );
        rUnitConverter.convertMeasureToXML( aOut, nY );

        rStrExpValue = aOut.makeStringAndClear();

        bRet = true;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
