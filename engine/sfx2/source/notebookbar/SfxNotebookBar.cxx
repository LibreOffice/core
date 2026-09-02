/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <vcl/syswin.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/weldutils.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/kit.hxx>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/confignode.hxx>
#include <comphelper/types.hxx>
#include <vcl/weldutils.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <memory>
#include <vector>
#include <unordered_map>
#include <vcl/WeldedTabbedNotebookbar.hxx>

using namespace sfx2;
using namespace css::uno;
using namespace cpo::uno;
using namespace css::ui;
using namespace css;

constexpr OUString MENUBAR_STR = u"private:resource/menubar/menubar"_ustr;

bool SfxNotebookBar::m_bLock = false;

namespace
{

/** View specific notebook bar data */
struct NotebookBarViewData
{
    std::unique_ptr<WeldedTabbedNotebookbar> m_pWeldedWrapper;
    VclPtr<NotebookBar> m_pNotebookBar;
    std::unique_ptr<ToolbarUnoDispatcher> m_pToolbarUnoDispatcher;
    std::vector<std::unique_ptr<ToolbarUnoDispatcher>> m_aExtraToolbarUnoDispatchers;
    std::vector<css::uno::Reference<css::lang::XComponent>> m_aExtraPanelControllers;

    ~NotebookBarViewData()
    {
        releaseExtraPanels();

        if (m_pNotebookBar)
            m_pNotebookBar.disposeAndClear();
    }

    void releaseExtraPanels()
    {
        for (const auto& rController : m_aExtraPanelControllers)
            rController->dispose();
        m_aExtraPanelControllers.clear();
        m_aExtraToolbarUnoDispatchers.clear();
    }
};

/** Creates the UNO component which drives the non-toolbar widgets of a welded
    notebookbar panel, handing it the builder of that panel's .ui. */
css::uno::Reference<css::lang::XComponent>
CreateExtraPanelController(const OUString& rServiceName,
                           const css::uno::Reference<css::frame::XFrame>& rFrame,
                           SfxBindings& rBindings, weld::Toolbar& rToolbar,
                           weld::Builder& rBuilder)
{
    css::uno::Reference<css::awt::XWindow> xWidget(
        new weld::TransportAsXWindow(&rToolbar, &rBuilder));

    css::beans::PropertyValue aFrame;
    aFrame.Name = u"Frame"_ustr;
    aFrame.Value <<= rFrame;
    css::beans::PropertyValue aParent;
    aParent.Name = u"ParentWindow"_ustr;
    aParent.Value <<= xWidget;
    css::beans::PropertyValue aBindings;
    aBindings.Name = u"SfxBindings"_ustr;
    aBindings.Value <<= reinterpret_cast<sal_uInt64>(&rBindings);

    const cpo::uno::Sequence<cpo::uno::Any> aArguments{ cpo::uno::Any(aFrame),
                                                        cpo::uno::Any(aParent),
                                                        cpo::uno::Any(aBindings) };

    try
    {
        const css::uno::Reference<cpo::uno::XComponentContext>& xContext
            = comphelper::getProcessComponentContext();
        return css::uno::Reference<css::lang::XComponent>(
            xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                rServiceName, aArguments, xContext),
            css::uno::UNO_QUERY);
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sfx.appl", "cannot create " << rServiceName);
    }

    return nullptr;
}

/** Notebookbar instance manager is a singleton that is used for track the
 *  per-view instances of view specific data contained in NotebookBarViewData
 *  class.
 **/
class NotebookBarViewManager final
{
private:
    // map contains a view data instance for a view (SfxViewShell pointer)
    std::unordered_map<const SfxViewShell*, std::unique_ptr<NotebookBarViewData>> m_pViewDataList;

    // private constructor to prevent any other instantiation outside of get() method
    NotebookBarViewManager() = default;

    // prevent class copying
    NotebookBarViewManager(const NotebookBarViewManager&) = delete;
    NotebookBarViewManager& operator=(const NotebookBarViewManager&) = delete;

public:
    // Singleton get method - creates an instance on first get() call
    static NotebookBarViewManager& get()
    {
        static NotebookBarViewManager gNotebookBarManager;
        return gNotebookBarManager;
    }

