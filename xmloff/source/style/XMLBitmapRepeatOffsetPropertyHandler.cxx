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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "XMLBitmapRepeatOffsetPropertyHandler.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


using ::xmloff::token::GetXMLToken;
using ::xmloff::token::XML_VERTICAL;
using ::xmloff::token::XML_HORIZONTAL;


XMLBitmapRepeatOffsetPropertyHandler::XMLBitmapRepeatOffsetPropertyHandler( sal_Bool bX )
:   mbX( bX ),
    msVertical( GetXMLToken(XML_VERTICAL) ),
    msHorizontal( GetXMLToken(XML_HORIZONTAL) )
{
}

XMLBitmapRepeatOffsetPropertyHandler::~XMLBitmapRepeatOffsetPropertyHandler()
{
}

sal_Bool XMLBitmapRepeatOffsetPropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& ) const
{
    SvXMLTokenEnumerator aTokenEnum( rStrImpValue );
    OUString aToken;
    if( aTokenEnum.getNextToken( aToken ) )
    {
        sal_Int32 nValue;
        if( SvXMLUnitConverter::convertPercent( nValue, aToken ) )
        {
            if( aTokenEnum.getNextToken( aToken ) )
            {
                if( ( mbX && ( aToken == msHorizontal ) ) || ( !mbX && ( aToken == msVertical ) ) )
                {
                    rValue <<= nValue;
                    return sal_True;
                }
            }
        }
    }

    return sal_False;

}

sal_Bool XMLBitmapRepeatOffsetPropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;

    sal_Int32 nValue = 0;
    if( rValue >>= nValue )
    {
        SvXMLUnitConverter::convertPercent( aOut, nValue );
        aOut.append( sal_Unicode( ' ' ) );
        aOut.append( mbX ? msHorizontal : msVertical );
        rStrExpValue = aOut.makeStringAndClear();

        return sal_True;
    }

    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
