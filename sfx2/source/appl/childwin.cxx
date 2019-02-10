/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <vcl/toolbox.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>
#include <sal/log.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/dispatch.hxx>
#include <workwin.hxx>
#include <childwinimpl.hxx>

static const sal_uInt16 nVersion = 2;

SfxChildWinFactory::SfxChildWinFactory( SfxChildWinCtor pTheCtor, sal_uInt16 nID,
        sal_uInt16 n )
    : pCtor(pTheCtor)
    , nId( nID )
    , nPos(n)
{}

SfxChildWinFactory::~SfxChildWinFactory()
{
}

struct SfxChildWindow_Impl
{
    css::uno::Reference< css::frame::XFrame >             xFrame;
    css::uno::Reference< css::lang::XEventListener >      xListener;
    SfxChildWinFactory* pFact;
    bool                bHideNotDelete;
    bool                bVisible;
    bool                bWantsFocus;
    SfxModule*          pContextModule;
    SfxWorkWindow*      pWorkWin;
};


class DisposeListener : public ::cppu::WeakImplHelper< css::lang::XEventListener >
{
    public:
        DisposeListener( SfxChildWindow*      pOwner ,
                         SfxChildWindow_Impl* pData  )
            :   m_pOwner( pOwner )
            ,   m_pData ( pData  )
        {}

        virtual void SAL_CALL disposing( const css::lang::EventObject& aSource ) override
        {
            css::uno::Reference< css::lang::XEventListener > xSelfHold( this );

            css::uno::Reference< css::lang::XComponent > xComp( aSource.Source, css::uno::UNO_QUERY );
            if( xComp.is() )
                xComp->removeEventListener( this );

            if( !m_pOwner || !m_pData )
                return;

            m_pData->xListener.clear();

            if ( m_pData->pWorkWin )
            {
                // m_pOwner and m_pData will be killed
                m_pData->xFrame.clear();
                m_pData->pWorkWin->GetBindings().Execute( m_pOwner->GetType() );
            }
            else
            {
                delete m_pOwner;
            }

            m_pOwner = nullptr;
            m_pData  = nullptr;
        }

    private:
        SfxChildWindow*      m_pOwner;
        SfxChildWindow_Impl* m_pData ;
};


bool GetPosSizeFromString( const OUString& rStr, Point& rPos, Size& rSize )
{
    if ( comphelper::string::getTokenCount(rStr, '/') != 4 )
        return false;

    sal_Int32 nIdx = 0;
    rPos.setX( rStr.getToken(0, '/', nIdx).toInt32() );
    rPos.setY( rStr.getToken(0, '/', nIdx).toInt32() );
    rSize.setWidth( rStr.getToken(0, '/', nIdx).toInt32() );
    rSize.setHeight( rStr.getToken(0, '/', nIdx).toInt32() );

    // negative sizes are invalid
    return !(rSize.Width() < 0 || rSize.Height() < 0);
}

bool GetSplitSizeFromString( const OUString& rStr, Size& rSize )
{
    sal_Int32 nIndex = rStr.indexOf( ',' );
    if ( nIndex != -1 )
    {
        OUString aStr = rStr.copy( nIndex+1 );

        sal_Int32 nCount = comphelper::string::getTokenCount(aStr, ';');
        if ( nCount != 2 )
            return false;

        sal_Int32 nIdx{ 0 };
        rSize.setWidth( aStr.getToken(0, ';', nIdx ).toInt32() );
        rSize.setHeight( aStr.getToken(0, ';', nIdx ).toInt32() );

        // negative sizes are invalid
        return !(rSize.Width() < 0 || rSize.Height() < 0);
    }

    return false;
}


