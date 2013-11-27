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

#ifndef __ACCOBJECTWINMANAGER_HXX
#define __ACCOBJECTWINMANAGER_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#include <map>
#include <windows.h>
#include <vos/mutex.hxx>
#include <vcl/dllapi.h>
#include "ResIDGenerator.hxx"
#include "UAccCOM2.h"

class ResIDGenerator;
class AccObjectManagerAgent;
class AccEventListener;
class AccObject;

/*******************************************************************
AccObjectWinManager complete the functions:
1. Insert, delete,query,update Acc objects
2. Create, delete,fire AccEventLister, the lifecycle of AccEventListener is as same as
   Acc Object
 3.Return COM interface for AT,by the call back in salframe
 4.Pass accessible information to Acc objects
 *******************************************************************/
class AccObjectWinManager
{
    friend class AccObjectManagerAgent;

private:
    struct ltstr1
    {
        bool operator()(const void*  s1, const void*  s2) const
        {
            return long(s1)<long(s2);
        }
    };
    struct ltstr2
    {
        bool operator()(const HWND  s1, const HWND  s2) const
        {
            return long(s1)<long(s2);
        }
    };
    struct ltstr3
    {
        bool operator()(const long  s1, const long  s2) const
        {
            return long(s1)<long(s2);
        }
    };
    typedef std::map< void*, AccObject,ltstr1 > XIdToAccObjHash;
    typedef std::map< HWND,void*,ltstr2 > XHWNDToXAccHash;
    typedef std::map< const long, AccObject*,ltstr3 > XResIdToAccObjHash;

    typedef std::map< const long, com::sun::star::accessibility::XAccessible*,ltstr3 > XHWNDToDocumentHash;


    //XAccessible to AccObject
    XIdToAccObjHash  XIdAccList;

    //HWND to All XAccessible pointer
    XHWNDToXAccHash  HwndXAcc;

    //id_Child to AccObject
    XResIdToAccObjHash XResIdAccList;

    //for file name support
    XHWNDToDocumentHash XHWNDDocList;

    com::sun::star::accessibility::XAccessible* oldFocus;

    AccObjectManagerAgent*   pAgent;
    static AccObjectWinManager* me;
    ResIDGenerator ResIdGen;
    mutable ::vos::OMutex aDeleteMutex;
    mutable ::vos::OMutex aNotifyMutex;
    mutable ::vos::OMutex maATInterfaceMutex;

    AccObjectWinManager(AccObjectManagerAgent* Agent=NULL);

private:
    long ImpleGenerateResID();
    AccObject* GetAccObjByXAcc( com::sun::star::accessibility::XAccessible* pXAcc);

    com::sun::star::accessibility::XAccessible* GetXAccByAccObj(AccObject* pAccObj);

    AccObject* GetTopWindowAccObj(HWND hWnd);

    com::sun::star::accessibility::XAccessible* GetAccDocByHWND( long pWnd );

    void       DeleteAccListener( AccObject* pAccObj );
    void       InsertAccChildNode(AccObject* pCurObj,AccObject* pParentObj,HWND pWnd);
    void       DeleteAccChildNode(AccObject* pChild);
    void       DeleteFromHwndXAcc(com::sun::star::accessibility::XAccessible* pXAcc );
    int  UpdateAccSelection(com::sun::star::accessibility::XAccessible* pXAcc);

    AccEventListener* createAccEventListener(com::sun::star::accessibility::XAccessible* pXAcc, AccObjectManagerAgent* Agent);
public:
    virtual ~AccObjectWinManager();
    sal_Bool InsertAccObj( com::sun::star::accessibility::XAccessible* pXAcc,com::sun::star::accessibility::XAccessible* pParentXAcc,HWND pWnd);
    sal_Bool InsertChildrenAccObj( com::sun::star::accessibility::XAccessible* pXAcc,HWND pWnd=0);
    void DeleteAccObj( com::sun::star::accessibility::XAccessible* pXAcc );
    void DeleteChildrenAccObj(com::sun::star::accessibility::XAccessible* pAccObj);

    static  AccObjectWinManager* CreateAccObjectWinManagerInstance(AccObjectManagerAgent* Agent);

    sal_Bool NotifyAccEvent( com::sun::star::accessibility::XAccessible* pXAcc,short state = 0 );

    long Get_ToATInterface( HWND hWnd, long lParam, long wParam);

    void  DecreaseState( com::sun::star::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  IncreaseState( com::sun::star::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  UpdateState( com::sun::star::accessibility::XAccessible* pXAcc );
    void  SetLocation( com::sun::star::accessibility::XAccessible* pXAcc,
                       long Top = 0,long left = 0,long width = 0,long height = 0);

    void  SetValue( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any pAny );
    void  UpdateValue( com::sun::star::accessibility::XAccessible* pXAcc );

    void  SetAccName( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any newName);
    void  UpdateAccName( com::sun::star::accessibility::XAccessible* pXAcc );

    void  SetDescription( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any newDesc );
    void UpdateDescription( com::sun::star::accessibility::XAccessible* pXAcc );

    void  SetRole( com::sun::star::accessibility::XAccessible* pXAcc, long Role );

    void  UpdateAccFocus( com::sun::star::accessibility::XAccessible* newFocus );
    void  UpdateAction( com::sun::star::accessibility::XAccessible* pXAcc );

    sal_Bool IsContainer( com::sun::star::accessibility::XAccessible* pAccessible );

    IMAccessible* GetIMAccByXAcc( com::sun::star::accessibility::XAccessible* pXAcc );
    IMAccessible* GetIAccessibleFromResID(long resID);

    void NotifyDestroy( com::sun::star::accessibility::XAccessible* pXAcc );
    com::sun::star::accessibility::XAccessible* GetParentXAccessible( com::sun::star::accessibility::XAccessible* pXAcc );
    short GetParentRole( com::sun::star::accessibility::XAccessible* pXAcc );

    void SaveTopWindowHandle(HWND hWnd, com::sun::star::accessibility::XAccessible* pXAcc);

    void UpdateChildState(com::sun::star::accessibility::XAccessible* pXAcc);

    bool IsSpecialToolboItem(com::sun::star::accessibility::XAccessible* pXAcc);

    short GetRole(com::sun::star::accessibility::XAccessible* pXAcc);

    com::sun::star::accessibility::XAccessible* GetAccDocByAccTopWin( com::sun::star::accessibility::XAccessible* pXAcc );
    bool IsTopWinAcc( com::sun::star::accessibility::XAccessible* pXAcc );

    bool IsStateManageDescendant(com::sun::star::accessibility::XAccessible* pAccessible);

};
#endif
