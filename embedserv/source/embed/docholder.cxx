/*************************************************************************
 *
 *  $RCSfile: docholder.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 11:25:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "xwin.hxx"
#include "docholder.hxx"
#include "embeddoc.hxx"
#include "intercept.hxx"
#include "syswinwrapper.hxx"
#include "iipaobj.hxx"
#include "common.h"
#include <Windows.h>

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _COM_SUN_STAR_LANG_SYSTEMDEPENDENT_HPP_
#include <com/sun/star/lang/SystemDependent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMCHILDFACTORY_HPP_
#include <com/sun/star/awt/XSystemChildFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTWINDOWPERR_HPP_
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTMENUPEER_HPP_
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENT_HPP_
#include <drafts/com/sun/star/ui/XUIElement.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseAble.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVIEW_HPP_
#include <com/sun/star/awt/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_XBRIDGESUPPLIER2_HPP_
#include <com/sun/star/bridge/XBridgeSupplier2.hpp>
#endif
#ifndef _COM_SUN_STAR_BRIDGE_MODELDEPENDENT_HPP_
#include <com/sun/star/bridge/ModelDependent.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

using namespace ::com::sun::star;

extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );

// add mutex locking ???

DocumentHolder::DocumentHolder(
    const uno::Reference<lang::XMultiServiceFactory >& xFactory,
    EmbedDocument_Impl *pOLEInterface)
    :
    m_bAllowInPlace(true),
    m_pIOleIPSite(0),
    m_pIOleIPFrame(0),
    m_pIOleIPUIWindow(0),
    m_pCHatchWin(0),
    m_pOLEInterface(pOLEInterface),
    m_pInterceptor(0),
    // will be released
    m_pImpIOleIPActiveObject(new CIIAObj(pOLEInterface,this)),
    m_xFactory( xFactory ),
    m_bOnDeactivate(false),
    m_hWndxWinParent(NULL),
    m_hWndxWinCont(NULL),
    m_nMenuHandle(NULL),
    m_nMenuShared(NULL),
    m_nOLEMenu(NULL)
{
    static const ::rtl::OUString aServiceName (
        RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop(
        m_xFactory->createInstance( aServiceName ),
        uno::UNO_QUERY );
    if ( xDesktop.is() )
        xDesktop->addTerminateListener( (frame::XTerminateListener*)this );
}


DocumentHolder::~DocumentHolder()
{
    if ( m_xDocument.is() )
        CloseDocument();

    if( m_xFrame.is() )
        CloseFrame();

    if ( m_xFactory.is() )
        FreeOffice();

    delete m_pCHatchWin;
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
    return m_pIOleIPSite != 0;
}


#define MENUBARVISIBLE     6661
#define TOOLBARVISIBLE     5909
#define OBJECTBARVISIBLE   5905
#define STATUSBARVISIBLE   5920
#define FUNCTIONBARVISIBLE 5910
#define RULERVISIBLE      20211

static void disable_slot(
    uno::Reference<frame::XDispatchProvider>&
    xDispatchProvider,
    sal_Int32 id)
{
    util::URL aURL;
    aURL.Path = ::rtl::OUString::valueOf(id);
    aURL.Complete =
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("slot:")) +
        aURL.Path;
    aURL.Main = aURL.Complete;
    aURL.Protocol = ::rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("slot:"));

    uno::Reference<frame::XDispatch> xDispatch(
        xDispatchProvider->queryDispatch(
            aURL,
            ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("_self")),
            0));

    if(xDispatch.is()) {
        uno::Sequence<beans::PropertyValue> aSeq(1);

        switch(id) {
            case MENUBARVISIBLE:
                aSeq[0].Name = ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("MenuBarVisible"));
                break;
            case TOOLBARVISIBLE:
                aSeq[0].Name = ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("ToolBarVisible"));
                break;
            case OBJECTBARVISIBLE:
                aSeq[0].Name = ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("ObjectBarVisible"));
                break;
            case STATUSBARVISIBLE:
                aSeq[0].Name = ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("StatusBarVisible"));
                break;
            case FUNCTIONBARVISIBLE:
                aSeq[0].Name = ::rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("FunctionBarVisible"));
                break;
            case RULERVISIBLE:
                aSeq.realloc(0);
            default:
                break;
        }

        sal_Bool val(false);
        aSeq[0].Value <<= val;
        xDispatch->dispatch(aURL,aSeq);
    }
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

    if (NULL==pActiveSite)
        return ResultFromScode(E_INVALIDARG);

    if (NULL!=m_pIOleIPSite)
    {
        if (fIncludeUI)
            UIActivate();

        return NOERROR;
    }

    //1.  Initialization, obtaining interfaces, OnInPlaceActivate.
    hr=pActiveSite->QueryInterface(
        IID_IOleInPlaceSite,
        (void**) &m_pIOleIPSite);

    if (FAILED(hr))
        return hr;

    hr=m_pIOleIPSite->CanInPlaceActivate();

    if (NOERROR!=hr)
    {
        m_pIOleIPSite->Release(), m_pIOleIPSite=NULL;
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
    rtl_getGlobalProcessId((sal_uInt8*)aProcessIdent.getArray());

    if(!m_xEditWindow.is())
    {   // determine XWindow and window handle of parent
        HWND                          hWndxWinParent;
        uno::Reference<awt::XWindow>  xWin;

        static const ::rtl::OUString aToolkitServiceName(
            RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.awt.Toolkit" ) );
        uno::Reference<awt::XSystemChildFactory> xToolkit(
            m_xFactory->createInstance(aToolkitServiceName ),uno::UNO_QUERY);

        if(xToolkit.is()) {
            // create system window wrapper for hwnd
            if( !m_pCHatchWin )
                m_pCHatchWin = new winwrap::CHatchWin(
                    m_hInstance,this);

            if(m_pCHatchWin->Init(hWndSite,/*ID_HATCHWINDOW*/2000, NULL)) {
                m_pCHatchWin->RectsSet(&rcPos,&rcClip); //set visible area
                hWndxWinParent = m_pCHatchWin->Window();
                ShowWindow(hWndxWinParent,SW_SHOW);  //Make visible.
            }
            else {
                // no success initializing hatch window
                delete m_pCHatchWin, m_pCHatchWin = 0;
                hWndxWinParent = hWndSite;
            }

            aAny <<= sal_Int32(hWndxWinParent);
            xWin = uno::Reference<awt::XWindow>(
                xToolkit->createSystemChild(
                    aAny,
                    aProcessIdent,
                    lang::SystemDependent::SYSTEM_WIN32),
                uno::UNO_QUERY);
        }

        if(xWin.is()) {
            xWin->setPosSize(
                m_pCHatchWin ? HATCHWIN_BORDERWIDTHDEFAULT : 0,
                m_pCHatchWin ? HATCHWIN_BORDERWIDTHDEFAULT : 0,
                rcPos.right-rcPos.left,
                rcPos.bottom - rcPos.top,
                awt::PosSize::POSSIZE);
            xWin->setVisible(sal_True);

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
        m_xEditWindow->setVisible(sal_True);
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
        // create frame and initialize it with with the created window
        static const ::rtl::OUString aFrameServiceName(
            RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Frame" ) );
        m_xFrame = uno::Reference<frame::XFrame>(
            m_xFactory->createInstance(aFrameServiceName),
            uno::UNO_QUERY);

        if(!m_xFrame.is())
            return ERROR;

        m_xFrame->initialize(m_xEditWindow);

        uno::Reference<frame::XDispatchProviderInterception>
            xDPI(m_xFrame,uno::UNO_QUERY);
        if(xDPI.is())
            xDPI->registerDispatchProviderInterceptor(
                m_pInterceptor = new Interceptor(m_pOLEInterface,this));

        uno::Reference<beans::XPropertySet> xPS(m_xFrame,uno::UNO_QUERY);
        if( xPS.is() )
        {
            aAny = xPS->getPropertyValue(
                rtl::OUString::createFromAscii("LayoutManager"));
            aAny >>= m_xLayoutManager;
        }

        if(m_xLayoutManager.is())
            m_xLayoutManager->setDockingAreaAcceptor(this);

        // load the model into the frame
        uno::Reference<frame::XComponentLoader> xComponentLoader(
            m_xFrame,uno::UNO_QUERY);
        if(xComponentLoader.is())
        {
            uno::Any aAny;
            uno::Sequence<beans::PropertyValue> aSeq(3);

            aAny <<= uno::Reference<uno::XInterface>(
                GetDocument(),uno::UNO_QUERY);
            aSeq[0] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Model")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            aAny <<= sal_False;
            aSeq[1] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("ReadOnly")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            aAny <<= (sal_Int16) 3;
            aSeq[2] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("PluginMode")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            xComponentLoader->loadComponentFromURL(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("private:object")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
                0,
                aSeq);
        }

        static const ::rtl::OUString aDesktopServiceName (
            RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
        uno::Reference< frame::XFramesSupplier > xDesktop(
            m_xFactory->createInstance( aDesktopServiceName ),
            uno::UNO_QUERY );
        if(xDesktop.is())
            xDesktop->getFrames()->append(m_xFrame);

        // determine the menuhandle to get menutitems.
        if(m_xLayoutManager.is()) {
            uno::Reference<drafts::com::sun::star::ui::XUIElement> xUIEl(
                m_xLayoutManager->getElement(
                    rtl::OUString::createFromAscii(
                        "private:resource/menubar/menubar")));
            OSL_ENSURE(xUIEl.is(),"no menubar");
            uno::Reference<awt::XSystemDependentMenuPeer> xSDMP(
                xUIEl->getRealInterface(),
                uno::UNO_QUERY);
            aAny = xSDMP->getMenuHandle(
                aProcessIdent,lang::SystemDependent::SYSTEM_WIN32);
            sal_Int32 tmp;
            if( aAny >>= tmp )
                m_nMenuHandle = HMENU(tmp);
            m_xLayoutManager->hideElement(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "private:resource/menubar/menubar" )));
        }
    }

    // get document border and resize rects according to border
    GetDocumentBorder( &m_aBorder );
    SetObjectRects( &rcPos, &rcClip );

    m_pOLEInterface->ShowObject();

    // setTitle(m_aDocumentNamePart);
    if (fIncludeUI)
        hr=UIActivate();

    m_pIOleIPSite->DiscardUndoState();
    return hr;
}


