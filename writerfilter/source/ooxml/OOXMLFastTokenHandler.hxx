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



#ifndef INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX
#define INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastTokenHandler.hpp"

namespace css = ::com::sun::star;

namespace writerfilter {
namespace ooxml
{

class OOXMLFastTokenHandler:
    public ::cppu::WeakImplHelper1<
        css::xml::sax::XFastTokenHandler>
{
public:
    explicit OOXMLFastTokenHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const ::rtl::OUString & Identifier) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException);

private:
    OOXMLFastTokenHandler(OOXMLFastTokenHandler &); // not defined
    void operator =(OOXMLFastTokenHandler &); // not defined

    virtual ~OOXMLFastTokenHandler() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

}}
#endif // INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX
