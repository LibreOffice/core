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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_REGIONSW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_REGIONSW_HXX

#include <hintids.hxx>
#include <vcl/field.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/combobox.hxx>
#include <vcl/layout.hxx>
#include <vcl/treelistbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <editeng/brushitem.hxx>

#include "condedit.hxx"
#include <section.hxx>
#include <fmtclds.hxx>
#include <fmtftntx.hxx>
#include <fmtclbl.hxx>
#include "numberingtypelistbox.hxx"
#include <editeng/frmdiritem.hxx>
#include <vcl/image.hxx>
#include <svx/paraprev.hxx>
#include <editeng/lrspitem.hxx>

#include <memory>
#include <map>

class SwWrtShell;

namespace sfx2
{
    class DocumentInserter;
    class FileDialogHelper;
}

// dialog "edit regions"
class SectRepr;
typedef std::map<size_t, std::unique_ptr<SectRepr>> SectReprs_t;

class SwEditRegionDlg : public SfxDialogController
{
    bool            m_bSubRegionsFilled;

    SwWrtShell&             rSh;
    SectReprs_t             m_SectReprs;
    const SwSection*        pCurrSect;
    std::unique_ptr<sfx2::DocumentInserter> m_pDocInserter;

    bool            bDontCheckPasswd :1;

    std::unique_ptr<weld::Entry> m_xCurName;
    std::unique_ptr<weld::TreeView>  m_xTree;
    std::unique_ptr<weld::CheckButton> m_xFileCB;
    std::unique_ptr<weld::CheckButton> m_xDDECB;
    std::unique_ptr<weld::Widget> m_xDDEFrame;
    std::unique_ptr<weld::Label> m_xFileNameFT;
    std::unique_ptr<weld::Label> m_xDDECommandFT;
    std::unique_ptr<weld::Entry> m_xFileNameED;
    std::unique_ptr<weld::Button> m_xFilePB;
    std::unique_ptr<weld::Label> m_xSubRegionFT;
    std::unique_ptr<weld::ComboBox> m_xSubRegionED;
    std::unique_ptr<weld::CheckButton> m_xProtectCB;
    std::unique_ptr<weld::CheckButton> m_xPasswdCB;
    std::unique_ptr<weld::Button> m_xPasswdPB;
    std::unique_ptr<weld::CheckButton> m_xHideCB;
    std::unique_ptr<weld::Label> m_xConditionFT;
    std::unique_ptr<SwConditionEdit>  m_xConditionED;
    // #114856# edit in readonly sections
    std::unique_ptr<weld::CheckButton> m_xEditInReadonlyCB;
    std::unique_ptr<weld::Button> m_xOK;
    std::unique_ptr<weld::Button> m_xOptionsPB;
    std::unique_ptr<weld::Button> m_xDismiss;
    std::unique_ptr<weld::Widget> m_xHideFrame;

    void    RecurseList(const SwSectionFormat* pFormat, weld::TreeIter* pIter);
    size_t  FindArrPos(const SwSectionFormat* pFormat);

    DECL_LINK( GetFirstEntryHdl, weld::TreeView&, void );
    DECL_LINK( DeselectHdl, weld::TreeView&, void );

    DECL_LINK( OkHdl, weld::Button&, void );
    DECL_LINK( NameEditHdl, weld::Entry&, void );
    DECL_LINK( ConditionEditHdl, weld::Entry&, void );

    void ChangePasswd(bool bChange);
    DECL_LINK( TogglePasswdHdl, weld::ToggleButton&, void );
    DECL_LINK( ChangePasswdHdl, weld::Button&, void );
    DECL_LINK( ChangeProtectHdl, weld::ToggleButton&, void );
    DECL_LINK( ChangeHideHdl, weld::ToggleButton&, void );
    // #114856# edit in readonly sections
    DECL_LINK( ChangeEditInReadonlyHdl, weld::ToggleButton&, void );
    DECL_LINK( ChangeDismissHdl, weld::Button&, void);
    DECL_LINK( UseFileHdl, weld::ToggleButton&, void );
    DECL_LINK( FileSearchHdl, weld::Button&, void );
    DECL_LINK( OptionsHdl, weld::Button&, void );
    DECL_LINK( FileNameComboBoxHdl, weld::ComboBox&, void );
    DECL_LINK( FileNameEntryHdl, weld::Entry&, void );
    DECL_LINK( DDEHdl, weld::ToggleButton&, void );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper*, void );
    DECL_LINK( SubRegionEventHdl, weld::ComboBox&, void );

    bool CheckPasswd(weld::ToggleButton* pBox = nullptr);

public:
    SwEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh);
    virtual ~SwEditRegionDlg() override;

    void    SelectSection(const OUString& rSectionName);

};

// dialog "insert region"
class SwInsertSectionTabPage : public SfxTabPage
{
    OUString        m_sFileName;
    OUString        m_sFilterName;
    OUString        m_sFilePasswd;

    css::uno::Sequence <sal_Int8 > m_aNewPasswd;
    SwWrtShell*             m_pWrtSh;
    std::unique_ptr<sfx2::DocumentInserter> m_pDocInserter;

