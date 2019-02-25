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
#include <vcl/svtabbx.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>

class SvxPasswordDialog;

namespace basctl
{

enum class ObjectMode
{
    Library = 1,
    Module  = 2,
    Dialog  = 3,
};

class NewObjectDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xOKButton;
    bool m_bCheckName;

    DECL_LINK(OkButtonHandler, weld::Button&, void);
public:
    NewObjectDialog(weld::Window* pParent, ObjectMode, bool bCheckName = false);
    OUString GetObjectName() const { return m_xEdit->get_text(); }
    void SetObjectName(const OUString& rName)
    {
        m_xEdit->set_text(rName);
        m_xEdit->select_region(0, -1);
    }
};

class GotoLineDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Entry> m_xEdit;
    std::unique_ptr<weld::Button> m_xOKButton;
    DECL_LINK(OkButtonHandler, weld::Button&, void);
public:
    explicit GotoLineDialog(weld::Window* pParent);
    virtual ~GotoLineDialog() override;
    sal_Int32 GetLineNumber() const;
};

class ExportDialog : public weld::GenericDialogController
{
private:
    bool m_bExportAsPackage;

    std::unique_ptr<weld::RadioButton> m_xExportAsPackageButton;
    std::unique_ptr<weld::Button> m_xOKButton;

    DECL_LINK(OkButtonHandler, weld::Button&, void);

public:
    explicit ExportDialog(weld::Window * pParent);
    virtual ~ExportDialog() override;

    bool isExportAsPackage () const { return m_bExportAsPackage; }
};

class ExtTreeListBox final : public TreeListBox
{
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel  ) override;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

    virtual DragDropMode    NotifyStartDrag( TransferDataContainer& rData, SvTreeListEntry* pEntry ) override;
    virtual bool            NotifyAcceptDrop( SvTreeListEntry* pEntry ) override;

    virtual TriState    NotifyMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos ) override;
    virtual TriState    NotifyCopying( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos ) override;
    TriState            NotifyCopyingMoving( SvTreeListEntry* pTarget, SvTreeListEntry const * pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos, bool bMove );

public:
    ExtTreeListBox(vcl::Window* pParent, WinBits nStyle);
};

class CheckBox : public SvTabListBox
{
private:
    ObjectMode          eMode;
    std::unique_ptr<SvLBoxButtonData> pCheckButton;
    ScriptDocument      m_aDocument;
    void                Init();

public:
    CheckBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~CheckBox() override;
    virtual void    dispose() override;

    SvTreeListEntry*    DoInsertEntry( const OUString& rStr, sal_uLong nPos = LISTBOX_APPEND );
    SvTreeListEntry*    FindEntry( const OUString& rName );

    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind) override;
    virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& rSel ) override;
    virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

    void            SetDocument( const ScriptDocument& rDocument ) { m_aDocument = rDocument; }

    void            SetMode(ObjectMode);
};

class LibDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Frame> m_xStorageFrame;
    std::unique_ptr<weld::TreeView> m_xLibBox;
    std::unique_ptr<weld::CheckButton> m_xReferenceBox;
    std::unique_ptr<weld::CheckButton> m_xReplaceBox;

public:
    explicit LibDialog(weld::Window* pParent);
    virtual ~LibDialog() override;

    void            SetStorageName( const OUString& rName );

    weld::TreeView& GetLibBox()                 { return *m_xLibBox; }
    bool            IsReference() const         { return m_xReferenceBox->get_active(); }
    bool            IsReplace() const           { return m_xReplaceBox->get_active(); }

    void            EnableReference (bool b)    { m_xReferenceBox->set_sensitive(b); }
};

class OrganizeDialog : public TabDialog
{
private:
    VclPtr<TabControl> m_pTabCtrl;
    EntryDescriptor    m_aCurEntry;

public:
    OrganizeDialog( vcl::Window* pParent, sal_Int16 tabId, EntryDescriptor const & rDesc );
    virtual ~OrganizeDialog() override;
    virtual void    dispose() override;

    DECL_LINK( ActivatePageHdl, TabControl*, void );
};

class ObjectPage final : public TabPage
{
    VclPtr<ExtTreeListBox>     m_pBasicBox;
    VclPtr<PushButton>         m_pEditButton;
    VclPtr<PushButton>         m_pNewModButton;
    VclPtr<PushButton>         m_pNewDlgButton;
    VclPtr<PushButton>         m_pDelButton;

    DECL_LINK( BasicBoxHighlightHdl, SvTreeListBox*, void );
    DECL_LINK( ButtonHdl, Button *, void );
    void                CheckButtons();
    bool                GetSelection( ScriptDocument& rDocument, OUString& rLibName );
    void                DeleteCurrent();
    void                NewModule();
    void                NewDialog();
    void                EndTabDialog();

    VclPtr<TabDialog>          pTabDlg;

    virtual void        ActivatePage() override;
    virtual void        DeactivatePage() override;

public:
    ObjectPage(vcl::Window* pParent, const OString& rName, BrowseMode nMode);
    virtual ~ObjectPage() override;
    virtual void dispose() override;

    void                SetCurrentEntry( EntryDescriptor const & rDesc );
    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};


class LibPage final : public TabPage
{
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

    DECL_LINK( TreeListHighlightHdl, SvTreeListBox *, void );
    DECL_LINK( BasicSelectHdl, ListBox&, void );
    DECL_LINK( ButtonHdl, Button *, void );
    DECL_LINK( CheckPasswordHdl, SvxPasswordDialog *, bool );
    void                CheckButtons();
    void                DeleteCurrent();
    void                NewLib();
    void                InsertLib();
    void                implExportLib( const OUString& aLibName, const OUString& aTargetURL,
                                       const css::uno::Reference< css::task::XInteractionHandler >& Handler );
    void                Export();
    void                ExportAsPackage( const OUString& aLibName );
    void                ExportAsBasic( const OUString& aLibName );
    void                EndTabDialog();
    void                FillListBox();
    void                InsertListBoxEntry( const ScriptDocument& rDocument, LibraryLocation eLocation );
    void                SetCurLib();
    SvTreeListEntry*    ImpInsertLibEntry( const OUString& rLibName, sal_uLong nPos );
    virtual void        ActivatePage() override;
    virtual void        DeactivatePage() override;

    VclPtr<TabDialog>          pTabDlg;

public:
    explicit LibPage(vcl::Window* pParent);
    virtual             ~LibPage() override;
    virtual void        dispose() override;

    void                SetTabDlg( TabDialog* p ) { pTabDlg = p;}
};

// Helper functions
SbModule* createModImpl(weld::Window* pWin, const ScriptDocument& rDocument,
                        SbTreeListBox& rBasicBox, const OUString& rLibName, const OUString& aModName, bool bMain);
SbModule* createModImpl(weld::Window* pWin, const ScriptDocument& rDocument,
                        TreeListBox& rBasicBox, const OUString& rLibName, bool bMain);
void createLibImpl(weld::Window* pWin, const ScriptDocument& rDocument,
                   CheckBox* pLibBox, TreeListBox* pBasicBox);
void createLibImpl(weld::Window* pWin, const ScriptDocument& rDocument,
                   CheckBox* pLibBox, SbTreeListBox* pBasicBox);

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_MODULDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
