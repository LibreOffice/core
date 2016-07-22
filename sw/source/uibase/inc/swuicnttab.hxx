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

#include "tox.hxx"
#include <toxmgr.hxx>
#include <svx/checklbx.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/menubtn.hxx>
#include <svx/langbox.hxx>
#include <cnttab.hxx>
#include <vector>

class SwWrtShell;
class SwTOXMgr;
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

class SwOneExampleFrame;

struct SwIndexSections_Impl;

class SwMultiTOXTabDialog : public SfxTabDialog
{
    VclPtr<vcl::Window>     m_pExampleContainerWIN;
    VclPtr<CheckBox>        m_pShowExampleCB;
    SwTOXMgr*               pMgr;
    SwWrtShell&             rSh;

    sal_uInt16              m_nSelectId;
    sal_uInt16              m_nStylesId;
    sal_uInt16              m_nColumnId;
    sal_uInt16              m_nBackGroundId;
    sal_uInt16              m_nEntriesId;

    SwOneExampleFrame*      pExampleFrame;

    SwTOXDescription**      pDescArr;
    SwForm**                pFormArr;
    SwIndexSections_Impl**  pxIndexSectionsArr;

    SwTOXBase*              pParamTOXBase;

    CurTOXType              eCurrentTOXType;

    OUString                sUserDefinedIndex;
    sal_uInt16              nTypeCount;
    sal_uInt16              nInitialTOXType;

    bool                bEditTOX;
    bool                bExampleCreated;
    bool                bGlobalFlag;

    virtual short       Ok() override;
    SwTOXDescription*   CreateTOXDescFromTOXBase(const SwTOXBase*pCurTOX);

    DECL_LINK_TYPED(CreateExample_Hdl, SwOneExampleFrame&, void);
    DECL_LINK_TYPED(ShowPreviewHdl, Button*, void);

public:
    SwMultiTOXTabDialog(vcl::Window* pParent, const SfxItemSet& rSet,
                        SwWrtShell &rShell,
                        SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                        bool bGlobal = false);
    virtual ~SwMultiTOXTabDialog();
    virtual void        dispose() override;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

    SwForm*             GetForm(CurTOXType eType);

    const CurTOXType&   GetCurrentTOXType() const { return eCurrentTOXType;}
    void                SetCurrentTOXType(const CurTOXType& eSet)
                                {
                                    eCurrentTOXType = eSet;
                                }

    bool                IsTOXEditMode() const { return bEditTOX;}

    SwWrtShell&         GetWrtShell() {return rSh;}

    SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes);
    void                CreateOrUpdateExample(
                            TOXTypes nTOXIndex, sal_uInt16 nPage = 0, sal_uInt16 nCurLevel = USHRT_MAX);

    static bool IsNoNum(SwWrtShell& rSh, const OUString& rName);
};

class IndexEntryResource;
class IndexEntrySupplierWrapper;

class SwTOXSelectTabPage : public SfxTabPage
{
    VclPtr<Edit>           m_pTitleED;
    VclPtr<FixedText>      m_pTypeFT;
    VclPtr<ListBox>        m_pTypeLB;
    VclPtr<CheckBox>       m_pReadOnlyCB;

    VclPtr<VclContainer>   m_pAreaFrame;
    VclPtr<ListBox>        m_pAreaLB;
    VclPtr<FixedText>      m_pLevelFT;   //content, user
    VclPtr<NumericField>   m_pLevelNF;   //content, user

    //content
    VclPtr<VclContainer>   m_pCreateFrame;  // content, user, illustration
    VclPtr<CheckBox>       m_pFromHeadingsCB;
    VclPtr<CheckBox>       m_pAddStylesCB;
    VclPtr<PushButton>     m_pAddStylesPB;
    //user
    VclPtr<CheckBox>       m_pFromTablesCB;
    VclPtr<CheckBox>       m_pFromFramesCB;
    VclPtr<CheckBox>       m_pFromGraphicsCB;
    VclPtr<CheckBox>       m_pFromOLECB;
    VclPtr<CheckBox>       m_pLevelFromChapterCB;

    //illustration + table
    VclPtr<RadioButton>    m_pFromCaptionsRB;
    VclPtr<RadioButton>    m_pFromObjectNamesRB;

    //illustration and tables
    VclPtr<FixedText>      m_pCaptionSequenceFT;
    VclPtr<ListBox>        m_pCaptionSequenceLB;
    VclPtr<FixedText>      m_pDisplayTypeFT;
    VclPtr<ListBox>        m_pDisplayTypeLB;

    //all but illustration and table
    VclPtr<CheckBox>       m_pTOXMarksCB;

