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
#include <tools/rcid.h>
#include <unotools/moduleoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/dispatch.hxx>
#include "workwin.hxx"
#include "childwinimpl.hxx"

static const sal_uInt16 nVersion = 2;

SfxChildWinFactory::SfxChildWinFactory( SfxChildWinCtor pTheCtor, sal_uInt16 nID,
        sal_uInt16 n )
    : pCtor(pTheCtor)
    , nId( nID )
    , nPos(n)
    , pArr( NULL )
{}

SfxChildWinFactory::~SfxChildWinFactory()
{
    delete pArr;
}

struct SfxChildWindow_Impl
{
    css::uno::Reference< css::frame::XFrame >             xFrame;
    css::uno::Reference< css::lang::XEventListener >      xListener;
    SfxChildWinFactory* pFact;
    bool                bHideNotDelete;
    bool                bVisible;
    bool                bHideAtToggle;
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

        virtual void SAL_CALL disposing( const css::lang::EventObject& aSource ) throw (css::uno::RuntimeException, std::exception) override
        {
            css::uno::Reference< css::lang::XEventListener > xSelfHold( this );

            css::uno::Reference< css::lang::XComponent > xComp( aSource.Source, css::uno::UNO_QUERY );
            if( xComp.is() )
                xComp->removeEventListener( this );

            if( m_pOwner && m_pData )
            {
                m_pData->xListener = css::uno::Reference< css::lang::XEventListener >();

                if ( m_pData->pWorkWin )
                {
                    // m_pOwner and m_pData will be killed
                    m_pData->xFrame    = css::uno::Reference< css::frame::XFrame >();
                    m_pData->pWorkWin->GetBindings().Execute( m_pOwner->GetType() );
                }
                else
                {
                    delete m_pOwner;
                }

                m_pOwner = NULL;
                m_pData  = NULL;
            }
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
    rPos.X() = rStr.getToken(0, '/', nIdx).toInt32();
    rPos.Y() = rStr.getToken(0, '/', nIdx).toInt32();
    rSize.Width() = rStr.getToken(0, '/', nIdx).toInt32();
    rSize.Height() = rStr.getToken(0, '/', nIdx).toInt32();

    // negative sizes are invalid
    if ( rSize.Width() < 0 || rSize.Height() < 0 )
        return false;

    return true;
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

        rSize.Width() = aStr.getToken(0, ';' ).toInt32();
        rSize.Height() = aStr.getToken(1, ';' ).toInt32();

        // negative sizes are invalid
        if ( rSize.Width() < 0 || rSize.Height() < 0 )
            return false;

        return true;
    }

    return false;
}


SfxChildWindow::SfxChildWindow(vcl::Window *pParentWindow, sal_uInt16 nId)
    : pParent(pParentWindow)
    , nType(nId)
    , eChildAlignment(SfxChildAlignment::NOALIGNMENT)
{
    pImp = new SfxChildWindow_Impl;
    pImp->pFact = 0L;
    pImp->bHideNotDelete = false;
    pImp->bHideAtToggle = false;
    pImp->bWantsFocus = true;
    pImp->bVisible = true;
    pImp->pContextModule = NULL;
    pImp->pWorkWin = NULL;

    pContext = 0L;
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
                xClose->close( sal_True );
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
    if (pImp->pWorkWin)
    {
        if (pImp->pWorkWin->GetActiveChild_Impl() == pWindow)
            pImp->pWorkWin->SetActiveChild_Impl(NULL);
        pImp->pWorkWin = NULL;
    }
}

SfxChildWindow::~SfxChildWindow()
{
    delete pContext;
    pContext = NULL;
    ClearWorkwin();
    pWindow.disposeAndClear();
    delete pImp;
    pImp = NULL;
}


SfxChildWindow* SfxChildWindow::CreateChildWindow( sal_uInt16 nId,
        vcl::Window *pParent, SfxBindings* pBindings, SfxChildWinInfo& rInfo)
{
    SfxChildWindow *pChild=0;
    SfxChildWinFactory* pFact=0;
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

    SfxDispatcher *pDisp = pBindings ? pBindings->GetDispatcher_Impl() : NULL;
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) : NULL;
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

    if ( pChild && !pChild->pWindow )
    {
        DELETEZ(pChild);
        SAL_INFO("sfx2.appl", "ChildWindow has no Window!");
    }

    return pChild;
}


