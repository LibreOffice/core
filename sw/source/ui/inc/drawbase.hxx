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
#ifndef _SW_DRAWBASE_HXX
#define _SW_DRAWBASE_HXX

#include <tools/gen.hxx>

class SwView;
class SwWrtShell;
class SwEditWin;
class KeyEvent;
class MouseEvent;

/*************************************************************************
|*
|* base class for all functions
|*
\************************************************************************/

class SwDrawBase
{
protected:
    SwView*         m_pView;
    SwWrtShell*     m_pSh;
    SwEditWin*      m_pWin;
    Point           m_aStartPos;                 // position of BeginCreate
    Point           m_aMDPos;                // position of MouseButtonDown
    sal_uInt16          m_nSlotId;
    bool            m_bCreateObj  :1;
    bool            m_bInsForm   :1;

    Point           GetDefaultCenterPos();
public:
    SwDrawBase(SwWrtShell *pSh, SwEditWin* pWin, SwView* pView);
    virtual ~SwDrawBase();

    void         SetDrawPointer();
    void         EnterSelectMode(const MouseEvent& rMEvt);
    inline bool  IsInsertForm() const { return m_bInsForm; }
    inline bool  IsCreateObj() const { return m_bCreateObj; }

    // mouse- & key events; return value=sal_True: event was edited
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    void         BreakCreate();
    void         SetSlotId(sal_uInt16 nSlot) {m_nSlotId = nSlot;}
    sal_uInt16       GetSlotId() { return m_nSlotId;}

    virtual void Activate(const sal_uInt16 nSlotId);    // activate function
    virtual void Deactivate();                      // deactivate function

    virtual void CreateDefaultObject();

    // #i33136#
    virtual bool doConstructOrthogonal() const;
};



#endif      // _SW_DRAWBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
