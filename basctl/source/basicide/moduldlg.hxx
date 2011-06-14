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

class StarBASIC;


#define NEWOBJECTMODE_LIB       1
#define NEWOBJECTMODE_MOD       2
#define NEWOBJECTMODE_DLG       3
#define NEWOBJECTMODE_METH      4

class NewObjectDialog : public ModalDialog
{
private:
    FixedText       aText;
    Edit            aEdit;
    OKButton        aOKButton;
    CancelButton    aCancelButton;

    DECL_LINK(OkButtonHandler, Button *);

public:
    NewObjectDialog(Window * pParent, sal_uInt16 nMode, bool bCheckName = false);
                ~NewObjectDialog();

    String      GetObjectName() const { return aEdit.GetText(); }
    void        SetObjectName( const String& rName ) { aEdit.SetText( rName ); aEdit.SetSelection( Selection( 0, rName.Len() ) );}
};

class ExportDialog : public ModalDialog
{
private:
    RadioButton     maExportAsPackageButton;
    RadioButton     maExportAsBasicButton;
    OKButton        maOKButton;
    CancelButton    maCancelButton;

    sal_Bool        mbExportAsPackage;

    DECL_LINK(OkButtonHandler, Button *);

public:
    ExportDialog( Window * pParent );
    ~ExportDialog();

    sal_Bool        isExportAsPackage( void ) { return mbExportAsPackage; }
};


class ExtBasicTreeListBox : public BasicTreeListBox
{
protected:
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel  );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );

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
    sal_uInt16              nMode;
    SvLBoxButtonData*   pCheckButton;
    ScriptDocument      m_aDocument;
    void                Init();

public:
                    BasicCheckBox( Window* pParent, const ResId& rResId );
                    ~BasicCheckBox();

    SvLBoxEntry*    DoInsertEntry( const String& rStr, sal_uLong nPos = LISTBOX_APPEND );
    SvLBoxEntry*    FindEntry( const String& rName );

    void            CheckEntryPos( sal_uLong nPos, sal_Bool bCheck = sal_True );
    sal_Bool            IsChecked( sal_uLong nPos ) const;

    virtual void    InitEntry( SvLBoxEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind );
    virtual sal_Bool    EditingEntry( SvLBoxEntry* pEntry, Selection& rSel );
    virtual sal_Bool    EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );

    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    void            SetMode( sal_uInt16 n );
    sal_uInt16          GetMode() const         { return nMode; }
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

    void            SetStorageName( const String& rName );

    BasicCheckBox&  GetLibBox()                 { return aLibBox; }
    sal_Bool            IsReference() const         { return aReferenceBox.IsChecked(); }
    sal_Bool            IsReplace() const           { return aReplaceBox.IsChecked(); }

    void            EnableReference( sal_Bool b )   { aReferenceBox.Enable( b ); }
    void            EnableReplace( sal_Bool b )     { aReplaceBox.Enable( b ); }
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
    bool                GetSelection( ScriptDocument& rDocument, String& rLibName );
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
    BasicTreeListBox& rBasicBox, const String& rLibName, String aModName, bool bMain = false );
void createLibImpl( Window* pWin, const ScriptDocument& rDocument,
                    BasicCheckBox* pLibBox, BasicTreeListBox* pBasicBox );

#endif // _MODULDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
