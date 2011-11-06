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


#ifndef SW_INPUTWIN_HXX
#define SW_INPUTWIN_HXX

#include <vcl/edit.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#include <sfx2/childwin.hxx>

class SwFldMgr;
class SwWrtShell;
class SwView;
class SfxDispatcher;

//========================================================================
class InputEdit : public Edit
{
public:
                    InputEdit(Window* pParent, WinBits nStyle) :
                        Edit(pParent , nStyle){}

    void            UpdateRange(const String& aSel,
                                const String& aTblName );

protected:
    virtual void    KeyInput( const KeyEvent&  );
};

//========================================================================

class SwInputWindow : public ToolBox
{
friend class InputEdit;

    Edit            aPos;
    InputEdit       aEdit;
    PopupMenu       aPopMenu;
    SwFldMgr*       pMgr;
    SwWrtShell*     pWrtShell;
    SwView*         pView;
    SfxBindings*    pBindings;
    String          aAktTableName, sOldFml;
    sal_Int32       m_nActionCount;

    sal_Bool        bFirst : 1;  //Initialisierungen beim ersten Aufruf
    sal_Bool        bActive : 1; //fuer Hide/Show beim Dokumentwechsel
    sal_Bool        bIsTable : 1;
    sal_Bool        bDelSel : 1;
    bool            m_bDoesUndo : 1;
    bool            m_bResetUndo : 1;
    bool            m_bCallUndo : 1;

    void CleanupUglyHackWithUndo();

    void DelBoxCntnt();
    DECL_LINK( ModifyHdl, InputEdit* );

    using Window::IsActive;

protected:
    virtual void    Resize();
    virtual void    Click();
    DECL_LINK( MenuHdl, Menu * );
    DECL_LINK( DropdownClickHdl, ToolBox* );
    void            ApplyFormula();
    void            CancelFormula();

public:
                    SwInputWindow( Window* pParent, SfxBindings* pBindings );
    virtual         ~SwInputWindow();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void            SelectHdl( ToolBox*);

    void            ShowWin();

    sal_Bool            IsActive(){ return bActive; };

    DECL_LINK( SelTblCellsNotify, SwWrtShell * );

    void            SetFormula( const String& rFormula, sal_Bool bDelSel = sal_True );
    const SwView*   GetView() const{return pView;}
};

class SwInputChild : public SfxChildWindow
{
    sal_Bool            bObjVis;
    SfxDispatcher*  pDispatch;
public:
    SwInputChild( Window* ,
                        sal_uInt16 nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    ~SwInputChild();
    SFX_DECL_CHILDWINDOW( SwInputChild );
    void            SetFormula( const String& rFormula, sal_Bool bDelSel = sal_True )
                        { ((SwInputWindow*)pWindow)->SetFormula(
                                    rFormula, bDelSel ); }
    const SwView*   GetView() const{return ((SwInputWindow*)pWindow)->GetView();}

};

//==================================================================

#endif

