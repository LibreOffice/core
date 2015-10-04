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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_MODULDLG_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_MODULDLG_HXX

#include <bastype2.hxx>
#include <svtools/svtabbx.hxx>
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
    VclPtr<Edit>           m_pEdit;
    VclPtr<OKButton>       m_pOKButton;

    DECL_LINK_TYPED(OkButtonHandler, Button*, void);
public:
    NewObjectDialog (vcl::Window* pParent, ObjectMode::Mode, bool bCheckName = false);
    virtual ~NewObjectDialog();
    virtual void dispose() SAL_OVERRIDE;
    OUString GetObjectName() const { return m_pEdit->GetText(); }
    void SetObjectName( const OUString& rName )
    {
        m_pEdit->SetText( rName );
        m_pEdit->SetSelection(Selection( 0, rName.getLength()));
    }
};

class GotoLineDialog : public ModalDialog
{
    VclPtr<Edit>           m_pEdit;
    VclPtr<OKButton>       m_pOKButton;
    DECL_LINK_TYPED(OkButtonHandler, Button*, void);
public:
    explicit GotoLineDialog(vcl::Window * pParent);
    virtual ~GotoLineDialog();
    virtual void dispose() SAL_OVERRIDE;
    sal_Int32 GetLineNumber() const;
};

class ExportDialog : public ModalDialog
{
private:
    VclPtr<RadioButton>    m_pExportAsPackageButton;
    VclPtr<OKButton>       m_pOKButton;

    bool            mbExportAsPackage;

    DECL_LINK_TYPED(OkButtonHandler, Button*, void);

public:
    explicit ExportDialog( vcl::Window * pParent );
    virtual ~ExportDialog();
    virtual void dispose() SAL_OVERRIDE;

    bool isExportAsPackage () const { return mbExportAsPackage; }
};


class ExtTreeListBox : public TreeListBox
{
protected:
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel  ) SAL_OVERRIDE;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData, SvTreeListEntry* pEntry ) SAL_OVERRIDE;
    virtual bool            NotifyAcceptDrop( SvTreeListEntry* pEntry ) SAL_OVERRIDE;

    virtual TriState    NotifyMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos ) SAL_OVERRIDE;
    virtual TriState    NotifyCopying( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos ) SAL_OVERRIDE;
    TriState            NotifyCopyingMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos, bool bMove );

public:
    ExtTreeListBox(vcl::Window* pParent, WinBits nStyle);
};

class CheckBox : public SvTabListBox
{
private:
    ObjectMode::Mode    eMode;
    SvLBoxButtonData*   pCheckButton;
    ScriptDocument      m_aDocument;
    void                Init();

public:
    CheckBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~CheckBox();
    virtual void    dispose() SAL_OVERRIDE;

    SvTreeListEntry*    DoInsertEntry( const OUString& rStr, sal_uLong nPos = LISTBOX_APPEND );
    SvTreeListEntry*    FindEntry( const OUString& rName );

    void            CheckEntryPos( sal_uLong nPos );
    bool            IsChecked( sal_uLong nPos ) const;

    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind) SAL_OVERRIDE;
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel ) SAL_OVERRIDE;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    void            SetMode (ObjectMode::Mode);
};

class LibDialog: public ModalDialog
{
private:
    VclPtr<VclFrame>       m_pStorageFrame;
    VclPtr<CheckBox>       m_pLibBox;
    VclPtr< ::CheckBox>       m_pReferenceBox;
    VclPtr< ::CheckBox>       m_pReplaceBox;

public:
    explicit LibDialog(vcl::Window* pParent);
    virtual ~LibDialog();
    virtual void dispose() SAL_OVERRIDE;

    void            SetStorageName( const OUString& rName );

    CheckBox&       GetLibBox()                 { return *m_pLibBox; }
    bool            IsReference() const         { return m_pReferenceBox->IsChecked(); }
    bool            IsReplace() const           { return m_pReplaceBox->IsChecked(); }

