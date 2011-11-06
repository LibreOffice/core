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
#include "XMLTextOrientationHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLTextOrientationHdl::~XMLTextOrientationHdl()
{
}

sal_Bool XMLTextOrientationHdl::importXML(
    const ::rtl::OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bRetval( sal_False );

    if( IsXMLToken( rStrImpValue, XML_LTR ))
    {
        rValue <<= static_cast< sal_Bool >( sal_False );
        bRetval = sal_True;
    }
    else if( IsXMLToken( rStrImpValue, XML_TTB ))
    {
        rValue <<= static_cast< sal_Bool >( sal_True );
        bRetval = sal_True;
    }

    return bRetval;
}

sal_Bool XMLTextOrientationHdl::exportXML(
    ::rtl::OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bVal (sal_False );
    sal_Bool bRetval( sal_False );

    if( rValue >>= bVal )
    {
        if( bVal )
            rStrExpValue = GetXMLToken( XML_TTB );
        else
            rStrExpValue = GetXMLToken( XML_LTR );
        bRetval = sal_True;
    }

    return bRetval;
}