void DocumentHolder::InPlaceDeactivate(void)
{
    m_bOnDeactivate = true;

    UIDeactivate();
    if(m_xFrame.is()) m_xFrame->deactivate();

    if(m_xEditWindow.is()) {
        m_xEditWindow->setVisible(false);
        ShowWindow(m_hWndxWinParent,SW_HIDE);
        SetParent(m_hWndxWinParent,0);
    }

    if(m_xContainerWindow.is()) {
        m_xContainerWindow->setVisible(false);
        ShowWindow(m_hWndxWinCont,SW_HIDE);
        SetParent(m_hWndxWinCont,0);
    }

    if (NULL!=m_pIOleIPSite)
        m_pIOleIPSite->OnInPlaceDeactivate();

    if(m_pIOleIPFrame) m_pIOleIPFrame->Release(); m_pIOleIPFrame = 0;
    if(m_pIOleIPUIWindow) m_pIOleIPUIWindow->Release(); m_pIOleIPUIWindow = 0;
    if(m_pIOleIPSite) m_pIOleIPSite->Release(); m_pIOleIPSite = 0;

    m_pOLEInterface->SaveObject();
    return;
}


HRESULT DocumentHolder::UIActivate()
{
    // 1.  Call IOleInPlaceSite::UIActivate
    if (NULL!=m_pIOleIPSite)
        m_pIOleIPSite->OnUIActivate();

    //2.  Critical for accelerators to work initially.
    SetFocus(m_pCHatchWin->Window());
    // if(m_xEditWindow.is()) m_xEditWindow->setFocus();

    //3.  Set the active object

    OLECHAR starOffice[] = {'S','t','a','r','O','f','f','i','c','e',0};

    if (NULL!=m_pIOleIPFrame)
        m_pIOleIPFrame->SetActiveObject(
            m_pImpIOleIPActiveObject,starOffice);

    if (NULL!=m_pIOleIPUIWindow)
        m_pIOleIPUIWindow->SetActiveObject(
            m_pImpIOleIPActiveObject,starOffice);

    //4.  Create the shared menu.
    InPlaceMenuCreate();

    return NOERROR;
}

