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
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>
#include "XMLPercentOrMeasurePropertyHandler.hxx"
#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;


XMLPercentOrMeasurePropertyHandler::XMLPercentOrMeasurePropertyHandler( sal_Bool bPercent )
: mbPercent( bPercent )
{
}

XMLPercentOrMeasurePropertyHandler::~XMLPercentOrMeasurePropertyHandler()
{
}

sal_Bool XMLPercentOrMeasurePropertyHandler::importXML(
    const OUString& rStrImpValue,
    Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    if( (rStrImpValue.indexOf( sal_Unicode('%') ) != -1) != mbPercent )
        return sal_False;

    sal_Int32 nValue;

    if( mbPercent )
    {
        if( !rUnitConverter.convertPercent( nValue, rStrImpValue ) )
            return sal_False;
    }
    else
    {
        if( !rUnitConverter.convertMeasure( nValue, rStrImpValue ) )
            return sal_False;
    }

    rValue <<= nValue;
    return sal_True;
}

sal_Bool XMLPercentOrMeasurePropertyHandler::exportXML(
    OUString& rStrExpValue,
    const Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
      OUStringBuffer aOut;

    sal_Int32 nValue = 0;
    if( !(rValue >>= nValue ) )
        return sal_False;

    if( mbPercent )
    {
         rUnitConverter.convertPercent( aOut, nValue );
    }
    else
    {
         rUnitConverter.convertMeasure( aOut, nValue );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return sal_True;
}
