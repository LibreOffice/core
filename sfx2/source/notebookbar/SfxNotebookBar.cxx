/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <unotools/viewoptions.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <vcl/tabctrl.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <unotools/confignode.hxx>
#include <comphelper/types.hxx>

using namespace sfx2;
using namespace css::uno;
using namespace css::ui;
using namespace css;

#define MENUBAR_STR "private:resource/menubar/menubar"

bool SfxNotebookBar::m_bLock = false;
bool SfxNotebookBar::m_bHide = false;

static Reference<frame::XLayoutManager> lcl_getLayoutManager( const Reference<frame::XFrame>& xFrame )
{
    css::uno::Reference<css::frame::XLayoutManager> xLayoutManager;

    if (xFrame.is())
    {
        Reference<css::beans::XPropertySet> xPropSet(xFrame, UNO_QUERY);

        if (xPropSet.is())
        {
            Any aValue = xPropSet->getPropertyValue("LayoutManager");
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
        case vcl::EnumContext::Application::WriterForm:
            return OUString( "Writer" );
            break;
        case vcl::EnumContext::Application::Calc:
            return OUString( "Calc" );
            break;
        case vcl::EnumContext::Application::Impress:
            return OUString( "Impress" );
            break;
        case vcl::EnumContext::Application::Draw:
            return OUString( "Draw" );
            break;
        default:
            return OUString();
            break;
    }
}

static void lcl_setNotebookbarFileName( vcl::EnumContext::Application eApp, const OUString& sFileName )
{
    std::shared_ptr<comphelper::ConfigurationChanges> aBatch(
                comphelper::ConfigurationChanges::create( ::comphelper::getProcessComponentContext() ) );
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

static OUString lcl_getNotebookbarFileName( vcl::EnumContext::Application eApp )
{
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            return officecfg::Office::UI::ToolbarMode::ActiveWriter::get();
            break;
        case vcl::EnumContext::Application::Calc:
            return officecfg::Office::UI::ToolbarMode::ActiveCalc::get();
            break;
        case vcl::EnumContext::Application::Impress:
            return officecfg::Office::UI::ToolbarMode::ActiveImpress::get();
            break;
        case vcl::EnumContext::Application::Draw:
            return officecfg::Office::UI::ToolbarMode::ActiveDraw::get();
            break;

        default:
            break;
    }
    return OUString();
}

static utl::OConfigurationTreeRoot lcl_getCurrentImplConfigRoot()
{
    return utl::OConfigurationTreeRoot(::comphelper::getProcessComponentContext(),
                                       "org.openoffice.Office.UI.ToolbarMode/",
                                       true);
}

static const utl::OConfigurationNode lcl_getCurrentImplConfigNode(const Reference<css::frame::XFrame>& xFrame,
                                                                  utl::OConfigurationTreeRoot const & rNotebookbarNode )
{
    if (!rNotebookbarNode.isValid())
        return utl::OConfigurationNode();

    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );

    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( xModuleManager->identify( xFrame ) );
    OUString aActive = lcl_getNotebookbarFileName( eApp );

    const utl::OConfigurationNode aImplsNode = rNotebookbarNode.openNode("Applications/" + lcl_getAppName( eApp) + "/Modes");
    const Sequence<OUString> aModeNodeNames( aImplsNode.getNodeNames() );
    const sal_Int32 nCount( aModeNodeNames.getLength() );

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aImplNode( aImplsNode.openNode( aModeNodeNames[nReadIndex] ) );
        if ( !aImplNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aImplNode.getNodeValue( "CommandArg" ) );

        if ( aCommandArg == aActive )
        {
            return aImplNode;
        }
    }

    return utl::OConfigurationNode();
}

void SfxNotebookBar::CloseMethod(SfxBindings& rBindings)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    CloseMethod(rFrame.GetSystemWindow());
}

void SfxNotebookBar::CloseMethod(SystemWindow* pSysWindow)
{
    if (pSysWindow)
    {
        RemoveListeners(pSysWindow);
        if(pSysWindow->GetNotebookBar())
            pSysWindow->CloseNotebookBar();
        SfxNotebookBar::ShowMenubar(true);
    }
}

