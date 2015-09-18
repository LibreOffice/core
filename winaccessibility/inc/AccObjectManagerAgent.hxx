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

#ifndef INCLUDED_WINACCESSIBILITY_INC_ACCOBJECTMANAGERAGENT_HXX
#define INCLUDED_WINACCESSIBILITY_INC_ACCOBJECTMANAGERAGENT_HXX

#include <memory>

#include <com/sun/star/accessibility/XAccessible.hpp>

struct IMAccessible;
struct IAccessible;
class AccObjectWinManager;

/****************************************************************
AccObjectMangerAgent is used for manaing AccObjects,which encapsulates
platform differences,and call AccObjectWinManager for Windows platform. To do for
Linux platform
*****************************************************************/
class AccObjectManagerAgent
{
private:

    std::unique_ptr<AccObjectWinManager> pWinManager;

public:

    AccObjectManagerAgent();
    virtual ~AccObjectManagerAgent();

    virtual bool InsertAccObj(com::sun::star::accessibility::XAccessible* pXAcc,
                              com::sun::star::accessibility::XAccessible* pParentXAcc,
                              sal_Int64 pWnd=0);
    virtual void GetIAccessibleFromResID(long childID,IMAccessible**);
    virtual bool GetIAccessibleFromXAccessible(com::sun::star::accessibility::XAccessible* pXAcc, IAccessible** ppIA);

    virtual void DeleteAccObj( com::sun::star::accessibility::XAccessible* pXAcc );
    virtual IMAccessible*  GetIMAccByXAcc(com::sun::star::accessibility::XAccessible* pXAcc);

    bool NotifyAccEvent(short pEvent = 0, com::sun::star::accessibility::XAccessible* pXAcc = 0);

    bool InsertChildrenAccObj(com::sun::star::accessibility::XAccessible* pXAcc,
                              sal_Int64 pWnd=0);
    void DeleteChildrenAccObj( com::sun::star::accessibility::XAccessible* pXAcc );

    void  DecreaseState( com::sun::star::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  IncreaseState( com::sun::star::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  UpdateState( com::sun::star::accessibility::XAccessible* pXAcc );

    void  UpdateLocation( com::sun::star::accessibility::XAccessible* pXAcc,
                          long Top = 0,long left = 0,long width = 0,long height = 0 );
    void  UpdateAction( com::sun::star::accessibility::XAccessible* pXAcc );

    void  UpdateValue( com::sun::star::accessibility::XAccessible* pXAcc );
    void  UpdateValue( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any );

    void  UpdateAccName( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any newName);
    void  UpdateAccName( com::sun::star::accessibility::XAccessible* pXAcc);

    void  UpdateDescription( com::sun::star::accessibility::XAccessible* pXAcc, com::sun::star::uno::Any newDesc );
    void  UpdateDescription( com::sun::star::accessibility::XAccessible* pXAcc );

    void NotifyDestroy(com::sun::star::accessibility::XAccessible* pXAcc);

    com::sun::star::accessibility::XAccessible* GetParentXAccessible(
        com::sun::star::accessibility::XAccessible* pXAcc );
    short GetParentRole(com::sun::star::accessibility::XAccessible* pXAcc );
    bool IsContainer(com::sun::star::accessibility::XAccessible* pXAcc);

    void SaveTopWindowHandle(sal_Int64 hWnd,
            com::sun::star::accessibility::XAccessible* pXAcc);

    void UpdateChildState(com::sun::star::accessibility::XAccessible* pXAcc);

    bool IsSpecialToolboItem(com::sun::star::accessibility::XAccessible* pXAcc);

    short GetRole(com::sun::star::accessibility::XAccessible* pXAcc);

    com::sun::star::accessibility::XAccessible* GetAccDocByAccTopWin( com::sun::star::accessibility::XAccessible* pXAcc );
    bool IsTopWinAcc( com::sun::star::accessibility::XAccessible* pXAcc );

    bool IsStateManageDescendant(com::sun::star::accessibility::XAccessible* pXAcc);

    sal_Int64 Get_ToATInterface(sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
