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
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "docholder.hxx"
#include "embeddoc.hxx"
#include "intercept.hxx"
#include "syswinwrapper.hxx"
#include "iipaobj.hxx"

#include "common.h"
#include <windows.h>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XSystemChildFactory.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#include <com/sun/star/bridge/ModelDependent.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <comphelper/processfactory.hxx>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <rtl/process.h>

using namespace ::com::sun::star;

// add mutex locking ???

DocumentHolder::DocumentHolder(
    const uno::Reference<lang::XMultiServiceFactory >& xFactory,
    const ::rtl::Reference< EmbeddedDocumentInstanceAccess_Impl >& xOleAccess )
    :
    m_bAllowInPlace(true),
    m_pIOleIPSite(nullptr),
    m_pIOleIPFrame(nullptr),
    m_pIOleIPUIWindow(nullptr),
    m_pCHatchWin(nullptr),
    m_xOleAccess( xOleAccess ),
    m_pInterceptor(nullptr),
    m_xFactory( xFactory ),
    m_bOnDeactivate(false),
    m_hWndxWinParent(nullptr),
    m_hWndxWinCont(nullptr),
    m_nMenuHandle(nullptr),
    m_nMenuShared(nullptr),
    m_nOLEMenu(nullptr),
    m_nMacroExecMode( document::MacroExecMode::USE_CONFIG ),
    m_bLink( false )
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(comphelper::getComponentContext(m_xFactory));
    xDesktop->addTerminateListener( static_cast<frame::XTerminateListener*>(this) );
}


DocumentHolder::~DocumentHolder()
{
    delete m_pCHatchWin;

    ClearInterceptorInternally();
}


void DocumentHolder::LoadDocInFrame( bool bPluginMode )
{
    uno::Reference<frame::XComponentLoader> xComponentLoader(
        m_xFrame,uno::UNO_QUERY);
    if( xComponentLoader.is() && m_xDocument.is() )
    {
        uno::Reference< task::XInteractionHandler2 > xHandler(
            task::InteractionHandler::createWithParent(comphelper::getComponentContext(m_xFactory), nullptr) );

        sal_Int32 nLen = 3;
        uno::Sequence<beans::PropertyValue> aSeq( nLen );

        aSeq[0] = beans::PropertyValue(
            "Model",
            -1,
            uno::Any(uno::Reference<uno::XInterface>(m_xDocument, uno::UNO_QUERY)),
            beans::PropertyState_DIRECT_VALUE);

        aSeq[1] = beans::PropertyValue(
            "ReadOnly",
            -1,
            uno::Any(false),
            beans::PropertyState_DIRECT_VALUE);

        aSeq[2] = beans::PropertyValue(
            "NoAutoSave",
            -1,
            uno::Any(true),
            beans::PropertyState_DIRECT_VALUE);

        if ( bPluginMode )
        {
            aSeq.realloc( ++nLen );
            aSeq[nLen-1] = beans::PropertyValue(
                "PluginMode",
                -1,
                uno::Any((sal_Int16) 3),
                beans::PropertyState_DIRECT_VALUE);
        }

        aSeq.realloc( nLen+=2 );
        aSeq[nLen-2] = beans::PropertyValue(
            "InteractionHandler",
            -1,
            uno::Any(xHandler),
            beans::PropertyState_DIRECT_VALUE);

        aSeq[nLen-1] = beans::PropertyValue(
            "MacroExecutionMode",
            -1,
            uno::Any(m_nMacroExecMode),
            beans::PropertyState_DIRECT_VALUE);

        xComponentLoader->loadComponentFromURL(
            "private:object",
            "_self",
            0,
            aSeq);

        uno::Sequence< beans::PropertyValue > aResArgs = m_xDocument->getArgs();
        for ( int nInd = 0; nInd < aResArgs.getLength(); nInd++ )
            if ( aResArgs[nInd].Name == "MacroExecutionMode" )
            {
                aResArgs[nInd].Value >>= m_nMacroExecMode;
                break;
            }
    }
}

void DocumentHolder::OnPosRectChanged(LPRECT lpRect) const
{
    lpRect->left += m_aBorder.left;
    lpRect->right -= m_aBorder.right;
    lpRect->top += m_aBorder.top;
    lpRect->bottom -= m_aBorder.bottom;
    if(m_pIOleIPSite)
        m_pIOleIPSite->OnPosRectChange(lpRect);
}


void DocumentHolder::DisableInplaceActivation(BOOL b)
{
    m_bAllowInPlace = ! b;
}

BOOL DocumentHolder::isActive() const
{
    return m_pIOleIPSite != nullptr;
}

