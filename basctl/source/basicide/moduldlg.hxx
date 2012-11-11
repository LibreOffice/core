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

#ifndef BASCTL_MODULDLG_HXX
#define BASCTL_MODULDLG_HXX

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

class SvxPasswordDialog;

namespace basctl
{

namespace ObjectMode
{
    enum Mode
    {
        Library = 1,
        Module  = 2,
        Dialog  = 3,
        Method  = 4,
    };
}

class NewObjectDialog : public ModalDialog
{
private:
    FixedText       aText;
    Edit            aEdit;
    OKButton        aOKButton;
    CancelButton    aCancelButton;

    DECL_LINK(OkButtonHandler, void *);

public:
    NewObjectDialog (Window* pParent, ObjectMode::Mode, bool bCheckName = false);
    virtual ~NewObjectDialog ();

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


class ExtTreeListBox : public TreeListBox
{
protected:
    virtual sal_Bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel  );
    virtual sal_Bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText );

    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData, SvTreeListEntry* pEntry );
    virtual sal_Bool            NotifyAcceptDrop( SvTreeListEntry* pEntry );

    virtual sal_Bool    NotifyMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos );
    virtual sal_Bool    NotifyCopying( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos );
    sal_Bool            NotifyCopyingMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos, sal_Bool bMove );

public:
    ExtTreeListBox( Window* pParent, const ResId& rRes );
    ~ExtTreeListBox();
};

class CheckBox : public SvTabListBox
{
private:
    ObjectMode::Mode    eMode;
    SvLBoxButtonData*   pCheckButton;
    ScriptDocument      m_aDocument;
    void                Init();

public:
    CheckBox( Window* pParent, const ResId& rResId );
    ~CheckBox();

    SvTreeListEntry*    DoInsertEntry( const String& rStr, sal_uLong nPos = LISTBOX_APPEND );
    SvTreeListEntry*    FindEntry( const String& rName );

    void            CheckEntryPos( sal_uLong nPos );
    bool            IsChecked( sal_uLong nPos ) const;

    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind);
    virtual sal_Bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel );
    virtual sal_Bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText );

    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    void            SetMode (ObjectMode::Mode);
    ObjectMode::Mode GetMode () const { return eMode; }
};

class LibDialog: public ModalDialog
{
private:
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    FixedText       aStorageName;
    CheckBox        aLibBox;
    FixedLine       aFixedLine;
    ::CheckBox      aReferenceBox;
    ::CheckBox      aReplaceBox;

public:
                    LibDialog( Window* pParent );
                    ~LibDialog();

    void            SetStorageName( const OUString& rName );

    CheckBox&       GetLibBox()                 { return aLibBox; }
    bool            IsReference() const         { return aReferenceBox.IsChecked(); }
    bool            IsReplace() const           { return aReplaceBox.IsChecked(); }

    void            EnableReference (bool b)    { aReferenceBox.Enable(b); }
    void            EnableReplace (bool b)      { aReplaceBox.Enable(b); }
};


class OrganizeDialog : public TabDialog
{
private:
    TabControl         aTabCtrl;
    EntryDescriptor    m_aCurEntry;

public:
    OrganizeDialog( Window* pParent, sal_Int16 tabId, EntryDescriptor& rDesc );
    ~OrganizeDialog();

    virtual short   Execute();

    DECL_LINK( ActivatePageHdl, TabControl * );
};

class ObjectPage: public TabPage
{
protected:
    FixedText           aLibText;
    ExtTreeListBox      aBasicBox;
    PushButton          aEditButton;
    CancelButton        aCloseButton;
    PushButton          aNewModButton;
    PushButton          aNewDlgButton;
    PushButton          aDelButton;

    DECL_LINK( BasicBoxHighlightHdl, TreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    void                CheckButtons();
    bool                GetSelection( ScriptDocument& rDocument, OUString& rLibName );
    void                DeleteCurrent();
    void                NewModule();
    void                NewDialog();
    void                EndTabDialog( sal_uInt16 nRet );

    TabDialog*          pTabDlg;

    virtual void        ActivatePage();
    virtual void        DeactivatePage();

public:
                        ObjectPage( Window* pParent, const ResId& rResId, sal_uInt16 nMode );

    void                SetCurrentEntry( EntryDescriptor& rDesc );
    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};


class LibPage: public TabPage
{
protected:
    FixedText           aBasicsText;
    ListBox             aBasicsBox;
    FixedText           aLibText;
    CheckBox            aLibBox;
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
    SvTreeListEntry*        ImpInsertLibEntry( const String& rLibName, sal_uLong nPos );
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
    TreeListBox& rBasicBox, const OUString& rLibName, OUString aModName, bool bMain = false );
void createLibImpl( Window* pWin, const ScriptDocument& rDocument,
                    CheckBox* pLibBox, TreeListBox* pBasicBox );

} // namespace basctl

#endif // BASCTL_MODULDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
