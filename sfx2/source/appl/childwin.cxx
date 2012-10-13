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


#include <vcl/toolbox.hxx>
#include <tools/rcid.h>
#include <unotools/viewoptions.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/string.hxx>
#include <cppuhelper/implbase1.hxx>

#include <sfx2/childwin.hxx>
#include <sfx2/app.hxx>
#include "arrdecl.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/dispatch.hxx>
#include "workwin.hxx"

static const sal_uInt16 nVersion = 2;

DBG_NAME(SfxChildWindow)

struct SfxChildWindow_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >      xListener;
    SfxChildWinFactory* pFact;
    sal_Bool                bHideNotDelete;
    sal_Bool                bVisible;
    sal_Bool                bHideAtToggle;
    sal_Bool                bWantsFocus;
    SfxModule*          pContextModule;
    SfxWorkWindow*      pWorkWin;
};

// -----------------------------------------------------------------------

class DisposeListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
    public:
        DisposeListener( SfxChildWindow*      pOwner ,
                         SfxChildWindow_Impl* pData  )
            :   m_pOwner( pOwner )
            ,   m_pData ( pData  )
        {}

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aSource ) throw (::com::sun::star::uno::RuntimeException)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > xSelfHold( this );

            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComp( aSource.Source, ::com::sun::star::uno::UNO_QUERY );
            if( xComp.is() )
                xComp->removeEventListener( this );

            if( m_pOwner && m_pData )
            {
                m_pData->xListener = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >();

                if ( m_pData->pWorkWin )
                {
                    // m_pOwner and m_pData will be killed
                    m_pData->xFrame    = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >();
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

// -----------------------------------------------------------------------

sal_Bool GetPosSizeFromString( const String& rStr, Point& rPos, Size& rSize )
{
    if ( comphelper::string::getTokenCount(rStr, '/') != 4 )
        return sal_False;

    xub_StrLen nIdx = 0;
    rPos.X() = rStr.GetToken(0, '/', nIdx).ToInt32();
    rPos.Y() = rStr.GetToken(0, '/', nIdx).ToInt32();
    rSize.Width() = rStr.GetToken(0, '/', nIdx).ToInt32();
    rSize.Height() = rStr.GetToken(0, '/', nIdx).ToInt32();

    // negative sizes are invalid
    if ( rSize.Width() < 0 || rSize.Height() < 0 )
        return sal_False;

    return sal_True;
}

sal_Bool GetSplitSizeFromString( const String& rStr, Size& rSize )
{
    xub_StrLen nIndex = rStr.Search( ',' );
    if ( nIndex != STRING_NOTFOUND )
    {
        String aStr = rStr.Copy( nIndex+1 );

        sal_Int32 nCount = comphelper::string::getTokenCount(aStr, ';');
        if ( nCount != 2 )
            return sal_False;

        rSize.Width() = aStr.GetToken(0, ';' ).ToInt32();
        rSize.Height() = aStr.GetToken(1, ';' ).ToInt32();

        // negative sizes are invalid
        if ( rSize.Width() < 0 || rSize.Height() < 0 )
            return sal_False;

        return sal_True;
    }

    return sal_False;
}

//=========================================================================
SfxChildWindow::SfxChildWindow(Window *pParentWindow, sal_uInt16 nId)
    : pParent(pParentWindow)
    , nType(nId)
    , eChildAlignment(SFX_ALIGN_NOALIGNMENT)
    , pWindow(0L)
{
    pImp = new SfxChildWindow_Impl;
    pImp->pFact = 0L;
    pImp->bHideNotDelete = sal_False;
    pImp->bHideAtToggle = sal_False;
    pImp->bWantsFocus = sal_True;
    pImp->bVisible = sal_True;
    pImp->pContextModule = NULL;
    pImp->pWorkWin = NULL;

    pContext = 0L;
    DBG_CTOR(SfxChildWindow,0);
}

void SfxChildWindow::Destroy()
{
    if ( GetFrame().is() )
    {
        pImp->pWorkWin = NULL;
        try
        {
            ::com::sun::star::uno::Reference < ::com::sun::star::util::XCloseable > xClose( GetFrame(), ::com::sun::star::uno::UNO_QUERY );
            if ( xClose.is() )
                xClose->close( sal_True );
            else
                GetFrame()->dispose();
        }
        catch (const com::sun::star::uno::Exception&)
        {
        }
    }
    else
        delete this;
}

//-------------------------------------------------------------------------
SfxChildWindow::~SfxChildWindow()
{
    DBG_DTOR(SfxChildWindow,0);
    delete pContext;
    delete pWindow;
    delete pImp;
}

//-------------------------------------------------------------------------
SfxChildWindow* SfxChildWindow::CreateChildWindow( sal_uInt16 nId,
        Window *pParent, SfxBindings* pBindings, SfxChildWinInfo& rInfo)
{
    SfxChildWindow *pChild=0;
    SfxChildWinFactory* pFact=0;
    sal_uInt16 nOldMode = Application::GetSystemWindowMode();

    // First search for ChildWindow in SDT; "Overloading has to be realized
    // by using ChildWindowContext
    SfxApplication *pApp = SFX_APP();
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
        {
            pFact = rFactories[nFactory];
            if ( pFact->nId == nId )
            {
                SfxChildWinInfo& rFactInfo = pFact->aInfo;
                if ( rInfo.bVisible )
                {
                    if ( pBindings )
                        pBindings->ENTERREGISTRATIONS();
                    SfxChildWinInfo aInfo = rFactInfo;
                    Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_NOAUTOMODE );
                    pChild = pFact->pCtor( pParent, nId, pBindings, &aInfo );
                    Application::SetSystemWindowMode( nOldMode );
                    if ( pBindings )
                        pBindings->LEAVEREGISTRATIONS();
                }

                break;
            }
        }
    }

    SfxDispatcher *pDisp = pBindings->GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :0;
    if ( !pChild &&  pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == nId )
                {
                    SfxChildWinInfo& rFactInfo = pFact->aInfo;
                    if ( rInfo.bVisible )
                    {
                        if ( pBindings )
                            pBindings->ENTERREGISTRATIONS();
                        SfxChildWinInfo aInfo = rFactInfo;
                        Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_NOAUTOMODE );
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
        DBG_WARNING("ChildWindow has no Window!");
    }

    return pChild;
}

