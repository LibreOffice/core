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
    basegfx::B2DPoint   m_aStartPos;                 // Position von BeginCreate
    basegfx::B2DPoint   m_aMDPos;                // Position von MouseButtonDown
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