HRESULT DocumentHolder::InPlaceActivate(
    LPOLECLIENTSITE pActiveSite,
    BOOL fIncludeUI)
{
    m_bOnDeactivate = false;

    if(!m_bAllowInPlace)
        return ERROR;

    HRESULT                 hr;
    HWND                    hWndSite;
    RECT                    rcPos;
    RECT                    rcClip;
    OLEINPLACEFRAMEINFO     frameInfo;

    if (nullptr==pActiveSite)
        return ResultFromScode(E_INVALIDARG);

    if (nullptr!=m_pIOleIPSite)
    {
        if (fIncludeUI)
            UIActivate();

        return NOERROR;
    }

    if ( !m_xDocument.is() )
        return ERROR;

    //1.  Initialization, obtaining interfaces, OnInPlaceActivate.
    hr=pActiveSite->QueryInterface(
        IID_IOleInPlaceSite,
        reinterpret_cast<void**>(&m_pIOleIPSite));

    if (FAILED(hr))
        return hr;

    hr=m_pIOleIPSite->CanInPlaceActivate();

    if (NOERROR!=hr)
    {
        m_pIOleIPSite->Release();
        m_pIOleIPSite=nullptr;
        return ResultFromScode(E_FAIL);
    }

    m_pIOleIPSite->OnInPlaceActivate();

    //2. Get the site window
    //3. and determine container frame and
    //   document window for tools and menus, as well
    //   as frameInfo for accelerators
    m_pIOleIPSite->GetWindow(&hWndSite);

    frameInfo.cb=sizeof(OLEINPLACEFRAMEINFO);
    m_pIOleIPSite->GetWindowContext(
        &m_pIOleIPFrame,&m_pIOleIPUIWindow,&rcPos,&rcClip,&frameInfo);

    // initialize the office as, with hwnd as parentwindow
    uno::Any                      aAny;
    uno::Sequence<sal_Int8> aProcessIdent(16);
    rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8*>(aProcessIdent.getArray()));

    try
    {
        if(!m_xEditWindow.is())
        {   // determine XWindow and window handle of parent
            HWND                          hWndxWinParent(nullptr);
            uno::Reference<awt::XWindow>  xWin;

            uno::Reference<awt::XToolkit2> xToolkit =
                awt::Toolkit::create(comphelper::getComponentContext(m_xFactory));

            // create system window wrapper for hwnd
            if( !m_pCHatchWin )
                m_pCHatchWin = new winwrap::CHatchWin(
                    m_hInstance,this);

            if(m_pCHatchWin->Init(hWndSite,/*ID_HATCHWINDOW*/2000, nullptr)) {
                m_pCHatchWin->RectsSet(&rcPos,&rcClip); //set visible area
                hWndxWinParent = m_pCHatchWin->Window();
                ShowWindow(hWndxWinParent,SW_SHOW);  //Make visible.
            }
            else {
                // no success initializing hatch window
                delete m_pCHatchWin;
                m_pCHatchWin = nullptr;
                hWndxWinParent = hWndSite;
            }

            xWin.set(
                xToolkit->createSystemChild(
                    uno::Any(sal_Int32(hWndxWinParent)),
                    aProcessIdent,
                    lang::SystemDependent::SYSTEM_WIN32),
                uno::UNO_QUERY);

            if(xWin.is()) {
                xWin->setPosSize(
                    m_pCHatchWin ? HATCHWIN_BORDERWIDTHDEFAULT : 0,
                    m_pCHatchWin ? HATCHWIN_BORDERWIDTHDEFAULT : 0,
                    rcPos.right-rcPos.left,
                    rcPos.bottom - rcPos.top,
                    awt::PosSize::POSSIZE);
                xWin->setVisible(true);

                m_xEditWindow = xWin;
                m_hWndxWinParent = hWndxWinParent;
            }
            else
                return ERROR;
        }
        else {
            if(m_hWndxWinParent) {
                SetParent(m_hWndxWinParent,hWndSite);
                ShowWindow(m_hWndxWinParent,SW_SHOW);  //Make visible.
            }

            if ( !m_xFrame.is() )
                // initially set size to "empty", this guarantees that the final resize
                // is always executed (will be done by "SetObjectRects" after getting internal border)
                m_xEditWindow->setPosSize(
                    0,
                    0,
                    0,
                    0,
                    awt::PosSize::POSSIZE);
            m_xEditWindow->setVisible(true);
        }

        if(m_xContainerWindow.is()) {
            if(m_hWndxWinCont) {
                if(m_pIOleIPFrame) {
                    HWND  hWndCont;
                    m_pIOleIPFrame->GetWindow(&hWndCont);
                    SetParent(m_hWndxWinCont,hWndCont);
                    ShowWindow(m_hWndxWinCont,SW_SHOW);
                }
            }
            m_xContainerWindow->setVisible(true);
        }

        if(m_xFrame.is())
            m_xFrame->activate();
        else {
            // create frame and initialize it with the created window
            m_xFrame = frame::Frame::create( comphelper::getComponentContext(m_xFactory) );
            m_xFrame->initialize(m_xEditWindow);

            m_xFrame->registerDispatchProviderInterceptor( CreateNewInterceptor() );

            m_xLayoutManager.set( m_xFrame->getLayoutManager(), uno::UNO_QUERY );

            if(m_xLayoutManager.is())
                m_xLayoutManager->setDockingAreaAcceptor(this);

            // load the model into the frame
            LoadDocInFrame( true );

            uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(comphelper::getComponentContext(m_xFactory));
            xDesktop->getFrames()->append(m_xFrame);

            // determine the menuhandle to get menuitems.
            if(m_xLayoutManager.is()) {
                uno::Reference< css::ui::XUIElement > xUIEl(
                    m_xLayoutManager->getElement(
                        "private:resource/menubar/menubar"));
                OSL_ENSURE(xUIEl.is(),"no menubar");
                uno::Reference<awt::XSystemDependentMenuPeer> xSDMP(
                    xUIEl->getRealInterface(),
                    uno::UNO_QUERY);
                aAny = xSDMP->getMenuHandle(
                    aProcessIdent,lang::SystemDependent::SYSTEM_WIN32);
                sal_Int64 tmp;
                if( aAny >>= tmp )
                    m_nMenuHandle = HMENU(tmp);
                m_xLayoutManager->hideElement(
                    "private:resource/menubar/menubar" );
            }
        }

        // TODO/cd: Workaround for status indicator bug. It always makes the
        // document window visible, when someone tries to use the status
        // indicator. As we save our document when we get the deactivation
        // from OLE this conflict to hide floating windows.
        if(m_xLayoutManager.is())
            m_xLayoutManager->setVisible(true);

        // get document border and resize rects according to border
        GetDocumentBorder( &m_aBorder );
        SetObjectRects( &rcPos, &rcClip );

        if ( m_xOleAccess.is() )
        {
            LockedEmbedDocument_Impl aDocLock = m_xOleAccess->GetEmbedDocument();
            if ( aDocLock.GetEmbedDocument() )
                aDocLock.GetEmbedDocument()->ShowObject();
        }

        // setTitle(m_aDocumentNamePart);
        if (fIncludeUI)
            hr=UIActivate();

        m_pIOleIPSite->DiscardUndoState();
    }
    catch( const uno::Exception& )
    {
        hr = ERROR;
    }

    return hr;
}