//-------------------------------------------------------------------------
void SfxChildWindow::SaveStatus(const SfxChildWinInfo& rInfo)
{
    sal_uInt16 nID = GetType();

    rtl::OUStringBuffer aWinData;
    aWinData.append('V').append(static_cast<sal_Int32>(nVersion)).
        append(',').append(rInfo.bVisible ? 'V' : 'H').append(',').
        append(static_cast<sal_Int32>(rInfo.nFlags));
    if ( rInfo.aExtraString.Len() )
    {
        aWinData.append(',');
        aWinData.append(rInfo.aExtraString);
    }

    SvtViewOptions aWinOpt( E_WINDOW, String::CreateFromInt32( nID ) );
    aWinOpt.SetWindowState(rtl::OStringToOUString(rInfo.aWinState, RTL_TEXTENCODING_UTF8));

    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq(1);
    aSeq[0].Name = ::rtl::OUString("Data");
    aSeq[0].Value <<= aWinData.makeStringAndClear();
    aWinOpt.SetUserData( aSeq );

    // ... but save status at runtime!
    pImp->pFact->aInfo = rInfo;
}

//-------------------------------------------------------------------------
void SfxChildWindow::SetAlignment(SfxChildAlignment eAlign)
{
    DBG_CHKTHIS(SfxChildWindow,0);

    eChildAlignment = eAlign;
}

//-------------------------------------------------------------------------
SfxChildWinInfo SfxChildWindow::GetInfo() const
{
    DBG_CHKTHIS(SfxChildWindow,0);

    SfxChildWinInfo aInfo;
    aInfo.aPos  = pWindow->GetPosPixel();
    aInfo.aSize = pWindow->GetSizePixel();
    if ( pWindow->IsSystemWindow() )
    {
        sal_uIntPtr nMask = WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE;
        if ( pWindow->GetStyle() & WB_SIZEABLE )
            nMask |= ( WINDOWSTATE_MASK_WIDTH | WINDOWSTATE_MASK_HEIGHT );
        aInfo.aWinState = ((SystemWindow*)pWindow)->GetWindowState( nMask );
    }
    else if ( pWindow->GetType() == RSC_DOCKINGWINDOW )
    {
        if (((DockingWindow*)pWindow)->GetFloatingWindow() )
            aInfo.aWinState = ((DockingWindow*)pWindow)->GetFloatingWindow()->GetWindowState();
        else
        {
            SfxChildWinInfo aTmpInfo;
            ((SfxDockingWindow*)pWindow)->FillInfo( aTmpInfo );
            aInfo.aExtraString = aTmpInfo.aExtraString;
        }
    }

    aInfo.bVisible = pImp->bVisible;
    aInfo.nFlags = 0;
    return aInfo;
}

//-------------------------------------------------------------------------
sal_uInt16 SfxChildWindow::GetPosition()
{
    return pImp->pFact->nPos;
}

