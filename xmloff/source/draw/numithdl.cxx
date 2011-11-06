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
#include <numithdl.hxx>

using namespace ::com::sun::star;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLNumRulePropHdl
//

XMLNumRulePropHdl::XMLNumRulePropHdl( ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > xNumRuleCompare )
: mxNumRuleCompare( xNumRuleCompare )
{
}

XMLNumRulePropHdl::~XMLNumRulePropHdl()
{
    // Nothing to do
}

bool XMLNumRulePropHdl::equals( const uno::Any& r1, const uno::Any& r2 ) const
{
    return mxNumRuleCompare.is() && mxNumRuleCompare->compare( r1, r2 ) == 0;
}

sal_Bool XMLNumRulePropHdl::importXML( const ::rtl::OUString& /*rStrImpValue*/, ::com::sun::star::uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_False;
}

sal_Bool XMLNumRulePropHdl::exportXML( ::rtl::OUString& /*rStrExpValue*/, const ::com::sun::star::uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_False;
}