void SfxChildWindow::SaveStatus(const SfxChildWinInfo& rInfo)
{
    sal_uInt16 nID = GetType();

    OUStringBuffer aWinData;
    aWinData.append('V').append(static_cast<sal_Int32>(nVersion)).
        append(',').append(rInfo.bVisible ? 'V' : 'H').append(',').
        append(static_cast<sal_Int32>(rInfo.nFlags));
    if ( !rInfo.aExtraString.isEmpty() )
    {
        aWinData.append(',');
        aWinData.append(rInfo.aExtraString);
    }

    OUString sName(OUString::number(nID));
    //Try and save window state per-module, e.g. sidebar on in one application
    //but off in another
    if (!rInfo.aModule.isEmpty())
        sName = rInfo.aModule + "/" + sName;
    SvtViewOptions aWinOpt(E_WINDOW, sName);
    aWinOpt.SetWindowState(OStringToOUString(rInfo.aWinState, RTL_TEXTENCODING_UTF8));

    css::uno::Sequence < css::beans::NamedValue > aSeq
        { { "Data", css::uno::makeAny(aWinData.makeStringAndClear()) } };
    aWinOpt.SetUserData( aSeq );

    // ... but save status at runtime!
    pImp->pFact->aInfo = rInfo;
}


void SfxChildWindow::SetAlignment(SfxChildAlignment eAlign)
{

    eChildAlignment = eAlign;
}


