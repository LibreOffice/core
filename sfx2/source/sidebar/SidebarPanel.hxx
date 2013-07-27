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

#ifndef SFX_SIDEBAR_SIDEBAR_PANEL_HXX
#define SFX_SIDEBAR_SIDEBAR_PANEL_HXX

#include <tools/link.hxx>
#include <com/sun/star/ui/XSidebarPanel.hpp>

#include <boost/noncopyable.hpp>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::ui::XSidebarPanel
        > SidebarPanelInterfaceBase;
}


class DockingWindow;
class VclWindowEvent;

namespace sfx2 { namespace sidebar {

class Panel;

class SidebarPanel
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SidebarPanelInterfaceBase
{
public:
    static cssu::Reference<css::ui::XSidebarPanel> Create (Panel* pPanel);

protected:
    SidebarPanel(
        Panel* pPanel);
    virtual ~SidebarPanel (void);

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEventObject)
        throw(cssu::RuntimeException);

    virtual void SAL_CALL disposing (void);

private:
    Panel* mpPanel;
    cssu::Reference<css::rendering::XCanvas> mxCanvas;

    DECL_LINK(HandleWindowEvent, VclWindowEvent*);
};


} } // end of namespace sfx2::sidebar

#endif
