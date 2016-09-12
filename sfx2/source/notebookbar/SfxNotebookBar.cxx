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
#include <vcl/notebookbar.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include "NotebookBarPopupMenu.hxx"
#include <officecfg/Office/UI/Notebookbar.hxx>
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
Reference<css::frame::XLayoutManager> SfxNotebookBar::m_xLayoutManager;
css::uno::Reference<css::frame::XFrame> SfxNotebookBar::m_xFrame;

static OUString lcl_getAppName( vcl::EnumContext::Application eApp )
{
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Application_Writer:
            return OUString( "Writer" );
            break;
        case vcl::EnumContext::Application::Application_Calc:
            return OUString( "Calc" );
            break;
        case vcl::EnumContext::Application::Application_Impress:
            return OUString( "Impress" );
            break;
        case vcl::EnumContext::Application::Application_Draw:
            return OUString( "Draw" );
            break;
        default:
            return OUString( "" );
            break;
    }
}

static const utl::OConfigurationNode lcl_getCurrentImplConfigNode()
{
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );

    OUStringBuffer aPath("org.openoffice.Office.UI.Notebookbar/");

    const utl::OConfigurationTreeRoot aNotebookbarNode(
                                        ::comphelper::getProcessComponentContext(),
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aNotebookbarNode.isValid() )
        return utl::OConfigurationNode();

    OUString aActive = comphelper::getString( aNotebookbarNode.getNodeValue( "Active" ) );

    const utl::OConfigurationNode aImplsNode = aNotebookbarNode.openNode("Implementations");
    const Sequence<OUString> aModeNodeNames( aImplsNode.getNodeNames() );
    const sal_Int32 nCount( aModeNodeNames.getLength() );

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aImplNode( aImplsNode.openNode( aModeNodeNames[nReadIndex] ) );
        if ( !aImplNode.isValid() )
            continue;

        OUString aCommandArg = comphelper::getString( aImplNode.getNodeValue( "File" ) );

        if ( aCommandArg.compareTo( aActive ) == 0 )
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
    }
    SfxNotebookBar::ShowMenubar(true);

    m_xLayoutManager.clear();
    m_xFrame.clear();
}

bool SfxNotebookBar::IsActive()
{
    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( ::comphelper::getProcessComponentContext() );
    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(m_xFrame));

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

        if ( aCommandArg.compareTo( aActive ) == 0 )
        {
            return comphelper::getBOOL( aModeNode.getNodeValue( "HasNotebookbar" ) );
        }
    }
    return false;
}

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings, const OUString& rUIName)
{
    // Save active UI file name
    if ( !rUIName.isEmpty() )
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
                comphelper::ConfigurationChanges::create( ::comphelper::getProcessComponentContext() ) );
        officecfg::Office::UI::Notebookbar::Active::set( rUIName, batch );
        batch->commit();
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
    assert(pSysWindow);

    m_xFrame = xFrame;

    if (!m_xLayoutManager.is())
    {
        Reference<css::beans::XPropertySet> xPropSet(xFrame, UNO_QUERY);

        if (xPropSet.is())
        {
            Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= m_xLayoutManager;
        }
    }

    if (IsActive())
    {
        RemoveListeners(pSysWindow);

        OUString sFile = officecfg::Office::UI::Notebookbar::Active::get();
        OUString sNewFile = rUIFile + sFile;
        OUString sCurrentFile;
        if ( pSysWindow->GetNotebookBar() )
            sCurrentFile = OStringToOUString( pSysWindow->GetNotebookBar()->getUIFile(), RTL_TEXTENCODING_ASCII_US );

        bool bChangedFile = true;
        if ( sCurrentFile.getLength() && sNewFile.getLength() )
        {
            // delete "/"
            sCurrentFile = sCurrentFile.copy( 0, sCurrentFile.getLength() - 1 );
            // delete ".ui"
            sNewFile = sNewFile.copy( 0, sNewFile.getLength() - 3 );

            bChangedFile = ( sNewFile.compareTo( sCurrentFile ) != 0 );
        }

        if ( !sFile.isEmpty() && bChangedFile )
        {
            OUStringBuffer aBuf(rUIFile);
            aBuf.append( sFile );

            // setup if necessary
            pSysWindow->SetNotebookBar(aBuf.makeStringAndClear(), xFrame);
            pSysWindow->GetNotebookBar()->Show();
            pSysWindow->GetNotebookBar()->SetIconClickHdl(LINK(nullptr, SfxNotebookBar, OpenNotebookbarPopupMenu));

            const utl::OConfigurationNode aModeNode( lcl_getCurrentImplConfigNode() );
            SfxNotebookBar::ShowMenubar( comphelper::getBOOL( aModeNode.getNodeValue( "HasMenubar" ) ) );

            SfxViewFrame* pView = SfxViewFrame::Current();

            if(pView)
            {
                Reference<XContextChangeEventMultiplexer> xMultiplexer
                            = ContextChangeEventMultiplexer::get(
                                    ::comphelper::getProcessComponentContext());

                if(xFrame.is() && xMultiplexer.is())
                {
                    xMultiplexer->addContextChangeEventListener(
                                        pSysWindow->GetNotebookBar()->getContextChangeEventListener(),
                                        xFrame->getController());
                }
            }
        }

        return true;
    }
    else if (auto pNotebookBar = pSysWindow->GetNotebookBar())
    {
        pNotebookBar->Hide();
        SfxNotebookBar::ShowMenubar(true);
    }

    return false;
}

void SfxNotebookBar::RemoveListeners(SystemWindow* pSysWindow)
{
    Reference<XContextChangeEventMultiplexer> xMultiplexer
                        = ContextChangeEventMultiplexer::get(
                                ::comphelper::getProcessComponentContext());

    if (pSysWindow->GetNotebookBar() && xMultiplexer.is())
    {
        xMultiplexer->removeAllContextChangeEventListeners(
                           pSysWindow->GetNotebookBar()->getContextChangeEventListener());
    }
}

IMPL_STATIC_LINK_TYPED(SfxNotebookBar, OpenNotebookbarPopupMenu, NotebookBar*, pNotebookbar, void)
{
    if (pNotebookbar)
    {
        ScopedVclPtrInstance<NotebookBarPopupMenu> pMenu;
        pMenu->Execute(pNotebookbar, m_xFrame);
    }
}

void SfxNotebookBar::ShowMenubar(bool bShow)
{
    if (!m_bLock && m_xLayoutManager.is())
    {
        m_bLock = true;
        m_xLayoutManager->lock();

        if (m_xLayoutManager->getElement(MENUBAR_STR).is())
        {
            if (m_xLayoutManager->isElementVisible(MENUBAR_STR) && !bShow)
                m_xLayoutManager->hideElement(MENUBAR_STR);
            else if(!m_xLayoutManager->isElementVisible(MENUBAR_STR) && bShow)
                m_xLayoutManager->showElement(MENUBAR_STR);
        }

        m_xLayoutManager->unlock();
        m_bLock = false;
    }
}

void SfxNotebookBar::ToggleMenubar()
{
    if (m_xLayoutManager.is() && m_xLayoutManager->getElement(MENUBAR_STR).is())
    {
        if (m_xLayoutManager->isElementVisible(MENUBAR_STR))
            SfxNotebookBar::ShowMenubar(false);
        else
            SfxNotebookBar::ShowMenubar(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