    void            EnableReference (bool b)    { m_pReferenceBox->Enable(b); }
};

class OrganizeDialog : public TabDialog
{
private:
    VclPtr<TabControl> m_pTabCtrl;
    EntryDescriptor    m_aCurEntry;

public:
    OrganizeDialog( vcl::Window* pParent, sal_Int16 tabId, EntryDescriptor& rDesc );
    virtual ~OrganizeDialog();
    virtual void    dispose() SAL_OVERRIDE;

    virtual short   Execute() SAL_OVERRIDE;

    DECL_LINK_TYPED( ActivatePageHdl, TabControl*, void );
};

class ObjectPage: public TabPage
{
protected:
    VclPtr<ExtTreeListBox>     m_pBasicBox;
    VclPtr<PushButton>         m_pEditButton;
    VclPtr<PushButton>         m_pNewModButton;
    VclPtr<PushButton>         m_pNewDlgButton;
    VclPtr<PushButton>         m_pDelButton;

    DECL_LINK_TYPED( BasicBoxHighlightHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( ButtonHdl, Button *, void );
    void                CheckButtons();
    bool                GetSelection( ScriptDocument& rDocument, OUString& rLibName );
    void                DeleteCurrent();
    void                NewModule();
    void                NewDialog();
    void                EndTabDialog( sal_uInt16 nRet );

    VclPtr<TabDialog>          pTabDlg;

    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual void        DeactivatePage() SAL_OVERRIDE;

public:
    ObjectPage(vcl::Window* pParent, const OString& rName, sal_uInt16 nMode);
    virtual ~ObjectPage();
    virtual void dispose() SAL_OVERRIDE;

    void                SetCurrentEntry( EntryDescriptor& rDesc );
    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};


class LibPage: public TabPage
{
protected:
    VclPtr<ListBox>            m_pBasicsBox;
    VclPtr<CheckBox>           m_pLibBox;
    VclPtr<PushButton>         m_pEditButton;
    VclPtr<PushButton>         m_pPasswordButton;
    VclPtr<PushButton>         m_pNewLibButton;
    VclPtr<PushButton>         m_pInsertLibButton;
    VclPtr<PushButton>         m_pExportButton;
    VclPtr<PushButton>         m_pDelButton;

    ScriptDocument      m_aCurDocument;
    LibraryLocation     m_eCurLocation;

    DECL_LINK_TYPED( TreeListHighlightHdl, SvTreeListBox *, void );
    DECL_LINK_TYPED( BasicSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( ButtonHdl, Button *, void );
    DECL_LINK_TYPED( CheckPasswordHdl, SvxPasswordDialog *, bool );
    void                CheckButtons();
    void                DeleteCurrent();
    void                NewLib();
    void                InsertLib();
    void                implExportLib( const OUString& aLibName, const OUString& aTargetURL,
                                       const css::uno::Reference< css::task::XInteractionHandler >& Handler );
    void                Export();
    void                ExportAsPackage( const OUString& aLibName );
    void                ExportAsBasic( const OUString& aLibName );
    void                EndTabDialog( sal_uInt16 nRet );
    void                FillListBox();
    void                InsertListBoxEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                SetCurLib();
    SvTreeListEntry*    ImpInsertLibEntry( const OUString& rLibName, sal_uLong nPos );
    virtual void        ActivatePage() SAL_OVERRIDE;
    virtual void        DeactivatePage() SAL_OVERRIDE;

    VclPtr<TabDialog>          pTabDlg;

public:
    explicit LibPage(vcl::Window* pParent);
    virtual             ~LibPage();
    virtual void        dispose() SAL_OVERRIDE;

    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};

// Helper functions
SbModule* createModImpl( vcl::Window* pWin, const ScriptDocument& rDocument,
    TreeListBox& rBasicBox, const OUString& rLibName, const OUString& aModName, bool bMain = false );
void createLibImpl( vcl::Window* pWin, const ScriptDocument& rDocument,
                    CheckBox* pLibBox, TreeListBox* pBasicBox );

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_MODULDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
