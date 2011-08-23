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


#include "xmluconv.hxx"

#include <rtl/ustrbuf.hxx>

#include "XMLFillBitmapSizePropertyHandler.hxx"

namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

XMLFillBitmapSizePropertyHandler::XMLFillBitmapSizePropertyHandler()
{
}

XMLFillBitmapSizePropertyHandler::~XMLFillBitmapSizePropertyHandler()
{
}

sal_Bool XMLFillBitmapSizePropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int32 nValue;
    sal_Bool bRet;

    if( rStrImpValue.indexOf( sal_Unicode('%') ) != -1 )
    {
        bRet = rUnitConverter.convertPercent( nValue, rStrImpValue );
        nValue *= -1;
    }
    else
    {
        bRet = rUnitConverter.convertMeasure( nValue, rStrImpValue );
    }

    if( bRet )
        rValue <<= nValue;

    return bRet;
}

sal_Bool XMLFillBitmapSizePropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    sal_Int32 nValue;
    if( rValue >>= nValue )
    {
        if( nValue < 0 )
        {
            rUnitConverter.convertPercent( aOut, -nValue );
        }
        else
        {
            rUnitConverter.convertMeasure( aOut, nValue );
        }

        rStrExpValue = aOut.makeStringAndClear();
        return sal_True;
    }

    return sal_False;
}

}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
