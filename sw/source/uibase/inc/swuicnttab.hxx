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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWUICNTTAB_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWUICNTTAB_HXX

#include <svx/stddlg.hxx>

#include <vcl/button.hxx>

#include <vcl/edit.hxx>

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/tabdlg.hxx>

#include <tox.hxx>
#include "toxmgr.hxx"
#include <svx/checklbx.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/menubtn.hxx>
#include <svx/langbox.hxx>
#include "cnttab.hxx"
#include <vector>

class IndexEntryResource;
class IndexEntrySupplierWrapper;
class SwTOXWidget;
class SwTOXEdit;
class SwTOXButton;
class SwTOXEntryTabPage;
class SwOneExampleFrame;
class SwWrtShell;

namespace com{namespace sun{namespace star{
    namespace text{
        class XTextSection;
        class XDocumentIndex;
    }
}}}

struct SwIndexSections_Impl
{
    css::uno::Reference< css::text::XTextSection >    xContainerSection;
    css::uno::Reference< css::text::XDocumentIndex >    xDocumentIndex;
};

class SwMultiTOXTabDialog : public SfxTabDialog
{
    VclPtr<vcl::Window>     m_pExampleContainerWIN;
    VclPtr<CheckBox>        m_pShowExampleCB;
    std::unique_ptr<SwTOXMgr> m_pMgr;
    SwWrtShell&             m_rWrtShell;

    sal_uInt16              m_nSelectId;
    sal_uInt16              m_nColumnId;
    sal_uInt16              m_nBackGroundId;
    sal_uInt16              m_nEntriesId;

    std::unique_ptr<SwOneExampleFrame> m_pExampleFrame;

    struct TypeData
    {
        std::unique_ptr<SwForm> m_pForm;
        std::unique_ptr<SwTOXDescription> m_pDescription;
        std::unique_ptr<SwIndexSections_Impl> m_pxIndexSections;
    };
    std::vector<TypeData>   m_vTypeData;

    SwTOXBase*              m_pParamTOXBase;

    CurTOXType              m_eCurrentTOXType;

    OUString const          m_sUserDefinedIndex;
    sal_uInt16 const        m_nInitialTOXType;

    bool                m_bEditTOX;
    bool                m_bExampleCreated;
    bool const          m_bGlobalFlag;

    virtual short       Ok() override;
    std::unique_ptr<SwTOXDescription> CreateTOXDescFromTOXBase(const SwTOXBase*pCurTOX);

    DECL_LINK(CreateExample_Hdl, SwOneExampleFrame&, void);
    DECL_LINK(ShowPreviewHdl, Button*, void);

public:
    SwMultiTOXTabDialog(vcl::Window* pParent, const SfxItemSet& rSet,
                        SwWrtShell &rShell,
                        SwTOXBase* pCurTOX, sal_uInt16 nToxType,
                        bool bGlobal);
    virtual ~SwMultiTOXTabDialog() override;
    virtual void        dispose() override;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

    SwForm*             GetForm(CurTOXType eType);

    const CurTOXType&   GetCurrentTOXType() const { return m_eCurrentTOXType;}
    void                SetCurrentTOXType(const CurTOXType& eSet)
                                {
                                    m_eCurrentTOXType = eSet;
                                }

    bool                IsTOXEditMode() const { return m_bEditTOX;}

    SwWrtShell&         GetWrtShell() {return m_rWrtShell;}

    SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes);
    void                CreateOrUpdateExample(
                            TOXTypes nTOXIndex, sal_uInt16 nPage = 0, sal_uInt16 nCurLevel = USHRT_MAX);

    static bool IsNoNum(SwWrtShell& rSh, const OUString& rName);
};

class SwTOXSelectTabPage : public SfxTabPage
{
    std::unique_ptr<IndexEntryResource> pIndexRes;

    OUString        aStyleArr[MAXLEVEL];
    OUString        sAutoMarkURL;
    OUString const  sAutoMarkType;
    OUString        sAddStyleUser;
    OUString        sAddStyleContent;

    std::unique_ptr<const IndexEntrySupplierWrapper> pIndexEntryWrapper;

    bool            m_bWaitingInitialSettings;

    std::unique_ptr<weld::Entry> m_xTitleED;
    std::unique_ptr<weld::Label> m_xTypeFT;
    std::unique_ptr<weld::ComboBox> m_xTypeLB;
    std::unique_ptr<weld::CheckButton> m_xReadOnlyCB;

    std::unique_ptr<weld::Widget> m_xAreaFrame;
    std::unique_ptr<weld::ComboBox> m_xAreaLB;
    std::unique_ptr<weld::Widget> m_xLevelFT;   //content, user
    std::unique_ptr<weld::SpinButton> m_xLevelNF;   //content, user