SfxChildWindow::SfxChildWindow(vcl::Window *pParentWindow, sal_uInt16 nId)
    : pParent(pParentWindow)
    , nType(nId)
    , eChildAlignment(SfxChildAlignment::NOALIGNMENT)
    , pImpl(new SfxChildWindow_Impl)
{
    pImpl->pFact = nullptr;
    pImpl->bHideNotDelete = false;
    pImpl->bWantsFocus = true;
    pImpl->bVisible = true;
    pImpl->pContextModule = nullptr;
    pImpl->pWorkWin = nullptr;

    pContext = nullptr;
}

void SfxChildWindow::Destroy()
{
    if ( GetFrame().is() )
    {
        ClearWorkwin();
        try
        {
            css::uno::Reference < css::util::XCloseable > xClose( GetFrame(), css::uno::UNO_QUERY );
            if ( xClose.is() )
                xClose->close( true );
            else
                GetFrame()->dispose();
        }
        catch (const css::uno::Exception&)
        {
        }
    }
    else
        delete this;
}

void SfxChildWindow::ClearWorkwin()
{
    if (pImpl->pWorkWin)
    {
        if (pImpl->pWorkWin->GetActiveChild_Impl() == pWindow)
            pImpl->pWorkWin->SetActiveChild_Impl(nullptr);
        pImpl->pWorkWin = nullptr;
    }
}

SfxChildWindow::~SfxChildWindow()
{
    pContext.reset();
    ClearWorkwin();
    if (xController)
        xController->DeInit();
    else
        pWindow.disposeAndClear();
}


std::unique_ptr<SfxChildWindow> SfxChildWindow::CreateChildWindow( sal_uInt16 nId,
        vcl::Window *pParent, SfxBindings* pBindings, SfxChildWinInfo const & rInfo)
{
    std::unique_ptr<SfxChildWindow> pChild;
    SfxChildWinFactory* pFact=nullptr;
    SystemWindowFlags nOldMode = Application::GetSystemWindowMode();

    // First search for ChildWindow in SDT; Overlay windows are realized
    // by using ChildWindowContext
    SfxApplication *pApp = SfxGetpApp();
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( size_t nFactory = 0; nFactory < rFactories.size(); ++nFactory )
        {
            pFact = &rFactories[nFactory];
            if ( pFact->nId == nId )
            {
                SfxChildWinInfo& rFactInfo = pFact->aInfo;
                if ( rInfo.bVisible )
                {
                    if ( pBindings )
                        pBindings->ENTERREGISTRATIONS();
                    SfxChildWinInfo aInfo = rFactInfo;
                    Application::SetSystemWindowMode( SystemWindowFlags::NOAUTOMODE );
                    pChild = pFact->pCtor( pParent, nId, pBindings, &aInfo );
                    Application::SetSystemWindowMode( nOldMode );
                    if ( pBindings )
                        pBindings->LEAVEREGISTRATIONS();
                }

                break;
            }
        }
    }

    SfxDispatcher *pDisp = pBindings ? pBindings->GetDispatcher_Impl() : nullptr;
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) : nullptr;
    if (!pChild && pMod)
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( size_t nFactory = 0; nFactory < rFactories.size(); ++nFactory )
            {
                pFact = &rFactories[nFactory];
                if ( pFact->nId == nId )
                {
                    SfxChildWinInfo& rFactInfo = pFact->aInfo;
                    if ( rInfo.bVisible )
                    {
                        if ( pBindings )
                            pBindings->ENTERREGISTRATIONS();
                        SfxChildWinInfo aInfo = rFactInfo;
                        Application::SetSystemWindowMode( SystemWindowFlags::NOAUTOMODE );
                        pChild = pFact->pCtor( pParent, nId, pBindings, &aInfo );
                        Application::SetSystemWindowMode( nOldMode );
                        if ( pBindings )
                            pBindings->LEAVEREGISTRATIONS();
                    }

                    break;
                }
            }
        }
    }

    if ( pChild )
        pChild->SetFactory_Impl( pFact );

    DBG_ASSERT(pFact && (pChild || !rInfo.bVisible), "ChildWindow-Typ not registered!");

    if (pChild && (!pChild->pWindow && !pChild->xController))
    {
        pChild.reset();
        SAL_INFO("sfx.appl", "ChildWindow has no Window!");
    }

    return pChild;
}