void DocumentHolder::InPlaceDeactivate()
{
    m_bOnDeactivate = true;

    UIDeactivate();
    if(m_xFrame.is()) m_xFrame->deactivate();

    if(m_xEditWindow.is()) {
        m_xEditWindow->setVisible(false);
        ShowWindow(m_hWndxWinParent,SW_HIDE);
        SetParent(m_hWndxWinParent,nullptr);
    }

    if(m_xContainerWindow.is()) {
        m_xContainerWindow->setVisible(false);
        ShowWindow(m_hWndxWinCont,SW_HIDE);
        SetParent(m_hWndxWinCont,nullptr);
    }

    // TODO/cd: Workaround for status indicator bug. It always makes the
    // document window visible, when someone tries to use the status
    // indicator. As we save our document when we get the deactivation
    // from OLE this conflict to hide floating windows.
    if (m_xLayoutManager.is())
        m_xLayoutManager->setVisible(false);

    if (nullptr!=m_pIOleIPSite)
        m_pIOleIPSite->OnInPlaceDeactivate();

    if(m_pIOleIPFrame) m_pIOleIPFrame->Release(); m_pIOleIPFrame = nullptr;
    if(m_pIOleIPUIWindow) m_pIOleIPUIWindow->Release(); m_pIOleIPUIWindow = nullptr;
    if(m_pIOleIPSite) m_pIOleIPSite->Release(); m_pIOleIPSite = nullptr;

    if ( m_xOleAccess.is() )
    {
        LockedEmbedDocument_Impl aDocLock = m_xOleAccess->GetEmbedDocument();
        if ( aDocLock.GetEmbedDocument() )
        {
            aDocLock.GetEmbedDocument()->SaveObject();
        }
    }

    return;
}


HRESULT DocumentHolder::UIActivate()
{
    // 1.  Call IOleInPlaceSite::UIActivate
    if (nullptr!=m_pIOleIPSite)
        m_pIOleIPSite->OnUIActivate();

    //2.  Critical for accelerators to work initially.
    SetFocus(m_pCHatchWin->Window());
    // if(m_xEditWindow.is()) m_xEditWindow->setFocus();

    //3.  Set the active object

    OLECHAR starOffice[] = {'S','t','a','r','O','f','f','i','c','e',0};
    CComPtr< IOleInPlaceActiveObject > pObj = new CIIAObj( this );

    if (nullptr!=m_pIOleIPFrame)
        m_pIOleIPFrame->SetActiveObject(
            pObj, starOffice );

    if (nullptr!=m_pIOleIPUIWindow)
        m_pIOleIPUIWindow->SetActiveObject(
            pObj, starOffice );

    //4.  Create the shared menu.
    InPlaceMenuCreate();

    return NOERROR;
}

void DocumentHolder::UIDeactivate()
{
    //1.  Remove the shared menu.
    InPlaceMenuDestroy();

    if (nullptr!=m_pIOleIPFrame)
        m_pIOleIPFrame->SetActiveObject(nullptr, nullptr);

    if (nullptr!=m_pIOleIPUIWindow)
        m_pIOleIPUIWindow->SetActiveObject(nullptr, nullptr);

    //3.  Call IOleInPlaceSite::OnUIDeactivate
    if (nullptr!=m_pIOleIPSite)
        m_pIOleIPSite->OnUIDeactivate(FALSE);

    return;
}