    //content
    std::unique_ptr<weld::Widget> m_xCreateFrame;  // content, user, illustration
    std::unique_ptr<weld::CheckButton> m_xFromHeadingsCB;
    std::unique_ptr<weld::CheckButton> m_xStylesCB;
    std::unique_ptr<weld::CheckButton> m_xAddStylesCB;
    std::unique_ptr<weld::Button> m_xAddStylesPB;
    //user
    std::unique_ptr<weld::CheckButton> m_xFromTablesCB;
    std::unique_ptr<weld::CheckButton> m_xFromFramesCB;
    std::unique_ptr<weld::CheckButton> m_xFromGraphicsCB;
    std::unique_ptr<weld::CheckButton> m_xFromOLECB;
    std::unique_ptr<weld::CheckButton> m_xLevelFromChapterCB;

    //illustration + table
    std::unique_ptr<weld::RadioButton> m_xFromCaptionsRB;
    std::unique_ptr<weld::RadioButton> m_xFromObjectNamesRB;

    //illustration and tables
    std::unique_ptr<weld::Label> m_xCaptionSequenceFT;
    std::unique_ptr<weld::ComboBox> m_xCaptionSequenceLB;
    std::unique_ptr<weld::Label> m_xDisplayTypeFT;
    std::unique_ptr<weld::ComboBox> m_xDisplayTypeLB;

    //all but illustration and table
    std::unique_ptr<weld::CheckButton> m_xTOXMarksCB;

    //index only
    std::unique_ptr<weld::Widget>   m_xIdxOptionsFrame;
    std::unique_ptr<weld::CheckButton> m_xCollectSameCB;
    std::unique_ptr<weld::CheckButton> m_xUseFFCB;
    std::unique_ptr<weld::CheckButton> m_xUseDashCB;
    std::unique_ptr<weld::CheckButton> m_xCaseSensitiveCB;
    std::unique_ptr<weld::CheckButton> m_xInitialCapsCB;
    std::unique_ptr<weld::CheckButton> m_xKeyAsEntryCB;
    std::unique_ptr<weld::CheckButton> m_xFromFileCB;
    std::unique_ptr<weld::MenuButton> m_xAutoMarkPB;

    // object only
    std::unique_ptr<weld::TreeView> m_xFromObjCLB;
    std::unique_ptr<weld::Widget> m_xFromObjFrame;

    std::unique_ptr<weld::CheckButton> m_xSequenceCB;
    std::unique_ptr<weld::ComboBox> m_xBracketLB;
    std::unique_ptr<weld::Widget> m_xAuthorityFrame;

    //all
    std::unique_ptr<weld::Widget> m_xSortFrame;
    std::unique_ptr<LanguageBox> m_xLanguageLB;
    std::unique_ptr<weld::ComboBox> m_xSortAlgorithmLB;

    DECL_LINK(TOXTypeHdl,   weld::ComboBox&, void );
    DECL_LINK(AddStylesHdl, weld::Button&, void );
    DECL_LINK(MenuEnableHdl, weld::ToggleButton&, void);
    DECL_LINK(MenuExecuteHdl, const OString&, void);
    DECL_LINK(LanguageListBoxHdl, weld::ComboBox&, void);
    void LanguageHdl(const weld::ComboBox*);
    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void );
    DECL_LINK(RadioButtonHdl, weld::ToggleButton&, void);
    DECL_LINK(ModifyEntryHdl, weld::Entry&, void);
    DECL_LINK(ModifySpinHdl, weld::SpinButton&, void);
    DECL_LINK(ModifyListBoxHdl, weld::ComboBox&, void);

    void ModifyHdl();
    void ApplyTOXDescription();
    void FillTOXDescription();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXSelectTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXSelectTabPage() override;
    virtual void        dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    void                SelectType(TOXTypes eSet);  //preset TOXType, GlobalDoc
    void                SetWrtShell(SwWrtShell const & rSh);
};

class SwTokenWindow
{
    SwForm*         m_pForm;
    sal_uInt16      m_nLevel;
    bool            m_bValid;
    OUString        m_aButtonTexts[TOKEN_END]; // Text of the buttons
    OUString        m_aButtonHelpTexts[TOKEN_END]; // QuickHelpText of the buttons
    OUString const  m_sCharStyle;
    Link<SwFormToken&,void>   m_aButtonSelectedHdl;
    SwTOXWidget* m_pActiveCtrl;
    Link<LinkParamNone*,void> m_aModifyHdl;
    OUString        m_sAccessibleName;
    OUString        m_sAdditionalAccnameString1;
    OUString        m_sAdditionalAccnameString2;
    OUString        m_sAdditionalAccnameString3;

