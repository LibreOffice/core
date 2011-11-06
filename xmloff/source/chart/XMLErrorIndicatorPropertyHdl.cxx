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
#include "XMLErrorIndicatorPropertyHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star;

XMLErrorIndicatorPropertyHdl::~XMLErrorIndicatorPropertyHdl()
{}

sal_Bool XMLErrorIndicatorPropertyHdl::importXML( const ::rtl::OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bValue;
    SvXMLUnitConverter::convertBool( bValue, rStrImpValue );

    // modify existing value
    chart::ChartErrorIndicatorType eType = chart::ChartErrorIndicatorType_NONE;
    if( rValue.hasValue())
        rValue >>= eType;

    if( bValue )    // enable flag
    {
        if( eType != chart::ChartErrorIndicatorType_TOP_AND_BOTTOM )
        {
            if( mbUpperIndicator )
                eType = ( eType == chart::ChartErrorIndicatorType_LOWER )
                    ? chart::ChartErrorIndicatorType_TOP_AND_BOTTOM
                    : chart::ChartErrorIndicatorType_UPPER;
            else
                eType = ( eType == chart::ChartErrorIndicatorType_UPPER )
                    ? chart::ChartErrorIndicatorType_TOP_AND_BOTTOM
                    : chart::ChartErrorIndicatorType_LOWER;
        }
    }
    else            // disable flag
    {
        if( eType != chart::ChartErrorIndicatorType_NONE )
        {
            if( mbUpperIndicator )
                eType = ( eType == chart::ChartErrorIndicatorType_UPPER )
                    ? chart::ChartErrorIndicatorType_NONE
                    : chart::ChartErrorIndicatorType_LOWER;
            else
                eType = ( eType == chart::ChartErrorIndicatorType_LOWER )
                    ? chart::ChartErrorIndicatorType_NONE
                    : chart::ChartErrorIndicatorType_UPPER;
        }
    }

    rValue <<= eType;

    return sal_True;
}

sal_Bool XMLErrorIndicatorPropertyHdl::exportXML( ::rtl::OUString& rStrExpValue,
                                                  const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    rtl::OUStringBuffer aBuffer;
    chart::ChartErrorIndicatorType eType;

    rValue >>= eType;
    sal_Bool bValue = ( eType == chart::ChartErrorIndicatorType_TOP_AND_BOTTOM ||
                        ( mbUpperIndicator
                          ? ( eType == chart::ChartErrorIndicatorType_UPPER )
                          : ( eType == chart::ChartErrorIndicatorType_LOWER )));

    if( bValue )
    {
        SvXMLUnitConverter::convertBool( aBuffer, bValue );
        rStrExpValue = aBuffer.makeStringAndClear();
    }

    // only export if set to true
    return bValue;
}
