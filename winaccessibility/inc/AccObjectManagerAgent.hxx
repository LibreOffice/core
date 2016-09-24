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
AccObjectMangerAgent is used for managing AccObjects, which encapsulates
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

    virtual bool InsertAccObj(css::accessibility::XAccessible* pXAcc,
                              css::accessibility::XAccessible* pParentXAcc,
                              sal_Int64 pWnd=0);
    virtual void GetIAccessibleFromResID(long childID,IMAccessible**);
    virtual bool GetIAccessibleFromXAccessible(css::accessibility::XAccessible* pXAcc, IAccessible** ppIA);

    virtual void DeleteAccObj( css::accessibility::XAccessible* pXAcc );
    virtual IMAccessible*  GetIMAccByXAcc(css::accessibility::XAccessible* pXAcc);

    bool NotifyAccEvent(short pEvent = 0, css::accessibility::XAccessible* pXAcc = 0);

    bool InsertChildrenAccObj(css::accessibility::XAccessible* pXAcc,
                              sal_Int64 pWnd=0);
    void DeleteChildrenAccObj( css::accessibility::XAccessible* pXAcc );

    void  DecreaseState( css::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  IncreaseState( css::accessibility::XAccessible* pXAcc,unsigned short pState );
    void  UpdateState( css::accessibility::XAccessible* pXAcc );

    void  UpdateLocation( css::accessibility::XAccessible* pXAcc,
                          long Top = 0,long left = 0,long width = 0,long height = 0 );
    void  UpdateAction( css::accessibility::XAccessible* pXAcc );

    void  UpdateValue( css::accessibility::XAccessible* pXAcc );
    void  UpdateValue( css::accessibility::XAccessible* pXAcc, css::uno::Any );

    void  UpdateAccName( css::accessibility::XAccessible* pXAcc, css::uno::Any newName);
    void  UpdateAccName( css::accessibility::XAccessible* pXAcc);

    void  UpdateDescription( css::accessibility::XAccessible* pXAcc, css::uno::Any newDesc );
    void  UpdateDescription( css::accessibility::XAccessible* pXAcc );

    void NotifyDestroy(css::accessibility::XAccessible* pXAcc);

    css::accessibility::XAccessible* GetParentXAccessible( css::accessibility::XAccessible* pXAcc );
    short GetParentRole(css::accessibility::XAccessible* pXAcc );
    bool IsContainer(css::accessibility::XAccessible* pXAcc);

    void SaveTopWindowHandle(sal_Int64 hWnd,
            css::accessibility::XAccessible* pXAcc);

    void UpdateChildState(css::accessibility::XAccessible* pXAcc);

    bool IsSpecialToolboItem(css::accessibility::XAccessible* pXAcc);

    short GetRole(css::accessibility::XAccessible* pXAcc);

    css::accessibility::XAccessible* GetAccDocByAccTopWin( css::accessibility::XAccessible* pXAcc );
    bool IsTopWinAcc( css::accessibility::XAccessible* pXAcc );

    bool IsStateManageDescendant(css::accessibility::XAccessible* pXAcc);

    sal_Int64 Get_ToATInterface(sal_Int64 hWnd, sal_Int64 lParam, sal_Int64 wParam);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
