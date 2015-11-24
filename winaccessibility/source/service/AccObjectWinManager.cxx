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

#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <oleacc.h>
#include "AccObjectWinManager.hxx"
#include "AccEventListener.hxx"
#include "AccComponentEventListener.hxx"
#include "AccContainerEventListener.hxx"
#include "AccDialogEventListener.hxx"
#include "AccWindowEventListener.hxx"
#include "AccFrameEventListener.hxx"
#include "AccMenuEventListener.hxx"
#include "AccObjectContainerEventListener.hxx"
#include "AccParagraphEventListener.hxx"
#include "AccTextComponentEventListener.hxx"
#include "AccListEventListener.hxx"
#include "AccTreeEventListener.hxx"
#include "AccTableEventListener.hxx"
#include "AccObject.hxx"
#include "unomsaaevent.hxx"


using namespace std;
using namespace com::sun::star::accessibility;
using namespace com::sun::star::accessibility::AccessibleRole;
using namespace com::sun::star::accessibility::AccessibleStateType;
using namespace com::sun::star::uno;

/**
   * constructor
   * @param   Agent The agent kept in all listeners,it's the sole interface by which
   *                listener communicate with windows manager.
   *          pEventAccObj The present event accobject.
   *          oldFocus     Last focused object.
   *          isSelectionChanged flag that identifies if there is selection changed.
   *          selectionChildObj  Selected object.
   *          dChildID  Chile resource ID.
   *          hAcc TopWindowHWND
   * @return
   */
AccObjectWinManager::AccObjectWinManager( AccObjectManagerAgent* Agent ):
        pAgent( Agent ),
        oldFocus( NULL )
{
}

/**
   * Destructor,clear all resource.
   * @param
   * @return
   */
AccObjectWinManager::~AccObjectWinManager()
{
    XIdAccList.clear();
    HwndXAcc.clear();
    XResIdAccList.clear();
    XHWNDDocList.clear();
#ifdef ACC_DEBUG

    fclose( pFile );
#endif
}


/**
   * Get valid com object interface when notifying some MSAA event
   * @param pWND The top window handle that contains that event control.
   * @param wParam Windows system interface.
   * @return Com interface with event.
   */

