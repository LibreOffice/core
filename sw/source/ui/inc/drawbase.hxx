/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawbase.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:57:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SW_DRAWBASE_HXX
#define _SW_DRAWBASE_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

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
    USHORT          m_nSlotId;
    BOOL            m_bCreateObj  :1;
    BOOL            m_bInsForm   :1;

    Point           GetDefaultCenterPos();
public:
    SwDrawBase(SwWrtShell *pSh, SwEditWin* pWin, SwView* pView);
    virtual ~SwDrawBase();

    void         SetDrawPointer();
    void         EnterSelectMode(const MouseEvent& rMEvt);
    inline BOOL  IsInsertForm() const { return m_bInsForm; }
    inline BOOL  IsCreateObj() const { return m_bCreateObj; }

    // Mouse- & Key-Events; Returnwert=TRUE: Event wurde bearbeitet
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    void         BreakCreate();
    void         SetSlotId(USHORT nSlot) {m_nSlotId = nSlot;}
    USHORT       GetSlotId() { return m_nSlotId;}

    virtual void Activate(const USHORT nSlotId);    // Function aktivieren
    virtual void Deactivate();                      // Function deaktivieren

    virtual void CreateDefaultObject();

    // #i33136#
    virtual bool doConstructOrthogonal() const;
};



#endif      // _SW_DRAWBASE_HXX