void DocumentHolder::UIDeactivate()
{
    //1.  Remove the shared menu.
    InPlaceMenuDestroy();

    if (NULL!=m_pIOleIPFrame)
        m_pIOleIPFrame->SetActiveObject(NULL, NULL);

    if (NULL!=m_pIOleIPUIWindow)
        m_pIOleIPUIWindow->SetActiveObject(NULL, NULL);

    //3.  Call IOleInPlaceSite::OnUIDeactivate
    if (NULL!=m_pIOleIPSite)
        m_pIOleIPSite->OnUIDeactivate(FALSE);

    return;
}

void CopyToOLEMenu(HMENU hOrig,WORD origPos,HMENU hDest,WORD destPos)
{
    HMENU subMenu(NULL);
    UINT uTemp = MF_BYPOSITION | MF_POPUP;
    char buffer[256];

    subMenu = GetSubMenu(hOrig,origPos);
    GetMenuString(hOrig,origPos,buffer,256,MF_BYPOSITION);
    InsertMenu(hDest,destPos,uTemp,
               (UINT)subMenu,LPCTSTR(buffer));

    MENUITEMINFOW mi;
    memset(&mi,0,sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_DATA;
    if(GetMenuItemInfoW(hOrig,origPos,TRUE,&mi))
        SetMenuItemInfoW(hDest,(WORD)destPos,TRUE,&mi);
}

BOOL DocumentHolder::InPlaceMenuCreate(void)
{
    HMENU               hMenu;
    UINT                uTemp = MF_BYPOSITION | MF_POPUP;
    UINT                i;
    OLEMENUGROUPWIDTHS  mgw;

    for (i=0; i<6; i++)
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
    for(int i = 2; i < help-1; ++i,++pos)
        CopyToOLEMenu(m_nMenuHandle,i,hMenu,pos);
    mgw.width[3] = help - 3;

    // insert help menu
    pos = (WORD)(mgw.width[0] + mgw.width[1] + mgw.width[2] +
                 mgw.width[3] + mgw.width[4]);
    CopyToOLEMenu(m_nMenuHandle,help,hMenu,pos);
    mgw.width[5] = 1;

    m_nMenuShared = hMenu;
    m_nOLEMenu = OleCreateMenuDescriptor(m_nMenuShared,&mgw);

    uno::Reference<awt::XSystemDependentWindowPeer> xSysDepWin(m_xContainerWindow,uno::UNO_QUERY);
    if(xSysDepWin.is()) {
        uno::Sequence<sal_Int8> aProcessIdent(16);
        rtl_getGlobalProcessId((sal_uInt8*)aProcessIdent.getArray());
        uno::Any aAny = xSysDepWin->getWindowHandle(aProcessIdent,lang::SystemDependent::SYSTEM_WIN32);
        sal_Int32 tmp;
        aAny >>= tmp;
        HWND aHwnd = (HWND) tmp;
        m_pIOleIPFrame->SetMenu(
            m_nMenuShared,m_nOLEMenu,aHwnd);
    }
    else
        m_pIOleIPFrame->SetMenu(
            m_nMenuShared,m_nOLEMenu,::GetWindow(m_hWndxWinParent,GW_CHILD));
    return TRUE;
}

BOOL DocumentHolder::InPlaceMenuDestroy(void)
{
    if( NULL == m_nMenuShared )
        return TRUE;

    m_pIOleIPFrame->SetMenu(NULL,NULL,NULL);

    OleDestroyMenuDescriptor(m_nOLEMenu),m_nOLEMenu = NULL;
    return TRUE;
}

void DocumentHolder::OpenIntoWindow(void)
{
    // not implemented
}

BOOL DocumentHolder::Undo(void)
{
    // not implemented
    return false;
}


void DocumentHolder::FreeOffice()
{
    const ::rtl::OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.frame.Desktop" ) );
    uno::Reference< frame::XDesktop > xDesktop(
        m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );
    if ( xDesktop.is() )
    {
        xDesktop->removeTerminateListener(
            (frame::XTerminateListener*)this );

        uno::Reference< frame::XFramesSupplier > xFramesSupplier(
            xDesktop, uno::UNO_QUERY );
        if ( xFramesSupplier.is() )
        {
            uno::Reference< frame::XFrames > xFrames(
                xFramesSupplier->getFrames());

            if ( xFrames.is() && !xFrames->hasElements() )
            {
                try
                {
                    xDesktop->terminate();
                }
                catch( uno::Exception & )
                {}
            }
        }

        m_xFactory = uno::Reference< lang::XMultiServiceFactory >();
    }
}


void DocumentHolder::CloseDocument()
{
    uno::Reference< util::XCloseBroadcaster > xBroadcaster(
        m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
    {
        xBroadcaster->removeCloseListener( (util::XCloseListener*)this );

        uno::Reference< util::XCloseable > xCloseable(
            xBroadcaster, uno::UNO_QUERY );

        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {}
        }
    }

    m_pIDispatch = NULL;
    m_xDocument = uno::Reference< frame::XModel >();
}


void DocumentHolder::CloseFrame()
{
    uno::Reference<util::XCloseable> xCloseable(
        m_xFrame,uno::UNO_QUERY);
    if(xCloseable.is())
        try {
            xCloseable->close(sal_True);
        }
        catch( const uno::Exception& ) {
        }
    else {
        uno::Reference<lang::XComponent> xComp(m_xFrame,uno::UNO_QUERY);
        if(xComp.is())
            xComp->dispose();
    }

    m_xFrame = uno::Reference< frame::XFrame >();
}


void DocumentHolder::SetDocument( const uno::Reference< frame::XModel >& xDoc)
{
    if ( m_xDocument.is() )
        CloseDocument();

    m_xDocument = xDoc;

    uno::Reference< util::XCloseBroadcaster > xBroadcaster(
        m_xDocument, uno::UNO_QUERY );

    if ( xBroadcaster.is() )
        xBroadcaster->addCloseListener( (util::XCloseListener*)this );

    if(m_xDocument.is())
    {
        // set the document mode to embedded
        uno::Sequence< beans::PropertyValue > aSeq(1);
        aSeq[0].Name = ::rtl::OUString::createFromAscii( "SetEmbedded" );
        aSeq[0].Value <<= sal_True;
        m_xDocument->attachResource(::rtl::OUString(),aSeq);
    }
}


uno::Reference< frame::XFrame > DocumentHolder::DocumentFrame()
{
    if(! m_xFrame.is() )
    {
        rtl::OUString aDesktopSrvNm(
            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"));

        uno::Reference<frame::XDesktop> xDesktop(
            m_xFactory->createInstance(aDesktopSrvNm),
            uno::UNO_QUERY);

        uno::Reference<frame::XFrame> xFrame(
            xDesktop,uno::UNO_QUERY);

        if( xFrame.is() )
            m_xFrame = xFrame->findFrame(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank")),0);
    }

    if( m_xFrame.is() )
    {
        // intercept
        uno::Reference<frame::XDispatchProviderInterception>
            xDPI(m_xFrame,uno::UNO_QUERY);
        if(xDPI.is())
            xDPI->registerDispatchProviderInterceptor(
                m_pInterceptor = new Interceptor(m_pOLEInterface,this));
    }

    return m_xFrame;
}


void DocumentHolder::ClearInterceptor()
{
    m_pInterceptor = 0;
}


void DocumentHolder::show()
{
    if(m_xFrame.is()) {
        m_xFrame->activate();
        uno::Reference<awt::XTopWindow> xTopWindow(
            m_xFrame->getContainerWindow(),uno::UNO_QUERY);
        if(xTopWindow.is())
            xTopWindow->toFront();
    }
    else {
        uno::Reference<frame::XComponentLoader> xComponentLoader(
            DocumentFrame(),uno::UNO_QUERY);

        if(xComponentLoader.is())
        {
            uno::Sequence<beans::PropertyValue> aSeq(2);

            uno::Any aAny;
            aAny <<= uno::Reference<uno::XInterface>(
                GetDocument(),uno::UNO_QUERY);
            aSeq[0] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("Model")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            aAny <<= sal_False;
            aSeq[1] = beans::PropertyValue(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("ReadOnly")),
                -1,
                aAny,
                beans::PropertyState_DIRECT_VALUE);

            xComponentLoader->loadComponentFromURL(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("private:object")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
                0,
                aSeq);
        }
        setTitle(m_aDocumentNamePart);
    }
}

void DocumentHolder::resizeWin( const SIZEL& rNewSize )
{
    if ( m_xFrame.is() && m_pOLEInterface )
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
                HDC hdc = GetDC( NULL );
                SetMapMode( hdc, MM_HIMETRIC );

                POINT aOldOffset;
                aOldOffset.x = aOldSize.cx;
                aOldOffset.y = aOldSize.cy;
                BOOL bIsOk = LPtoDP( hdc, &aOldOffset, 1 );

                POINT aNewOffset;
                aNewOffset.x = rNewSize.cx;
                aNewOffset.y = rNewSize.cy;
                bIsOk = LPtoDP( hdc, &aNewOffset, 1 );

                ReleaseDC( NULL, hdc );

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

void DocumentHolder::setTitle(const rtl::OUString& aDocumentName)
{
    if(m_xFrame.is())
    {
        if(m_aFilterName.getLength() == 0)
        {
            rtl::OUString aFilterName;
            uno::Sequence<beans::PropertyValue> aSeq;
            if(m_xDocument.is())
            {
                aSeq =
                    m_xDocument->getArgs();
                for(sal_Int32 j = 0; j < aSeq.getLength(); ++j)
                {
                    if(aSeq[j].Name ==
                       rtl::OUString(
                           RTL_CONSTASCII_USTRINGPARAM("FilterName")))
                    {
                        aSeq[j].Value >>= aFilterName;
                        break;
                    }
                }
            }

            if(aFilterName.getLength())
            {
                uno::Reference<container::XNameAccess> xNameAccess(
                    m_xFactory->createInstance(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.document.FilterFactory"))),
                    uno::UNO_QUERY);
                try {
                    if(xNameAccess.is() &&
                       (xNameAccess->getByName(aFilterName) >>= aSeq))
                    {
                        for(sal_Int32 j = 0; j < aSeq.getLength(); ++j)
                            if(aSeq[j].Name ==
                               rtl::OUString(
                                   RTL_CONSTASCII_USTRINGPARAM("UIName")))
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
        uno::Reference<beans::XPropertySet> xPropSet(
            m_xFrame,uno::UNO_QUERY);
        if(xPropSet.is()) {
            uno::Any aAny;
            static const sal_Unicode u[] = { ' ','(',0 };
            static const sal_Unicode c[] = { ')',0 };
            rtl::OUString aTotalName(m_aFilterName);
            aTotalName += rtl::OUString(u);
            aTotalName += aDocumentName;
            aTotalName += rtl::OUString(c);
            aAny <<= aTotalName;
            try {
                xPropSet->setPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
                    aAny);
            }
            catch( const uno::Exception& ) {
            }
        }
    }

    m_aDocumentNamePart = aDocumentName;

    if(m_pInterceptor)
        m_pInterceptor->generateFeatureStateEvent();
}


void DocumentHolder::setContainerName(const rtl::OUString& aContainerName)
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
        const ::rtl::OUString aServiceName (
            RTL_CONSTASCII_USTRINGPARAM (
                "com.sun.star.bridge.OleBridgeSupplier2" ) );
        uno::Reference< bridge::XBridgeSupplier2 > xSupplier(
            m_xFactory->createInstance( aServiceName ), uno::UNO_QUERY );

        if ( xSupplier.is() )
        {
            uno::Sequence< sal_Int8 > aProcId( 16 );
            rtl_getGlobalProcessId( (sal_uInt8*)aProcId.getArray() );

            try {
                uno::Any anyResult = xSupplier->createBridge(
                    uno::makeAny( m_xDocument ),
                    aProcId,
                    bridge::ModelDependent::UNO,
                    bridge::ModelDependent::OLE );

                if ( anyResult.getValueTypeClass() ==
                     getCppuType((sal_uInt32*) 0).getTypeClass() )
                {
                    VARIANT* pVariant = *(VARIANT**)anyResult.getValue();
                    if ( pVariant->vt == VT_DISPATCH )
                        m_pIDispatch = pVariant->pdispVal;

                    VariantClear( pVariant );
                    CoTaskMemFree( pVariant );
                }
            }
            catch ( uno::Exception& )
            {}
        }
    }

    return m_pIDispatch;
}


HRESULT DocumentHolder::SetVisArea( const RECTL *pRect )
{
    if ( pRect && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // should allways be there
                uno::Sequence< sal_Int32 > aRect(4);

                aRect[0] = pRect->left;
                aRect[1] = pRect->top;
                aRect[2] = pRect->right;
                aRect[3] = pRect->bottom;

                aArgs[nInd].Value <<= aRect;

                m_xDocument->attachResource( m_xDocument->getURL(), aArgs );
                return S_OK;
            }

        OSL_ENSURE( sal_False, "WinExtent seems not to be implemented!\n" );
    }

    return E_FAIL;
}

HRESULT DocumentHolder::GetVisArea( RECTL *pRect )
{
    if ( pRect && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
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

HRESULT DocumentHolder::GetDocumentBorder( RECT *pRect )
{
    if ( pRect && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "DocumentBorder" ) )
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
    if ( pSize && m_xDocument.is() )
    {
        uno::Sequence< beans::PropertyValue > aArgs = m_xDocument->getArgs();
        for ( sal_Int32 nInd = 0; nInd < aArgs.getLength(); nInd++ )
            if ( aArgs[nInd].Name.equalsAscii( "WinExtent" ) )
            {
                // should allways be there
                uno::Sequence< sal_Int32 > aRect;
                if( ( aArgs[nInd].Value >>= aRect ) && aRect.getLength() == 4 )
                {
                    aRect[2] = aRect[0] + pSize->cx; // right = left + cx
                    aRect[3] = aRect[1] + pSize->cy; // bottom = top + cy

                    aArgs[nInd].Value <<= aRect;

                    m_xDocument->attachResource( m_xDocument->getURL(), aArgs );
                    return S_OK;
                }
            }

        OSL_ENSURE( sal_False, "WinExtent seems not to be implemented!\n" );
    }

    return E_FAIL;
}

HRESULT DocumentHolder::GetExtent( SIZEL *pSize )
{
    RECTL aRect;
    if ( pSize && SUCCEEDED( GetVisArea( &aRect ) ) )
    {
        pSize->cx = aRect.right - aRect.left;
        pSize->cy = aRect.top - aRect.bottom;

        return S_OK;
    }

    return E_FAIL;
}


HRESULT DocumentHolder::SetContRects(LPCRECT aRect)
{
    if(m_xContainerWindow.is()) {
        RECT wi;
        memset(&wi,0,sizeof(wi));
        if(m_pIOleIPFrame) {
            m_pIOleIPFrame->GetBorder((LPRECT)&wi);
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
    ((LPRECT)aRect)->left -= m_aBorder.left;
    ((LPRECT)aRect)->right += m_aBorder.right;
    ((LPRECT)aRect)->top -= m_aBorder.top;
    ((LPRECT)aRect)->bottom += m_aBorder.bottom;
    ((LPRECT)aClip)->left -= m_aBorder.left;
    ((LPRECT)aClip)->right += m_aBorder.right;
    ((LPRECT)aClip)->top -= m_aBorder.top;
    ((LPRECT)aClip)->bottom += m_aBorder.bottom;

    if(m_pCHatchWin)
        m_pCHatchWin->RectsSet((LPRECT)aRect, (LPRECT)aClip);
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


::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XWindow> SAL_CALL
DocumentHolder::getContainerWindow(
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    if(m_xContainerWindow.is())
        return m_xContainerWindow;

    uno::Reference<awt::XWindow> xWin(0);

    static const ::rtl::OUString aToolkitServiceName(
        RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.awt.Toolkit" ) );
    uno::Reference<awt::XSystemChildFactory> xToolkit(
        m_xFactory->createInstance(aToolkitServiceName ),uno::UNO_QUERY);

    if(xToolkit.is() && m_pIOleIPFrame) {
        HWND hWnd;
        m_pIOleIPFrame->GetWindow(&hWnd);

        uno::Sequence<sal_Int8> aProcessIdent(16);
        rtl_getGlobalProcessId((sal_uInt8*)aProcessIdent.getArray());

        uno::Any aAny;
        aAny <<= sal_Int32(hWnd);
        xWin = uno::Reference<awt::XWindow>(
            xToolkit->createSystemChild(
                aAny,
                aProcessIdent,
                lang::SystemDependent::SYSTEM_WIN32),
            uno::UNO_QUERY);

        RECT wi;
        memset(&wi,0,sizeof(wi));
        if(xWin.is() && m_pIOleIPFrame->GetBorder((LPRECT)&wi) == NOERROR) {
            xWin->setVisible(true);
            xWin->setPosSize(
                0,0,
                wi.right-wi.left,
                wi.bottom - wi.top,
                awt::PosSize::POSSIZE);

            uno::Reference<awt::XSystemDependentWindowPeer> xSysWin(
                xWin,uno::UNO_QUERY);
            if(xSysWin.is()) {
                aAny = xSysWin->getWindowHandle(
                    aProcessIdent,lang::SystemDependent::SYSTEM_WIN32);
                sal_Int32 tmp;
                if( aAny >>= tmp )
                    SetContainerWindowHandle((HWND) tmp);
            }
        }
    }

    m_xContainerWindow= xWin;
    return xWin;
}



sal_Bool SAL_CALL
DocumentHolder::requestDockingAreaSpace(
    const ::com::sun::star::awt::Rectangle& RequestedSpace
)
    throw(
        ::com::sun::star::uno::RuntimeException
    )
{
    if(m_bOnDeactivate)
        return sal_True;

    BORDERWIDTHS bw;
    SetRect((LPRECT)&bw,
            RequestedSpace.X,RequestedSpace.Y,
            RequestedSpace.Width,RequestedSpace.Height);
    if( m_pIOleIPFrame )
        return m_pIOleIPFrame->RequestBorderSpace(&bw) == NOERROR ;
    else
        return sal_Bool(false);
}


void SAL_CALL
DocumentHolder::setDockingAreaSpace(
    const ::com::sun::star::awt::Rectangle& BorderSpace
)
    throw (
        ::com::sun::star::uno::RuntimeException
    )
{
    if(m_bOnDeactivate)
        return;

    BORDERWIDTHS bw;
    SetRect((LPRECT)&bw,
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


void SAL_CALL
DocumentHolder::disposing(
    const com::sun::star::lang::EventObject& aSource
)
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_pIDispatch = NULL;
        m_xDocument = uno::Reference< frame::XModel >();
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame = uno::Reference< frame::XFrame >();
}


void SAL_CALL
DocumentHolder::queryClosing(
    const lang::EventObject& aSource,
    sal_Bool bGetsOwnership
)
    throw(
        util::CloseVetoException
    )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
        throw util::CloseVetoException();
}


void SAL_CALL
DocumentHolder::notifyClosing(
    const lang::EventObject& aSource )
{
    uno::Reference< util::XCloseBroadcaster > xEventBroadcaster(
        aSource.Source, uno::UNO_QUERY );
    if ( xEventBroadcaster.is() )
        xEventBroadcaster->removeCloseListener( (util::XCloseListener*)this );

    if ( m_xDocument.is() && m_xDocument == aSource.Source )
    {
        m_pIDispatch = NULL;
        m_xDocument = uno::Reference< frame::XModel >();
    }

    if( m_xFrame.is() && m_xFrame == aSource.Source )
        m_xFrame = uno::Reference< frame::XFrame >();
}

void SAL_CALL
DocumentHolder::queryTermination(
    const lang::EventObject& aSource
)
    throw(
        frame::TerminationVetoException
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
        xDesktop->removeTerminateListener( (frame::XTerminateListener*)this );
}



void SAL_CALL DocumentHolder::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    if(m_pOLEInterface)
        m_pOLEInterface->notify();
}



//     if(m_pOLEInterface->GetGUID() == OID_WriterTextServer) {
//         // edit group
//         CopyToOLEMenu(m_nMenuHandle,1,hMenu,(WORD)mgw.width[0]);
//         CopyToOLEMenu(m_nMenuHandle,2,hMenu,1+(WORD)mgw.width[0]);
//         CopyToOLEMenu(m_nMenuHandle,3,hMenu,2+(WORD)mgw.width[0]);
//         CopyToOLEMenu(m_nMenuHandle,4,hMenu,3+(WORD)mgw.width[0]);
//         mgw.width[1]=4;

//         // object group
//         CopyToOLEMenu(
//             m_nMenuHandle,5,
//             hMenu,4+(WORD)mgw.width[0]+(WORD)mgw.width[2]);
//         mgw.width[3]=1;

//         // help group
//         CopyToOLEMenu(
//             m_nMenuHandle,7,
//             hMenu,5+(WORD)mgw.width[0]+(WORD)mgw.width[2]+(WORD)mgw.width[4]);
//         mgw.width[5]=1;
//     }
//     else if(m_pOLEInterface->GetGUID() == OID_CalcServer) {
//         // edit group
//         CopyToOLEMenu(m_nMenuHandle,1,hMenu,(WORD)mgw.width[0]);
//         CopyToOLEMenu(m_nMenuHandle,2,hMenu,1+(WORD)mgw.width[0]);
//         CopyToOLEMenu(m_nMenuHandle,3,hMenu,2+(WORD)mgw.width[0]);
//         CopyToOLEMenu(m_nMenuHandle,4,hMenu,3+(WORD)mgw.width[0]);
//         mgw.width[1]=4;

//         // object group
//         CopyToOLEMenu(
//             m_nMenuHandle,5,
//             hMenu,4+(WORD)mgw.width[0]+(WORD)mgw.width[2]);
//         CopyToOLEMenu(
//             m_nMenuHandle,6,
//             hMenu,5+(WORD)mgw.width[0]+(WORD)mgw.width[2]);
//         mgw.width[3]=2;

//         // help group
//         CopyToOLEMenu(
//             m_nMenuHandle,8,
//             hMenu,6+(WORD)mgw.width[0]+(WORD)mgw.width[2]+(WORD)mgw.width[4]);
//         mgw.width[5]=1;
//     }