LRESULT
AccObjectWinManager::Get_ToATInterface(HWND hWnd, long lParam, WPARAM wParam)
{
    IMAccessible* pRetIMAcc = NULL;

    if(lParam == OBJID_CLIENT )
    {
        AccObject* topWindowAccObj = GetTopWindowAccObj(hWnd);
        if(topWindowAccObj)
        {
            pRetIMAcc = topWindowAccObj->GetIMAccessible();
            if(pRetIMAcc)
                pRetIMAcc->AddRef();//increase COM reference count
        }
    }

    if ( pRetIMAcc && lParam == OBJID_CLIENT )
    {
        IAccessible* pTemp = dynamic_cast<IAccessible*>( pRetIMAcc );
        LRESULT result = LresultFromObject(IID_IAccessible, wParam, pTemp);
        pTemp->Release();
        return result;
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
    if( pXAcc == NULL)
        return NULL;

    XIdToAccObjHash::iterator pIndTemp = XIdAccList.find( (void*)pXAcc );
    if ( pIndTemp == XIdAccList.end() )
        return NULL;

    return &(pIndTemp->second);
}

/**
   * get acc object of top window by its handle
   * @param hWnd, top window handle
   * @return pointer to AccObject
   */
AccObject* AccObjectWinManager::GetTopWindowAccObj(HWND hWnd)
{
    XHWNDToXAccHash::iterator iterResult =HwndXAcc.find(hWnd);
    if(iterResult == HwndXAcc.end())
        return NULL;
    XAccessible* pXAcc = (XAccessible*)(iterResult->second);
    return GetAccObjByXAcc(pXAcc);
}

/**
   * Simulate MSAA event via XAccessible interface and event type.
   * @param pXAcc XAccessible interface.
   * @param state Customize Interface
   * @return The terminate result that identifies if the call is successful.
   */
sal_Bool AccObjectWinManager::NotifyAccEvent(XAccessible* pXAcc,short state)
{
    Reference< XAccessibleContext > pRContext;

    if( pXAcc == NULL)
        return sal_False;


    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return sal_False;


    AccObject* selfAccObj= GetAccObjByXAcc(pXAcc);

    if(selfAccObj==NULL)
        return sal_False;

    long dChildID = selfAccObj->GetResID();
    HWND hAcc = selfAccObj->GetParentHWND();

    switch(state)
    {
    case UM_EVENT_STATE_FOCUSED:
        {
            UpdateAccFocus(pXAcc);
            if( selfAccObj )
                selfAccObj->UpdateDefaultAction( );
            UpdateValue(pXAcc);
            NotifyWinEvent( EVENT_OBJECT_FOCUS,hAcc, OBJID_CLIENT,dChildID  );
            break;
        }
    case UM_EVENT_STATE_BUSY:
        NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_STATE_CHECKED:
        NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_STATE_PRESSED:
        NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;

    //Removed fire out selected event
    //case UM_EVENT_STATE_SELECTED:
    //  NotifyWinEvent( EVENT_OBJECT_STATECHANGE,hAcc, OBJID_CLIENT,dChildID  );
    //  break;
    case UM_EVENT_STATE_ARMED:
        UpdateAccFocus(pXAcc);
        NotifyWinEvent( EVENT_OBJECT_FOCUS,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_MENU_START:
        NotifyWinEvent( EVENT_SYSTEM_MENUSTART,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_MENU_END:
        NotifyWinEvent( EVENT_SYSTEM_MENUEND,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_MENUPOPUPSTART:
        NotifyWinEvent( EVENT_SYSTEM_MENUPOPUPSTART,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_MENUPOPUPEND:
        NotifyWinEvent( EVENT_SYSTEM_MENUPOPUPEND,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_SELECTION_CHANGED:
        NotifyWinEvent( EVENT_OBJECT_SELECTION,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_SELECTION_CHANGED_ADD:
        NotifyWinEvent( EVENT_OBJECT_SELECTIONADD,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_SELECTION_CHANGED_REMOVE:
        NotifyWinEvent( EVENT_OBJECT_SELECTIONREMOVE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_SELECTION_CHANGED_WITHIN:
        NotifyWinEvent( EVENT_OBJECT_SELECTIONWITHIN,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_VALUECHANGE:
        UpdateValue(pXAcc);
        NotifyWinEvent( EVENT_OBJECT_VALUECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_NAMECHANGE:
        NotifyWinEvent( EVENT_OBJECT_NAMECHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_DESCRIPTIONCHANGE:
        NotifyWinEvent( EVENT_OBJECT_DESCRIPTIONCHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_DEFACTIONCHANGE:
        NotifyWinEvent( IA2_EVENT_ACTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_CARETCHANGE:
        NotifyWinEvent( IA2_EVENT_TEXT_CARET_MOVED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_TEXTCHANGE:
        NotifyWinEvent( IA2_EVENT_TEXT_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_ACTIVE_DESCENDANT_CHANGED:
        UpdateAccFocus(pXAcc);
        NotifyWinEvent( EVENT_OBJECT_FOCUS,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_BOUNDRECT_CHANGED:
        NotifyWinEvent( EVENT_OBJECT_LOCATIONCHANGE,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_VISIBLE_DATA_CHANGED:
        NotifyWinEvent( IA2_EVENT_VISIBLE_DATA_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_SHOW :
        NotifyWinEvent( EVENT_OBJECT_SHOW,hAcc, OBJID_CLIENT,dChildID  );
        NotifyWinEvent( EVENT_SYSTEM_FOREGROUND,hAcc, OBJID_CLIENT,dChildID  );
    break;
    case UM_EVENT_TABLE_CAPTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_CAPTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TABLE_COLUMN_DESCRIPTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_COLUMN_DESCRIPTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TABLE_COLUMN_HEADER_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_COLUMN_HEADER_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TABLE_MODEL_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_MODEL_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TABLE_ROW_HEADER_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_ROW_HEADER_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TABLE_SUMMARY_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_SUMMARY_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TABLE_ROW_DESCRIPTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TABLE_ROW_DESCRIPTION_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_REORDER:
        NotifyWinEvent( EVENT_OBJECT_REORDER,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_PAGE_CHANGED:
        NotifyWinEvent( IA2_EVENT_PAGE_CHANGED,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_CHILD_REMOVED:
        NotifyWinEvent( EVENT_OBJECT_DESTROY,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_CHILD_ADDED:
        NotifyWinEvent( EVENT_OBJECT_CREATE ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_OBJECT_PAGECHANGED:
        NotifyWinEvent( IA2_EVENT_PAGE_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_TEXT_SELECTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_TEXT_SELECTION_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_SECTION_CHANGED:
        NotifyWinEvent( IA2_EVENT_SECTION_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    case UM_EVENT_COLUMN_CHANGED:
        NotifyWinEvent( IA2_EVENT_TEXT_COLUMN_CHANGED ,hAcc, OBJID_CLIENT,dChildID  );
        break;
    default:
        break;
    }

    return sal_True;
}

/**
   * Get Parent XAccessible interface by XAccessible interface.
   * @param pXAcc XAccessible interface.
   * @return Parent XAccessible interface.
   */
XAccessible* AccObjectWinManager::GetParentXAccessible( XAccessible* pXAcc )
{
    AccObject* pObj= GetAccObjByXAcc(pXAcc);
    if( pObj ==NULL )
        return NULL;
    if(pObj->GetParentObj())
    {
        pObj = pObj->GetParentObj();
        return pObj->GetXAccessible().get();
    }
    return NULL;
}

/**
   * Get Parent role by XAccessible interface.
   * @param pXAcc XAccessible interface.
   * @return Parent role.
   */
short AccObjectWinManager::GetParentRole( XAccessible* pXAcc )
{
    AccObject* pObj= GetAccObjByXAcc(pXAcc);
    if( pObj ==NULL )
        return -1;
    if(pObj->GetParentObj())
    {
        pObj = pObj->GetParentObj();
        if(pObj->GetXAccessible().is())
        {
            XAccessible* pXAcc = pObj->GetXAccessible().get();
            Reference< XAccessibleContext > pRContext = pXAcc->getAccessibleContext();
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
   * Update selected object by new focused XAccessible interface.
   * @param pXAcc XAccessible interface that has selected child changed.
   * @return Selected children count.
   */
int AccObjectWinManager::UpdateAccSelection(XAccessible* pXAcc)
{
    Reference< XAccessibleContext > pRContext;

    if( pXAcc == NULL)
        return sal_False;

    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return sal_False;

    Reference< XAccessibleSelection > pRSelection(pRContext,UNO_QUERY);
    if( !pRSelection.is() )
        return sal_False;

    AccObject* pAccObj = GetAccObjByXAcc(pXAcc);
    if(pAccObj==NULL)
        return sal_False;

    Reference<XAccessible> pRChild = NULL;
    AccObject* pAccChildObj = NULL;
    int selectNum= pRSelection->getSelectedAccessibleChildCount();

    IAccSelectionList oldSelection = pAccObj->GetSelection();

    if(selectNum > 4)//for selected.
        return selectNum;
    if(selectNum == 1 && oldSelection.size() == 0)
        return 1;

    for (int i=0;i<selectNum;i++)
    {
        pRChild = pRSelection->getSelectedAccessibleChild(i);
        if(!pRChild.is())
        {
            continue;
        }
        Reference<XAccessibleContext> pRChildContext = pRChild->getAccessibleContext();
        if(!pRChildContext.is())
        {
            continue;
        }
        long index = pRChildContext->getAccessibleIndexInParent();
        IAccSelectionList::iterator temp = oldSelection.find(index);
        if ( temp != oldSelection.end() )
        {
            oldSelection.erase(index);
            continue;
        }

        pAccChildObj = GetAccObjByXAcc(pRChild.get());
        if(!pAccChildObj)
        {
            InsertAccObj(pRChild.get(), pXAcc,pAccObj->GetParentHWND());
            pAccChildObj = GetAccObjByXAcc(pRChild.get());
        }

        pAccObj->AddSelect(index, pAccChildObj);

        if(pAccChildObj != NULL)
            NotifyWinEvent(EVENT_OBJECT_SELECTIONADD,pAccObj->GetParentHWND(), OBJID_CLIENT,pAccChildObj->GetResID());
    }

    IAccSelectionList::iterator iter = oldSelection.begin();
    while(iter!=oldSelection.end())
    {
        pAccObj->GetSelection().erase(iter->first);
        pAccChildObj = (AccObject*)(iter->second);
        if(pAccChildObj != NULL)
            NotifyWinEvent(EVENT_OBJECT_SELECTIONREMOVE,pAccObj->GetParentHWND(), OBJID_CLIENT,pAccChildObj->GetResID());
        ++iter;
    }
    return 0;

}

/**
   * Delete child element from children list.
   * @param pObj Child element that should be removed from parant child list.
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
void AccObjectWinManager::DeleteFromHwndXAcc(XAccessible* pXAcc )
{
    XHWNDToXAccHash::iterator iter = HwndXAcc.begin();
    while(iter!=HwndXAcc.end())
    {
        if(iter->second == pXAcc )
        {
            HwndXAcc.erase(iter);
            return;
        }
        ++iter;
    }
}

/**
   * Delete all children with the tree root of XAccessible pointer
   * @param pXAcc Tree root XAccessible interface.
   * @return
   */
void AccObjectWinManager::DeleteChildrenAccObj(XAccessible* pXAcc)
{
    AccObject* currentObj=NULL;
    AccObject* childObj=NULL;

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
    if( pXAcc == NULL )
        return;
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
    DeleteAccListener( &accObj );
    if( accObj.GetIMAccessible() )
    {
        accObj.GetIMAccessible()->Release();
    }
    size_t i = XResIdAccList.erase(accObj.GetResID());
    assert(i != 0);
    DeleteFromHwndXAcc(pXAcc);
    if( accObj.GetRole() == DOCUMENT ||
        accObj.GetRole() == DOCUMENT_PRESENTATION ||
        accObj.GetRole() == DOCUMENT_SPREADSHEET ||
        accObj.GetRole() == DOCUMENT_TEXT )
    {
        XHWNDDocList.erase(accObj.GetParentHWND());
    }
    XIdAccList.erase(pXAcc); // note: this invalidates accObj so do it last!
}

/**
   * Delete listener that inspects some XAccessible object
   * @param pAccObj Accobject pointer.
   * @return
   */
void AccObjectWinManager::DeleteAccListener( AccObject*  pAccObj )
{
    AccEventListener* listener = pAccObj->getListener();
    if( listener==NULL )
        return;
    listener->RemoveMeFromBroadcaster();
    pAccObj->SetListener(NULL);
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
sal_Bool AccObjectWinManager::InsertChildrenAccObj( css::accessibility::XAccessible* pXAcc,
        HWND pWnd)
{
    if(!IsContainer(pXAcc))
        return sal_False;

    Reference< XAccessibleContext > pRContext;

    if( pXAcc == NULL)
        return sal_False;
    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return sal_False;

    short role = pRContext->getAccessibleRole();

    if(css::accessibility::AccessibleRole::DOCUMENT == role ||
            css::accessibility::AccessibleRole::DOCUMENT_PRESENTATION == role ||
            css::accessibility::AccessibleRole::DOCUMENT_SPREADSHEET == role ||
            css::accessibility::AccessibleRole::DOCUMENT_TEXT == role)
    {
        if(IsStateManageDescendant(pXAcc))
        {
            return sal_True;
        }
    }

    int count = pRContext->getAccessibleChildCount();
    for (int i=0;i<count;i++)
    {
        Reference<XAccessible> mxAccessible
        = pRContext->getAccessibleChild(i);
        XAccessible* mpAccessible = mxAccessible.get();
        if(mpAccessible != NULL)
        {
            InsertAccObj( mpAccessible,pXAcc,pWnd );
            InsertChildrenAccObj(mpAccessible,pWnd);
        }
    }

    return sal_True;
}

/**
   * Insert child object.
   * @param pCurObj The child object
   * @param pParentObj The parant object
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
   * @param pParentObj The parant object
   * @param pWnd Top window handle.
   * @return
   */
sal_Bool AccObjectWinManager::InsertAccObj( XAccessible* pXAcc,XAccessible* pParentXAcc,HWND pWnd )
{
    XIdToAccObjHash::iterator itXacc = XIdAccList.find( (void*)pXAcc );
    if (itXacc != XIdAccList.end() )
    {
        short nCurRole =GetRole(pXAcc);
        if (AccessibleRole::SHAPE == nCurRole)
        {
            AccObject &objXacc = itXacc->second;
            AccObject *pObjParent = objXacc.GetParentObj();
            if (pObjParent &&
                    pObjParent->GetXAccessible().is() &&
                    pObjParent->GetXAccessible().get() != pParentXAcc)
            {
                XIdToAccObjHash::iterator itXaccParent  = XIdAccList.find( (void*)pParentXAcc );
                if(itXaccParent != XIdAccList.end())
                {
                    objXacc.SetParentObj(&(itXaccParent->second));
                }
            }
        }
        return sal_False;
    }


    Reference< XAccessibleContext > pRContext;

    if( pXAcc == NULL)
        return sal_False;

    pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return sal_False;

    if( pWnd == NULL )
    {
        if(pParentXAcc)
        {
            AccObject* pObj = GetAccObjByXAcc(pParentXAcc);
            if(pObj)
                pWnd = pObj->GetParentHWND();
        }
        if( pWnd == NULL )
            return sal_False;
    }

    AccObject pObj( pXAcc,pAgent );
    if( pObj.GetIMAccessible() == NULL )
        return sal_False;
    pObj.SetResID( this->ImpleGenerateResID());
    pObj.SetParentHWND( pWnd );

    //for file name support
    if( pObj.GetRole() == DOCUMENT ||
        pObj.GetRole() == DOCUMENT_PRESENTATION ||
        pObj.GetRole() == DOCUMENT_SPREADSHEET ||
        pObj.GetRole() == DOCUMENT_TEXT )
    {
        XHWNDToDocumentHash::iterator aIter = XHWNDDocList.find(pWnd);
        if ( aIter != XHWNDDocList.end() )
        {
            XHWNDDocList.erase( aIter );
        }
        XHWNDDocList.insert( XHWNDToDocumentHash::value_type(pWnd, pXAcc) );
    }
    //end of file name

    ::rtl::Reference<AccEventListener> const pListener =
        CreateAccEventListener(pXAcc);
    if (!pListener.is())
        return sal_False;
    Reference<XAccessibleComponent> xComponent(pRContext,UNO_QUERY);
    Reference<XAccessibleEventBroadcaster> broadcaster(xComponent,UNO_QUERY);
    if (broadcaster.is())
    {
        Reference<XAccessibleEventListener> const xListener(pListener.get());
        broadcaster->addAccessibleEventListener(xListener);
    }
    else
        return sal_False;

    XIdAccList.insert( XIdToAccObjHash::value_type( (void*)pXAcc, pObj ));
    XIdToAccObjHash::iterator pIndTemp = XIdAccList.find( (void*)pXAcc );
    XResIdAccList.insert(XResIdToAccObjHash::value_type(pObj.GetResID(),&(pIndTemp->second)));

    AccObject* pCurObj = GetAccObjByXAcc(pXAcc);
    if( pCurObj )
    {
        pCurObj->SetListener(pListener);
    }

    AccObject* pParentObj = GetAccObjByXAcc(pParentXAcc);
    InsertAccChildNode(pCurObj,pParentObj,pWnd);
    if( pCurObj )
        pCurObj->UpdateAccessibleInfoFromUnoToMSAA();
    return sal_True;
}


/**
   * save the pair <topwindowhandle, XAccessible>
   * @param hWnd, top window handle
   * @param pXAcc XAccessible interface for top window
   * @return void
   */
void AccObjectWinManager::SaveTopWindowHandle(HWND hWnd, css::accessibility::XAccessible* pXAcc)
{
    HwndXAcc.insert( XHWNDToXAccHash::value_type( hWnd,(void*)pXAcc ) );
}


/** Create the corresponding listener.
 *  @param pXAcc XAccessible interface.
 */
::rtl::Reference<AccEventListener>
AccObjectWinManager::CreateAccEventListener(XAccessible* pXAcc)
{
    ::rtl::Reference<AccEventListener> pRet;
    Reference<XAccessibleContext> xContext(pXAcc->getAccessibleContext(),UNO_QUERY);
    if(xContext.is())
    {
        switch( xContext->getAccessibleRole() )
        {
        case /*AccessibleRole::*/DIALOG:
            pRet = new AccDialogEventListener(pXAcc,pAgent);
            break;
        case /*AccessibleRole::*/FRAME:
            pRet = new AccFrameEventListener(pXAcc,pAgent);
            break;
        case /*AccessibleRole::*/WINDOW:
            pRet = new AccWindowEventListener(pXAcc,pAgent);
            break;
        case /*AccessibleRole::*/ROOT_PANE:
            pRet = new AccFrameEventListener(pXAcc,pAgent);
            break;
            //Container
        case /*AccessibleRole::*/CANVAS:
        case /*AccessibleRole::*/COMBO_BOX:
        case /*AccessibleRole::*/DOCUMENT:
        case /*AccessibleRole::*/DOCUMENT_PRESENTATION:
        case /*AccessibleRole::*/DOCUMENT_SPREADSHEET:
        case /*AccessibleRole::*/DOCUMENT_TEXT:
        case /*AccessibleRole::*/END_NOTE:
        case /*AccessibleRole::*/FILLER:
        case /*AccessibleRole::*/FOOTNOTE:
        case /*AccessibleRole::*/FOOTER:
        case /*AccessibleRole::*/HEADER:
        case /*AccessibleRole::*/LAYERED_PANE:
        case /*AccessibleRole::*/MENU_BAR:
        case /*AccessibleRole::*/POPUP_MENU:
        case /*AccessibleRole::*/OPTION_PANE:
        case /*AccessibleRole::*/PAGE_TAB:
        case /*AccessibleRole::*/PAGE_TAB_LIST:
        case /*AccessibleRole::*/PANEL:
        case /*AccessibleRole::*/SCROLL_PANE:
        case /*AccessibleRole::*/SPLIT_PANE:
        case /*AccessibleRole::*/STATUS_BAR:
        case /*AccessibleRole::*/TABLE_CELL:
        case /*AccessibleRole::*/TOOL_BAR:
        case /*AccessibleRole::*/VIEW_PORT:
            pRet = new AccContainerEventListener(pXAcc,pAgent);
            break;
        case /*AccessibleRole::*/PARAGRAPH:
        case /*AccessibleRole::*/HEADING:
            pRet = new AccParagraphEventListener(pXAcc,pAgent);
            break;
            //Component
        case /*AccessibleRole::*/CHECK_BOX:
        case /*AccessibleRole::*/ICON:
        case /*AccessibleRole::*/LABEL:
        case /*AccessibleRole::*/MENU_ITEM:
        case /*AccessibleRole::*/CHECK_MENU_ITEM:
        case /*AccessibleRole::*/RADIO_MENU_ITEM:
        case /*AccessibleRole::*/PUSH_BUTTON:
        case /*AccessibleRole::*/RADIO_BUTTON:
        case /*AccessibleRole::*/SCROLL_BAR:
        case /*AccessibleRole::*/SEPARATOR:
        case /*AccessibleRole::*/TOGGLE_BUTTON:
        case /*AccessibleRole::*/BUTTON_DROPDOWN:
        case /*AccessibleRole::*/TOOL_TIP:
        case /*AccessibleRole::*/SPIN_BOX:
        case DATE_EDITOR:
            pRet = new AccComponentEventListener(pXAcc,pAgent);
            break;
            //text component
        case /*AccessibleRole::*/TEXT:
            pRet = new AccTextComponentEventListener(pXAcc,pAgent);
            break;
            //menu
        case /*AccessibleRole::*/MENU:
            pRet = new AccMenuEventListener(pXAcc,pAgent);
            break;
            //object container
        case /*AccessibleRole::*/SHAPE:

        case /*AccessibleRole::*/EMBEDDED_OBJECT:
        case /*AccessibleRole::*/GRAPHIC:
        case /*AccessibleRole::*/TEXT_FRAME:
            pRet = new AccObjectContainerEventListener(pXAcc,pAgent);
            break;
            //descendmanager
        case /*AccessibleRole::*/LIST:
            pRet = new AccListEventListener(pXAcc,pAgent);
            break;
        case /*AccessibleRole::*/TREE:
            pRet = new AccTreeEventListener(pXAcc,pAgent);
            break;
            //special
        case /*AccessibleRole::*/COLUMN_HEADER:
        case /*AccessibleRole::*/TABLE:
            pRet = new AccTableEventListener(pXAcc,pAgent);
            break;
        default:
            pRet = new AccContainerEventListener(pXAcc,pAgent);
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
void AccObjectWinManager::DecreaseState( XAccessible* pXAcc,unsigned short pState )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->DecreaseState( pState );
}

/**
   * state is a combination integer, each bit of which represents a single state,such as focused,1 for
   * the state on,0 for the state off. Here call COM interface to modify the state value, including
   * IncreaseState.
   * @param pXAcc XAccessible interface.
   * @param pState Changed state.
   * @return
   */
void AccObjectWinManager::IncreaseState( XAccessible* pXAcc,unsigned short pState )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->IncreaseState( pState );
}

void  AccObjectWinManager::UpdateState( css::accessibility::XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->UpdateState( );
}

/**
   * Set corresponding com object's accessible name via XAccessilbe interface and new
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

void AccObjectWinManager::UpdateDescription( XAccessible* pXAcc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if ( pAccObj )
        pAccObj->UpdateDescription();
}

/**
   * Set corresponding com object's accessible location via XAccessilbe interface and new
   * location.
   * @param pXAcc XAccessible interface.
   * @return
   */
void  AccObjectWinManager::SetLocation( XAccessible* pXAcc, long /*top*/, long /*left*/, long /*width*/, long /*height*/ )
{
    AccObject* pObj = GetAccObjByXAcc( pXAcc );
    //get the location from XComponent.
    Reference< XAccessibleContext > pRContext = pXAcc->getAccessibleContext();
    if( pObj )
        pObj->UpdateLocation();
}

/**
   * Set corresponding com object's value  via XAccessilbe interface and new value.
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
   * Set corresponding com object's value  via XAccessilbe interface.
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
   * Set corresponding com object's name via XAccessilbe interface and new name.
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
   * Set corresponding com object's description via XAccessilbe interface and new description.
   * @param pXAcc XAccessible interface.
   * @param newDesc new description
   * @return
   */
void  AccObjectWinManager::SetDescription( XAccessible* pXAcc, Any newDesc )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->SetDescription( newDesc );
}

/**
   * Set corresponding com object's role via XAccessilbe interface and new role.
   * @param pXAcc XAccessible interface.
   * @param Role new role
   * @return
   */
void  AccObjectWinManager::SetRole( XAccessible* pXAcc, long Role )
{
    AccObject* pAccObj = GetAccObjByXAcc( pXAcc );
    if( pAccObj )
        pAccObj->SetRole( (short)Role );
}

/**
   * Judge if a XAccessible object is a container object.
   * @param pAccessible XAccessible interface.
   * @return If XAccessible object is container.
   */
sal_Bool AccObjectWinManager::IsContainer(XAccessible* pAccessible)
{
    try
    {
        if(pAccessible)
        {
            Reference<XAccessibleContext> xContext(pAccessible->getAccessibleContext(),UNO_QUERY);
            if(xContext.is())
            {
                switch( xContext->getAccessibleRole() )
                {
                case /*AccessibleRole::*/DIALOG:
                case /*AccessibleRole::*/FRAME:
                case /*AccessibleRole::*/WINDOW:
                case /*AccessibleRole::*/ROOT_PANE:
                case /*AccessibleRole::*/CANVAS:
                case /*AccessibleRole::*/COMBO_BOX:
                case /*AccessibleRole::*/DOCUMENT:
                case /*AccessibleRole::*/DOCUMENT_PRESENTATION:
                case /*AccessibleRole::*/DOCUMENT_SPREADSHEET:
                case /*AccessibleRole::*/DOCUMENT_TEXT:
                case /*AccessibleRole::*/EMBEDDED_OBJECT:
                case /*AccessibleRole::*/END_NOTE:
                case /*AccessibleRole::*/FILLER:
                case /*AccessibleRole::*/FOOTNOTE:
                case /*AccessibleRole::*/FOOTER:
                case /*AccessibleRole::*/GRAPHIC:
                case /*AccessibleRole::*/GROUP_BOX:
                case /*AccessibleRole::*/HEADER:
                case /*AccessibleRole::*/LAYERED_PANE:
                case /*AccessibleRole::*/MENU_BAR:
                case /*AccessibleRole::*/POPUP_MENU:
                case /*AccessibleRole::*/OPTION_PANE:
                case /*AccessibleRole::*/PAGE_TAB:
                case /*AccessibleRole::*/PAGE_TAB_LIST:
                case /*AccessibleRole::*/PANEL:
                case /*AccessibleRole::*/SCROLL_PANE:
                case /*AccessibleRole::*/SPLIT_PANE:
                case /*AccessibleRole::*/STATUS_BAR:
                case /*AccessibleRole::*/TABLE_CELL:
                case /*AccessibleRole::*/TEXT_FRAME:
                case /*AccessibleRole::*/TOOL_BAR:
                case /*AccessibleRole::*/VIEW_PORT:
                case /*AccessibleRole::*/SHAPE:
                    return sal_True;
                    break;
                case /*AccessibleRole::*/COLUMN_HEADER:
                case /*AccessibleRole::*/TABLE:
                    if(!IsStateManageDescendant(pAccessible))
                        return sal_True;
                    break;
                case /*AccessibleRole::*/MENU:
                    return sal_True;
                    break;
                default:
                    return sal_False;
                }
            }
        }
    }
    catch(...)
    {
        return sal_False;
    }
    return sal_False;
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
        Reference<XAccessibleContext> xContext(pAccessible->getAccessibleContext(),UNO_QUERY);
        if(xContext.is())
        {
            Reference< XAccessibleStateSet > pRState = xContext->getAccessibleStateSet();
            if( !pRState.is() )
                return sal_False;

            Sequence<short> pStates = pRState->getStates();
            int count = pStates.getLength();
            for( int iIndex = 0;iIndex < count;iIndex++ )
            {
                if(pStates[iIndex] == /*AccessibleStateType::*/MANAGES_DESCENDANTS)
                    return sal_True;
            }
        }
    }
    return sal_False;
}

/**
   * Query and get IAccessible interface by XAccessible interface from list.
   * @param pXAcc XAccessible interface.
   * @return Com accobject interface.
   */
IMAccessible* AccObjectWinManager::GetIMAccByXAcc(XAccessible* pXAcc)
{
    AccObject* pAccObj = GetAccObjByXAcc(pXAcc);
    if(pAccObj)
    {
        return pAccObj->GetIMAccessible();
    }
    else
    {
        return NULL;
    }
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
        return NULL;

    AccObject* pObj = pIndTemp->second;

    if(pObj->GetIMAccessible())
        return pObj->GetIMAccessible();
    return NULL;
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
        accObj->NotifyDestroy(sal_True);
    }
}


void AccObjectWinManager::UpdateChildState(css::accessibility::XAccessible* pAccSubMenu)
{
    Reference<css::accessibility::XAccessibleContext> xContext(pAccSubMenu,UNO_QUERY);
    if (!xContext.is())
    {
        return;
    }
    sal_Int32 nCount = xContext->getAccessibleChildCount();
    for (sal_Int32 i = 0 ; i < nCount ; ++i)
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


bool AccObjectWinManager::IsSpecialToolboItem(css::accessibility::XAccessible* pXAcc)
{
    if (pXAcc && oldFocus != pXAcc)
    {
        if(GetParentRole(pXAcc) == TOOL_BAR)
        {
            Reference< XAccessibleContext > pRContext(pXAcc->getAccessibleContext());
            if (pRContext.is())
            {
                if(pRContext->getAccessibleRole() == TOGGLE_BUTTON)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

short AccObjectWinManager::GetRole(css::accessibility::XAccessible* pXAcc)
{
    assert(pXAcc != NULL);
    Reference<css::accessibility::XAccessibleContext> xContext(pXAcc->getAccessibleContext(),UNO_QUERY);
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

    return NULL;
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
        bRet = ( pAccObj->GetParentObj() == NULL );
    }
    return bRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