void CopyToOLEMenu(HMENU hOrig,WORD origPos,HMENU hDest,WORD destPos)
{
    HMENU subMenu(nullptr);
    UINT uTemp = MF_BYPOSITION | MF_POPUP;
    char buffer[256];

    subMenu = GetSubMenu(hOrig,origPos);
    GetMenuString(hOrig,origPos,buffer,256,MF_BYPOSITION);
    InsertMenu(hDest,destPos,uTemp,
               reinterpret_cast<UINT_PTR>(subMenu),LPCTSTR(buffer));

    MENUITEMINFOW mi;
    memset(&mi,0,sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_DATA;
    if(GetMenuItemInfoW(hOrig,origPos,TRUE,&mi))
        SetMenuItemInfoW(hDest,(WORD)destPos,TRUE,&mi);
}

BOOL DocumentHolder::InPlaceMenuCreate()
{
    HMENU               hMenu;
    OLEMENUGROUPWIDTHS  mgw;

    for (UINT i=0; i<6; i++)
        mgw.width[i]=0;

    //We already have popup menu handles in m_pFR->m_phMenu[]

    //Create the new shared menu and let container do its thing
    hMenu=CreateMenu();
    m_pIOleIPFrame->InsertMenus(hMenu,&mgw);

    int count = GetMenuItemCount(m_nMenuHandle);
    int help = count-1;

    // start with 1, because we don't include "File"
    WORD pos = (WORD)mgw.width[0];
    CopyToOLEMenu(m_nMenuHandle,1,hMenu,pos);
    mgw.width[1] = 1;

    // insert object menu here
    pos = ((WORD)(mgw.width[0] + mgw.width[1] + mgw.width[2]));
    for(WORD i = 2; i < help-1; ++i,++pos)
        CopyToOLEMenu(m_nMenuHandle,i,hMenu,pos);
    mgw.width[3] = help - 3;

    // insert help menu
    pos = (WORD)(mgw.width[0] + mgw.width[1] + mgw.width[2] +
                 mgw.width[3] + mgw.width[4]);
    CopyToOLEMenu(m_nMenuHandle,WORD(help),hMenu,pos);
    mgw.width[5] = 1;

    m_nMenuShared = hMenu;
    m_nOLEMenu = OleCreateMenuDescriptor(m_nMenuShared,&mgw);

    uno::Reference<awt::XSystemDependentWindowPeer> xSysDepWin(m_xContainerWindow,uno::UNO_QUERY);
    if(xSysDepWin.is()) {
        uno::Sequence<sal_Int8> aProcessIdent(16);
        rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8*>(aProcessIdent.getArray()));
        uno::Any aAny = xSysDepWin->getWindowHandle(aProcessIdent,lang::SystemDependent::SYSTEM_WIN32);
        sal_Int64 tmp;
        aAny >>= tmp;
        HWND aHwnd = reinterpret_cast<HWND>(tmp);
        m_pIOleIPFrame->SetMenu(
            m_nMenuShared,m_nOLEMenu,aHwnd);
    }
    else
        m_pIOleIPFrame->SetMenu(
            m_nMenuShared,m_nOLEMenu,::GetWindow(m_hWndxWinParent,GW_CHILD));
    return TRUE;
}

BOOL DocumentHolder::InPlaceMenuDestroy()
{
    if( nullptr == m_nMenuShared )
        return TRUE;

    m_pIOleIPFrame->SetMenu(nullptr,nullptr,nullptr);

    OleDestroyMenuDescriptor(m_nOLEMenu);
    m_nOLEMenu = nullptr;
    return TRUE;
}

void DocumentHolder::OpenIntoWindow()
{
    // not implemented
}

BOOL DocumentHolder::Undo()
{
    // not implemented
    return false;
}


void DocumentHolder::FreeOffice()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(comphelper::getComponentContext(m_xFactory));
    xDesktop->removeTerminateListener(
        static_cast<frame::XTerminateListener*>(this) );
}

void DocumentHolder::DisconnectFrameDocument( bool bComplete )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xModifiable( m_xDocument, uno::UNO_QUERY_THROW );
        xModifiable->removeModifyListener( static_cast<util::XModifyListener*>(this) );
    }
    catch( const uno::Exception& )
    {}

    try
    {
        uno::Reference< util::XCloseBroadcaster > xBroadcaster(
            m_xDocument, uno::UNO_QUERY_THROW );
        xBroadcaster->removeCloseListener( static_cast<util::XCloseListener*>(this) );
    }
    catch( const uno::Exception& )
    {}

    try
    {
        uno::Reference< util::XCloseBroadcaster > xBroadcaster(
            m_xFrame, uno::UNO_QUERY_THROW );
        xBroadcaster->removeCloseListener( static_cast<util::XCloseListener*>(this) );
    }
    catch( const uno::Exception& )
    {}

    if ( bComplete )
    {
        m_xFrame.clear();
        m_pIDispatch = nullptr;
        m_xDocument.clear();
    }
}

void DocumentHolder::CloseDocument()
{
    DisconnectFrameDocument();

    uno::Reference< util::XCloseable > xCloseable(
            m_xDocument, uno::UNO_QUERY );

    if ( xCloseable.is() )
    {
        try
        {
            xCloseable->close( true );
        }
        catch( const uno::Exception& )
        {}
    }

    m_pIDispatch = nullptr;
    m_xDocument.clear();
}


