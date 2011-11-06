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
#include "kernihdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>

// --
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLKerningPropHdl
//

XMLKerningPropHdl::~XMLKerningPropHdl()
{
    // nothing to do
}

sal_Bool XMLKerningPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_True;
    sal_Int32 nKerning = 0;

    if( ! IsXMLToken( rStrImpValue, XML_KERNING_NORMAL ) )
    {
        bRet = rUnitConverter.convertMeasure( nKerning, rStrImpValue );
    }

    rValue <<= (sal_Int16)nKerning;

    return bRet;
}

sal_Bool XMLKerningPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Int16 nValue = sal_Int16();

    if( rValue >>= nValue )
    {
        OUStringBuffer aOut;

        if( nValue == 0 )
            aOut.append( GetXMLToken(XML_KERNING_NORMAL) );
        else
            rUnitConverter.convertMeasure( aOut, nValue );

        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}
