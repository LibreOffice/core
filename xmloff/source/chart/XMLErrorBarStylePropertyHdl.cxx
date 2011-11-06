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
#include "XMLErrorBarStylePropertyHdl.hxx"
#include <unotools/saveopt.hxx>

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/uno/Any.hxx>

using ::rtl::OUString;
using namespace com::sun::star;

XMLErrorBarStylePropertyHdl::XMLErrorBarStylePropertyHdl(  const SvXMLEnumMapEntry* pEnumMap, const ::com::sun::star::uno::Type & rType )
        : XMLEnumPropertyHdl( pEnumMap, rType )
{
}

XMLErrorBarStylePropertyHdl::~XMLErrorBarStylePropertyHdl()
{
}

sal_Bool XMLErrorBarStylePropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    uno::Any aValue(rValue);
    const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentVersion < SvtSaveOptions::ODFVER_012 )
    {
        sal_Int32 nValue = 0;
        if(rValue >>= nValue )
        {
            if( nValue == ::com::sun::star::chart::ErrorBarStyle::STANDARD_ERROR
                || nValue == ::com::sun::star::chart::ErrorBarStyle::FROM_DATA )
            {
                nValue = ::com::sun::star::chart::ErrorBarStyle::NONE;
                aValue = uno::makeAny(nValue);
            }
        }
    }

    return XMLEnumPropertyHdl::exportXML( rStrExpValue, aValue, rUnitConverter );
}
