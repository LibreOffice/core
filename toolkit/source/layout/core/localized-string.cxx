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



#include "localized-string.hxx"

#include <toolkit/helper/property.hxx>
#include <vcl/window.hxx>

namespace layoutimpl
{

namespace css = ::com::sun::star;
using namespace css;
using rtl::OUString;

LocalizedString::LocalizedString()
    : VCLXWindow()
{
}

void LocalizedString::ImplGetPropertyIds( std::list< sal_uInt16 > &ids )
{
    PushPropertyIds( ids, BASEPROPERTY_TEXT, 0);
    VCLXWindow::ImplGetPropertyIds( ids );
}

// XInterface
uno::Any LocalizedString::queryInterface( uno::Type const& rType )
    throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                            SAL_STATIC_CAST( awt::XFixedText*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

void LocalizedString::setText( OUString const& s )
    throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( Window *w = GetWindow() )
        return w->SetText( s );
}

OUString LocalizedString::getText()
    throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( Window *w = GetWindow() )
        return w->GetText();
    return OUString();
}

} // namespace layoutimpl