void SfxNotebookBar::LockNotebookBar()
{
    m_bHide = true;
}

void SfxNotebookBar::UnlockNotebookBar()
{
    m_bHide = false;
}

bool SfxNotebookBar::IsActive()
{
    if (m_bHide)
        return false;

    vcl::EnumContext::Application eApp = vcl::EnumContext::Application::Any;

    if (SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (!xFrame.is())
            return false;

        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
        eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
    }
    else
        return false;

    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    aPath.append( lcl_getAppName( eApp ) );

    const utl::OConfigurationTreeRoot aAppNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aAppNode.isValid() )
        return false;

    OUString aActive = comphelper::getString( aAppNode.getNodeValue( "Active" ) );

    const utl::OConfigurationNode aModesNode = aAppNode.openNode("Modes");
    const Sequence<OUString> aModeNodeNames( aModesNode.getNodeNames() );
    const sal_Int32 nCount( aModeNodeNames.getLength() );

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aModeNode( aModesNode.openNode( aModeNodeNames[nReadIndex] ) );
        if ( !aModeNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aModeNode.getNodeValue( "CommandArg" ) );

        if ( aCommandArg == aActive )
        {
            return comphelper::getBOOL( aModeNode.getNodeValue( "HasNotebookbar" ) );
        }
    }
    return false;
}

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings, const OUString& rUIName)
{
    // Save active UI file name
    if ( !rUIName.isEmpty() && SfxViewFrame::Current() )
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (xFrame.is())
        {
            const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
            vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(xFrame));
            lcl_setNotebookbarFileName( eApp, rUIName );
        }
    }

    // trigger the StateMethod
    rBindings.Invalidate(SID_NOTEBOOKBAR);
    rBindings.Update();
}

bool SfxNotebookBar::StateMethod(SfxBindings& rBindings, const OUString& rUIFile)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();
    return StateMethod(rFrame.GetSystemWindow(), rFrame.GetFrameInterface(), rUIFile);
}

bool SfxNotebookBar::StateMethod(SystemWindow* pSysWindow,
                                 const Reference<css::frame::XFrame> & xFrame,
                                 const OUString& rUIFile)
{
    if (!pSysWindow)
    {
        if (SfxViewFrame::Current() && SfxViewFrame::Current()->GetWindow().GetSystemWindow())
            pSysWindow = SfxViewFrame::Current()->GetWindow().GetSystemWindow();
        else
            return false;
    }

    if (IsActive())
    {
        css::uno::Reference<css::uno::XComponentContext> xContext = comphelper::getProcessComponentContext();
        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
        OUString aModuleName = xModuleManager->identify( xFrame );
        vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum( aModuleName );
        OUString sFile = lcl_getNotebookbarFileName( eApp );
        OUString sNewFile = rUIFile + sFile;
        OUString sCurrentFile;
        VclPtr<NotebookBar> pNotebookBar = pSysWindow->GetNotebookBar();
        if ( pNotebookBar )
            sCurrentFile = OStringToOUString( pNotebookBar->getUIFile(), RTL_TEXTENCODING_ASCII_US );

        bool bChangedFile = true;
        if ( sCurrentFile.getLength() && sNewFile.getLength() )
        {
            // delete "/"
            sCurrentFile = sCurrentFile.copy( 0, sCurrentFile.getLength() - 1 );
            // delete ".ui"
            sNewFile = sNewFile.copy( 0, sNewFile.getLength() - 3 );

            bChangedFile = sNewFile != sCurrentFile;
        }

        if ( ( !sFile.isEmpty() && bChangedFile ) || !pNotebookBar || !pNotebookBar->IsVisible() )
        {
            RemoveListeners(pSysWindow);

            OUStringBuffer aBuf(rUIFile);
            aBuf.append( sFile );

            // setup if necessary
            pSysWindow->SetNotebookBar(aBuf.makeStringAndClear(), xFrame);
            pNotebookBar = pSysWindow->GetNotebookBar();
            pNotebookBar->Show();
            pNotebookBar->GetParent()->Resize();

            utl::OConfigurationTreeRoot aRoot(lcl_getCurrentImplConfigRoot());
            const utl::OConfigurationNode aModeNode(lcl_getCurrentImplConfigNode(xFrame, aRoot));
            SfxNotebookBar::ShowMenubar( comphelper::getBOOL( aModeNode.getNodeValue( "HasMenubar" ) ) );

            SfxViewFrame* pView = SfxViewFrame::Current();

            if(pView)
            {
                Reference<XContextChangeEventMultiplexer> xMultiplexer
                            = ContextChangeEventMultiplexer::get( xContext );

                if(xFrame.is())
                {
                    xMultiplexer->addContextChangeEventListener(
                                        pNotebookBar->getContextChangeEventListener(),
                                        xFrame->getController());
                }
            }
        }

        return true;
    }
    else if (auto pNotebookBar = pSysWindow->GetNotebookBar())
    {
        pNotebookBar->Hide();
        pNotebookBar->GetParent()->Resize();
        SfxNotebookBar::ShowMenubar(true);
    }

    return false;
}