    //index only
    VclPtr<VclContainer>   m_pIdxOptionsFrame;
    VclPtr<CheckBox>       m_pCollectSameCB;
    VclPtr<CheckBox>       m_pUseFFCB;
    VclPtr<CheckBox>       m_pUseDashCB;
    VclPtr<CheckBox>       m_pCaseSensitiveCB;
    VclPtr<CheckBox>       m_pInitialCapsCB;
    VclPtr<CheckBox>       m_pKeyAsEntryCB;
    VclPtr<CheckBox>       m_pFromFileCB;
    VclPtr<MenuButton>     m_pAutoMarkPB;

    // object only
    SwOLENames      aFromNames;
    VclPtr<SvxCheckListBox> m_pFromObjCLB;
    VclPtr<VclContainer>   m_pFromObjFrame;

    VclPtr<CheckBox>       m_pSequenceCB;
    VclPtr<ListBox>        m_pBracketLB;
    VclPtr<VclContainer>   m_pAuthorityFrame;

    //all
    VclPtr<VclContainer>   m_pSortFrame;
    VclPtr<SvxLanguageBox> m_pLanguageLB;
    VclPtr<ListBox>        m_pSortAlgorithmLB;

    IndexEntryResource* pIndexRes;

    OUString        aStyleArr[MAXLEVEL];
    OUString        sAutoMarkURL;
    OUString        sAutoMarkType;
    OUString        sAddStyleUser;
    OUString        sAddStyleContent;

    const IndexEntrySupplierWrapper* pIndexEntryWrapper;

    bool            m_bWaitingInitialSettings;

    DECL_LINK_TYPED(TOXTypeHdl,   ListBox&, void );
    DECL_LINK_TYPED(AddStylesHdl, Button*, void );
    DECL_LINK_TYPED(MenuEnableHdl, Menu*, bool);
    DECL_LINK_TYPED(MenuExecuteHdl, Menu*, bool);
    DECL_LINK_TYPED(LanguageListBoxHdl, ListBox&, void);
    void LanguageHdl(ListBox*);
    DECL_LINK_TYPED(CheckBoxHdl, Button*, void );
    DECL_LINK_TYPED(RadioButtonHdl, Button*, void);
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);
    DECL_LINK_TYPED(ModifyListBoxHdl, ListBox&, void);

    void  ApplyTOXDescription();
    void    FillTOXDescription();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXSelectTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXSelectTabPage();
    virtual void        dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);

    void                SelectType(TOXTypes eSet);  //preset TOXType, GlobalDoc
    void                SetWrtShell(SwWrtShell& rSh);
};

class SwTOXEdit;
class SwTOXButton;
class SwTOXEntryTabPage;

class SwTokenWindow : public VclHBox, public VclBuilderContainer
{
    typedef std::vector<VclPtr<Control> >::iterator ctrl_iterator;
    typedef std::vector<VclPtr<Control> >::const_iterator ctrl_const_iterator;
    typedef std::vector<VclPtr<Control> >::reverse_iterator ctrl_reverse_iterator;
    typedef std::vector<VclPtr<Control> >::const_reverse_iterator ctrl_const_reverse_iterator;

    VclPtr<Button> m_pLeftScrollWin;
    VclPtr<vcl::Window> m_pCtrlParentWin;
    VclPtr<Button> m_pRightScrollWin;
    std::vector<VclPtr<Control> >   aControlList;
    SwForm*         pForm;
    sal_uInt16      nLevel;
    bool            bValid;
    OUString        aButtonTexts[TOKEN_END]; // Text of the buttons
    OUString        aButtonHelpTexts[TOKEN_END]; // QuickHelpText of the buttons
    OUString        sCharStyle;
    Link<SwFormToken&,void>   aButtonSelectedHdl;
    VclPtr<Control>           pActiveCtrl;
    Link<LinkParamNone*,void> aModifyHdl;
    OUString        accessibleName;
    OUString        sAdditionalAccnameString1;
    OUString        sAdditionalAccnameString2;
    OUString        sAdditionalAccnameString3;

    VclPtr<SwTOXEntryTabPage>  m_pParent;

    DECL_LINK_TYPED( EditResize, Edit&, void );
    DECL_LINK_TYPED( NextItemHdl, SwTOXEdit&, void );
    DECL_LINK_TYPED( TbxFocusHdl, Control&, void );
    DECL_LINK_TYPED( NextItemBtnHdl, SwTOXButton&, void );
    DECL_LINK_TYPED( TbxFocusBtnHdl, Control&, void );
    DECL_LINK_TYPED( ScrollHdl, Button*, void );

    void    SetActiveControl(Control* pSet);

    Control*    InsertItem(const OUString& rText, const SwFormToken& aToken);
    void        AdjustPositions();
    void        AdjustScrolling();
    void        MoveControls(long nOffset);

public:
    SwTokenWindow(vcl::Window* pParent);
    virtual ~SwTokenWindow();
    virtual void dispose() override;

    void SetTabPage(SwTOXEntryTabPage *pParent) { m_pParent = pParent; }

    void        SetForm(SwForm& rForm, sal_uInt16 nLevel);
    sal_uInt16      GetLastLevel()const {return nLevel;};

