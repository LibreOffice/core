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

#include <cassert>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <oleacc.h>
#include <AccObjectWinManager.hxx>
#include <AccEventListener.hxx>
#include <AccComponentEventListener.hxx>
#include <AccContainerEventListener.hxx>
#include <AccDialogEventListener.hxx>
#include <AccWindowEventListener.hxx>
#include <AccFrameEventListener.hxx>
#include <AccMenuEventListener.hxx>
#include <AccObjectContainerEventListener.hxx>
#include <AccParagraphEventListener.hxx>
#include <AccTextComponentEventListener.hxx>
#include <AccListEventListener.hxx>
#include <AccTreeEventListener.hxx>
#include <AccTableEventListener.hxx>
#include <AccObject.hxx>
#include <unomsaaevent.hxx>


using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

AccObjectWinManager::AccObjectWinManager():
        oldFocus(nullptr)
{
}

/**
   * Destructor,clear all resource.
   * @param
   * @return
   */
AccObjectWinManager::~AccObjectWinManager()
{
    {
        std::scoped_lock l(m_Mutex);

        XIdAccList.clear();
        HwndXAcc.clear();
    }
    XResIdAccList.clear();
    XHWNDDocList.clear();
}


/**
   * Get valid com object interface when notifying some MSAA event
   * @param pWND The top window handle that contains that event control.
   * @param wParam Windows system interface.
   * @return Com interface with event.
   */

sal_Int64
AccObjectWinManager::Get_ToATInterface(sal_Int64 nHWnd, long lParam, WPARAM wParam)
{
    IMAccessible* pRetIMAcc = nullptr;

    if(lParam == OBJID_CLIENT )
    {
        HWND hWnd = reinterpret_cast<HWND>(nHWnd);
        pRetIMAcc = GetTopWindowIMAccessible(hWnd);
    }

    if ( pRetIMAcc && lParam == OBJID_CLIENT )
    {
        LRESULT result = LresultFromObject(IID_IAccessible, wParam, pRetIMAcc);
        pRetIMAcc->Release();
        return static_cast<sal_Int64>(result);
    }
    return 0;
}

/**
   * Search AccObject by XAccessible pointer from our container.
   * @param pXAcc XAccessible interface.
   * @return Pointer of accObject that is found.
   */
AccObject* AccObjectWinManager::GetAccObjByXAcc( XAccessible* pXAcc)
{
    if( pXAcc == nullptr)
        return nullptr;

    std::scoped_lock l(m_Mutex);

    XIdToAccObjHash::iterator pIndTemp = XIdAccList.find( pXAcc );
    if ( pIndTemp == XIdAccList.end() )
        return nullptr;

    return &(pIndTemp->second);
}

/**
   * get acc object of top window by its handle
   * @param hWnd, top window handle
   * @return pointer to AccObject
   */
IMAccessible * AccObjectWinManager::GetTopWindowIMAccessible(HWND hWnd)
{
    std::scoped_lock l(m_Mutex); // tdf#155794 for HwndXAcc and XIdAccList

    XHWNDToXAccHash::iterator iterResult =HwndXAcc.find(hWnd);
    if(iterResult == HwndXAcc.end())
        return nullptr;
    XAccessible* pXAcc = iterResult->second;
    AccObject *const pAccObject(GetAccObjByXAcc(pXAcc));
    if (!pAccObject)
    {
        return nullptr;
    }
    IMAccessible *const pRet(pAccObject->GetIMAccessible());
    if (!pRet)
    {
        return nullptr;
    }
    pRet->AddRef();
    return pRet;
}

/**
   * Simulate MSAA event via XAccessible interface and event type.
   * @param pXAcc XAccessible interface.
   * @param eEvent event type
   * @return The terminate result that identifies if the call is successful.
   */
