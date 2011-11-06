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



#ifndef _VCLCANVAS_WINDOWOUTDEVHOLDER_HXX
#define _VCLCANVAS_WINDOWOUTDEVHOLDER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <vcl/window.hxx>

#include "outdevprovider.hxx"

#include <boost/utility.hpp>

namespace vclcanvas
{
    class WindowOutDevHolder : public OutDevProvider,
                               private ::boost::noncopyable
    {
    public:
        explicit WindowOutDevHolder( const ::com::sun::star::uno::Reference<
                                           ::com::sun::star::awt::XWindow>& xWin );

    private:
        virtual OutputDevice&       getOutDev() { return mrOutputWindow; }
        virtual const OutputDevice& getOutDev() const { return mrOutputWindow; }

        // TODO(Q2): Lifetime issue. Though WindowGraphicDeviceBase
        // now listenes to the window component, I still consider
        // holding a naked reference unsafe here (especially as we
        // pass it around via getOutDev). This _only_ works reliably,
        // if disposing the SpriteCanvas correctly disposes all
        // entities which hold this pointer.
        // So: as soon as the protocol inside
        // vcl/source/window/window.cxx is broken, that disposes the
        // canvas during window deletion, we're riding a dead horse
        // here
        Window& mrOutputWindow;
    };
}

#endif /* _VCLCANVAS_WINDOWOUTDEVHOLDER_HXX */