//-------------------------------------------------------------------------
void SfxChildWindow::InitializeChildWinFactory_Impl( sal_uInt16 nId, SfxChildWinInfo& rInfo )
{
    // load configuration
    SvtViewOptions aWinOpt( E_WINDOW, String::CreateFromInt32( nId ) );

    if ( aWinOpt.Exists() && aWinOpt.HasVisible() )
        rInfo.bVisible  = aWinOpt.IsVisible(); // set state from configuration. Can be overwritten by UserData, see below

    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq = aWinOpt.GetUserData();

    ::rtl::OUString aTmp;
    if ( aSeq.getLength() )
        aSeq[0].Value >>= aTmp;

    String aWinData( aTmp );
    rInfo.aWinState = rtl::OUStringToOString(aWinOpt.GetWindowState(), RTL_TEXTENCODING_UTF8);


    if ( aWinData.Len() )
    {
        // Search for version ID
        if ( aWinData.GetChar((sal_uInt16)0) != 0x0056 ) // 'V' = 56h
            // A version ID, so do not use
            return;

        // Delete 'V'
        aWinData.Erase(0,1);

        // Read version
        char cToken = ',';
        sal_uInt16 nPos = aWinData.Search( cToken );
        sal_uInt16 nActVersion = (sal_uInt16)aWinData.Copy( 0, nPos + 1 ).ToInt32();
        if ( nActVersion != nVersion )
            return;

        aWinData.Erase(0,nPos+1);

        // Load Visibility: is coded as a char
        rInfo.bVisible = (aWinData.GetChar(0) == 0x0056); // 'V' = 56h
        aWinData.Erase(0,1);
        nPos = aWinData.Search( cToken );
        if (nPos != STRING_NOTFOUND)
        {
            sal_uInt16 nNextPos = aWinData.Search( cToken, 2 );
            if ( nNextPos != STRING_NOTFOUND )
            {
                // there is extra information
                rInfo.nFlags = (sal_uInt16)aWinData.Copy( nPos+1, nNextPos - nPos - 1 ).ToInt32();
                aWinData.Erase( nPos, nNextPos-nPos+1 );
                rInfo.aExtraString = aWinData;
            }
            else
                rInfo.nFlags = (sal_uInt16)aWinData.Copy( nPos+1 ).ToInt32();
        }
    }
}