SfxChildWinInfo SfxChildWindow::GetInfo() const
{

    SfxChildWinInfo aInfo(pImp->pFact->aInfo);
    aInfo.aPos  = pWindow->GetPosPixel();
    aInfo.aSize = pWindow->GetSizePixel();
    if ( pWindow->IsSystemWindow() )
    {
        sal_uIntPtr nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( pWindow->GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        aInfo.aWinState = static_cast<SystemWindow*>(pWindow.get())->GetWindowState( nMask );
    }
    else if ( pWindow->GetType() == RSC_DOCKINGWINDOW )
    {
        if (static_cast<DockingWindow*>(pWindow.get())->GetFloatingWindow() )
            aInfo.aWinState = static_cast<DockingWindow*>(pWindow.get())->GetFloatingWindow()->GetWindowState();
        else
        {
            SfxChildWinInfo aTmpInfo;
            static_cast<SfxDockingWindow*>(pWindow.get())->FillInfo( aTmpInfo );
            aInfo.aExtraString = aTmpInfo.aExtraString;
        }
    }

    aInfo.bVisible = pImp->bVisible;
    aInfo.nFlags = SfxChildWindowFlags::NONE;
    return aInfo;
}


sal_uInt16 SfxChildWindow::GetPosition()
{
    return pImp->pFact->nPos;
}


void SfxChildWindow::InitializeChildWinFactory_Impl(sal_uInt16 nId, SfxChildWinInfo& rInfo)
{
    // load configuration

    std::unique_ptr<SvtViewOptions> xWinOpt;
    // first see if a module specific id exists
    if (rInfo.aModule.getLength())
        xWinOpt.reset(new SvtViewOptions(E_WINDOW, rInfo.aModule + "/" + OUString::number(nId)));

    // if not then try the generic id
    if (!xWinOpt || !xWinOpt->Exists())
        xWinOpt.reset(new SvtViewOptions(E_WINDOW, OUString::number(nId)));

    if (xWinOpt->Exists() && xWinOpt->HasVisible() )
        rInfo.bVisible  = xWinOpt->IsVisible(); // set state from configuration. Can be overwritten by UserData, see below

    css::uno::Sequence < css::beans::NamedValue > aSeq = xWinOpt->GetUserData();

    OUString aTmp;
    if ( aSeq.getLength() )
        aSeq[0].Value >>= aTmp;

    OUString aWinData( aTmp );
    rInfo.aWinState = OUStringToOString(xWinOpt->GetWindowState(), RTL_TEXTENCODING_UTF8);

    if ( !aWinData.isEmpty() )
    {
        // Search for version ID
        if ( aWinData[0] != 0x0056 ) // 'V' = 56h
            // A version ID, so do not use
            return;

        // Delete 'V'
        aWinData = aWinData.copy(1);

        // Read version
        char cToken = ',';
        sal_Int32 nPos = aWinData.indexOf( cToken );
        sal_uInt16 nActVersion = (sal_uInt16)aWinData.copy( 0, nPos + 1 ).toInt32();
        if ( nActVersion != nVersion )
            return;

        aWinData = aWinData.copy(nPos+1);

        // Load Visibility: is coded as a char
        rInfo.bVisible = (aWinData[0] == 0x0056); // 'V' = 56h
        aWinData = aWinData.copy(1);
        nPos = aWinData.indexOf( cToken );
        if (nPos != -1)
        {
            sal_Int32 nNextPos = aWinData.indexOf( cToken, 2 );
            if ( nNextPos != -1 )
            {
                // there is extra information
                rInfo.nFlags = static_cast<SfxChildWindowFlags>((sal_uInt16)aWinData.copy( nPos+1, nNextPos - nPos - 1 ).toInt32());
                aWinData = aWinData.replaceAt( nPos, nNextPos-nPos+1, "" );
                rInfo.aExtraString = aWinData;
            }
            else
                rInfo.nFlags = static_cast<SfxChildWindowFlags>((sal_uInt16)aWinData.copy( nPos+1 ).toInt32());
        }
    }
}

void SfxChildWindow::CreateContext( sal_uInt16 nContextId, SfxBindings& rBindings )
{
    SfxChildWindowContext *pCon = NULL;
    SfxChildWinFactory* pFact=0;
    SfxApplication *pApp = SfxGetpApp();
    SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :0;
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
                            pImp->pContextModule = pMod;
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
                        pImp->pContextModule = NULL;
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

    if ( pContext )
        delete( pContext );
    pContext = pCon;
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

FloatingWindow* SfxChildWindowContext::GetFloatingWindow() const
{
    vcl::Window *pParent = pWindow->GetParent();
    if (pParent->GetType() == WINDOW_DOCKINGWINDOW || pParent->GetType() == WINDOW_TOOLBOX)
    {
        return static_cast<DockingWindow*>(pParent)->GetFloatingWindow();
    }
    else if (pParent->GetType() == WINDOW_FLOATINGWINDOW)
    {
        return static_cast<FloatingWindow*>(pParent);
    }
    else
    {
        OSL_FAIL("No FloatingWindow-Context!");
        return NULL;
    }
}

void SfxChildWindowContext::Resizing( Size& )
{
}

void SfxChildWindow::SetFactory_Impl( SfxChildWinFactory *pF )
{
    pImp->pFact = pF;
}

void SfxChildWindow::SetHideNotDelete( bool bOn )
{
    pImp->bHideNotDelete = bOn;
}

bool SfxChildWindow::IsHideNotDelete() const
{
    return pImp->bHideNotDelete;
}

bool SfxChildWindow::IsHideAtToggle() const
{
    return pImp->bHideAtToggle;
}

void SfxChildWindow::SetWantsFocus( bool bSet )
{
    pImp->bWantsFocus = bSet;
}

bool SfxChildWindow::WantsFocus() const
{
    return pImp->bWantsFocus;
}

bool SfxChildWinInfo::GetExtraData_Impl
(
    SfxChildAlignment   *pAlign,
    SfxChildAlignment   *pLastAlign,
    Size                *pSize,
    sal_uInt16          *pLine,
    sal_uInt16          *pPos
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
        *pAlign = (SfxChildAlignment) (sal_uInt16) aStr.toInt32();

    // then the LastAlignment
    nPos = aStr.indexOf(',');
    if ( nPos == -1 )
        return false;
    aStr = aStr.copy(nPos+1);
    if ( pLastAlign )
        *pLastAlign = (SfxChildAlignment) (sal_uInt16) aStr.toInt32();

    // Then the splitting information
    nPos = aStr.indexOf(',');
    if ( nPos == -1 )
        // No docking in a Splitwindow
        return true;
    aStr = aStr.copy(nPos+1);
    Point aChildPos;
    Size aChildSize;
    if ( GetPosSizeFromString( aStr, aChildPos, aChildSize ) )
    {
        if ( pSize )
            *pSize = aChildSize;
        if ( pLine )
            *pLine = (sal_uInt16) aChildPos.X();
        if ( pPos )
            *pPos = (sal_uInt16) aChildPos.Y();
        return true;
    }
    return false;
}

bool SfxChildWindow::IsVisible() const
{
    return pImp->bVisible;
}

void SfxChildWindow::SetVisible_Impl( bool bVis )
{
    pImp->bVisible = bVis;
}

void SfxChildWindow::Hide()
{
    switch ( pWindow->GetType() )
    {
        case RSC_DOCKINGWINDOW :
            static_cast<DockingWindow*>(pWindow.get())->Hide();
            break;
        case RSC_TOOLBOX :
            static_cast<ToolBox*>(pWindow.get())->Hide();
            break;
        default:
            pWindow->Hide();
            break;
    }
}

void SfxChildWindow::Show( ShowFlags nFlags )
{
    switch ( pWindow->GetType() )
    {
        case RSC_DOCKINGWINDOW :
            static_cast<DockingWindow*>(pWindow.get())->Show( true, nFlags );
            break;
        case RSC_TOOLBOX :
            static_cast<ToolBox*>(pWindow.get())->Show( true, nFlags );
            break;
        default:
            pWindow->Show( true, nFlags );
            break;
    }
}

vcl::Window* SfxChildWindow::GetContextWindow( SfxModule *pModule ) const
{
    return pModule == pImp->pContextModule && pContext ? pContext->GetWindow(): 0;
}

void SfxChildWindow::SetWorkWindow_Impl( SfxWorkWindow* pWin )
{
    pImp->pWorkWin = pWin;
    if ( pWin && pWindow->HasChildPathFocus() )
        pImp->pWorkWin->SetActiveChild_Impl( pWindow );
}

void SfxChildWindow::Activate_Impl()
{
    if(pImp->pWorkWin!=NULL)
        pImp->pWorkWin->SetActiveChild_Impl( pWindow );
}

void SfxChildWindow::Deactivate_Impl()
{
}

bool SfxChildWindow::QueryClose()
{
    bool bAllow = true;

    if ( pImp->xFrame.is() )
    {
        css::uno::Reference< css::frame::XController >  xCtrl = pImp->xFrame->getController();
        if ( xCtrl.is() )
            bAllow = xCtrl->suspend( sal_True );
    }

    if ( bAllow )
        bAllow = !GetWindow()->IsInModalMode();

    return bAllow;
}

css::uno::Reference< css::frame::XFrame >  SfxChildWindow::GetFrame()
{
    return pImp->xFrame;
}

void SfxChildWindow::SetFrame( const css::uno::Reference< css::frame::XFrame > & rFrame )
{
    // Do nothing if nothing will be changed ...
    if( pImp->xFrame != rFrame )
    {
        // ... but stop listening on old frame, if connection exist!
        if( pImp->xFrame.is() )
            pImp->xFrame->removeEventListener( pImp->xListener );

        // If new frame is not NULL -> we must guarantee valid listener for disposing events.
        // Use already existing or create new one.
        if( rFrame.is() )
            if( !pImp->xListener.is() )
                pImp->xListener = css::uno::Reference< css::lang::XEventListener >( new DisposeListener( this, pImp ) );

        // Set new frame in data container
        // and build new listener connection, if necessary.
        pImp->xFrame = rFrame;
        if( pImp->xFrame.is() )
            pImp->xFrame->addEventListener( pImp->xListener );
    }
}

bool SfxChildWindow::CanGetFocus() const
{
    return !(pImp->pFact->aInfo.nFlags & SfxChildWindowFlags::CANTGETFOCUS);
}

void SfxChildWindowContext::RegisterChildWindowContext(SfxModule* pMod, sal_uInt16 nId, SfxChildWinContextFactory* pFact)
{
    SfxGetpApp()->RegisterChildWindowContext_Impl( pMod, nId, pFact );
}

void SfxChildWindow::RegisterChildWindow(SfxModule* pMod, SfxChildWinFactory* pFact)
{
    SfxGetpApp()->RegisterChildWindow_Impl( pMod, pFact );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
