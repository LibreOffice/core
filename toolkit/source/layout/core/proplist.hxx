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



#ifndef LAYOUT_CORE_PROPLIST_HXX
#define LAYOUT_CORE_PROPLIST_HXX

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/xml/input/XAttributes.hpp>
#include <list>
#include <rtl/ustring.hxx>
#include <toolkit/dllapi.h>
\
namespace layoutimpl
{

namespace css = ::com::sun::star;

typedef std::list< std::pair< rtl::OUString, rtl::OUString > > PropList;

void propsFromAttributes( const css::uno::Reference<css::xml::input::XAttributes> & xAttributes,
                          PropList &rProps, sal_Int32 nNamespace );

void setProperties( css::uno::Reference< css::uno::XInterface > const& xPeer,
                    PropList const& rProps);

void setProperty( css::uno::Reference< css::uno::XInterface > const& xPeer,
                  rtl::OUString const& attr, rtl::OUString const& value );

long getAttributeProps( PropList &rProps );
bool findAndRemove( const char *pAttr, PropList &rProps, rtl::OUString &rValue);

// Helpers - unfortunately VCLXWindows don't implement XPropertySet
// but containers do - these helpers help us to hide this
namespace prophlp
{

// can we set properties on this handle ?
bool TOOLKIT_DLLPUBLIC canHandleProps( const css::uno::Reference< css::uno::XInterface > &xRef );
// if so which properties ?
css::uno::Reference< css::beans::XPropertySetInfo > TOOLKIT_DLLPUBLIC queryPropertyInfo(
    const css::uno::Reference< css::uno::XInterface > &xRef );
// set / get ...
void TOOLKIT_DLLPUBLIC setProperty( const css::uno::Reference< css::uno::XInterface > &xRef,
                  const rtl::OUString &rName,
                  css::uno::Any aValue );
css::uno::Any TOOLKIT_DLLPUBLIC getProperty( const css::uno::Reference< css::uno::XInterface > &xRef,
                           const rtl::OUString &rName );
} // namespace prophlp

} // namespace layoutimpl


#if !OUSTRING_CSTR_PARANOIA
#define OUSTRING_CSTR( str ) \
    rtl::OUStringToOString( str, RTL_TEXTENCODING_ASCII_US ).getStr()
#else

inline char const* OUSTRING_CSTR( rtl::OUString const& str )
{
    rtl::OString *leak
        = new rtl::OString (rtl::OUStringToOString (str, RTL_TEXTENCODING_ASCII_US));
    return leak->getStr();
}

#endif

#endif /* LAYOUT_CORE_PROPLIST_HXX */
