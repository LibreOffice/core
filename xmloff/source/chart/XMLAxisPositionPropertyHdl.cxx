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
#include "XMLAxisPositionPropertyHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::xmloff::token;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace com::sun::star;

XMLAxisPositionPropertyHdl::XMLAxisPositionPropertyHdl( bool bCrossingValue )
        : m_bCrossingValue( bCrossingValue )
{}

XMLAxisPositionPropertyHdl::~XMLAxisPositionPropertyHdl()
{}

sal_Bool XMLAxisPositionPropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = false;

    if( rStrImpValue.equals( GetXMLToken(XML_START) ) )
    {
        if( !m_bCrossingValue )
        {
            rValue <<= ::com::sun::star::chart::ChartAxisPosition_START;
            bResult = true;
        }
    }
    else if( rStrImpValue.equals( GetXMLToken(XML_END) ) )
    {
        if( !m_bCrossingValue )
        {
            rValue <<= ::com::sun::star::chart::ChartAxisPosition_END;
            bResult = true;
        }
    }
    else
    {
        if( !m_bCrossingValue )
        {
            rValue <<= ::com::sun::star::chart::ChartAxisPosition_VALUE;
            bResult = true;
        }
        else
        {
            double fDblValue=0.0;
            bResult = SvXMLUnitConverter::convertDouble( fDblValue, rStrImpValue );
            rValue <<= fDblValue;
        }
    }

    return bResult;
}

sal_Bool XMLAxisPositionPropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = sal_False;

    rtl::OUStringBuffer sValueBuffer;
    if( m_bCrossingValue )
    {
        if(rStrExpValue.getLength() == 0)
        {
            double fValue = 0.0;
            rValue >>= fValue;
            SvXMLUnitConverter::convertDouble( sValueBuffer, fValue );
            rStrExpValue = sValueBuffer.makeStringAndClear();
            bResult = true;
        }
    }
    else
    {
        ::com::sun::star::chart::ChartAxisPosition ePosition( ::com::sun::star::chart::ChartAxisPosition_ZERO );
        rValue >>= ePosition;
        switch(ePosition)
        {
            case ::com::sun::star::chart::ChartAxisPosition_START:
                rStrExpValue = GetXMLToken( XML_START );
                bResult = true;
                break;
            case ::com::sun::star::chart::ChartAxisPosition_END:
                rStrExpValue = GetXMLToken( XML_END );
                bResult = true;
                break;
            case ::com::sun::star::chart::ChartAxisPosition_ZERO:
                SvXMLUnitConverter::convertDouble( sValueBuffer, 0.0 );
                rStrExpValue = sValueBuffer.makeStringAndClear();
                bResult = true;
                break;
            default:
                break;
        }
    }
    return bResult;
}