void DocumentHolder::CloseFrame()
{
    try
    {
        uno::Reference< util::XCloseBroadcaster > xBroadcaster(
            m_xFrame, uno::UNO_QUERY_THROW );
        xBroadcaster->removeCloseListener( static_cast<util::XCloseListener*>(this) );
    }
    catch( const uno::Exception& )
    {}

    uno::Reference<util::XCloseable> xCloseable(
        m_xFrame,uno::UNO_QUERY);
    if(xCloseable.is())
        try {
            xCloseable->close(true);
        }
        catch( const uno::Exception& ) {
        }
    else {
        uno::Reference<lang::XComponent> xComp(m_xFrame, uno::UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }

    m_xFrame.clear();
}

void DocumentHolder::SetDocument( const uno::Reference< frame::XModel >& xDoc, bool bLink )
{
    if ( m_xDocument.is() )
        CloseDocument();

    m_xDocument = xDoc;
    m_bLink = bLink;

    uno::Reference< util::XCloseBroadcaster > xBroadcaster(
        m_xDocument, uno::UNO_QUERY );

    if ( xBroadcaster.is() )
        xBroadcaster->addCloseListener( static_cast<util::XCloseListener*>(this) );

    if ( m_xDocument.is() && !m_bLink )
    {
        // set the document mode to embedded
        uno::Sequence< beans::PropertyValue > aSeq(1);
        aSeq[0].Name = "SetEmbedded";
        aSeq[0].Value <<= true;
        m_xDocument->attachResource(OUString(),aSeq);
    }
}

bool DocumentHolder::ExecuteSuspendCloseFrame()
{
    if ( m_xFrame.is() && m_xFactory.is() )
    {
        try
        {
            uno::Reference< frame::XController > xController = m_xFrame->getController();
            if ( xController.is() )
            {
                if ( !xController->suspend( true ) )
                    return false;

                FreeOffice();
                try
                {
                    uno::Reference<util::XCloseable> xCloseable( m_xFrame, uno::UNO_QUERY );
                    if ( xCloseable.is() )
                        xCloseable->close(true);
                    else
                    {
                        m_xFrame->dispose();
                    }
                }
                catch( const util::CloseVetoException& )
                {
                    // should be called if the frame could not be closed
                    xController->suspend( false );
                }
            }
        }
        catch( uno::Exception& )
        {
        }

        m_xFrame.clear();
    }

    return true;
}

uno::Reference< frame::XFrame2 > DocumentHolder::DocumentFrame()
{
    if(! m_xFrame.is() )
    {
        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(comphelper::getComponentContext(m_xFactory));

        uno::Reference<frame::XFrame> xFrame(xDesktop,uno::UNO_QUERY);

        // the frame will be registered on desktop here, later when the document
        // is loaded into the frame in ::show() method the terminate listener will be removed
        // this is so only for outplace activation
        if( xFrame.is() )
            m_xFrame.set( xFrame->findFrame( "_blank", 0 ), uno::UNO_QUERY );

        uno::Reference< util::XCloseBroadcaster > xBroadcaster(
            m_xFrame, uno::UNO_QUERY );

        if ( xBroadcaster.is() )
        {
            xBroadcaster->addCloseListener( static_cast<util::XCloseListener*>(this) );
            FreeOffice(); // the frame is part of the desktop
        }
    }

    if( m_xFrame.is() )
    {
        // intercept
        m_xFrame->registerDispatchProviderInterceptor( CreateNewInterceptor() );
    }

    return m_xFrame;
}


uno::Reference< frame::XDispatchProviderInterceptor > DocumentHolder::CreateNewInterceptor()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ClearInterceptorInternally();

    uno::Reference< frame::XDispatchProviderInterceptor > xInterceptor( m_pInterceptor = new Interceptor( m_xOleAccess, this, m_bLink ) );
    m_xInterceptorLocker = xInterceptor;
    return xInterceptor;
}

void DocumentHolder::ClearInterceptorInternally()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    uno::Reference< frame::XDispatchProviderInterceptor > xInterceptor( m_xInterceptorLocker );
    if ( xInterceptor.is() && m_pInterceptor )
        m_pInterceptor->DisconnectDocHolder();

    m_xInterceptorLocker.clear();
    m_pInterceptor = nullptr;
}

void DocumentHolder::ClearInterceptor()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xInterceptorLocker.clear();
    m_pInterceptor = nullptr;
}


void DocumentHolder::show()
{
    try
    {
        if(m_xFrame.is())
        {
            m_xFrame->activate();
            uno::Reference<awt::XTopWindow> xTopWindow(
                m_xFrame->getContainerWindow(),uno::UNO_QUERY);
            if(xTopWindow.is())
                xTopWindow->toFront();
        }
        else if( DocumentFrame().is() )
        {
            LoadDocInFrame( false );

            // get rid of second closer if it is there
            uno::Reference< beans::XPropertySet > xLMProps( m_xFrame->getLayoutManager(), uno::UNO_QUERY );
            if ( xLMProps.is() )
            {
                xLMProps->setPropertyValue("MenuBarCloser",
                                            uno::makeAny( uno::Reference< frame::XStatusListener >() ) );
            }

            if ( !m_bLink )
            {
                try
                {
                    uno::Reference< util::XModifyBroadcaster > xModifiable( m_xDocument, uno::UNO_QUERY_THROW );
                    xModifiable->addModifyListener( static_cast<util::XModifyListener*>(this) );
                }
                catch( const uno::Exception& )
                {}
            }

            if ( !m_bLink )
                setTitle(m_aDocumentNamePart);
        }
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "Can not show the frame!\n" );
    }

}