void SfxChildWindow::CreateContext( sal_uInt16 nContextId, SfxBindings& rBindings )
{
    SfxChildWindowContext *pCon = NULL;
    SfxChildWinFactory* pFact=0;
    SfxApplication *pApp = SFX_APP();
    SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? SfxModule::GetActiveModule( pDisp->GetFrame() ) :0;
    if ( pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == GetType() )
                {
                    DBG_ASSERT( pFact->pArr, "No context registered!" );
                    if ( !pFact->pArr )
                        break;

                    for ( sal_uInt16 n=0; n<pFact->pArr->size(); ++n )
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
        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.size(); ++nFactory )
        {
            pFact = rFactories[nFactory];
            if ( pFact->nId == GetType() )
            {
                DBG_ASSERT( pFact->pArr, "No context registered!" );
                if ( !pFact->pArr )
                    break;

                for ( sal_uInt16 n=0; n<pFact->pArr->size(); ++n )
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
    : pWindow( NULL )
    , nContextId( nId )
{
}

SfxChildWindowContext::~SfxChildWindowContext()
{
    delete pWindow;
}

FloatingWindow* SfxChildWindowContext::GetFloatingWindow() const
{
    Window *pParent = pWindow->GetParent();
    if ( pParent->GetType() == RSC_DOCKINGWINDOW || pParent->GetType() == RSC_TOOLBOX )
    {
        return ((DockingWindow*)pParent)->GetFloatingWindow();
    }
    else if ( pParent->GetType() == RSC_FLOATINGWINDOW )
    {
        return (FloatingWindow*) pParent;
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

sal_Bool SfxChildWindowContext::Close()
{
    return sal_True;
}

void SfxChildWindow::SetFactory_Impl( SfxChildWinFactory *pF )
{
    pImp->pFact = pF;
}

void SfxChildWindow::SetHideNotDelete( sal_Bool bOn )
{
    pImp->bHideNotDelete = bOn;
}

sal_Bool SfxChildWindow::IsHideNotDelete() const
{
    return pImp->bHideNotDelete;
}

sal_Bool SfxChildWindow::IsHideAtToggle() const
{
    return pImp->bHideAtToggle;
}

void SfxChildWindow::SetWantsFocus( sal_Bool bSet )
{
    pImp->bWantsFocus = bSet;
}

sal_Bool SfxChildWindow::WantsFocus() const
{
    return pImp->bWantsFocus;
}

sal_Bool SfxChildWinInfo::GetExtraData_Impl
(
    SfxChildAlignment   *pAlign,
    SfxChildAlignment   *pLastAlign,
    Size                *pSize,
    sal_uInt16          *pLine,
    sal_uInt16          *pPos
)   const
{
    // invalid?
    if ( !aExtraString.Len() )
        return sal_False;
    String aStr;
    sal_uInt16 nPos = aExtraString.SearchAscii("AL:");
    if ( nPos == STRING_NOTFOUND )
        return sal_False;

    // Try to read the alignment string "ALIGN :(...)", but if
    // it is not present, then use an older version
    if ( nPos != STRING_NOTFOUND )
    {
        sal_uInt16 n1 = aExtraString.Search('(', nPos);
        if ( n1 != STRING_NOTFOUND )
        {
            sal_uInt16 n2 = aExtraString.Search(')', n1);
            if ( n2 != STRING_NOTFOUND )
            {
                // Cut out Alignment string
                aStr = aExtraString.Copy(nPos, n2 - nPos + 1);
                aStr.Erase(nPos, n1-nPos+1);
            }
        }
    }

    // First extract the Alignment
    if ( !aStr.Len() )
        return sal_False;
    if ( pAlign )
        *pAlign = (SfxChildAlignment) (sal_uInt16) aStr.ToInt32();

    // then the LastAlignment
    nPos = aStr.Search(',');
    if ( nPos == STRING_NOTFOUND )
        return sal_False;
    aStr.Erase(0, nPos+1);
    if ( pLastAlign )
        *pLastAlign = (SfxChildAlignment) (sal_uInt16) aStr.ToInt32();

    // Then the splitting information
    nPos = aStr.Search(',');
    if ( nPos == STRING_NOTFOUND )
        // No docking in a Splitwindow
        return sal_True;
    aStr.Erase(0, nPos+1);
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
        return sal_True;
    }
    return sal_False;
}

sal_Bool SfxChildWindow::IsVisible() const
{
    return pImp->bVisible;
}

void SfxChildWindow::SetVisible_Impl( sal_Bool bVis )
{
    pImp->bVisible = bVis;
}

void SfxChildWindow::Hide()
{
    switch ( pWindow->GetType() )
    {
        case RSC_DOCKINGWINDOW :
            ((DockingWindow*)pWindow)->Hide();
            break;
        case RSC_TOOLBOX :
            ((ToolBox*)pWindow)->Hide();
            break;
        default:
            pWindow->Hide();
            break;
    }
}

void SfxChildWindow::Show( sal_uInt16 nFlags )
{
    switch ( pWindow->GetType() )
    {
        case RSC_DOCKINGWINDOW :
            ((DockingWindow*)pWindow)->Show( sal_True, nFlags );
            break;
        case RSC_TOOLBOX :
            ((ToolBox*)pWindow)->Show( sal_True, nFlags );
            break;
        default:
            pWindow->Show( sal_True, nFlags );
            break;
    }
}

Window* SfxChildWindow::GetContextWindow( SfxModule *pModule ) const
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

sal_Bool SfxChildWindow::QueryClose()
{
    sal_Bool bAllow = sal_True;

    if ( pImp->xFrame.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >  xCtrl = pImp->xFrame->getController();
        if ( xCtrl.is() )
            bAllow = xCtrl->suspend( sal_True );
    }

    if ( bAllow )
        bAllow = !GetWindow()->IsInModalMode();

    return bAllow;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  SfxChildWindow::GetFrame()
{
    return pImp->xFrame;
}

void SfxChildWindow::SetFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & rFrame )
{
    // Do nothing if nothing will be changed ...
    if( pImp->xFrame != rFrame )
    {
        // ... but stop listening on old frame, if connection exist!
        if( pImp->xFrame.is() )
            pImp->xFrame->removeEventListener( pImp->xListener );

        // If new frame isnt NULL -> we must guarantee valid listener for disposing events.
        // Use already existing or create new one.
        if( rFrame.is() )
            if( !pImp->xListener.is() )
                pImp->xListener = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >( new DisposeListener( this, pImp ) );

        // Set new frame in data container
        // and build new listener connection, if neccessary.
        pImp->xFrame = rFrame;
        if( pImp->xFrame.is() )
            pImp->xFrame->addEventListener( pImp->xListener );
    }
}

sal_Bool SfxChildWindow::CanGetFocus() const
{
    return !(pImp->pFact->aInfo.nFlags & SFX_CHILDWIN_CANTGETFOCUS);
}

void SfxChildWindowContext::RegisterChildWindowContext(SfxModule* pMod, sal_uInt16 nId, SfxChildWinContextFactory* pFact)
{
    SFX_APP()->RegisterChildWindowContext_Impl( pMod, nId, pFact );
}

void SfxChildWindow::RegisterChildWindow(SfxModule* pMod, SfxChildWinFactory* pFact)
{
    SFX_APP()->RegisterChildWindow_Impl( pMod, pFact );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
