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

class SwTokenWindow : public VclHBox, public VclBuilderContainer
{
    VclPtr<Button> m_pLeftScrollWin;
    VclPtr<vcl::Window> m_pCtrlParentWin;
    VclPtr<Button> m_pRightScrollWin;
    std::vector<VclPtr<Control> >   m_aControlList;
    SwForm*         m_pForm;
    sal_uInt16      m_nLevel;
    bool            m_bValid;
    OUString        m_aButtonTexts[TOKEN_END]; // Text of the buttons
    OUString        m_aButtonHelpTexts[TOKEN_END]; // QuickHelpText of the buttons
    OUString const  m_sCharStyle;
    Link<SwFormToken&,void>   m_aButtonSelectedHdl;
    VclPtr<Control>           m_pActiveCtrl;
    Link<LinkParamNone*,void> m_aModifyHdl;
    OUString        m_sAccessibleName;
    OUString        m_sAdditionalAccnameString1;
    OUString        m_sAdditionalAccnameString2;
    OUString        m_sAdditionalAccnameString3;

    VclPtr<SwTOXEntryTabPage>  m_pParent;

    DECL_LINK( EditResize, Edit&, void );
    DECL_LINK( NextItemHdl, SwTOXEdit&, void );
    DECL_LINK( TbxFocusHdl, Control&, void );
    DECL_LINK( NextItemBtnHdl, SwTOXButton&, void );
    DECL_LINK( TbxFocusBtnHdl, Control&, void );
    DECL_LINK( ScrollHdl, Button*, void );

    void    SetActiveControl(Control* pSet);

    Control*    InsertItem(const OUString& rText, const SwFormToken& aToken);
    void        AdjustPositions();
    void        AdjustScrolling();
    void        MoveControls(long nOffset);

public:
    SwTokenWindow(vcl::Window* pParent);
    virtual ~SwTokenWindow() override;
    virtual void dispose() override;

    void SetTabPage(SwTOXEntryTabPage *pParent) { m_pParent = pParent; }

    void        SetForm(SwForm& rForm, sal_uInt16 nLevel);
    sal_uInt16      GetLastLevel()const {return m_nLevel;};

    bool        IsValid() const {return m_bValid;}

    void        SetInvalid() {m_bValid = false;}

    OUString    GetPattern() const;

    void        SetButtonSelectedHdl(const Link<SwFormToken&,void>& rLink)
                { m_aButtonSelectedHdl = rLink;}

    void        SetModifyHdl(const Link<LinkParamNone*,void>& rLink){m_aModifyHdl = rLink;}

    Control*    GetActiveControl() { return m_pActiveCtrl; }

    void        InsertAtSelection(const OUString& rText, const SwFormToken& aToken);
    void        RemoveControl(SwTOXButton* pDel, bool bInternalCall = false);

    bool        Contains(FormTokenType) const;

    //helper for pattern buttons and edits
    bool        CreateQuickHelp(Control const * pCtrl,
                    const SwFormToken& rToken, const HelpEvent& );

    virtual void    GetFocus() override;
    virtual void    setAllocation(const Size &rAllocation) override;
    void SetFocus2theAllBtn();
private:
    sal_uInt32 GetControlIndex(FormTokenType eType) const;
};

class SwIdxTreeListBox : public SvTreeListBox
{
    VclPtr<SwTOXEntryTabPage> pParent;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
public:
    SwIdxTreeListBox(vcl::Window* pPar, WinBits nStyle);
    virtual ~SwIdxTreeListBox() override;
    virtual void dispose() override;

    void SetTabPage(SwTOXEntryTabPage* pPar) { pParent = pPar; }
};

class SwTOXEntryTabPage : public SfxTabPage
{
    VclPtr<FixedText>          m_pLevelFT;
    VclPtr<SwIdxTreeListBox>   m_pLevelLB;

    VclPtr<SwTokenWindow>  m_pTokenWIN;

    VclPtr<PushButton>     m_pAllLevelsPB;

    VclPtr<PushButton>     m_pEntryNoPB;
    VclPtr<PushButton>     m_pEntryPB;
    VclPtr<PushButton>     m_pTabPB;
    VclPtr<PushButton>     m_pChapterInfoPB;
    VclPtr<PushButton>     m_pPageNoPB;
    VclPtr<PushButton>     m_pHyperLinkPB;