    VclPtr<SwTOXEntryTabPage>  m_pParent;
    std::unique_ptr<weld::Builder> m_xBuilder;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Button> m_xLeftScrollWin;
    std::unique_ptr<weld::Container> m_xCtrlParentWin;
    std::unique_ptr<weld::ScrolledWindow> m_xScrollWin;
    std::unique_ptr<weld::Button> m_xRightScrollWin;
    std::vector<std::unique_ptr<SwTOXWidget>> m_aControlList;

    DECL_LINK(EditResize, SwTOXEdit&, void);
    DECL_LINK(NextItemHdl, SwTOXEdit&, void);
    DECL_LINK(TbxFocusHdl, SwTOXWidget&, void);
    DECL_LINK(NextItemBtnHdl, SwTOXButton&, void);
    DECL_LINK(TbxFocusBtnHdl, SwTOXWidget&, void);
    DECL_LINK(ScrollBtnHdl, weld::Button&, void);
    DECL_LINK(ScrollHdl, weld::ScrolledWindow&, void);

    void    SetActiveControl(SwTOXWidget* pSet);

    SwTOXWidget* InsertItem(const OUString& rText, const SwFormToken& aToken);
    void        AdjustPositions();
    void        AdjustScrolling();
    void        MoveControls(long nOffset);

public:
    SwTokenWindow(std::unique_ptr<weld::Container> xParent);
    weld::Container* get_child_container() { return m_xCtrlParentWin.get(); }
    ~SwTokenWindow();

    void SetTabPage(SwTOXEntryTabPage *pParent) { m_pParent = pParent; }

    void        SetForm(SwForm& rForm, sal_uInt16 nLevel);
    sal_uInt16  GetLastLevel()const {return m_nLevel;};

    bool        IsValid() const {return m_bValid;}

    void        SetInvalid() {m_bValid = false;}

    OUString    GetPattern() const;

    void        SetButtonSelectedHdl(const Link<SwFormToken&,void>& rLink)
                { m_aButtonSelectedHdl = rLink;}

    void        SetModifyHdl(const Link<LinkParamNone*,void>& rLink){m_aModifyHdl = rLink;}

    SwTOXWidget* GetActiveControl() { return m_pActiveCtrl; }

    void        InsertAtSelection(const SwFormToken& aToken);
    void        RemoveControl(const SwTOXButton* pDel, bool bInternalCall = false);

    bool        Contains(FormTokenType) const;

    //helper for pattern buttons and edits
    OUString    CreateQuickHelp(const SwFormToken& rToken);

    void SetFocus2theAllBtn();
private:
    sal_uInt32 GetControlIndex(FormTokenType eType) const;
};

class SwTOXEntryTabPage : public SfxTabPage
{
    OUString const  sDelimStr;
    OUString        sLevelStr;
    OUString        sAuthTypeStr;

    OUString const  sNoCharStyle;
    SwForm*         m_pCurrentForm;

    CurTOXType      aLastTOXType;
    bool            bInLevelHdl;