bool AccObjectWinManager::NotifyAccEvent(XAccessible* pXAcc, UnoMSAAEvent eEvent)
{
    Reference< XAccessibleContext > pRContext;

    if( pXAcc == nullptr)
        return false;


    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return false;


    AccObject* selfAccObj= GetAccObjByXAcc(pXAcc);

    if(selfAccObj==nullptr)
        return false;

    long dChildID = selfAccObj->GetResID();
    HWND hAcc = selfAccObj->GetParentHWND();

    switch(eEvent)
    {
    case UnoMSAAEvent::STATE_FOCUSED:
        {
            UpdateAccFocus(pXAcc);
            selfAccObj->UpdateDefaultAction( );
            UpdateValue(pXAcc);
            NotifyWinEvent( EVENT_OBJECT_FOCUS,hAcc, OBJID_CLIENT,dChildID  );
            break;
        }
    case UnoMSAAEvent::STATE_BUSY:
        NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::STATE_CHECKED:
        NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::STATE_PRESSED:
        NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;

    //Removed fire out selected event
    //case UnoMSAAEvent::STATE_SELECTED:
    //  NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
    //  break;
    case UnoMSAAEvent::STATE_ARMED:
        UpdateAccFocus(pXAcc);
        NotifyWinEvent( EVENT_OBJECT_FOCUS,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::STATE_SHOWING:
        // send EVENT_SYSTEM_ALERT when notification gets shown
        if (pRContext->getAccessibleRole() == AccessibleRole::NOTIFICATION)
            NotifyWinEvent(EVENT_SYSTEM_ALERT, hAcc, OBJID_CLIENT, dChildID);
        break;
    case UnoMSAAEvent::MENU_START:
        NotifyWinEvent( EVENT_SYSTEM_MENUSTART,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::MENU_END:
        NotifyWinEvent( EVENT_SYSTEM_MENUEND,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::MENUPOPUPSTART:
        NotifyWinEvent( EVENT_SYSTEM_MENUPOPUPSTART,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::MENUPOPUPEND:
        NotifyWinEvent( EVENT_SYSTEM_MENUPOPUPEND,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::SELECTION_CHANGED:
        NotifyWinEvent( EVENT_OBJECT_SELECTION,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::SELECTION_CHANGED_ADD:
        NotifyWinEvent( EVENT_OBJECT_SELECTIONADD,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::SELECTION_CHANGED_REMOVE:
        NotifyWinEvent( EVENT_OBJECT_SELECTIONREMOVE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::SELECTION_CHANGED_WITHIN:
        NotifyWinEvent( EVENT_OBJECT_SELECTIONWITHIN,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_VALUECHANGE:
        UpdateValue(pXAcc);
        NotifyWinEvent( EVENT_OBJECT_VALUECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_NAMECHANGE:
        NotifyWinEvent( EVENT_OBJECT_NAMECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_DESCRIPTIONCHANGE:
        NotifyWinEvent( EVENT_OBJECT_DESCRIPTIONCHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_DEFACTIONCHANGE:
        NotifyWinEvent( IA2_EVENT_ACTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_CARETCHANGE:
        NotifyWinEvent( IA2_EVENT_TEXT_CARET_MOVED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_TEXTCHANGE:
        NotifyWinEvent( IA2_EVENT_TEXT_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::ACTIVE_DESCENDANT_CHANGED:
        UpdateAccFocus(pXAcc);
        NotifyWinEvent( EVENT_OBJECT_FOCUS,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::BOUNDRECT_CHANGED:
        NotifyWinEvent( EVENT_OBJECT_LOCATIONCHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::VISIBLE_DATA_CHANGED:
        NotifyWinEvent( IA2_EVENT_VISIBLE_DATA_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::SHOW :
        NotifyWinEvent( EVENT_OBJECT_SHOW,hAcc, OBJID_CLIENT,dChildID  );
        NotifyWinEvent( EVENT_SYSTEM_FOREGROUND,hAcc, OBJID_CLIENT,dChildID  );
    break;
    case UnoMSAAEvent::TABLE_CAPTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_CAPTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TABLE_COLUMN_DESCRIPTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_COLUMN_DESCRIPTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TABLE_COLUMN_HEADER_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_COLUMN_HEADER_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TABLE_MODEL_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_MODEL_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TABLE_ROW_HEADER_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_ROW_HEADER_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TABLE_SUMMARY_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_SUMMARY_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TABLE_ROW_DESCRIPTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_ROW_DESCRIPTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_REORDER:
        NotifyWinEvent( EVENT_OBJECT_REORDER,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::PAGE_CHANGED:
        NotifyWinEvent( IA2_EVENT_PAGE_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::CHILD_REMOVED:
        NotifyWinEvent( EVENT_OBJECT_DESTROY,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::CHILD_ADDED:
        NotifyWinEvent( EVENT_OBJECT_CREATE ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::OBJECT_PAGECHANGED:
        NotifyWinEvent( IA2_EVENT_PAGE_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::TEXT_SELECTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TEXT_SELECTION_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::SECTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_SECTION_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UnoMSAAEvent::COLUMN_CHANGED:
        NotifyWinEvent( IA2_EVENT_TEXT_COLUMN_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    default:
        break;
    }

    return true;
}

/**
   * Get Parent XAccessible interface by XAccessible interface.
   * @param pXAcc XAccessible interface.
   * @return Parent XAccessible interface.
   */
XAccessible* AccObjectWinManager::GetParentXAccessible( XAccessible* pXAcc )
{
    AccObject* pObj= GetAccObjByXAcc(pXAcc);
    if( pObj ==nullptr )
        return nullptr;
    if(pObj->GetParentObj())
    {
        pObj = pObj->GetParentObj();
        return pObj->GetXAccessible().get();
    }
    return nullptr;
}

/**
   * Get Parent role by XAccessible interface.
   * @param pXAcc XAccessible interface.
   * @return Parent role.
   */
short AccObjectWinManager::GetParentRole( XAccessible* pXAcc )
{
    AccObject* pObj= GetAccObjByXAcc(pXAcc);
    if( pObj ==nullptr )
        return -1;
    if(pObj->GetParentObj())
    {
        pObj = pObj->GetParentObj();
        if(pObj->GetXAccessible().is())
        {
            Reference< XAccessibleContext > pRContext = pObj->GetXAccessible()->getAccessibleContext();
            if(pRContext.is())
                return pRContext->getAccessibleRole();
        }
    }
    return -1;
}

/**
   * Update focus object by new focused XAccessible interface.
   * @param newFocus New XAccessible interface that gets focus.
   * @return
   */
void AccObjectWinManager::UpdateAccFocus(XAccessible* newFocus)
{
    AccObject* pAccObjNew = GetAccObjByXAcc(newFocus);
    if(pAccObjNew)
    {
        AccObject* pAccObjOld = GetAccObjByXAcc(oldFocus);
        oldFocus = newFocus;
        pAccObjNew->setFocus();
        //if old == new, the pAccObjNew will be without focused state
        if (pAccObjOld && pAccObjOld != pAccObjNew)
            pAccObjOld->unsetFocus();
    }
}

/**
   * Delete child element from children list.
   * @param pObj Child element that should be removed from parent child list.
   * @return
   */
void AccObjectWinManager::DeleteAccChildNode( AccObject* pObj )
{
    AccObject *parentAccObj = pObj->GetParentObj();
    if( parentAccObj )
        parentAccObj->DeleteChild( pObj );
}

/**
   * Delete XAccessible items in top window handle hashtable
   * @param pXAcc XAccessible interface.
   * @return
   */
void AccObjectWinManager::DeleteFromHwndXAcc(XAccessible const * pXAcc )
{
    std::scoped_lock l(m_Mutex);

    auto iter = std::find_if(HwndXAcc.begin(), HwndXAcc.end(),
        [&pXAcc](XHWNDToXAccHash::value_type& rEntry) { return rEntry.second == pXAcc; });
    if (iter != HwndXAcc.end())
        HwndXAcc.erase(iter);
}

/**
   * Delete all children with the tree root of XAccessible pointer
   * @param pXAcc Tree root XAccessible interface.
   * @return
   */
void AccObjectWinManager::DeleteChildrenAccObj(XAccessible* pXAcc)
{
    AccObject* currentObj=nullptr;
    AccObject* childObj=nullptr;

    currentObj =  GetAccObjByXAcc( pXAcc);
    if(currentObj)
    {
        childObj = currentObj->NextChild();
        while(childObj)
        {
            XAccessible *const pTmpXAcc = childObj->GetXAccessible().get();
            if(pTmpXAcc)
            {
                DeleteChildrenAccObj(pTmpXAcc);
                DeleteAccObj(pTmpXAcc);
            }
            childObj = currentObj->NextChild();
        }
    }
}

/**
   * Delete Acc object self.
   * @param pXAcc The XAccessible interface.
   * @return
   */
void AccObjectWinManager::DeleteAccObj( XAccessible* pXAcc )
{
    if( pXAcc == nullptr )
        return;

    rtl::Reference<AccEventListener> pListener;

    {
        std::scoped_lock l(m_Mutex);

        XIdToAccObjHash::iterator temp = XIdAccList.find(pXAcc);
        if( temp != XIdAccList.end() )
        {
            ResIdGen.SetSub( temp->second.GetResID() );
        }
        else
        {
            return;
        }

        AccObject& accObj = temp->second;
        DeleteAccChildNode( &accObj );
        pListener = DeleteAccListener(&accObj);
        accObj.NotifyDestroy();
        if( accObj.GetIMAccessible() )
        {
            accObj.GetIMAccessible()->Release();
        }
        size_t i = XResIdAccList.erase(accObj.GetResID());
        assert(i != 0);
        (void) i;
        DeleteFromHwndXAcc(pXAcc);
        if (accObj.GetRole() == AccessibleRole::DOCUMENT ||
            accObj.GetRole() == AccessibleRole::DOCUMENT_PRESENTATION ||
            accObj.GetRole() == AccessibleRole::DOCUMENT_SPREADSHEET ||
            accObj.GetRole() == AccessibleRole::DOCUMENT_TEXT)
        {
            XHWNDDocList.erase(accObj.GetParentHWND());
        }
        XIdAccList.erase(pXAcc); // note: this invalidates accObj so do it last!
    }
    if (pListener)
    {
        pListener->RemoveMeFromBroadcaster(false);
    }
}

/**
   * Delete listener that inspects some XAccessible object
   * @param pAccObj Accobject pointer.
   * @return
   */
rtl::Reference<AccEventListener> AccObjectWinManager::DeleteAccListener( AccObject*  pAccObj )
{
    return pAccObj->SetListener(nullptr);
}

/**
   * Generate a child ID, which is used for AT
   * @param
   * @return New resource ID.
   */
inline long AccObjectWinManager::ImpleGenerateResID()
{
    return ResIdGen.GenerateNewResID();
}

/**
   * Insert all children of the current acc object
   * @param pXAcc XAccessible interface
   * @param pWnd  Top Window handle
   * @return The calling result.
   */
bool AccObjectWinManager::InsertChildrenAccObj( css::accessibility::XAccessible* pXAcc,
        HWND pWnd)
{
    if(!IsContainer(pXAcc))
        return false;

    Reference< XAccessibleContext > pRContext;

    if( pXAcc == nullptr)
        return false;
    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return false;

    short role = pRContext->getAccessibleRole();

    if(css::accessibility::AccessibleRole::DOCUMENT == role ||
            css::accessibility::AccessibleRole::DOCUMENT_PRESENTATION == role ||
            css::accessibility::AccessibleRole::DOCUMENT_SPREADSHEET == role ||
            css::accessibility::AccessibleRole::DOCUMENT_TEXT == role)
    {
        if(IsStateManageDescendant(pXAcc))
        {
            return true;
        }
    }

    const sal_Int64 nCount = pRContext->getAccessibleChildCount();
    for (sal_Int64 i = 0; i < nCount; i++)
    {
        Reference<XAccessible> mxAccessible
        = pRContext->getAccessibleChild(i);
        XAccessible* mpAccessible = mxAccessible.get();
        if(mpAccessible != nullptr)
        {
            InsertAccObj( mpAccessible,pXAcc,pWnd );
            InsertChildrenAccObj(mpAccessible,pWnd);
        }
    }

    return true;
}

/**
   * Insert child object.
   * @param pCurObj The child object
   * @param pParentObj The parent object
   * @param pWnd Top window handle.
   * @return
   */
void AccObjectWinManager::InsertAccChildNode( AccObject* pCurObj, AccObject* pParentObj, HWND /* pWnd */ )
{
    if(pCurObj)
    {
        if(pParentObj)
        {
            pParentObj->InsertChild(pCurObj);
        }
        else
        {
            pCurObj->UpdateValidWindow();
        }
    }
}

/**
   * Insert child object.
   * @param pCurObj The child object
   * @param pParentObj The parent object
   * @param pWnd Top window handle.
   * @return
   */
bool AccObjectWinManager::InsertAccObj( XAccessible* pXAcc,XAccessible* pParentXAcc,HWND pWnd )
{
    Reference< XAccessibleContext > pRContext;

    if( pXAcc == nullptr)
        return false;

    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return false;

    {
        short nCurRole = GetRole(pXAcc);

        std::scoped_lock l(m_Mutex);

        XIdToAccObjHash::iterator itXacc = XIdAccList.find( pXAcc );
        if (itXacc != XIdAccList.end() )
        {
            if (AccessibleRole::SHAPE == nCurRole)
            {
                AccObject &objXacc = itXacc->second;
                AccObject *pObjParent = objXacc.GetParentObj();
                if (pObjParent &&
                        pObjParent->GetXAccessible().is() &&
                        pObjParent->GetXAccessible().get() != pParentXAcc)
                {
                    XIdToAccObjHash::iterator itXaccParent  = XIdAccList.find( pParentXAcc );
                    if(itXaccParent != XIdAccList.end())
                    {
                        objXacc.SetParentObj(&(itXaccParent->second));
                    }
                }
            }
            return false;
        }
    }

    if( pWnd == nullptr )
    {
        if(pParentXAcc)
        {
            AccObject* pObj = GetAccObjByXAcc(pParentXAcc);
            if(pObj)
                pWnd = pObj->GetParentHWND();
        }
        if( pWnd == nullptr )
            return false;
    }

    AccObject pObj(pXAcc, this);
    if( pObj.GetIMAccessible() == nullptr )
        return false;
    pObj.SetResID( this->ImpleGenerateResID());
    pObj.SetParentHWND( pWnd );

    //for file name support
    if (pObj.GetRole() == AccessibleRole::DOCUMENT ||
        pObj.GetRole() == AccessibleRole::DOCUMENT_PRESENTATION ||
        pObj.GetRole() == AccessibleRole::DOCUMENT_SPREADSHEET ||
        pObj.GetRole() == AccessibleRole::DOCUMENT_TEXT)
    {
        XHWNDToDocumentHash::iterator aIter = XHWNDDocList.find(pWnd);
        if ( aIter != XHWNDDocList.end() )
        {
            XHWNDDocList.erase( aIter );
        }
        XHWNDDocList.emplace( pWnd, pXAcc );
    }
    //end of file name

    ::rtl::Reference<AccEventListener> const pListener =
        CreateAccEventListener(pXAcc);
    if (!pListener.is())
        return false;
    Reference<XAccessibleComponent> xComponent(pRContext,UNO_QUERY);
    Reference<XAccessibleEventBroadcaster> broadcaster(xComponent,UNO_QUERY);
    if (broadcaster.is())
    {
        Reference<XAccessibleEventListener> const xListener(pListener);
        broadcaster->addAccessibleEventListener(xListener);
    }
    else
        return false;

    {
        std::scoped_lock l(m_Mutex);

        XIdAccList.emplace(pXAcc, pObj);
        XIdToAccObjHash::iterator pIndTemp = XIdAccList.find( pXAcc );
        XResIdAccList.emplace(pObj.GetResID(),&(pIndTemp->second));
    }

    AccObject* pCurObj = GetAccObjByXAcc(pXAcc);
    if( pCurObj )
    {
        pCurObj->SetListener(pListener);
    }

    AccObject* pParentObj = GetAccObjByXAcc(pParentXAcc);
    InsertAccChildNode(pCurObj,pParentObj,pWnd);
    if( pCurObj )
        pCurObj->UpdateAccessibleInfoFromUnoToMSAA();
    return true;
}


/**
   * save the pair <topwindowhandle, XAccessible>
   * @param hWnd, top window handle
   * @param pXAcc XAccessible interface for top window
   * @return void
   */
void AccObjectWinManager::SaveTopWindowHandle(HWND hWnd, css::accessibility::XAccessible* pXAcc)
{
    std::scoped_lock l(m_Mutex);

    HwndXAcc.emplace(hWnd,pXAcc);
}


/** Create the corresponding listener.
 *  @param pXAcc XAccessible interface.
 */
::rtl::Reference<AccEventListener>
AccObjectWinManager::CreateAccEventListener(XAccessible* pXAcc)
{
    ::rtl::Reference<AccEventListener> pRet;
    Reference<XAccessibleContext> xContext = pXAcc->getAccessibleContext();
    if(xContext.is())
    {
        switch( xContext->getAccessibleRole() )
        {
        case AccessibleRole::DIALOG:
            pRet = new AccDialogEventListener(pXAcc, *this);
            break;
        case AccessibleRole::FRAME:
            pRet = new AccFrameEventListener(pXAcc, *this);
            break;
        case AccessibleRole::WINDOW:
            pRet = new AccWindowEventListener(pXAcc, *this);
            break;
        case AccessibleRole::ROOT_PANE:
            pRet = new AccFrameEventListener(pXAcc, *this);
            break;
            //Container
        case AccessibleRole::CANVAS:
        case AccessibleRole::COMBO_BOX:
        case AccessibleRole::DOCUMENT:
        case AccessibleRole::DOCUMENT_PRESENTATION:
        case AccessibleRole::DOCUMENT_SPREADSHEET:
        case AccessibleRole::DOCUMENT_TEXT:
        case AccessibleRole::END_NOTE:
        case AccessibleRole::FILLER:
        case AccessibleRole::FOOTNOTE:
        case AccessibleRole::FOOTER:
        case AccessibleRole::HEADER:
        case AccessibleRole::LAYERED_PANE:
        case AccessibleRole::MENU_BAR:
        case AccessibleRole::POPUP_MENU:
        case AccessibleRole::OPTION_PANE:
        case AccessibleRole::PAGE_TAB:
        case AccessibleRole::PAGE_TAB_LIST:
        case AccessibleRole::PANEL:
        case AccessibleRole::SCROLL_PANE:
        case AccessibleRole::SPLIT_PANE:
        case AccessibleRole::STATUS_BAR:
        case AccessibleRole::TABLE_CELL:
        case AccessibleRole::TOOL_BAR:
        case AccessibleRole::VIEW_PORT:
            pRet = new AccContainerEventListener(pXAcc, *this);
            break;
        case AccessibleRole::BLOCK_QUOTE:
        case AccessibleRole::PARAGRAPH:
        case AccessibleRole::HEADING:
            pRet = new AccParagraphEventListener(pXAcc, *this);
            break;
            //Component
        case AccessibleRole::CHECK_BOX:
        case AccessibleRole::ICON:
        case AccessibleRole::LABEL:
        case AccessibleRole::STATIC:
        case AccessibleRole::NOTIFICATION:
        case AccessibleRole::MENU_ITEM:
        case AccessibleRole::CHECK_MENU_ITEM:
        case AccessibleRole::RADIO_MENU_ITEM:
        case AccessibleRole::PUSH_BUTTON:
        case AccessibleRole::RADIO_BUTTON:
        case AccessibleRole::SCROLL_BAR:
        case AccessibleRole::SEPARATOR:
        case AccessibleRole::TOGGLE_BUTTON:
        case AccessibleRole::BUTTON_DROPDOWN:
        case AccessibleRole::TOOL_TIP:
        case AccessibleRole::SPIN_BOX:
        case AccessibleRole::DATE_EDITOR:
            pRet = new AccComponentEventListener(pXAcc, *this);
            break;
            //text component
        case AccessibleRole::TEXT:
            pRet = new AccTextComponentEventListener(pXAcc, *this);
            break;
            //menu
        case AccessibleRole::MENU:
            pRet = new AccMenuEventListener(pXAcc, *this);
            break;
            //object container
        case AccessibleRole::SHAPE:

        case AccessibleRole::EMBEDDED_OBJECT:
        case AccessibleRole::GRAPHIC:
        case AccessibleRole::TEXT_FRAME:
            pRet = new AccObjectContainerEventListener(pXAcc, *this);
            break;
            //descendmanager
        case AccessibleRole::LIST:
            pRet = new AccListEventListener(pXAcc, *this);
            break;
        case AccessibleRole::TREE:
            pRet = new AccTreeEventListener(pXAcc, *this);
            break;
            //special
        case AccessibleRole::COLUMN_HEADER:
        case AccessibleRole::TABLE:
            pRet = new AccTableEventListener(pXAcc, *this);
            break;
        default:
            pRet = new AccContainerEventListener(pXAcc, *this);
            break;
        }
    }
    return pRet;
}

/**
   * state is a combination integer, each bit of which represents a single state,
   * such as focused,1 for the state on,0 for the state off. Here call COM interface
   * to modify the state value, including DecreaseState.
   * @param pXAcc XAccessible interface.
   * @param pState Changed state.
   * @return
   */
void AccObjectWinManager::DecreaseState(XAccessible* pXAcc, sal_Int64 nState)
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->DecreaseState(nState);
}

/**
   * state is a combination integer, each bit of which represents a single state,such as focused,1 for
   * the state on,0 for the state off. Here call COM interface to modify the state value, including
   * IncreaseState.
   * @param pXAcc XAccessible interface.
   * @param pState Changed state.
   * @return
   */
void AccObjectWinManager::IncreaseState(XAccessible* pXAcc, sal_Int64 nState)
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->IncreaseState(nState);
}

void  AccObjectWinManager::UpdateState( css::accessibility::XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->UpdateState( );
}

/**
   * Set corresponding com object's accessible name via XAccessible interface and new
   * name
   * @param pXAcc XAccessible interface.
   * @return
   */
void  AccObjectWinManager::UpdateAccName( XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->UpdateName();
}

void  AccObjectWinManager::UpdateAction( XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->UpdateAction();
}

/**
   * Set corresponding com object's value  via XAccessible interface and new value.
   * @param pXAcc XAccessible interface.
   * @param pAny new value.
   * @return
   */
void  AccObjectWinManager::SetValue( XAccessible* pXAcc, Any pAny )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->SetValue( pAny );
}

/**
   * Set corresponding com object's value  via XAccessible interface.
   * @param pXAcc XAccessible interface.
   * @return
   */
void  AccObjectWinManager::UpdateValue( XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->UpdateValue();
}

/**
   * Set corresponding com object's name via XAccessible interface and new name.
   * @param pXAcc XAccessible interface.
   * @param newName new name
   * @return
   */
void  AccObjectWinManager::SetAccName( XAccessible* pXAcc, Any newName)
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->SetName( newName );
}

/**
   * Judge if a XAccessible object is a container object.
   * @param pAccessible XAccessible interface.
   * @return If XAccessible object is container.
   */
bool AccObjectWinManager::IsContainer(XAccessible* pAccessible)
{
    try
    {
        if(pAccessible)
        {
            Reference<XAccessibleContext> xContext = pAccessible->getAccessibleContext();
            if(xContext.is())
            {
                switch( xContext->getAccessibleRole() )
                {
                case AccessibleRole::DIALOG:
                case AccessibleRole::FRAME:
                case AccessibleRole::WINDOW:
                case AccessibleRole::ROOT_PANE:
                case AccessibleRole::CANVAS:
                case AccessibleRole::COMBO_BOX:
                case AccessibleRole::DOCUMENT:
                case AccessibleRole::DOCUMENT_PRESENTATION:
                case AccessibleRole::DOCUMENT_SPREADSHEET:
                case AccessibleRole::DOCUMENT_TEXT:
                case AccessibleRole::EMBEDDED_OBJECT:
                case AccessibleRole::END_NOTE:
                case AccessibleRole::FILLER:
                case AccessibleRole::FOOTNOTE:
                case AccessibleRole::FOOTER:
                case AccessibleRole::GRAPHIC:
                case AccessibleRole::GROUP_BOX:
                case AccessibleRole::HEADER:
                case AccessibleRole::LAYERED_PANE:
                case AccessibleRole::MENU_BAR:
                case AccessibleRole::POPUP_MENU:
                case AccessibleRole::OPTION_PANE:
                case AccessibleRole::PAGE_TAB:
                case AccessibleRole::PAGE_TAB_LIST:
                case AccessibleRole::PANEL:
                case AccessibleRole::SCROLL_PANE:
                case AccessibleRole::SPLIT_PANE:
                case AccessibleRole::STATUS_BAR:
                case AccessibleRole::TABLE_CELL:
                case AccessibleRole::TEXT_FRAME:
                case AccessibleRole::TOOL_BAR:
                case AccessibleRole::VIEW_PORT:
                case AccessibleRole::SHAPE:
                    return true;
                case AccessibleRole::COLUMN_HEADER:
                case AccessibleRole::TABLE:
                    if(!IsStateManageDescendant(pAccessible))
                        return true;
                    break;
                case AccessibleRole::MENU:
                    return true;
                default:
                    return false;
                }
            }
        }
    }
    catch(...)
    {
        return false;
    }
    return false;
}

/**
   * Judge if a XAccessible object has ManageDescendant event.
   * @param pAccessible XAccessible interface.
   * @return If XAccessible object is managedescendant.
   */
bool AccObjectWinManager::IsStateManageDescendant(XAccessible* pAccessible)
{
    if(pAccessible)
    {
        Reference<XAccessibleContext> xContext = pAccessible->getAccessibleContext();
        if(xContext.is())
        {
            sal_Int64 nRState = xContext->getAccessibleStateSet();
            return nRState & AccessibleStateType::MANAGES_DESCENDANTS;
        }
    }
    return false;
}

/**
   * Query and get IAccessible interface by XAccessible interface from list.
   * @param pXAcc XAccessible interface.
   * @return Com accobject interface.
   */
IMAccessible* AccObjectWinManager::GetIAccessibleFromXAccessible(XAccessible* pXAcc)
{
    AccObject* pAccObj = GetAccObjByXAcc(pXAcc);
    if (pAccObj)
        return pAccObj->GetIMAccessible();

    return nullptr;
}

/**
   * Query and get IAccessible interface by child id from list.
   * @param resID, childID.
   * @return Com accobject interface.
   */
IMAccessible * AccObjectWinManager::GetIAccessibleFromResID(long resID)
{
    XResIdToAccObjHash::iterator pIndTemp = XResIdAccList.find( resID );
    if ( pIndTemp == XResIdAccList.end() )
        return nullptr;

    AccObject* pObj = pIndTemp->second;

    if(pObj->GetIMAccessible())
        return pObj->GetIMAccessible();
    return nullptr;
}
/**
   * Notify some object will be destroyed.
   * @param pXAcc XAccessible interface.
   * @return Com accobject interface.
   */
void AccObjectWinManager::NotifyDestroy(XAccessible* pXAcc)
{
    AccObject* accObj = GetAccObjByXAcc(pXAcc);
    if(accObj)
    {
        accObj->NotifyDestroy();
    }
}


void AccObjectWinManager::UpdateChildState(css::accessibility::XAccessible* pAccSubMenu)
{
    Reference<css::accessibility::XAccessibleContext> xContext(pAccSubMenu,UNO_QUERY);
    if (!xContext.is())
    {
        return;
    }
    const sal_Int64 nCount = xContext->getAccessibleChildCount();
    for (sal_Int64 i = 0 ; i < nCount; ++i)
    {
        Reference<css::accessibility::XAccessible> xChild = xContext->getAccessibleChild(i);
        if (xChild.is())
        {
            AccObject *pObj =  GetAccObjByXAcc(xChild.get());
            if (pObj)
            {
                pObj->UpdateState();
            }
        }
    }
}

short AccObjectWinManager::GetRole(css::accessibility::XAccessible* pXAcc)
{
    assert(pXAcc != nullptr);
    Reference<css::accessibility::XAccessibleContext> xContext = pXAcc->getAccessibleContext();
    if(xContext.is())
    {
        return xContext->getAccessibleRole();
    }
    return -1;
}

XAccessible* AccObjectWinManager::GetAccDocByHWND(HWND pWnd)
{
    XHWNDToDocumentHash::iterator aIter;
    aIter = XHWNDDocList.find( pWnd );
    if ( aIter != XHWNDDocList.end() )
    {
        return aIter->second;
    }

    return nullptr;
}

XAccessible* AccObjectWinManager::GetAccDocByAccTopWin( XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    HWND hWnd = pAccObj->GetParentHWND();
    return GetAccDocByHWND(hWnd);
}

bool AccObjectWinManager::IsTopWinAcc( css::accessibility::XAccessible* pXAcc )
{
    bool bRet = false;
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if ( pAccObj )
    {
        bRet = ( pAccObj->GetParentObj() == nullptr );
    }
    return bRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