void DocumentHolder::resizeWin( const SIZEL& rNewSize )
{
    LockedEmbedDocument_Impl aDocLock;

    if ( m_xOleAccess.is() )
        aDocLock = m_xOleAccess->GetEmbedDocument();

    if ( m_xFrame.is() && aDocLock.GetEmbedDocument() )
    {
        uno::Reference< awt::XWindow > xWindow(
            m_xFrame->getContainerWindow(), uno::UNO_QUERY );
        uno::Reference< awt::XView > xView( xWindow, uno::UNO_QUERY );

        if ( xWindow.is() && xView.is() )
        {
            float fScale = 1;
            xView->setZoom( fScale, fScale );

            SIZEL aOldSize;
            GetExtent( &aOldSize );

            if ( aOldSize.cx != rNewSize.cx || aOldSize.cy != rNewSize.cy )
            {
                HDC hdc = GetDC( nullptr );
                SetMapMode( hdc, MM_HIMETRIC );

                POINT aOldOffset;
                aOldOffset.x = aOldSize.cx;
                aOldOffset.y = aOldSize.cy;
                BOOL bIsOk = LPtoDP( hdc, &aOldOffset, 1 );

                POINT aNewOffset;
                aNewOffset.x = rNewSize.cx;
                aNewOffset.y = rNewSize.cy;
                bIsOk = LPtoDP( hdc, &aNewOffset, 1 );

                ReleaseDC( nullptr, hdc );

                awt::Rectangle aWinRect = xWindow->getPosSize();

                sal_Int32 aWidthDelta = aWinRect.Width - aOldOffset.x;
                sal_Int32 aHeightDelta = aWinRect.Height - aOldOffset.y;

                if ( aWidthDelta > 0 && aHeightDelta > 0 )
                    xWindow->setPosSize(0,
                                        0,
                                        aNewOffset.x + aWidthDelta,
                                        aNewOffset.y + aHeightDelta,
                                        awt::PosSize::SIZE );
            }
        }
    }
}

void DocumentHolder::setTitle(const OUString& aDocumentName)
{
    if(m_xFrame.is())
    {
        if(m_aFilterName.getLength() == 0)
        {
            OUString aFilterName;
            uno::Sequence<beans::PropertyValue> aSeq;
            if(m_xDocument.is())
            {
                aSeq =
                    m_xDocument->getArgs();
                for(sal_Int32 j = 0; j < aSeq.getLength(); ++j)
                {
                    if(aSeq[j].Name == "FilterName")
                    {
                        aSeq[j].Value >>= aFilterName;
                        break;
                    }
                }
            }

            if(aFilterName.getLength())
            {
                uno::Reference<container::XNameAccess> xNameAccess(
                    m_xFactory->createInstance("com.sun.star.document.FilterFactory"),
                    uno::UNO_QUERY);
                try {
                    if(xNameAccess.is() &&
                       (xNameAccess->getByName(aFilterName) >>= aSeq))
                    {
                        for(sal_Int32 j = 0; j < aSeq.getLength(); ++j)
                            if(aSeq[j].Name ==
                               "UIName")
                            {
                                aSeq[j].Value >>= m_aFilterName;
                                break;
                            }
                    }
                }
                catch(const uno::Exception& ) {
                    // nothing better to do here
                    m_aFilterName = aFilterName;
                }
            }
        }
        // set the title
        static const sal_Unicode u[] = { ' ','(',0 };
        static const sal_Unicode c[] = { ')',0 };
        rtl::OUString aTotalName(m_aFilterName);
        aTotalName += rtl::OUString(u);
        aTotalName += aDocumentName;
        aTotalName += rtl::OUString(c);
        try {
            m_xFrame->setTitle( aTotalName );
        }
        catch( const uno::Exception& ) {
        }
    }

    m_aDocumentNamePart = aDocumentName;

    if(m_pInterceptor)
    {
        ::osl::ClearableMutexGuard aGuard( m_aMutex );

        Interceptor* pTmpInter = nullptr;
        uno::Reference< frame::XDispatchProviderInterceptor > xLock( m_xInterceptorLocker );
        if ( xLock.is() && m_pInterceptor )
            pTmpInter = m_pInterceptor;

        aGuard.clear();

        if ( pTmpInter )
            pTmpInter->generateFeatureStateEvent();
    }
}


void DocumentHolder::setContainerName(const OUString& aContainerName)
{
    m_aContainerName = aContainerName;
}


void DocumentHolder::hide()
{
    if(m_xFrame.is()) m_xFrame->deactivate();

    //todo: sendadvise
    // after hiding the window it is always allowed to InPlaceActivate it
    m_bAllowInPlace = true;
}

IDispatch* DocumentHolder::GetIDispatch()
{
    if ( !m_pIDispatch && m_xDocument.is() )
    {
        const OUString aServiceName (
                "com.sun.star.bridge.OleBridgeSupplier2" );
        uno::Reference< bridge::XBridgeSupplier2 > xSupplier(
            m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );

        if ( xSupplier.is() )
        {
            uno::Sequence< sal_Int8 > aProcId( 16 );
            rtl_getGlobalProcessId( reinterpret_cast<sal_uInt8*>(aProcId.getArray()) );

            try {
                uno::Any anyResult = xSupplier->createBridge(
                    uno::makeAny( m_xDocument ),
                    aProcId,
                    bridge::ModelDependent::UNO,
                    bridge::ModelDependent::OLE );

                if ( auto var = o3tl::tryAccess<sal_uIntPtr>(anyResult) )
                {
                    VARIANT* pVariant = reinterpret_cast<VARIANT*>(*var);
                    if ( pVariant->vt == VT_DISPATCH )
                        m_pIDispatch = pVariant->pdispVal;

                    VariantClear( pVariant );
                    CoTaskMemFree( pVariant );
                }
            }
            catch ( const uno::Exception& )
            {}
        }
    }

    return m_pIDispatch;
}