void SfxChildWindow::SaveStatus(const SfxChildWinInfo& rInfo)
{
    sal_uInt16 nID = GetType();

    OUString aInfoVisible = rInfo.bVisible ? OUString("V") : OUString("H");

    OUString aWinData = "V"
                      + OUString::number(static_cast<sal_Int32>(nVersion))
                      + ","
                      + aInfoVisible
                      + ","
                      + OUString::number(static_cast<sal_Int32>(rInfo.nFlags));

    if ( !rInfo.aExtraString.isEmpty() )
        aWinData += "," + rInfo.aExtraString;

    OUString sName(OUString::number(nID));
    //Try and save window state per-module, e.g. sidebar on in one application
    //but off in another
    if (!rInfo.aModule.isEmpty())
        sName = rInfo.aModule + "/" + sName;
    SvtViewOptions aWinOpt(EViewType::Window, sName);
    aWinOpt.SetWindowState(OStringToOUString(rInfo.aWinState, RTL_TEXTENCODING_UTF8));

    css::uno::Sequence < css::beans::NamedValue > aSeq
        { { "Data", css::uno::makeAny(aWinData) } };
    aWinOpt.SetUserData( aSeq );

    // ... but save status at runtime!
    pImpl->pFact->aInfo = rInfo;
}

void SfxChildWindow::SetAlignment(SfxChildAlignment eAlign)
{
    eChildAlignment = eAlign;
}

SfxChildWinInfo SfxChildWindow::GetInfo() const
{
    SfxChildWinInfo aInfo(pImpl->pFact->aInfo);
    if (xController)
    {
        weld::Dialog* pDialog = xController->getDialog();
        aInfo.aPos  = pDialog->get_position();
        aInfo.aSize = pDialog->get_size();
        WindowStateMask nMask = WindowStateMask::Pos | WindowStateMask::State;
        if (pDialog->get_resizable())
            nMask |= WindowStateMask::Width | WindowStateMask::Height;
        aInfo.aWinState = pDialog->get_window_state(nMask);
    }
    else if (pWindow)
    {
        aInfo.aPos  = pWindow->GetPosPixel();
        aInfo.aSize = pWindow->GetSizePixel();
        if ( pWindow->IsSystemWindow() )
        {
            WindowStateMask nMask = WindowStateMask::Pos | WindowStateMask::State;
            if ( pWindow->GetStyle() & WB_SIZEABLE )
                nMask |= WindowStateMask::Width | WindowStateMask::Height;
            aInfo.aWinState = static_cast<SystemWindow*>(pWindow.get())->GetWindowState( nMask );
        }
        else if (DockingWindow* pDockingWindow = dynamic_cast<DockingWindow*>(pWindow.get()))
        {
            if (pDockingWindow->GetFloatingWindow())
                aInfo.aWinState = pDockingWindow->GetFloatingWindow()->GetWindowState();
            else if (SfxDockingWindow* pSfxDockingWindow = dynamic_cast<SfxDockingWindow*>(pDockingWindow))
            {
                SfxChildWinInfo aTmpInfo;
                pSfxDockingWindow->FillInfo( aTmpInfo );
                aInfo.aExtraString = aTmpInfo.aExtraString;
            }
        }
    }

    aInfo.bVisible = pImpl->bVisible;
    aInfo.nFlags = SfxChildWindowFlags::NONE;
    return aInfo;
}

sal_uInt16 SfxChildWindow::GetPosition()
{
    return pImpl->pFact->nPos;
}