    std::unique_ptr<weld::EntryTreeView> m_xCurName;
    std::unique_ptr<weld::CheckButton> m_xFileCB;
    std::unique_ptr<weld::CheckButton> m_xDDECB;
    std::unique_ptr<weld::Label> m_xDDECommandFT;
    std::unique_ptr<weld::Label> m_xFileNameFT;
    std::unique_ptr<weld::Entry> m_xFileNameED;
    std::unique_ptr<weld::Button> m_xFilePB;
    std::unique_ptr<weld::Label> m_xSubRegionFT;
    std::unique_ptr<weld::ComboBox> m_xSubRegionED;
    std::unique_ptr<weld::CheckButton> m_xProtectCB;
    std::unique_ptr<weld::CheckButton> m_xPasswdCB;
    std::unique_ptr<weld::Button> m_xPasswdPB;
    std::unique_ptr<weld::CheckButton> m_xHideCB;
    std::unique_ptr<weld::Label> m_xConditionFT;
    std::unique_ptr<SwConditionEdit> m_xConditionED;
    // #114856# edit in readonly sections
    std::unique_ptr<weld::CheckButton> m_xEditInReadonlyCB;

    void ChangePasswd(bool bChange);

    DECL_LINK( ChangeHideHdl, weld::ToggleButton&, void );
    DECL_LINK( ChangeProtectHdl, weld::ToggleButton&, void );
    DECL_LINK( ChangePasswdHdl, weld::Button&, void );
    DECL_LINK( TogglePasswdHdl, weld::ToggleButton&, void );
    DECL_LINK( NameEditHdl, weld::ComboBox&, void );
    DECL_LINK( UseFileHdl, weld::ToggleButton&, void );
    DECL_LINK( FileSearchHdl, weld::Button&, void );
    DECL_LINK( DDEHdl, weld::ToggleButton&, void );
    DECL_LINK( DlgClosedHdl, sfx2::FileDialogHelper*, void );

public:
    SwInsertSectionTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet);
    virtual ~SwInsertSectionTabPage() override;

    void    SetWrtShell(SwWrtShell& rSh);

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);
};

class SwSectionFootnoteEndTabPage : public SfxTabPage
{
    std::unique_ptr<weld::CheckButton> m_xFootnoteNtAtTextEndCB;
    std::unique_ptr<weld::CheckButton> m_xFootnoteNtNumCB;
    std::unique_ptr<weld::Label> m_xFootnoteOffsetLbl;
    std::unique_ptr<weld::SpinButton> m_xFootnoteOffsetField;
    std::unique_ptr<weld::CheckButton> m_xFootnoteNtNumFormatCB;
    std::unique_ptr<weld::Label> m_xFootnotePrefixFT;
    std::unique_ptr<weld::Entry> m_xFootnotePrefixED;
    std::unique_ptr<SwNumberingTypeListBox> m_xFootnoteNumViewBox;
    std::unique_ptr<weld::Label> m_xFootnoteSuffixFT;
    std::unique_ptr<weld::Entry> m_xFootnoteSuffixED;
    std::unique_ptr<weld::CheckButton> m_xEndNtAtTextEndCB;
    std::unique_ptr<weld::CheckButton> m_xEndNtNumCB;
    std::unique_ptr<weld::Label> m_xEndOffsetLbl;
    std::unique_ptr<weld::SpinButton> m_xEndOffsetField;
    std::unique_ptr<weld::CheckButton> m_xEndNtNumFormatCB;
    std::unique_ptr<weld::Label> m_xEndPrefixFT;
    std::unique_ptr<weld::Entry> m_xEndPrefixED;
    std::unique_ptr<SwNumberingTypeListBox> m_xEndNumViewBox;
    std::unique_ptr<weld::Label> m_xEndSuffixFT;
    std::unique_ptr<weld::Entry> m_xEndSuffixED;

    DECL_LINK(FootEndHdl, weld::ToggleButton&, void);
    void ResetState( bool bFootnote, const SwFormatFootnoteEndAtTextEnd& );

public:
    SwSectionFootnoteEndTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet);
    virtual ~SwSectionFootnoteEndTabPage() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);
};

class SwSectionIndentTabPage : public SfxTabPage
{
    SvxParaPrevWindow m_aPreviewWin;
    std::unique_ptr<weld::MetricSpinButton> m_xBeforeMF;
    std::unique_ptr<weld::MetricSpinButton> m_xAfterMF;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin;

    DECL_LINK(IndentModifyHdl, weld::MetricSpinButton&, void);
public:
    SwSectionIndentTabPage(TabPageParent pParent, const SfxItemSet &rAttrSet);
    virtual ~SwSectionIndentTabPage() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    void    SetWrtShell(SwWrtShell const & rSh);
};

class SwInsertSectionTabDialog : public SfxTabDialogController
{
    SwWrtShell&     rWrtSh;
    std::unique_ptr<SwSectionData> m_pSectionData;

protected:
    virtual void    PageCreated(const OString& rId, SfxTabPage &rPage) override;
    virtual short   Ok() override;
public:
    SwInsertSectionTabDialog(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwInsertSectionTabDialog() override;

    void        SetSectionData(SwSectionData const& rSect);
    SwSectionData * GetSectionData() { return m_pSectionData.get(); }
};

class SwSectionPropertyTabDialog : public SfxTabDialogController
{
    SwWrtShell& rWrtSh;

protected:
    virtual void    PageCreated(const OString& rId, SfxTabPage &rPage) override;
public:
    SwSectionPropertyTabDialog(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh);
    virtual ~SwSectionPropertyTabDialog() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