HRESULT DocumentHolder::GetDocumentBorder( RECT *pRect )
{
    if ( pRect && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name == "DocumentBorder" )
            {
                uno::Sequence< sal_Int32 > aRect;
                if ( ( aArgs[nInd].Value >>= aRect ) && aRect.getLength() == 4 )
                {
                    pRect->left   = aRect[0];
                    pRect->top    = aRect[1];
                    pRect->right  = aRect[2];
                    pRect->bottom = aRect[3];

                    return S_OK;
                }

                break;
            }
    }

    return E_FAIL;
}

HRESULT DocumentHolder::SetExtent( const SIZEL *pSize )
{
    if ( pSize )
    {
        uno::Reference< embed::XVisualObject > xVisObj( m_xDocument, uno::UNO_QUERY );
        if ( xVisObj.is() )
        {
            try
            {
                awt::Size aNewSize( pSize->cx, pSize->cy );

                sal_Int32 aMapMode = xVisObj->getMapUnit( DVASPECT_CONTENT );

                // TODO/LATER: in future UNO API should be used for the conversion, currently there is no
                if ( aMapMode == embed::EmbedMapUnits::TWIP )
                {
                    // conversion from ONE_100TH_MM
                    aNewSize.Width = aNewSize.Width * 144 / 254;
                    aNewSize.Height = aNewSize.Height * 144 / 254;
                }


                xVisObj->setVisualAreaSize( DVASPECT_CONTENT, aNewSize );

                return S_OK;
            }
            catch( const uno::Exception& )
            {}
        }
    }

    return E_FAIL;
}

HRESULT DocumentHolder::GetExtent( SIZEL *pSize )
{
    if ( pSize )
    {
        uno::Reference< embed::XVisualObject > xVisObj( m_xDocument, uno::UNO_QUERY );
        if ( xVisObj.is() )
        {
            try
            {
                awt::Size aDocSize = xVisObj->getVisualAreaSize( DVASPECT_CONTENT );

                sal_Int32 aMapMode = xVisObj->getMapUnit( DVASPECT_CONTENT );

                // TODO/LATER: in future UNO API should be used for the conversion, currently there is no
                if ( aMapMode == embed::EmbedMapUnits::TWIP )
                {
                    // conversion to ONE_100TH_MM
                    aDocSize.Width = aDocSize.Width * 254 / 144;
                    aDocSize.Height = aDocSize.Height * 254 / 144;
                }

                pSize->cx = aDocSize.Width;
                pSize->cy = aDocSize.Height;

                return S_OK;
            }
            catch( const uno::Exception& )
            {}
        }
    }

    return E_FAIL;
}


HRESULT DocumentHolder::SetContRects(LPCRECT aRect)
{
    if(m_xContainerWindow.is()) {
        RECT wi;
        memset(&wi,0,sizeof(wi));
        if(m_pIOleIPFrame) {
            m_pIOleIPFrame->GetBorder(&wi);
            m_xContainerWindow->setPosSize(
                0,0,
                wi.right - wi.left,
                wi.bottom - wi.top,
                awt::PosSize::POSSIZE);
        }
        else
           m_xContainerWindow->setPosSize(
            0,0,
            aRect->right - aRect->left,
            aRect->bottom - aRect->top,
            awt::PosSize::POSSIZE);
        return NOERROR;
    }
    else {
        return ERROR;
    }
}


HRESULT DocumentHolder::SetObjectRects(LPCRECT aRect, LPCRECT aClip)
{
    const_cast<LPRECT>(aRect)->left -= m_aBorder.left;
    const_cast<LPRECT>(aRect)->right += m_aBorder.right;
    const_cast<LPRECT>(aRect)->top -= m_aBorder.top;
    const_cast<LPRECT>(aRect)->bottom += m_aBorder.bottom;
    const_cast<LPRECT>(aClip)->left -= m_aBorder.left;
    const_cast<LPRECT>(aClip)->right += m_aBorder.right;
    const_cast<LPRECT>(aClip)->top -= m_aBorder.top;
    const_cast<LPRECT>(aClip)->bottom += m_aBorder.bottom;

    if(m_pCHatchWin)
        m_pCHatchWin->RectsSet(const_cast<LPRECT>(aRect), const_cast<LPRECT>(aClip));
    if(m_xEditWindow.is()) {
        m_xEditWindow->setVisible(false);
        m_xEditWindow->setPosSize(
            m_pCHatchWin ? HATCHWIN_BORDERWIDTHDEFAULT : 0,
            m_pCHatchWin ? HATCHWIN_BORDERWIDTHDEFAULT : 0,
            aRect->right - aRect->left,
            aRect->bottom - aRect->top,
            awt::PosSize::POSSIZE);
        m_xEditWindow->setVisible(true);
    }
    return NOERROR;
}


