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



#ifndef __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_
#define __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XStringAbbreviation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase2.hxx>

namespace framework
{

class UriAbbreviation:    public ::cppu::WeakImplHelper2< ::com::sun::star::util::XStringAbbreviation, css::lang::XServiceInfo>
{
public:
    explicit UriAbbreviation(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

    DECLARE_XSERVICEINFO

    // ::com::sun::star::util::XStringAbbreviation:
    virtual ::rtl::OUString SAL_CALL abbreviateString(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const ::rtl::OUString & aString) throw (::com::sun::star::uno::RuntimeException);

private:
    UriAbbreviation(UriAbbreviation &); // not defined
    void operator =(UriAbbreviation &); // not defined

    virtual ~UriAbbreviation() {}

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  m_xContext;
};

} //    namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_
