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
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include "DrawAspectHdl.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLMeasurePropHdl
//

SvXMLEnumMapEntry __READONLY_DATA pXML_DrawAspect_Enum[] =
{
    { XML_CONTENT,          1   },
    { XML_THUMBNAIL,        2   },
    { XML_ICON,             4   },
    { XML_PRINT,            8   },
    { XML_TOKEN_INVALID, 0 }
};

DrawAspectHdl::~DrawAspectHdl()
{
    // nothing to do
}

sal_Bool DrawAspectHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int64 nAspect = 0;

    SvXMLUnitConverter::convertNumber64( nAspect, rStrImpValue );
    rValue <<= nAspect;

    return nAspect > 0;
}

sal_Bool DrawAspectHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
      OUStringBuffer aOut;

    sal_Int64 nAspect = 0;
    if( ( rValue >>= nAspect ) && nAspect > 0 )
    {
        // store the aspect as an integer value
        aOut.append( nAspect );

        rStrExpValue = aOut.makeStringAndClear();

        bRet = sal_True;
    }

    return bRet;
}

