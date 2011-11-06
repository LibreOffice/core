/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include "XMLFillBitmapSizePropertyHandler.hxx"
#include <comphelper/extract.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


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

    sal_Int32 nValue = 0;
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