void SfxChildWindow::InitializeChildWinFactory_Impl(sal_uInt16 nId, SfxChildWinInfo& rInfo)
{
    // load configuration

    std::unique_ptr<SvtViewOptions> xWinOpt;
    // first see if a module specific id exists
    if (rInfo.aModule.getLength())
        xWinOpt.reset(new SvtViewOptions(EViewType::Window, rInfo.aModule + "/" + OUString::number(nId)));

    // if not then try the generic id
    if (!xWinOpt || !xWinOpt->Exists())
        xWinOpt.reset(new SvtViewOptions(EViewType::Window, OUString::number(nId)));

    if (xWinOpt->Exists() && xWinOpt->HasVisible() )
        rInfo.bVisible  = xWinOpt->IsVisible(); // set state from configuration. Can be overwritten by UserData, see below

    css::uno::Sequence < css::beans::NamedValue > aSeq = xWinOpt->GetUserData();

    OUString aTmp;
    if ( aSeq.getLength() )
        aSeq[0].Value >>= aTmp;

    OUString aWinData( aTmp );
    rInfo.aWinState = OUStringToOString(xWinOpt->GetWindowState(), RTL_TEXTENCODING_UTF8);

    if ( aWinData.isEmpty() )
        return;

    // Search for version ID
    if ( aWinData[0] != 0x0056 ) // 'V' = 56h
        // A version ID, so do not use
        return;

    // Delete 'V'
    aWinData = aWinData.copy(1);

    // Read version
    char cToken = ',';
    sal_Int32 nPos = aWinData.indexOf( cToken );
    sal_uInt16 nActVersion = static_cast<sal_uInt16>(aWinData.copy( 0, nPos + 1 ).toInt32());
    if ( nActVersion != nVersion )
        return;

    aWinData = aWinData.copy(nPos+1);

    // Load Visibility: is coded as a char
    rInfo.bVisible = (aWinData[0] == 0x0056); // 'V' = 56h
    aWinData = aWinData.copy(1);
    nPos = aWinData.indexOf( cToken );
    if (nPos == -1)
        return;

    sal_Int32 nNextPos = aWinData.indexOf( cToken, 2 );
    if ( nNextPos != -1 )
    {
        // there is extra information
        rInfo.nFlags = static_cast<SfxChildWindowFlags>(static_cast<sal_uInt16>(aWinData.copy( nPos+1, nNextPos - nPos - 1 ).toInt32()));
        aWinData = aWinData.replaceAt( nPos, nNextPos-nPos+1, "" );
        rInfo.aExtraString = aWinData;
    }
    else
        rInfo.nFlags = static_cast<SfxChildWindowFlags>(static_cast<sal_uInt16>(aWinData.copy( nPos+1 ).toInt32()));
}

void SfxChildWindow::CreateContext( sal_uInt16 nContextId, SfxBindings& rBindings )
{
    std::unique_ptr<SfxChildWindowContext> pCon;
    SfxChildWinFactory* pFact=nullptr;
    SfxApplication *pApp = SfxGetpApp();
    SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :nullptr;
    if ( pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( size_t nFactory = 0; nFactory < rFactories.size(); ++nFactory )
            {
                pFact = &rFactories[nFactory];
                if ( pFact->nId == GetType() )
                {
                    DBG_ASSERT( pFact->pArr, "No context registered!" );
                    if ( !pFact->pArr )
                        break;

                    for ( size_t n=0; n<pFact->pArr->size(); ++n )
                    {
                        SfxChildWinContextFactory *pConFact = &(*pFact->pArr)[n];
                        rBindings.ENTERREGISTRATIONS();
                        if ( pConFact->nContextId == nContextId )
                        {
                            SfxChildWinInfo aInfo = pFact->aInfo;
                            pCon = pConFact->pCtor( GetWindow(), &rBindings, &aInfo );
                            pCon->nContextId = pConFact->nContextId;
                            pImpl->pContextModule = pMod;
                        }
                        rBindings.LEAVEREGISTRATIONS();
                    }
                    break;
                }
            }
        }
    }

    if ( !pCon )
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( size_t nFactory = 0; nFactory < rFactories.size(); ++nFactory )
        {
            pFact = &rFactories[nFactory];
            if ( pFact->nId == GetType() )
            {
                DBG_ASSERT( pFact->pArr, "No context registered!" );
                if ( !pFact->pArr )
                    break;

                for ( size_t n=0; n<pFact->pArr->size(); ++n )
                {
                    SfxChildWinContextFactory *pConFact = &(*pFact->pArr)[n];
                    rBindings.ENTERREGISTRATIONS();
                    if ( pConFact->nContextId == nContextId )
                    {
                        SfxChildWinInfo aInfo = pFact->aInfo;
                        pCon = pConFact->pCtor( GetWindow(), &rBindings, &aInfo );
                        pCon->nContextId = pConFact->nContextId;
                        pImpl->pContextModule = nullptr;
                    }
                    rBindings.LEAVEREGISTRATIONS();
                }
                break;
            }
        }
    }

    if ( !pCon )
    {
        OSL_FAIL( "No suitable context found! ");
        return;
    }

    pContext = std::move(pCon);
    pContext->GetWindow()->SetSizePixel( pWindow->GetOutputSizePixel() );
    pContext->GetWindow()->Show();
}

