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

#include "precompiled_sd.hxx"

#include "SidebarFactory.hxx"
#include "framework/Pane.hxx"
#include "ViewShellBase.hxx"
#include "DrawController.hxx"
#include "LayoutMenu.hxx"
#include "CurrentMasterPagesSelector.hxx"
#include "RecentMasterPagesSelector.hxx"
#include "AllMasterPagesSelector.hxx"
#include "CustomAnimationPanel.hxx"
#include "TableDesignPanel.hxx"
#include "SlideTransitionPanel.hxx"

#include <sfx2/viewfrm.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace cssu;
using namespace ::sd::framework;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace sd { namespace sidebar {

namespace {
    /** Note that these names have to be identical to (the tail of)
        the entries in officecfg/registry/data/org/openoffice/Office/Impress.xcu
        for the TaskPanelFactory.
    */
    const static char* gsResourceNameCustomAnimations = "/CustomAnimations";
    const static char* gsResourceNameLayouts = "/Layouts";
    const static char* gsResourceNameAllMasterPages = "/AllMasterPages";
    const static char* gsResourceNameRecentMasterPages = "/RecentMasterPages";
    const static char* gsResourceNameUsedMasterPages = "/UsedMasterPages";
    const static char* gsResourceNameSlideTransitions = "/SlideTransitions";
    const static char* gsResourceNameTableDesign = "/TableDesign";
}

Reference<lang::XEventListener> mxControllerDisposeListener;



// ----- Service functions ----------------------------------------------------

Reference<XInterface> SAL_CALL SidebarFactory_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new SidebarFactory(rxContext)));
}




::rtl::OUString SidebarFactory_getImplementationName (void) throw(RuntimeException)
{
    return ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.Draw.framework.SidebarFactory"));
}




Sequence<rtl::OUString> SAL_CALL SidebarFactory_getSupportedServiceNames (void)
    throw (RuntimeException)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.SidebarFactory"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




//----- SidebarFactory --------------------------------------------------------

SidebarFactory::SidebarFactory(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    : SidebarFactoryInterfaceBase(m_aMutex)
{
}




SidebarFactory::~SidebarFactory (void)
{
}




void SAL_CALL SidebarFactory::disposing (void)
{
}




// XInitialization

void SAL_CALL SidebarFactory::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
}




// XUIElementFactory

Reference<ui::XUIElement> SAL_CALL SidebarFactory::createUIElement (
    const ::rtl::OUString& rsUIElementResourceURL,
    const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        css::container::NoSuchElementException,
        css::lang::IllegalArgumentException,
        cssu::RuntimeException)
{
    // Process arguments.
    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    Reference<ui::XSidebar> xSidebar (aArguments.getOrDefault("Sidebar", Reference<ui::XSidebar>()));

    // Throw exceptions when the arguments are not as expected.
    ::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( ! xParentWindow.is() || pParentWindow==NULL)
        throw RuntimeException(
            A2S("SidebarFactory::createUIElement called without ParentWindow"),
            NULL);
    if ( ! xFrame.is())
        throw RuntimeException(
            A2S("SidebarFactory::createUIElement called without XFrame"),
            NULL);

    // Tunnel through the controller to obtain a ViewShellBase.
    ViewShellBase* pBase = NULL;
    Reference<lang::XUnoTunnel> xTunnel (xFrame->getController(), UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != NULL)
            pBase = pController->GetViewShellBase();
    }
    if (pBase == NULL)
        throw RuntimeException(A2S("can not get ViewShellBase for frame"), NULL);

    // Create a framework view.
    ::Window* pControl = NULL;

#define EndsWith(s,t) s.endsWithAsciiL(t,strlen(t))
    if (EndsWith(rsUIElementResourceURL, gsResourceNameCustomAnimations))
        pControl = new CustomAnimationPanel(pParentWindow, *pBase);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameLayouts))
        pControl = new LayoutMenu(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameAllMasterPages))
        pControl = AllMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameRecentMasterPages))
        pControl = RecentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameUsedMasterPages))
        pControl = CurrentMasterPagesSelector::Create(pParentWindow, *pBase, xSidebar);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameSlideTransitions))
        pControl = new SlideTransitionPanel(pParentWindow, *pBase);
    else if (EndsWith(rsUIElementResourceURL, gsResourceNameTableDesign))
        pControl = new TableDesignPanel(pParentWindow, *pBase);
#undef EndsWith

    if (pControl == NULL)
        throw lang::IllegalArgumentException();

    // Create a wrapper around pane and view and return it as
    // XUIElement.
    Reference<ui::XUIElement> xUIElement;
    try
    {
        xUIElement.set(
            sfx2::sidebar::SidebarPanelBase::Create(
                rsUIElementResourceURL,
                xFrame,
                pControl,
                ui::LayoutSize(-1,-1,-1)));
    }
    catch(Exception& rException)
    {
        // Creation of XUIElement failed. mxUIElement remains empty.
    }

    Reference<lang::XComponent> xComponent (xUIElement, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(this);

    return xUIElement;
}




void SAL_CALL SidebarFactory::disposing (const ::css::lang::EventObject& rEvent)
    throw(cssu::RuntimeException)
{
    /*
    if (mpImplementation
        && rEvent.Source == mpImplementation->mxUIElement)
    {
        mpImplementation->mxUIElement.clear();
    }
    */
}




} } // end of namespace sd::sidebar