    std::unique_ptr<weld::Label> m_xTypeFT;
    std::unique_ptr<weld::Label> m_xLevelFT;
    std::unique_ptr<weld::TreeView> m_xLevelLB;
    std::unique_ptr<weld::Button> m_xAllLevelsPB;
    std::unique_ptr<weld::Button> m_xEntryNoPB;
    std::unique_ptr<weld::Button> m_xEntryPB;
    std::unique_ptr<weld::Button> m_xTabPB;
    std::unique_ptr<weld::Button> m_xChapterInfoPB;
    std::unique_ptr<weld::Button> m_xPageNoPB;
    std::unique_ptr<weld::Button> m_xHyperLinkPB;
    std::unique_ptr<weld::ComboBox> m_xAuthFieldsLB;
    std::unique_ptr<weld::Button> m_xAuthInsertPB;
    std::unique_ptr<weld::Button> m_xAuthRemovePB;
    std::unique_ptr<weld::ComboBox> m_xCharStyleLB;       // character style of the current token
    std::unique_ptr<weld::Button> m_xEditStylePB;
    std::unique_ptr<weld::Label> m_xChapterEntryFT;
    std::unique_ptr<weld::ComboBox> m_xChapterEntryLB;    // type of chapter info
    std::unique_ptr<weld::Label> m_xNumberFormatFT;
    std::unique_ptr<weld::ComboBox> m_xNumberFormatLB;    //!< format for numbering (E#)
    std::unique_ptr<weld::Label> m_xEntryOutlineLevelFT;    //!< Fixed text, for i53420
    std::unique_ptr<weld::SpinButton> m_xEntryOutlineLevelNF;   //!< level to evaluate outline level to, for i53420
    std::unique_ptr<weld::Label> m_xFillCharFT;
    std::unique_ptr<weld::ComboBox> m_xFillCharCB;        // fill char for tab stop
    std::unique_ptr<weld::Label> m_xTabPosFT;
    std::unique_ptr<weld::MetricSpinButton> m_xTabPosMF;          // tab stop position
    std::unique_ptr<weld::CheckButton> m_xAutoRightCB;
    std::unique_ptr<weld::Widget> m_xFormatFrame;
    std::unique_ptr<weld::Label> m_xMainEntryStyleFT;
    std::unique_ptr<weld::ComboBox> m_xMainEntryStyleLB;  // character style of main entries in indexes
    std::unique_ptr<weld::CheckButton> m_xAlphaDelimCB;
    std::unique_ptr<weld::CheckButton> m_xCommaSeparatedCB;
    std::unique_ptr<weld::CheckButton> m_xRelToStyleCB;      // position relative to the right margin of the para style
    std::unique_ptr<weld::Widget> m_xSortingFrame;
    std::unique_ptr<weld::RadioButton> m_xSortDocPosRB;
    std::unique_ptr<weld::RadioButton> m_xSortContentRB;
    std::unique_ptr<weld::Widget> m_xSortKeyFrame;
    std::unique_ptr<weld::ComboBox> m_xFirstKeyLB;
    std::unique_ptr<weld::RadioButton> m_xFirstSortUpRB;
    std::unique_ptr<weld::RadioButton> m_xFirstSortDownRB;
    std::unique_ptr<weld::ComboBox> m_xSecondKeyLB;
    std::unique_ptr<weld::RadioButton> m_xSecondSortUpRB;
    std::unique_ptr<weld::RadioButton> m_xSecondSortDownRB;
    std::unique_ptr<weld::ComboBox> m_xThirdKeyLB;
    std::unique_ptr<weld::RadioButton> m_xThirdSortUpRB;
    std::unique_ptr<weld::RadioButton> m_xThirdSortDownRB;
    std::unique_ptr<SwTokenWindow> m_xTokenWIN;

    DECL_LINK(StyleSelectHdl, weld::ComboBox&, void);
    DECL_LINK(EditStyleHdl, weld::Button&, void);
    DECL_LINK(InsertTokenHdl, weld::Button&, void);
    DECL_LINK(LevelHdl, weld::TreeView&, void);
    DECL_LINK(AutoRightHdl, weld::ToggleButton&, void);
    DECL_LINK(TokenSelectedHdl, SwFormToken&, void);
    DECL_LINK(TabPosHdl, weld::MetricSpinButton&, void);
    DECL_LINK(FillCharHdl, weld::ComboBox&, void);
    DECL_LINK(RemoveInsertAuthHdl, weld::Button&, void);
    DECL_LINK(SortKeyHdl, weld::ToggleButton&, void);
    DECL_LINK(ChapterInfoHdl, weld::ComboBox&, void);
    DECL_LINK(ChapterInfoOutlineHdl, weld::SpinButton&, void);
    DECL_LINK(NumberFormatHdl, weld::ComboBox&, void);

    DECL_LINK(AllLevelsHdl, weld::Button&, void);

    void            WriteBackLevel();
    void            UpdateDescriptor();
    DECL_LINK(ModifyHdl, LinkParamNone*, void);
    void OnModify(bool bAllLevels);
    DECL_LINK(ModifyClickHdl, weld::ToggleButton&, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXEntryTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXEntryTabPage() override;
    virtual void dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                       const SfxItemSet* rAttrSet);
    void                SetWrtShell(SwWrtShell& rSh);

    void                PreTokenButtonRemoved(const SwFormToken& rToken);
    void SetFocus2theAllBtn();
    virtual bool EventNotify( NotifyEvent& rNEvt ) override;
};

class SwTOXStylesTabPage : public SfxTabPage
{
    std::unique_ptr<SwForm> m_pCurrentForm;

    std::unique_ptr<weld::TreeView> m_xLevelLB;
    std::unique_ptr<weld::Button> m_xAssignBT;
    std::unique_ptr<weld::TreeView> m_xParaLayLB;
    std::unique_ptr<weld::Button> m_xStdBT;
    std::unique_ptr<weld::Button> m_xEditStyleBT;

    DECL_LINK(EditStyleHdl, weld::Button&, void);
    DECL_LINK(StdHdl, weld::Button&, void);
    DECL_LINK(EnableSelectHdl, weld::TreeView&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, void);
    DECL_LINK(AssignHdl, weld::Button&, void);
    void Modify();

    SwForm&     GetForm()
    {
        SwMultiTOXTabDialog* pDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
        return *pDlg->GetForm(pDlg->GetCurrentTOXType());
    }

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXStylesTabPage(TabPageParent pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXStylesTabPage() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                       const SfxItemSet* rAttrSet);

};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWUICNTTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