SfxChildWindowContext::SfxChildWindowContext( sal_uInt16 nId )
    : nContextId( nId )
{
}

SfxChildWindowContext::~SfxChildWindowContext()
{
    pWindow.disposeAndClear();
}

FloatingWindow* SfxChildWindowContext::GetFloatingWindow(vcl::Window *pParent)
{
    if (pParent->GetType() == WindowType::DOCKINGWINDOW || pParent->GetType() == WindowType::TOOLBOX)
    {
        return static_cast<DockingWindow*>(pParent)->GetFloatingWindow();
    }
    if (pParent->GetType() == WindowType::FLOATINGWINDOW)
    {
        return static_cast<FloatingWindow*>(pParent);
    }
    return nullptr;
}

void SfxChildWindow::SetFactory_Impl( SfxChildWinFactory *pF )
{
    pImpl->pFact = pF;
}

void SfxChildWindow::SetHideNotDelete( bool bOn )
{
    pImpl->bHideNotDelete = bOn;
}

bool SfxChildWindow::IsHideNotDelete() const
{
    return pImpl->bHideNotDelete;
}

void SfxChildWindow::SetWantsFocus( bool bSet )
{
    pImpl->bWantsFocus = bSet;
}

bool SfxChildWindow::WantsFocus() const
{
    return pImpl->bWantsFocus;
}

bool SfxChildWinInfo::GetExtraData_Impl
(
    SfxChildAlignment   *pAlign
)   const
{
    // invalid?
    if ( aExtraString.isEmpty() )
        return false;
    OUString aStr;
    sal_Int32 nPos = aExtraString.indexOf("AL:");
    if ( nPos == -1 )
        return false;

    // Try to read the alignment string "ALIGN :(...)", but if
    // it is not present, then use an older version
    sal_Int32 n1 = aExtraString.indexOf('(', nPos);
    if ( n1 != -1 )
    {
        sal_Int32 n2 = aExtraString.indexOf(')', n1);
        if ( n2 != -1 )
        {
            // Cut out Alignment string
            aStr = aExtraString.copy(nPos, n2 - nPos + 1);
            aStr = aStr.replaceAt(nPos, n1-nPos+1, "");
        }
    }

    // First extract the Alignment
    if ( aStr.isEmpty() )
        return false;
    if ( pAlign )
        *pAlign = static_cast<SfxChildAlignment>(static_cast<sal_uInt16>(aStr.toInt32()));

    // then the LastAlignment
    nPos = aStr.indexOf(',');
    if ( nPos == -1 )
        return false;
    aStr = aStr.copy(nPos+1);

    // Then the splitting information
    nPos = aStr.indexOf(',');
    if ( nPos == -1 )
        // No docking in a Splitwindow
        return true;
    aStr = aStr.copy(nPos+1);
    Point aChildPos;
    Size aChildSize;
    return GetPosSizeFromString( aStr, aChildPos, aChildSize );
}

