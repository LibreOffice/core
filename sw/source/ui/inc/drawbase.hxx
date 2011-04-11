/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SW_DRAWBASE_HXX
#define _SW_DRAWBASE_HXX

#include <tools/gen.hxx>

class SwView;
class SwWrtShell;
class SwEditWin;
class KeyEvent;
class MouseEvent;

#define MIN_FREEHAND_DISTANCE   10

/*************************************************************************
|*
|* Basisklasse fuer alle Funktionen
|*
\************************************************************************/

class SwDrawBase
{
protected:
    SwView*         m_pView;
    SwWrtShell*     m_pSh;
    SwEditWin*      m_pWin;
    Point           m_aStartPos;                 // Position von BeginCreate
    Point           m_aMDPos;                // Position von MouseButtonDown
    sal_uInt16          m_nSlotId;
    sal_Bool            m_bCreateObj  :1;
    sal_Bool            m_bInsForm   :1;

    Point           GetDefaultCenterPos();
public:
    SwDrawBase(SwWrtShell *pSh, SwEditWin* pWin, SwView* pView);
    virtual ~SwDrawBase();

    void         SetDrawPointer();
    void         EnterSelectMode(const MouseEvent& rMEvt);
    inline sal_Bool  IsInsertForm() const { return m_bInsForm; }
    inline sal_Bool  IsCreateObj() const { return m_bCreateObj; }

    // Mouse- & Key-Events; Returnwert=sal_True: Event wurde bearbeitet
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    void         BreakCreate();
    void         SetSlotId(sal_uInt16 nSlot) {m_nSlotId = nSlot;}
    sal_uInt16       GetSlotId() { return m_nSlotId;}

    virtual void Activate(const sal_uInt16 nSlotId);    // Function aktivieren
    virtual void Deactivate();                      // Function deaktivieren

    virtual void CreateDefaultObject();

    // #i33136#
    virtual bool doConstructOrthogonal() const;
};



#endif      // _SW_DRAWBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