    NotebookBarViewData& getViewData(const SfxViewShell* pViewShell)
    {
        auto aFound = m_pViewDataList.find(pViewShell);
        if (aFound != m_pViewDataList.end()) // found
            return *aFound->second;

        // Create new view data instance
        NotebookBarViewData* pViewData = new NotebookBarViewData;
        m_pViewDataList.emplace(pViewShell, std::unique_ptr<NotebookBarViewData>(pViewData));
        return *pViewData;
    }

    void removeViewData(const SfxViewShell* pViewShell)
    {
        m_pViewDataList.erase(pViewShell);
    }
};

} // end anonymous namespace

static Reference<frame::XLayoutManager> lcl_getLayoutManager( const Reference<frame::XFrame>& xFrame )
{
    css::uno::Reference<css::frame::XLayoutManager> xLayoutManager;

    if (xFrame.is())
    {
        Reference<css::beans::XPropertySet> xPropSet(xFrame, UNO_QUERY);

        if (xPropSet.is())
        {
            Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
            aValue >>= xLayoutManager;
        }
    }

    return xLayoutManager;
}

static OUString lcl_getAppName( vcl::EnumContext::Application eApp )
{
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            return u"Writer"_ustr;
        case vcl::EnumContext::Application::Calc:
            return u"Calc"_ustr;
        case vcl::EnumContext::Application::Impress:
            return u"Impress"_ustr;
        case vcl::EnumContext::Application::Draw:
            return u"Draw"_ustr;
        case vcl::EnumContext::Application::Formula:
            return u"Formula"_ustr;
        default:
            return OUString();
    }
}

static void lcl_setNotebookbarFileName( vcl::EnumContext::Application eApp, const OUString& sFileName )
{
    std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
                comphelper::ConfigurationChanges::create() );
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            officecfg::Office::UI::ToolbarMode::ActiveWriter::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Calc:
            officecfg::Office::UI::ToolbarMode::ActiveCalc::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Impress:
            officecfg::Office::UI::ToolbarMode::ActiveImpress::set( sFileName, aBatch );
            break;
        case vcl::EnumContext::Application::Draw:
            officecfg::Office::UI::ToolbarMode::ActiveDraw::set( sFileName, aBatch );
            break;
        default:
            break;
    }
    aBatch->commit();
}

void SfxNotebookBar::RemoveCurrentKitWrapper()
{
    const SfxViewShell* pViewShell = SfxViewShell::Current();
    auto& rViewData = NotebookBarViewManager::get().getViewData(pViewShell);

    if (rViewData.m_pNotebookBar)
    {
        // Calls STATIC_LINK SfxNotebookBar -> VclDisposeHdl
        // which clears the whole InstanceManager
        rViewData.m_pNotebookBar.disposeAndClear();
    }
}

void SfxNotebookBar::CloseMethod(SfxBindings& rBindings)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    CloseMethod(rFrame.GetSystemWindow());
}

void SfxNotebookBar::CloseMethod(SystemWindow* /*pSysWindow*/)
{
    if (comphelper::COKit::isActive())
        RemoveCurrentKitWrapper();
}

bool SfxNotebookBar::IsActive(bool bConsiderSingleToolbar)
{
    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;

    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
        if (!xFrame.is())
            return false;

        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
        try
        {
            eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
        }
        catch (cpo::uno::Exception& e)
        {
            SAL_WARN("sfx.appl", "SfxNotebookBar::IsActive(): " + e.Message);
            return false;
        }
    }
    else
        return false;

    OUString appName(lcl_getAppName( eApp ));

    if (appName.isEmpty())
        return false;


    OUString aPath = "org.openoffice.Office.UI.ToolbarMode/Applications/" + appName;

    const utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath,
                                        false);
    if ( !aAppNode.isValid() )
        return false;

    OUString aActive = comphelper::getString( aAppNode.getNodeValue( u"Active"_ustr ) );

    if (bConsiderSingleToolbar && aActive == "Single")
        return true;

    if (comphelper::COKit::isActive() && aActive == "notebookbar_online.ui")
        return true;

    const utl::OConfigurationNode aModesNode = aAppNode.openNode(u"Modes"_ustr);
    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );

    for ( const auto& rModeNodeName : aModeNodeNames )
    {
        const utl::OConfigurationNode aModeNode( aModesNode.openNode( rModeNodeName ) );
        if ( !aModeNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( u"CommandArg"_ustr ) );

        if ( aCommandArg == aActive )
        {
            return comphelper::getBOOL( aModeNode.getNodeValue( u"HasNotebookbar"_ustr ) );
        }
    }
    return false;
}

