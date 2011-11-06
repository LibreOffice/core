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
#include "shdwdhdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>

// --
#include <xmloff/xmltoken.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFmtSplitPropHdl
//

XMLShadowedPropHdl::~XMLShadowedPropHdl()
{
    // nothing to do
}

sal_Bool XMLShadowedPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    sal_Bool bValue = ! IsXMLToken( rStrImpValue, XML_NONE );
    rValue <<= sal_Bool(bValue);
    bRet = sal_True;

    return bRet;
}

sal_Bool XMLShadowedPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        if( bValue )
        {
            rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "1pt 1pt" ) );
        }
        else
        {
            rStrExpValue = GetXMLToken( XML_NONE );
        }

        bRet = sal_True;
    }

    return bRet;
}
