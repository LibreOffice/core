/*************************************************************************
 *
 *  $RCSfile: moduldlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MODULDLG_HXX
#define _MODULDLG_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#include <bastype2.hxx>

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_TABDLG_HXX //autogen
#include <vcl/tabdlg.hxx>
#endif

#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif


#include <vcl/tabctrl.hxx>
#include <vcl/lstbox.hxx>

class StarBASIC;


#define NEWOBJECTMODE_LIB       1
#define NEWOBJECTMODE_MOD       2
#define NEWOBJECTMODE_DLG       3

class NewObjectDialog : public ModalDialog
{
private:
    FixedText       aText;
    Edit            aEdit;
    OKButton        aOKButton;
    CancelButton    aCancelButton;

    // NEWOBJECTMODE_LIB
    CheckBox        aCheckBox;

public:

                NewObjectDialog( Window* pParent, USHORT nMode );
                ~NewObjectDialog();

    String      GetObjectName() const { return aEdit.GetText(); }
    void        SetObjectName( const String& rName ) { aEdit.SetText( rName ); aEdit.SetSelection( Selection( 0, rName.Len() ) );}

    BOOL        IsSeparateFile() const              { return aCheckBox.IsChecked(); }
    void        EnableSeparateFile( BOOL bEnable )  { aCheckBox.Enable( bEnable ); }
};


class ExtBasicTreeListBox : public BasicTreeListBox
{
protected:
    virtual BOOL    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  );
    virtual BOOL    EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );

    virtual DragDropMode    NotifyBeginDrag( SvLBoxEntry* );
    virtual BOOL    NotifyQueryDrop( SvLBoxEntry* );

    virtual BOOL    NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos );
    virtual BOOL    NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos );
    BOOL            NotifyCopyingMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos, BOOL bMove );

public:
    ExtBasicTreeListBox( Window* pParent, const ResId& rRes );
    ~ExtBasicTreeListBox();

};

#define LIBMODE_CHOOSER     1
#define LIBMODE_MANAGER     2

class BasicCheckBox : public SvTabListBox
{
private:
    USHORT              nMode;
    SvLBoxButtonData*   pCheckButton;
    BasicManager*       pBasMgr;
    void                Init();

protected:
    virtual void    CheckButtonHdl();

public:
                    BasicCheckBox( Window* pParent, const ResId& rResId );
                    ~BasicCheckBox();

    SvLBoxEntry*    InsertEntry( const String& rStr, ULONG nPos = LISTBOX_APPEND );
    void            RemoveEntry( ULONG nPos );
    SvLBoxEntry*    FindEntry( const String& rName );

    void            SelectEntryPos( ULONG nPos, BOOL bSelect = TRUE );
    ULONG           GetSelectEntryPos() const;

    ULONG           GetCheckedEntryCount() const;
    void            CheckEntryPos( ULONG nPos, BOOL bCheck = TRUE );
    BOOL            IsChecked( ULONG nPos ) const;

    virtual BOOL    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel );
    virtual BOOL    EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );

    void            SetBasicManager( BasicManager* pMgr )   { pBasMgr = pMgr; }
    BasicManager*   GetBasicManager() const                 { return pBasMgr; }

    void            SetMode( USHORT n )     { nMode = n; }
    USHORT          GetMode() const         { return nMode; }

};

class BasicCheckBoxBitmaps : public Resource
{
private:
    const Bitmap    aUncheckedBmp;
    const Bitmap    aCheckedBmp;
    const Bitmap    aHiCheckedBmp;
    const Bitmap    aHiUncheckedBmp;
    const Bitmap    aTriStateBmp;
    const Bitmap    aHiTriStateBmp;

public:
                    BasicCheckBoxBitmaps();

    const Bitmap&   GetUncheckedBmp   () const { return aUncheckedBmp; }
    const Bitmap&   GetCheckedBmp     () const { return aCheckedBmp; }
    const Bitmap&   GetHiCheckedBmp   () const { return aHiCheckedBmp; }
    const Bitmap&   GetHiUncheckedBmp () const { return aHiUncheckedBmp; }
    const Bitmap&   GetTriStateBmp    () const { return aTriStateBmp; }
    const Bitmap&   GetHiTriStateBmp  () const { return aHiTriStateBmp; }
};


class LibDialog: public ModalDialog
{
private:
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    FixedText       aStorageName;

    BasicCheckBox   aLibBox;

    GroupBox        aGroupBox;
    CheckBox        aReferenceBox;
    CheckBox        aSepFileBox;
    CheckBox        aReplaceBox;

    BOOL            bSepFileEnabled;

protected:
    DECL_LINK(      CheckBoxSelectHdl, CheckBox* );


public:
                    LibDialog( Window* pParent );
                    ~LibDialog();

    void            SetStorageName( const String& rName );

    BasicCheckBox&  GetLibBox()                 { return aLibBox; }
    BOOL            IsReference() const         { return aReferenceBox.IsChecked(); }
    BOOL            IsReplace() const           { return aReplaceBox.IsChecked(); }
    BOOL            IsSeparateFile() const          { return aSepFileBox.IsChecked(); }

    void            SetSeparateFileEnabled( BOOL b )    { bSepFileEnabled = b;
                                                            aSepFileBox.Enable( b ); }
    BOOL            IsSeparateFileEnabled() const       { return bSepFileEnabled; }
};


class OrganizeDialog : public TabDialog
{
private:
    TabControl      aTabCtrl;

public:
                    OrganizeDialog( Window* pParent );
                    ~OrganizeDialog();

    void            SetCurrentModule( const String& rMacroDescr );
    virtual short   Execute();

    DECL_LINK( ActivatePageHdl, TabControl * );
};

class ObjectPage: public TabPage
{
protected:
    PushButton          aEditButton;
    CancelButton        aCloseButton;
    PushButton          aNewModButton;
    PushButton          aNewDlgButton;
    PushButton          aDelButton;
    FixedText           aLibText;
//  Edit                aEdit;
    ExtBasicTreeListBox aBasicBox;

    String              aCurEntryDescr;

    DECL_LINK( BasicBoxHighlightHdl, BasicTreeListBox * );
//  DECL_LINK( EditModifyHdl, Edit * );
    DECL_LINK( ButtonHdl, Button * );
    void                CheckButtons();
    StarBASIC*          GetSelectedBasic();
    void                DeleteCurrent();
    void                NewModule();
    void                NewDialog();
    void                EndTabDialog( USHORT nRet );

    TabDialog*          pTabDlg;

    void                ImplMarkCurrentModule();

//  BOOL                UseEditText();
    virtual void        ActivatePage();
    virtual void        DeactivatePage();

public:
                        ObjectPage( Window* pParent );

    void                SetCurrentModule( const String& rMacroDescr ) { aCurEntryDescr = rMacroDescr; ImplMarkCurrentModule(); }
    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};

class LibPage: public TabPage
{
protected:
    PushButton          aEditButton;
    CancelButton        aCloseButton;
    PushButton          aNewLibButton;
    PushButton          aPasswordButton;
    PushButton          aInsertLibButton;
    PushButton          aDelButton;
    FixedText           aLibText;
    ListBox             aBasicsBox;
    BasicCheckBox       aLibBox;

    String              aCurBasMgr;

    DECL_LINK( TreeListHighlightHdl, SvTreeListBox * );
    DECL_LINK( BasicSelectHdl, ListBox * );
    DECL_LINK( CheckBoxHdl, SvTreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    void                CheckButtons();
    void                DeleteCurrent();
    void                NewLib();
    void                InsertLib();
    void                EndTabDialog( USHORT nRet );
    void                FillListBox();
    void                SetCurLib();
    SvLBoxEntry*        ImpInsertLibEntry( USHORT nLib );
    void                ActivateCurrentLibSettings();
    virtual void        ActivatePage();
    virtual void        DeactivatePage();

    TabDialog*          pTabDlg;

public:
                        LibPage( Window* pParent );

    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};

#endif // _MODULDLG_HXX