css::uno::Reference< css::awt::XWindow> SAL_CALL DocumentHolder::getContainerWindow()
{
    if(m_xContainerWindow.is())
        return m_xContainerWindow;

    uno::Reference<awt::XWindow> xWin(nullptr);

    uno::Reference<awt::XToolkit2> xToolkit = awt::Toolkit::create( comphelper::getComponentContext(m_xFactory) );

    if(m_pIOleIPFrame) {
        HWND hWnd;
        m_pIOleIPFrame->GetWindow(&hWnd);

        uno::Sequence<sal_Int8> aProcessIdent(16);
        rtl_getGlobalProcessId(reinterpret_cast<sal_uInt8*>(aProcessIdent.getArray()));

        xWin.set(
            xToolkit->createSystemChild(
                uno::Any(sal_Int32(hWnd)),
                aProcessIdent,
                lang::SystemDependent::SYSTEM_WIN32),
            uno::UNO_QUERY);

        RECT wi;
        memset(&wi,0,sizeof(wi));
        if(xWin.is() && m_pIOleIPFrame->GetBorder(&wi) == NOERROR) {
            xWin->setVisible(true);
            xWin->setPosSize(
                0,0,
                wi.right-wi.left,
                wi.bottom - wi.top,
                awt::PosSize::POSSIZE);

            uno::Reference<awt::XSystemDependentWindowPeer> xSysWin(
                xWin,uno::UNO_QUERY);
            if(xSysWin.is()) {
                uno::Any aAny = xSysWin->getWindowHandle(
                    aProcessIdent,lang::SystemDependent::SYSTEM_WIN32);
                sal_Int64 tmp;
                if( aAny >>= tmp )
                    SetContainerWindowHandle(reinterpret_cast<HWND>(tmp));
            }
        }
    }

    m_xContainerWindow= xWin;
    return xWin;
}


sal_Bool SAL_CALL DocumentHolder::requestDockingAreaSpace( const css::awt::Rectangle& RequestedSpace )
{
    if(m_bOnDeactivate)
        return true;

    BORDERWIDTHS bw;
    SetRect(&bw,
            RequestedSpace.X,RequestedSpace.Y,
            RequestedSpace.Width,RequestedSpace.Height);
    if( m_pIOleIPFrame )
        return m_pIOleIPFrame->RequestBorderSpace(&bw) == NOERROR ;
    else
        return false;
}


void SAL_CALL DocumentHolder::setDockingAreaSpace( const css::awt::Rectangle& BorderSpace )
{
    if(m_bOnDeactivate)
        return;

    BORDERWIDTHS bw;
    SetRect(&bw,
            BorderSpace.X,BorderSpace.Y,
            BorderSpace.Width,BorderSpace.Height);
    if( m_pIOleIPFrame ) {
        RECT aRect;
        GetClientRect(m_hWndxWinCont,&aRect);
        HRGN hrgn1 = CreateRectRgn(
            0,0,
            aRect.right,BorderSpace.Y);
        HRGN hrgn2 = CreateRectRgn(aRect.right-BorderSpace.Width,0,aRect.right,aRect.bottom);
        CombineRgn(hrgn1,hrgn1,hrgn2,RGN_OR);
        DeleteObject(hrgn2);
        hrgn2 = CreateRectRgn(0,aRect.bottom-BorderSpace.Height,aRect.right,aRect.bottom);
        CombineRgn(hrgn1,hrgn1,hrgn2,RGN_OR);
        DeleteObject(hrgn2);
        hrgn2 = CreateRectRgn(0,0,BorderSpace.X,aRect.bottom);
        CombineRgn(hrgn1,hrgn1,hrgn2,RGN_OR);
        DeleteObject(hrgn2);

        SetWindowRgn(m_hWndxWinCont,hrgn1,true);
        // not:: DeleteObject(hrgn1);
        m_pIOleIPFrame->SetBorderSpace(&bw);
    }
}


void SAL_CALL DocumentHolder::disposing( const css::lang::EventObject& aSource )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_pIDispatch = nullptr;
        m_xDocument.clear();
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame.clear();
}


void SAL_CALL
DocumentHolder::queryClosing(
    const lang::EventObject& aSource,
    sal_Bool /*bGetsOwnership*/
)
{
    if (!m_bLink
        && ((m_xDocument.is() && m_xDocument == aSource.Source)
            || (m_xFrame.is() && m_xFrame == aSource.Source)))
        throw util::CloseVetoException();
}


void SAL_CALL
DocumentHolder::notifyClosing(
    const lang::EventObject& aSource )
{
    try
    {
        uno::Reference< util::XCloseBroadcaster > xEventBroadcaster(
            aSource.Source, uno::UNO_QUERY_THROW );
        xEventBroadcaster->removeCloseListener( static_cast<util::XCloseListener*>(this) );
    }
    catch( const uno::Exception& )
    {}

    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        // can happen only in case of links
        m_pIDispatch = nullptr;
        m_xDocument.clear();
        m_xFrame.clear();

        LockedEmbedDocument_Impl aDocLock = m_xOleAccess->GetEmbedDocument();
        if ( aDocLock.GetEmbedDocument() )
            aDocLock.GetEmbedDocument()->OLENotifyClosing();
    }
    else if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame.clear();
}

void SAL_CALL
DocumentHolder::queryTermination(
    const lang::EventObject& /*aSource*/
)
{
    if ( m_xDocument.is() )
        throw frame::TerminationVetoException();
}

void SAL_CALL
DocumentHolder::notifyTermination(
    const lang::EventObject& aSource
)
{
    OSL_ENSURE( !m_xDocument.is(), "Just a disaster..." );
    uno::Reference< frame::XDesktop > xDesktop(
        aSource.Source, uno::UNO_QUERY );

    if ( xDesktop.is() )
        xDesktop->removeTerminateListener( static_cast<frame::XTerminateListener*>(this) );
}


void SAL_CALL DocumentHolder::modified( const lang::EventObject& /*aEvent*/ )
{
    if ( m_xOleAccess.is() )
    {
        LockedEmbedDocument_Impl aDocLock = m_xOleAccess->GetEmbedDocument();
        if ( aDocLock.GetEmbedDocument() )
            aDocLock.GetEmbedDocument()->notify();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
