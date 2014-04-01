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
#include <svtools/svtabbx.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

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
    Edit*           m_pEdit;
    OKButton*       m_pOKButton;

    DECL_LINK(OkButtonHandler, void *);
public:
    NewObjectDialog (Window* pParent, ObjectMode::Mode, bool bCheckName = false);
    OUString GetObjectName() const { return m_pEdit->GetText(); }
    void SetObjectName( const OUString& rName )
    {
        m_pEdit->SetText( rName );
        m_pEdit->SetSelection(Selection( 0, rName.getLength()));
    }
};

class GotoLineDialog : public ModalDialog
{
    Edit*           m_pEdit;
    OKButton*       m_pOKButton;
    DECL_LINK(OkButtonHandler, void *);
public:
    GotoLineDialog(Window * pParent);
    sal_Int32 GetLineNumber() const;
};

class ExportDialog : public ModalDialog
{
private:
    RadioButton*    m_pExportAsPackageButton;
    OKButton*       m_pOKButton;

    bool            mbExportAsPackage;

    DECL_LINK(OkButtonHandler, void *);

public:
    ExportDialog( Window * pParent );

    bool isExportAsPackage () const { return mbExportAsPackage; }
};


class ExtTreeListBox : public TreeListBox
{
protected:
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel  ) SAL_OVERRIDE;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData, SvTreeListEntry* pEntry ) SAL_OVERRIDE;
    virtual bool            NotifyAcceptDrop( SvTreeListEntry* pEntry ) SAL_OVERRIDE;

    virtual sal_Bool    NotifyMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos ) SAL_OVERRIDE;
    virtual sal_Bool    NotifyCopying( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos ) SAL_OVERRIDE;
    sal_Bool            NotifyCopyingMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos, sal_Bool bMove );

public:
    ExtTreeListBox(Window* pParent, WinBits nStyle);
    virtual ~ExtTreeListBox();
};

class CheckBox : public SvTabListBox
{
private:
    ObjectMode::Mode    eMode;
    SvLBoxButtonData*   pCheckButton;
    ScriptDocument      m_aDocument;
    void                Init();

public:
    CheckBox(Window* pParent, WinBits nStyle);
    virtual ~CheckBox();

    SvTreeListEntry*    DoInsertEntry( const OUString& rStr, sal_uLong nPos = LISTBOX_APPEND );
    SvTreeListEntry*    FindEntry( const OUString& rName );

    void            CheckEntryPos( sal_uLong nPos );
    bool            IsChecked( sal_uLong nPos ) const;

    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind) SAL_OVERRIDE;
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel ) SAL_OVERRIDE;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    void            SetMode (ObjectMode::Mode);
    ObjectMode::Mode GetMode () const { return eMode; }
};

class LibDialog: public ModalDialog
{
private:
    VclFrame*       m_pStorageFrame;
    CheckBox*       m_pLibBox;
    ::CheckBox*     m_pReferenceBox;
    ::CheckBox*     m_pReplaceBox;

public:
    LibDialog( Window* pParent );

    void            SetStorageName( const OUString& rName );

    CheckBox&       GetLibBox()                 { return *m_pLibBox; }
    bool            IsReference() const         { return m_pReferenceBox->IsChecked(); }
    bool            IsReplace() const           { return m_pReplaceBox->IsChecked(); }

    void            EnableReference (bool b)    { m_pReferenceBox->Enable(b); }
    void            EnableReplace (bool b)      { m_pReplaceBox->Enable(b); }
};

class OrganizeDialog : public TabDialog
{
private:
    TabControl*        m_pTabCtrl;
    EntryDescriptor    m_aCurEntry;

public:
    OrganizeDialog( Window* pParent, sal_Int16 tabId, EntryDescriptor& rDesc );
    virtual ~OrganizeDialog();

    virtual short   Execute() SAL_OVERRIDE;

    DECL_LINK( ActivatePageHdl, TabControl * );
};

class ObjectPage: public TabPage
{
protected:
    ExtTreeListBox*     m_pBasicBox;
    PushButton*         m_pEditButton;
    PushButton*         m_pNewModButton;
    PushButton*         m_pNewDlgButton;
    PushButton*         m_pDelButton;

    DECL_LINK( BasicBoxHighlightHdl, TreeListBox * );
    DECL_LINK( ButtonHdl, Button * );
    void                CheckButtons();
    bool                GetSelection( ScriptDocument& rDocument, OUString& rLibName );
    void                DeleteCurrent();
    void                NewModule();
    void                NewDialog();
    void                EndTabDialog( sal_uInt16 nRet );

    TabDialog*          pTabDlg;

    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual void        DeactivatePage() SAL_OVERRIDE;

public:
    ObjectPage(Window* pParent, const OString& rName, sal_uInt16 nMode);

    void                SetCurrentEntry( EntryDescriptor& rDesc );
    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};


class LibPage: public TabPage
{
protected:
    ListBox*            m_pBasicsBox;
    CheckBox*           m_pLibBox;
    PushButton*         m_pEditButton;
    PushButton*         m_pPasswordButton;
    PushButton*         m_pNewLibButton;
    PushButton*         m_pInsertLibButton;
    PushButton*         m_pExportButton;
    PushButton*         m_pDelButton;

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
    void                implExportLib( const OUString& aLibName, const OUString& aTargetURL,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler );
    void                Export();
    void                ExportAsPackage( const OUString& aLibName );
    void                ExportAsBasic( const OUString& aLibName );
    void                EndTabDialog( sal_uInt16 nRet );
    void                FillListBox();
    void                InsertListBoxEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                SetCurLib();
    SvTreeListEntry*        ImpInsertLibEntry( const OUString& rLibName, sal_uLong nPos );
    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual void        DeactivatePage() SAL_OVERRIDE;

    TabDialog*          pTabDlg;

public:
                        LibPage( Window* pParent );
    virtual             ~LibPage();

    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};

// Helper functions
SbModule* createModImpl( Window* pWin, const ScriptDocument& rDocument,
    TreeListBox& rBasicBox, const OUString& rLibName, const OUString& aModName, bool bMain = false );
void createLibImpl( Window* pWin, const ScriptDocument& rDocument,
                    CheckBox* pLibBox, TreeListBox* pBasicBox );

} // namespace basctl

#endif // BASCTL_MODULDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
