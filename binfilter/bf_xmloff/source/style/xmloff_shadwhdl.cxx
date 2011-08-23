/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "shadwhdl.hxx"


#include <rtl/ustrbuf.hxx>

// --

#include <com/sun/star/table/ShadowFormat.hpp>

#include "xmluconv.hxx"

namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLMeasurePropHdl
//

XMLShadowPropHdl::~XMLShadowPropHdl()
{
    // nothing to do
}

sal_Bool XMLShadowPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
    table::ShadowFormat aShadow;
    aShadow.Location = table::ShadowLocation_BOTTOM_RIGHT;

    sal_Bool bColorFound = sal_False;
    sal_Bool bOffsetFound = sal_False;
    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    Color aColor( 128,128, 128 );
    OUString aToken;

    while( aTokenEnum.getNextToken( aToken ) )
    {
        if( IsXMLToken( aToken, XML_NONE ) )
        {
            aShadow.Location = table::ShadowLocation_NONE;
            bRet = sal_True;
            break;
        }
        else if( !bColorFound && aToken.compareToAscii( "#", 1 ) == 0 )
        {
            bRet = rUnitConverter.convertColor( aColor, aToken );
            if( !bRet )
                return sal_False;

            bColorFound = sal_True;
        }
        else if( !bOffsetFound )
        {
            sal_Int32 nX = 0, nY = 0;

            bRet = rUnitConverter.convertMeasure( nX, aToken );
            if( bRet && aTokenEnum.getNextToken( aToken ) )
                bRet = rUnitConverter.convertMeasure( nY, aToken );

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

                if( nX < 0 ) nX *= -1;
                if( nY < 0 ) nY *= -1;

                aShadow.ShadowWidth = (nX + nY) >> 1;
            }
        }
    }

    if( bRet && ( bColorFound || bOffsetFound ) )
    {
        aShadow.IsTransparent = aColor.GetTransparency() > 0;
        aShadow.Color = aColor.GetColor();
        bRet = sal_True;
    }

    rValue <<= aShadow;

    return bRet; 
}

sal_Bool XMLShadowPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    sal_Bool bRet = sal_False;
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
                return sal_True;
        }

        nX *= aShadow.ShadowWidth;
        nY *= aShadow.ShadowWidth;

        rUnitConverter.convertColor( aOut, aShadow.Color );

        aOut.append( sal_Unicode(' ') );
        rUnitConverter.convertMeasure( aOut, nX );
        aOut.append( sal_Unicode(' ') );
        rUnitConverter.convertMeasure( aOut, nY );

        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