bool SfxChildWindow::IsVisible() const
{
    return pImpl->bVisible;
}

void SfxChildWindow::SetVisible_Impl( bool bVis )
{
    pImpl->bVisible = bVis;
}

void SfxChildWindow::Hide()
{
    if (xController)
        xController->EndDialog();
    else
        pWindow->Hide();
}

void SfxChildWindow::Show( ShowFlags nFlags )
{
    if (xController)
    {
        if (!xController->getDialog()->get_visible())
        {
            weld::DialogController::runAsync(xController,
                [this](sal_Int32 /*nResult*/){ xController->Close(); });
        }
    }
    else
        pWindow->Show(true, nFlags);
}

vcl::Window* SfxChildWindow::GetContextWindow( SfxModule const *pModule ) const
{
    return pModule == pImpl->pContextModule && pContext ? pContext->GetWindow(): nullptr;
}

void SfxChildWindow::SetWorkWindow_Impl( SfxWorkWindow* pWin )
{
    pImpl->pWorkWin = pWin;
    if (pWin)
    {
        if ( (xController && xController->getDialog()->has_toplevel_focus()) ||
             (pWindow && pWindow->HasChildPathFocus()) )
        {
            pImpl->pWorkWin->SetActiveChild_Impl( pWindow );
        }
    }
}

void SfxChildWindow::Activate_Impl()
{
    if(pImpl->pWorkWin!=nullptr)
        pImpl->pWorkWin->SetActiveChild_Impl( pWindow );
}

bool SfxChildWindow::QueryClose()
{
    bool bAllow = true;

    if ( pImpl->xFrame.is() )
    {
        css::uno::Reference< css::frame::XController >  xCtrl = pImpl->xFrame->getController();
        if ( xCtrl.is() )
            bAllow = xCtrl->suspend( true );
    }

    if ( bAllow )
    {
        if (GetController())
        {
            weld::Dialog* pDialog = GetController()->getDialog();
            bAllow = !pDialog->get_visible() || !pDialog->get_modal();
        }
        else if (GetWindow())
            bAllow = !GetWindow()->IsInModalMode();
    }

    return bAllow;
}

const css::uno::Reference< css::frame::XFrame >&  SfxChildWindow::GetFrame()
{
    return pImpl->xFrame;
}

void SfxChildWindow::SetFrame( const css::uno::Reference< css::frame::XFrame > & rFrame )
{
    // Do nothing if nothing will be changed ...
    if( pImpl->xFrame == rFrame )
        return;

    // ... but stop listening on old frame, if connection exist!
    if( pImpl->xFrame.is() )
        pImpl->xFrame->removeEventListener( pImpl->xListener );

    // If new frame is not NULL -> we must guarantee valid listener for disposing events.
    // Use already existing or create new one.
    if( rFrame.is() )
        if( !pImpl->xListener.is() )
            pImpl->xListener.set( new DisposeListener( this, pImpl.get() ) );

    // Set new frame in data container
    // and build new listener connection, if necessary.
    pImpl->xFrame = rFrame;
    if( pImpl->xFrame.is() )
        pImpl->xFrame->addEventListener( pImpl->xListener );
}

void SfxChildWindowContext::RegisterChildWindowContext(SfxModule* pMod, sal_uInt16 nId, std::unique_ptr<SfxChildWinContextFactory> pFact)
{
    SfxGetpApp()->RegisterChildWindowContext_Impl( pMod, nId, std::move(pFact) );
}

void SfxChildWindow::RegisterChildWindow(SfxModule* pMod, std::unique_ptr<SfxChildWinFactory> pFact)
{
    SfxGetpApp()->RegisterChildWindow_Impl( pMod, std::move(pFact) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
