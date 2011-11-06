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
#include "precompiled_canvas.hxx"

#include <com/sun/star/lang/NoSupportException.hpp>

#include "windowoutdevholder.hxx"
#include <toolkit/helper/vclunohelper.hxx>

using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        Window& windowFromXWin( const uno::Reference<awt::XWindow>& xWin )
        {
            Window* pWindow = VCLUnoHelper::GetWindow(xWin);
            if( !pWindow )
                throw lang::NoSupportException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                         "Parent window not VCL window, or canvas out-of-process!")),
                    NULL);
            return *pWindow;
        }
    }

    WindowOutDevHolder::WindowOutDevHolder( const uno::Reference<awt::XWindow>& xWin ) :
        mrOutputWindow( windowFromXWin(xWin) )
    {}
}
