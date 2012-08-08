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

#ifndef _MODULDLG_HXX
#define _MODULDLG_HXX

#include <svheader.hxx>

#include <bastype2.hxx>
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svtabbx.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include "com/sun/star/task/XInteractionHandler.hpp"

#include <vcl/tabctrl.hxx>
#include <vcl/lstbox.hxx>

enum NewObjectMode
{
    NEWOBJECTMODE_LIB  = 1,
    NEWOBJECTMODE_MOD  = 2,
    NEWOBJECTMODE_DLG  = 3,
    NEWOBJECTMODE_METH = 4
};

class NewObjectDialog : public ModalDialog
{
private:
    FixedText       aText;
    Edit            aEdit;
    OKButton        aOKButton;
    CancelButton    aCancelButton;

    DECL_LINK(OkButtonHandler, void *);

public:
    NewObjectDialog(Window * pParent, NewObjectMode nMode, bool bCheckName = false);
                ~NewObjectDialog();

    String      GetObjectName() const { return aEdit.GetText(); }
    void        SetObjectName( const String& rName ) { aEdit.SetText( rName ); aEdit.SetSelection( Selection( 0, rName.Len() ) );}
};

class GotoLineDialog : public ModalDialog
{
    FixedText       aText;
    Edit            aEdit;
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    DECL_LINK(OkButtonHandler, void *);
public:
    GotoLineDialog( Window * pParent );
    sal_Int32   GetLineNumber();
};

class ExportDialog : public ModalDialog
{
private:
    RadioButton     maExportAsPackageButton;
    RadioButton     maExportAsBasicButton;
    OKButton        maOKButton;
    CancelButton    maCancelButton;

    bool            mbExportAsPackage;

    DECL_LINK(OkButtonHandler, void *);

public:
    ExportDialog( Window * pParent );
    ~ExportDialog();

    bool isExportAsPackage () { return mbExportAsPackage; }
};


class ExtBasicTreeListBox : public BasicTreeListBox
{
protected:
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const rtl::OUString& rNewText );

    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData, SvLBoxEntry* pEntry );
    virtual sal_Bool            NotifyAcceptDrop( SvLBoxEntry* pEntry );

    virtual sal_Bool    NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos );
    virtual sal_Bool    NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos );
    sal_Bool            NotifyCopyingMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos, sal_Bool bMove );

public:
    ExtBasicTreeListBox( Window* pParent, const ResId& rRes );
    ~ExtBasicTreeListBox();
};

#define LIBMODE_CHOOSER     1
#define LIBMODE_MANAGER     2

class BasicCheckBox : public SvTabListBox
{
private:
    NewObjectMode       nMode;
    SvLBoxButtonData*   pCheckButton;
    ScriptDocument      m_aDocument;
    void                Init();

public:
                    BasicCheckBox( Window* pParent, const ResId& rResId );
                    ~BasicCheckBox();

    SvLBoxEntry*    DoInsertEntry( const String& rStr, sal_uLong nPos = LISTBOX_APPEND );
    SvLBoxEntry*    FindEntry( const String& rName );

    void            CheckEntryPos( sal_uLong nPos );
    bool            IsChecked( sal_uLong nPos ) const;

    virtual void    InitEntry( SvLBoxEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind );
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const rtl::OUString& rNewText );

    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    void            SetMode( NewObjectMode n );
    NewObjectMode   GetMode() const { return nMode; }
};

class LibDialog: public ModalDialog
{
private:
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    FixedText       aStorageName;
    BasicCheckBox   aLibBox;
    FixedLine       aFixedLine;
    CheckBox        aReferenceBox;
    CheckBox        aReplaceBox;

public:
                    LibDialog( Window* pParent );
                    ~LibDialog();

    void            SetStorageName( const ::rtl::OUString& rName );

    BasicCheckBox&  GetLibBox()                 { return aLibBox; }
    bool            IsReference() const         { return aReferenceBox.IsChecked(); }
    bool            IsReplace() const           { return aReplaceBox.IsChecked(); }

    void            EnableReference (bool b)    { aReferenceBox.Enable(b); }
    void            EnableReplace (bool b)      { aReplaceBox.Enable(b); }
};


class OrganizeDialog : public TabDialog
{
private:
    TabControl              aTabCtrl;
    BasicEntryDescriptor    m_aCurEntry;

public:
                    OrganizeDialog( Window* pParent, sal_Int16 tabId, BasicEntryDescriptor& rDesc );
                    ~OrganizeDialog();

    virtual short   Execute();

    DECL_LINK( ActivatePageHdl, TabControl * );
};

class ObjectPage: public TabPage
{
protected:
    FixedText           aLibText;
    ExtBasicTreeListBox aBasicBox;
    PushButton          aEditButton;
    CancelButton        aCloseButton;
    PushButton          aNewModButton;
    PushButton          aNewDlgButton;
    PushButton          aDelButton;

    DECL_LINK( BasicBoxHighlightHdl, BasicTreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    void                CheckButtons();
    bool                GetSelection( ScriptDocument& rDocument, ::rtl::OUString& rLibName );
    void                DeleteCurrent();
    void                NewModule();
    void                NewDialog();
    void                EndTabDialog( sal_uInt16 nRet );

    TabDialog*          pTabDlg;

    virtual void        ActivatePage();
    virtual void        DeactivatePage();

public:
                        ObjectPage( Window* pParent, const ResId& rResId, sal_uInt16 nMode );

    void                SetCurrentEntry( BasicEntryDescriptor& rDesc );
    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};


class SvxPasswordDialog;

class LibPage: public TabPage
{
protected:
    FixedText           aBasicsText;
    ListBox             aBasicsBox;
    FixedText           aLibText;
    BasicCheckBox       aLibBox;
    PushButton          aEditButton;
    CancelButton        aCloseButton;
    PushButton          aPasswordButton;
    PushButton          aNewLibButton;
    PushButton          aInsertLibButton;
    PushButton          aExportButton;
    PushButton          aDelButton;

    ScriptDocument      m_aCurDocument;
    LibraryLocation     m_eCurLocation;

    DECL_LINK( TreeListHighlightHdl, SvTreeListBox * );
    DECL_LINK( BasicSelectHdl, ListBox * );
    DECL_LINK( ButtonHdl, Button * );
    DECL_LINK( CheckPasswordHdl, SvxPasswordDialog * );
    void                CheckButtons();
    void                DeleteCurrent();
    void                NewLib();
    void                InsertLib();
    void                implExportLib( const String& aLibName, const String& aTargetURL,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );
    void                Export();
    void                ExportAsPackage( const String& aLibName );
    void                ExportAsBasic( const String& aLibName );
    void                EndTabDialog( sal_uInt16 nRet );
    void                FillListBox();
    void                InsertListBoxEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                SetCurLib();
    SvLBoxEntry*        ImpInsertLibEntry( const String& rLibName, sal_uLong nPos );
    virtual void        ActivatePage();
    virtual void        DeactivatePage();

    TabDialog*          pTabDlg;

public:
                        LibPage( Window* pParent );
    virtual             ~LibPage();

    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};

// Helper functions
SbModule* createModImpl( Window* pWin, const ScriptDocument& rDocument,
    BasicTreeListBox& rBasicBox, const ::rtl::OUString& rLibName, ::rtl::OUString aModName, bool bMain = false );
void createLibImpl( Window* pWin, const ScriptDocument& rDocument,
                    BasicCheckBox* pLibBox, BasicTreeListBox* pBasicBox );

#endif // _MODULDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
