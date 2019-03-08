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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DRAWBASE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DRAWBASE_HXX

#include <tools/gen.hxx>

class SwView;
class SwWrtShell;
class SwEditWin;
class KeyEvent;
class MouseEvent;

// base class for all functions
class SwDrawBase
{
protected:
    SwView*         m_pView;
    SwWrtShell*     m_pSh;
    VclPtr<SwEditWin> m_pWin;
    Point           m_aStartPos;                 // position of BeginCreate
    sal_uInt16      m_nSlotId;
    bool            m_bCreateObj  :1;
    bool            m_bInsForm   :1;

    Point           GetDefaultCenterPos();
public:
    SwDrawBase(SwWrtShell *pSh, SwEditWin* pWin, SwView* pView);
    virtual ~SwDrawBase();

    void         SetDrawPointer();
    void         EnterSelectMode(const MouseEvent& rMEvt);
    bool  IsInsertForm() const { return m_bInsForm; }
    bool  IsCreateObj() const { return m_bCreateObj; }

    // mouse- & key events; return value=true: event was edited
    bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    void         BreakCreate();
    void         SetSlotId(sal_uInt16 nSlot) {m_nSlotId = nSlot;}
    sal_uInt16       GetSlotId() { return m_nSlotId;}

    virtual void Activate(const sal_uInt16 nSlotId);    // activate function
    virtual void Deactivate();                      // deactivate function

    virtual void CreateDefaultObject();

    // #i33136#
    virtual bool doConstructOrthogonal() const;
};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_DRAWBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