    bool        IsValid() const {return bValid;}

    void        SetInvalid() {bValid = false;}

    OUString    GetPattern() const;

    void        SetButtonSelectedHdl(const Link<SwFormToken&,void>& rLink)
                { aButtonSelectedHdl = rLink;}

    void        SetModifyHdl(const Link<LinkParamNone*,void>& rLink){aModifyHdl = rLink;}

    Control*    GetActiveControl() { return pActiveCtrl; }

    void        InsertAtSelection(const OUString& rText, const SwFormToken& aToken);
    void        RemoveControl(SwTOXButton* pDel, bool bInternalCall = false);

    bool        Contains(FormTokenType) const;

    //helper for pattern buttons and edits
    bool        CreateQuickHelp(Control* pCtrl,
                    const SwFormToken& rToken, const HelpEvent& );

    virtual void    GetFocus() override;
    virtual void    setAllocation(const Size &rAllocation) override;
    void SetFocus2theAllBtn();
private:
    sal_uInt32 GetControlIndex(FormTokenType eType) const;
};

class SwTOXEntryTabPage;

class SwIdxTreeListBox : public SvTreeListBox
{
    VclPtr<SwTOXEntryTabPage> pParent;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
public:
    SwIdxTreeListBox(vcl::Window* pPar, WinBits nStyle);
    virtual ~SwIdxTreeListBox();
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

    OUString        sDelimStr;
    OUString        sLevelStr;
    OUString        sAuthTypeStr;

    OUString        sNoCharStyle;
    OUString        sNoCharSortKey;
    SwForm*         m_pCurrentForm;

    CurTOXType      aLastTOXType;
    bool            bInLevelHdl;

    DECL_LINK_TYPED(StyleSelectHdl, ListBox&, void);
    DECL_LINK_TYPED(EditStyleHdl, Button*, void);
    DECL_LINK_TYPED(InsertTokenHdl, Button*, void);
    DECL_LINK_TYPED(LevelHdl, SvTreeListBox*, void);
    DECL_LINK_TYPED(AutoRightHdl, Button*, void);
    DECL_LINK_TYPED(TokenSelectedHdl, SwFormToken&, void);
    DECL_LINK_TYPED(TabPosHdl, Edit&, void);
    DECL_LINK_TYPED(FillCharHdl, Edit&, void);
    DECL_LINK_TYPED(RemoveInsertAuthHdl, Button*, void);
    DECL_LINK_TYPED(SortKeyHdl, Button*, void);
    DECL_LINK_TYPED(ChapterInfoHdl, ListBox&, void);
    DECL_LINK_TYPED(ChapterInfoOutlineHdl, Edit&, void);
    DECL_LINK_TYPED(NumberFormatHdl, ListBox&, void);

    DECL_LINK_TYPED(AllLevelsHdl, Button*, void);

    void            WriteBackLevel();
    void            UpdateDescriptor();
    DECL_LINK_TYPED(ModifyHdl, LinkParamNone*, void);
    void OnModify(void*);
    DECL_LINK_TYPED(ModifyClickHdl, Button*, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXEntryTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXEntryTabPage();
    virtual void dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                       const SfxItemSet* rAttrSet);
    void                SetWrtShell(SwWrtShell& rSh);

    OUString            GetLevelHelp(sal_uInt16 nLevel) const;

    void                PreTokenButtonRemoved(const SwFormToken& rToken);
    void SetFocus2theAllBtn();
    virtual bool Notify( NotifyEvent& rNEvt ) override;
};

class SwTOXStylesTabPage : public SfxTabPage
{
    VclPtr<ListBox>        m_pLevelLB;
    VclPtr<PushButton>     m_pAssignBT;
    VclPtr<ListBox>        m_pParaLayLB;
    VclPtr<PushButton>     m_pStdBT;
    VclPtr<PushButton>     m_pEditStyleBT;

    SwForm*         m_pCurrentForm;

    DECL_LINK_TYPED( EditStyleHdl, Button *, void );
    DECL_LINK_TYPED( StdHdl, Button*, void );
    DECL_LINK_TYPED(EnableSelectHdl, ListBox&, void);
    DECL_LINK_TYPED( DoubleClickHdl, ListBox&, void );
    DECL_LINK_TYPED( AssignHdl, Button*, void );
    void Modify();

    SwForm&     GetForm()
        {
            SwMultiTOXTabDialog* pDlg = static_cast<SwMultiTOXTabDialog*>(GetTabDialog());
            return *pDlg->GetForm(pDlg->GetCurrentTOXType());
        }

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXStylesTabPage(vcl::Window* pParent, const SfxItemSet& rAttrSet);
    virtual ~SwTOXStylesTabPage();
    virtual void        dispose() override;

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet* ) override;

    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                       const SfxItemSet* rAttrSet);

};

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_SWUICNTTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
