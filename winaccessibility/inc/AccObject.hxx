/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef __ACCOBJECT_HXX
#define __ACCOBJECT_HXX

#include <vector>
#include <map>
#include <oleacc.h>
#include <windows.h>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>

#include "accHelper.hxx"
#include  "UAccCOM.h"

class AccEventListener;
class AccObjectManagerAgent;
class AccObject;

typedef std::map< const long, AccObject*,ltstr4 > IAccSelectionList;
typedef std::vector<AccObject *> IAccChildList;


class AccObject
{
private:

    short               m_accRole;
    long                m_resID;
    HWND                m_pParantID;
    sal_Bool                m_bShouldDestroy; //avoid access COM interface when acc object is deleted
    IMAccessible*       m_pIMAcc;
    AccObject*          m_pParentObj;
    IAccChildList       m_childrenList;
    AccEventListener*   m_accListener;
    IAccSelectionList   m_selectionList;

    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > m_xAccRef;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleAction > m_xAccActionRef;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext > m_xAccContextRef;

    sal_Bool ImplInitilizeCreateObj();//create COM object

    void UpdateActionDesc();
    void UpdateRole();

    DWORD GetMSAAStateFromUNO(short xState);//translate state from UNO to MSAA value
    ::com::sun::star::accessibility::XAccessibleSelection* GetXAccessibleSelection();
    void GetExpandedState(sal_Bool* isExpandable, sal_Bool* isExpanded);
    ::rtl::OUString GetMAccessibleValueFromAny(::com::sun::star::uno::Any pAny);

public:

    AccObject ( ::com::sun::star::accessibility::XAccessible* pXAcc = NULL,AccObjectManagerAgent* pAgent = NULL ,AccEventListener* accListener=NULL);
    virtual ~AccObject();

    sal_Bool UpdateAccessibleInfoFromUnoToMSAA(  ); //implement accessible information mapping
    void UpdateDefaultAction();

    IMAccessible*  GetIMAccessible();   //return COM interface in acc object
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > GetXAccessible();

    void SetResID(long id);//ResID means ChildID in MSAA
    long GetResID();


    void SetParentHWND(HWND hWnd);//need to set top window handle when send event to AT
    HWND GetParentHWND();

    void SetListener( AccEventListener* Listener );
    AccEventListener* getListener();

    void SetParentObj(AccObject* pParentAccObj);
    AccObject* GetParentObj();

    void InsertChild( AccObject* pChild,short pos = LAST_CHILD);
    void DeleteChild( AccObject* pChild );
    AccObject* NextChild();

    void NotifyDestroy(sal_Bool ifDelete);
    sal_Bool  ifShouldDestroy();

    void  DecreaseState(short xState );//call COM interface DecreaseState method
    void  IncreaseState( short xState );//call COM interface IncreaseState method

    void  SetName( com::sun::star::uno::Any newName);
    void  SetValue( com::sun::star::uno::Any pAny );
    void  SetDescription( com::sun::star::uno::Any newDesc );
    void  SetRole( short Role );

    short GetRole() const;

    void  UpdateState();
    void  UpdateName();
    void  UpdateValue();
    void  UpdateAction();
    void  UpdateDescription();
    void  UpdateValidWindow();
    void  UpdateLocation();

    void  setFocus();
    void  unsetFocus();

    void  AddSelect(long index, AccObject* accObj);
    IAccSelectionList& GetSelection();
    void  setLocalizedResourceString();
};

#endif
