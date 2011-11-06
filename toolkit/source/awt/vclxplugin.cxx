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



#include "vclxplugin.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/property.hxx>
#include <vcl/ctrl.hxx>

#include "forward.hxx"

namespace layoutimpl
{

using namespace ::com::sun::star;

VCLXPlugin::VCLXPlugin( Window *p, WinBits b )
    : VCLXWindow()
    , mpWindow( p )
    , mpPlugin( 0 )
    , mStyle( b )
{
}

VCLXPlugin::~VCLXPlugin()
{
}

void SAL_CALL VCLXPlugin::dispose() throw(uno::RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        lang::EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
    }

    VCLXWindow::dispose();
}

void VCLXPlugin::SetPlugin( ::Control *p )
{
    mpPlugin = p;
}

awt::Size SAL_CALL VCLXPlugin::getMinimumSize()
    throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OClearableGuard aGuard( GetMutex() );
    if ( mpPlugin )
        return AWTSize( mpPlugin->GetSizePixel() );
    return awt::Size();
}

} // namespace layoutimpl