void SfxNotebookBar::ResetActiveToolbarModeToDefault(vcl::EnumContext::Application eApp)
{
    const OUString appName( lcl_getAppName( eApp ) );

    if ( appName.isEmpty() )
        return;

    const OUString aPath = "org.openoffice.Office.UI.ToolbarMode/Applications/" + appName;

    utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath,
                                        true);
    if ( !aAppNode.isValid() )
        return;

    aAppNode.setNodeValue( u"Active"_ustr, Any( u"Default"_ustr ) );
    aAppNode.commit();
}

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings, const OUString& rUIName)
{
    // Save active UI file name
    if (!rUIName.isEmpty())
    {
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        {
            const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
            if (xFrame.is())
            {
                const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
                vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
                lcl_setNotebookbarFileName( eApp, rUIName );
            }
        }
    }

    StateMethod(rBindings);
}

bool SfxNotebookBar::StateMethod(SfxBindings& rBindings)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    return StateMethod(rFrame.GetSystemWindow(), rFrame.GetFrameInterface());
}

bool SfxNotebookBar::StateMethod(SystemWindow* pSysWindow,
                                 const Reference<css::frame::XFrame>& xFrame)
{
    if (!comphelper::COKit::isActive())
        return false;

    if (!pSysWindow)
    {
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (pViewFrm && pViewFrm->GetWindow().GetSystemWindow())
            pSysWindow = pViewFrm->GetWindow().GetSystemWindow();
        else
            return false;
    }

    const SfxViewShell* pViewShell = SfxViewShell::Current();
    auto& rViewData = NotebookBarViewManager::get().getViewData(pViewShell);
    bool hasWeldedWrapper = bool(rViewData.m_pWeldedWrapper);

    if (!IsActive())
    {
        // don't do anything to not close notebookbar of other session
        return hasWeldedWrapper;
    }

    if (hasWeldedWrapper)
        return true;

    if (!pViewShell)
        return false;

    const css::uno::Reference<cpo::uno::XComponentContext>& xContext = comphelper::getProcessComponentContext();
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
    OUString aModuleName = xModuleManager->identify( xFrame );
    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( aModuleName );

    OUString sUIDir;
    switch (eApp)
    {
        case vcl::EnumContext::Application::Writer:
            sUIDir = u"modules/swriter/ui/"_ustr;
            break;
        case vcl::EnumContext::Application::Calc:
            sUIDir = u"modules/scalc/ui/"_ustr;
            break;
        case vcl::EnumContext::Application::Impress:
            sUIDir = u"modules/simpress/ui/"_ustr;
            break;
        case vcl::EnumContext::Application::Draw:
            sUIDir = u"modules/sdraw/ui/"_ustr;
            break;
        default:
            return false;
    }

    // Notebookbar was loaded too early what caused:
    //   * in COKit: Paste Special feature was incorrectly initialized
    // Skip first request so Notebookbar will be initialized after document was loaded
    static std::map<const void*, bool> bSkippedFirstInit;
    if (eApp == vcl::EnumContext::Application::Writer
        && bSkippedFirstInit.find(pViewShell) == bSkippedFirstInit.end())
    {
        bSkippedFirstInit[pViewShell] = true;
        ResetActiveToolbarModeToDefault(eApp);
        return false;
    }

    // update the current COKit language and locale for the dialog tunneling
    comphelper::COKit::setLanguageTag(pViewShell->GetKitLanguageTag());
    comphelper::COKit::setLocale(pViewShell->GetKitLocale());

    OUString aBuf = sUIDir + "notebookbar_online.ui";
    VclPtr<NotebookBar> pNotebookBar = VclPtr<NotebookBar>::Create(pSysWindow, aBuf);
    rViewData.m_pNotebookBar = pNotebookBar;

    sal_uInt64 nWindowId = reinterpret_cast<sal_uInt64>(pViewShell);

    const std::vector<WeldedTabbedNotebookbar::ExtraPanel> aExtraPanels{
        { u"svx/ui/notebookbarshapeline.ui"_ustr, u"LineWeldedToolbar"_ustr,
          u"com.sun.star.svx.NotebookbarLineController"_ustr },
        { u"svx/ui/notebookbarpictureline.ui"_ustr, u"PictureLineWeldedToolbar"_ustr,
          OUString() },
    };

    rViewData.m_pWeldedWrapper.reset(
            new WeldedTabbedNotebookbar(pNotebookBar->GetMainContainer(),
                                        pNotebookBar->GetUIFilePath(),
                                        xFrame, nWindowId, aExtraPanels));
    pNotebookBar->SetDisposeCallback(LINK(nullptr, SfxNotebookBar, VclDisposeHdl), pViewShell);

    rViewData.m_pToolbarUnoDispatcher.reset(
        new ToolbarUnoDispatcher(rViewData.m_pWeldedWrapper->getWeldedToolbar(),
                                 rViewData.m_pWeldedWrapper->getBuilder(), xFrame));

    // Wire a UNO dispatcher for each present welded sub-toolbar
    for (auto& rExtra : rViewData.m_pWeldedWrapper->getExtraPanels())
    {
        if (!rExtra.m_xToolbar || !rExtra.m_xBuilder)
            continue;

        rViewData.m_aExtraToolbarUnoDispatchers.push_back(
            std::make_unique<ToolbarUnoDispatcher>(*rExtra.m_xToolbar,
                                                   *rExtra.m_xBuilder, xFrame));

        if (rExtra.m_aControllerService.isEmpty())
            continue;

        if (css::uno::Reference<css::lang::XComponent> xController
            = CreateExtraPanelController(rExtra.m_aControllerService, xFrame,
                                         pViewShell->GetViewFrame().GetBindings(),
                                         *rExtra.m_xToolbar, *rExtra.m_xBuilder))
            rViewData.m_aExtraPanelControllers.push_back(std::move(xController));
    }

    return true;
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (m_bLock)
        return;

    m_bLock = true;

    Reference<frame::XFrame> xFrame;
    vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
    const uno::Reference< cpo::uno::XComponentContext >& xContext = comphelper::getProcessComponentContext();
    const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create( xContext );

    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        xFrame = pViewFrm->GetFrame().GetFrameInterface();
        eCurrentApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );
    }

    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while( pViewFrame )
    {
        xFrame = pViewFrame->GetFrame().GetFrameInterface();
        if ( xFrame.is() )
        {
            vcl::EnumContext::Application eApp =
                    vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );

            if ( eApp == eCurrentApp )
            {
                const Reference<frame::XLayoutManager> xLayoutManager =
                                                        lcl_getLayoutManager( xFrame );

                if (xLayoutManager.is())
                {
                    if (xLayoutManager->getElement(MENUBAR_STR).is())
                    {
                        if (xLayoutManager->isElementVisible(MENUBAR_STR) && !bShow)
                            xLayoutManager->hideElement(MENUBAR_STR);
                        else if(!xLayoutManager->isElementVisible(MENUBAR_STR) && bShow)
                            xLayoutManager->showElement(MENUBAR_STR);
                    }
                }
            }
        }

        pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
    }
    m_bLock = false;
}

void SfxNotebookBar::ToggleMenubar()
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    const Reference<frame::XFrame>& xFrame = pViewFrm->GetFrame().GetFrameInterface();
    if (!xFrame.is())
        return;

    const Reference<frame::XLayoutManager> xLayoutManager =
                                            lcl_getLayoutManager(xFrame);

    if (xLayoutManager.is() && xLayoutManager->getElement(MENUBAR_STR).is())
    {
        if (xLayoutManager->isElementVisible(MENUBAR_STR))
            SfxNotebookBar::ShowMenubar(false);
        else
            SfxNotebookBar::ShowMenubar(true);
    }
}

IMPL_STATIC_LINK(SfxNotebookBar, VclDisposeHdl, const SfxViewShell*, pViewShell, void)
{
    NotebookBarViewManager::get().removeViewData(pViewShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
