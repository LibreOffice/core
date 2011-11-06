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
#include "precompiled_sc.hxx"



#include "pageuno.hxx"
#include "shapeuno.hxx"

using namespace ::com::sun::star;

//------------------------------------------------------------------------

ScPageObj::ScPageObj( SdrPage* pPage ) :
    SvxFmDrawPage( pPage )
{
}

ScPageObj::~ScPageObj() throw()
{
}

uno::Reference<drawing::XShape > ScPageObj::_CreateShape( SdrObject *pObj ) const throw()
{
    uno::Reference<drawing::XShape> xShape(SvxFmDrawPage::_CreateShape( pObj ));

    new ScShapeObj( xShape );       // aggregates object and modifies xShape

    return xShape;
}

::rtl::OUString SAL_CALL ScPageObj::getImplementationName()
                                throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScPageObj"));
}

sal_Bool SAL_CALL ScPageObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr(rServiceName);
    return aServiceStr.EqualsAscii( "com.sun.star.sheet.SpreadsheetDrawPage" );
}

uno::Sequence<rtl::OUString> SAL_CALL ScPageObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDrawPage"));
    return aRet;
}