void SfxNotebookBar::RemoveListeners(SystemWindow const * pSysWindow)
{
    Reference<XContextChangeEventMultiplexer> xMultiplexer
                        = ContextChangeEventMultiplexer::get(
                                ::comphelper::getProcessComponentContext());

    if (pSysWindow->GetNotebookBar())
    {
        xMultiplexer->removeAllContextChangeEventListeners(
                           pSysWindow->GetNotebookBar()->getContextChangeEventListener());
    }
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (!m_bLock)
    {
        m_bLock = true;

        Reference<frame::XFrame> xFrame;
        vcl::EnumContext::Application eCurrentApp = vcl::EnumContext::Application::NONE;
        uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        const Reference<frame::XModuleManager> xModuleManager = frame::ModuleManager::create( xContext );

        if ( SfxViewFrame::Current() )
        {
            xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
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
                    const Reference<frame::XLayoutManager>& xLayoutManager =
                                                            lcl_getLayoutManager( xFrame );

                    if (xLayoutManager.is())
                    {
                        xLayoutManager->lock();

                        if (xLayoutManager->getElement(MENUBAR_STR).is())
                        {
                            if (xLayoutManager->isElementVisible(MENUBAR_STR) && !bShow)
                                xLayoutManager->hideElement(MENUBAR_STR);
                            else if(!xLayoutManager->isElementVisible(MENUBAR_STR) && bShow)
                                xLayoutManager->showElement(MENUBAR_STR);
                        }

                        xLayoutManager->unlock();
                    }
                }
            }

            pViewFrame = SfxViewFrame::GetNext( *pViewFrame );
        }
        m_bLock = false;
    }
}

void SfxNotebookBar::ToggleMenubar()
{
    if (SfxViewFrame::Current())
    {
        const Reference<frame::XFrame>& xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
        if (xFrame.is())
        {
            const Reference<frame::XLayoutManager>& xLayoutManager =
                                                    lcl_getLayoutManager(xFrame);

            bool bShow = true;
            if (xLayoutManager.is() && xLayoutManager->getElement(MENUBAR_STR).is())
            {
                if (xLayoutManager->isElementVisible(MENUBAR_STR))
                {
                    SfxNotebookBar::ShowMenubar(false);
                    bShow = false;
                }
                else
                    SfxNotebookBar::ShowMenubar(true);
            }

            // Save menubar settings
            if (IsActive())
            {
                utl::OConfigurationTreeRoot aRoot(lcl_getCurrentImplConfigRoot());
                utl::OConfigurationNode aModeNode(lcl_getCurrentImplConfigNode(xFrame, aRoot));
                aModeNode.setNodeValue( "HasMenubar", toAny<bool>( bShow ) );
                aRoot.commit();
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
