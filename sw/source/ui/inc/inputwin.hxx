/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inputwin.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:19:26 $
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
#ifndef SW_INPUTWIN_HXX
#define SW_INPUTWIN_HXX


#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

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
    USHORT          nActionCnt;

    BOOL            bFirst : 1;  //Initialisierungen beim ersten Aufruf
    BOOL            bActive : 1; //fuer Hide/Show beim Dokumentwechsel
    BOOL            bIsTable : 1;
    BOOL            bDelSel : 1;
    BOOL            bDoesUndo : 1;
    BOOL            bResetUndo : 1;
    BOOL            bCallUndo : 1;


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

    BOOL            IsActive(){ return bActive; };

    DECL_LINK( SelTblCellsNotify, SwWrtShell * );

    void            SetFormula( const String& rFormula, BOOL bDelSel = TRUE );
    const SwView*   GetView() const{return pView;}
};

class SwInputChild : public SfxChildWindow
{
    BOOL            bObjVis;
    SfxDispatcher*  pDispatch;
public:
    SwInputChild( Window* ,
                        USHORT nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    ~SwInputChild();
    SFX_DECL_CHILDWINDOW( SwInputChild );
    void            SetFormula( const String& rFormula, BOOL bDelSel = TRUE )
                        { ((SwInputWindow*)pWindow)->SetFormula(
                                    rFormula, bDelSel ); }
    const SwView*   GetView() const{return ((SwInputWindow*)pWindow)->GetView();}

};

//==================================================================

#endif