    VclPtr<ListBox>        m_pAuthFieldsLB;
    VclPtr<PushButton>     m_pAuthInsertPB;
    VclPtr<PushButton>     m_pAuthRemovePB;

    VclPtr<ListBox>        m_pCharStyleLB;       // character style of the current token
    VclPtr<PushButton>     m_pEditStylePB;

    VclPtr<FixedText>      m_pChapterEntryFT;
    VclPtr<ListBox>        m_pChapterEntryLB;    // type of chapter info

    VclPtr<FixedText>      m_pNumberFormatFT;
    VclPtr<ListBox>        m_pNumberFormatLB;    //!< format for numbering (E#)

    VclPtr<FixedText>      m_pEntryOutlineLevelFT;    //!< Fixed text, for i53420
    VclPtr<NumericField>   m_pEntryOutlineLevelNF;   //!< level to evaluate outline level to, for i53420

    VclPtr<FixedText>      m_pFillCharFT;
    VclPtr<ComboBox>       m_pFillCharCB;        // fill char for tab stop

    VclPtr<FixedText>      m_pTabPosFT;
    VclPtr<MetricField>    m_pTabPosMF;          // tab stop position
    VclPtr<CheckBox>       m_pAutoRightCB;

    VclPtr<VclContainer>   m_pFormatFrame;
    VclPtr<FixedText>      m_pMainEntryStyleFT;
    VclPtr<ListBox>        m_pMainEntryStyleLB;  // character style of main entries in indexes
    VclPtr<CheckBox>       m_pAlphaDelimCB;
    VclPtr<CheckBox>       m_pCommaSeparatedCB;
    VclPtr<CheckBox>       m_pRelToStyleCB;      // position relative to the right margin of the para style

    VclPtr<VclContainer>   m_pSortingFrame;
    VclPtr<RadioButton>    m_pSortDocPosRB;
    VclPtr<RadioButton>    m_pSortContentRB;

    VclPtr<VclContainer>   m_pSortKeyFrame;
    VclPtr<ListBox>        m_pFirstKeyLB;
    VclPtr<RadioButton>    m_pFirstSortUpRB;
    VclPtr<RadioButton>    m_pFirstSortDownRB;

    VclPtr<ListBox>        m_pSecondKeyLB;
    VclPtr<RadioButton>    m_pSecondSortUpRB;
    VclPtr<RadioButton>    m_pSecondSortDownRB;

    VclPtr<ListBox>        m_pThirdKeyLB;
    VclPtr<RadioButton>    m_pThirdSortUpRB;
    VclPtr<RadioButton>    m_pThirdSortDownRB;

    OUString const  sDelimStr;
    OUString        sLevelStr;
    OUString        sAuthTypeStr;

    OUString const  sNoCharStyle;
    OUString const  sNoCharSortKey;
    SwForm*         m_pCurrentForm;

    CurTOXType      aLastTOXType;
    bool            bInLevelHdl;

    DECL_LINK(StyleSelectHdl, ListBox&, void);
    DECL_LINK(EditStyleHdl, Button*, void);
    DECL_LINK(InsertTokenHdl, Button*, void);
    DECL_LINK(LevelHdl, SvTreeListBox*, void);
    DECL_LINK(AutoRightHdl, Button*, void);
    DECL_LINK(TokenSelectedHdl, SwFormToken&, void);
    DECL_LINK(TabPosHdl, Edit&, void);
    DECL_LINK(FillCharHdl, Edit&, void);
    DECL_LINK(RemoveInsertAuthHdl, Button*, void);
    DECL_LINK(SortKeyHdl, Button*, void);
    DECL_LINK(ChapterInfoHdl, ListBox&, void);
    DECL_LINK(ChapterInfoOutlineHdl, Edit&, void);
    DECL_LINK(NumberFormatHdl, ListBox&, void);

    DECL_LINK(AllLevelsHdl, Button*, void);

    void            WriteBackLevel();
    void            UpdateDescriptor();
    DECL_LINK(ModifyHdl, LinkParamNone*, void);
    void OnModify(void const *);
    DECL_LINK(ModifyClickHdl, Button*, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXEntryTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXEntryTabPage() override;
    virtual void dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                       const SfxItemSet* rAttrSet);
    void                SetWrtShell(SwWrtShell& rSh);

    OUString            GetLevelHelp(sal_uInt16 nLevel) const;

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
