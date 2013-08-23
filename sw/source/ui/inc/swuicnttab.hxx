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
#ifndef _SWUI_CNTTAB_HXX
#define _SWUI_CNTTAB_HXX

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
    com::sun::star::uno::Reference< com::sun::star::text::XTextSection >    xContainerSection;
    com::sun::star::uno::Reference< com::sun::star::text::XDocumentIndex >    xDocumentIndex;
};

class SwOneExampleFrame;

struct SwIndexSections_Impl;

class SwMultiTOXTabDialog : public SfxTabDialog
{
    Window*                 m_pExampleContainerWIN;
    CheckBox*               m_pShowExampleCB;
    SwTOXMgr*               pMgr;
    SwWrtShell&             rSh;

    sal_uInt16              m_nSelectId;
    sal_uInt16              m_nStylesId;
    sal_uInt16              m_nColumnId;
    sal_uInt16              m_nBackGroundId;
    sal_uInt16              m_nEntriesId;

    SwOneExampleFrame*      pExampleFrame;

    SwTOXDescription**      pDescArr; //
    SwForm**                pFormArr; //
    SwIndexSections_Impl**  pxIndexSectionsArr;

    SwTOXBase*              pParamTOXBase;

    CurTOXType              eCurrentTOXType;

    OUString                sUserDefinedIndex;
    sal_uInt16              nTypeCount;
    sal_uInt16              nInitialTOXType;

    sal_Bool                bEditTOX;
    sal_Bool                bExampleCreated;
    sal_Bool                bGlobalFlag;

    virtual short       Ok();
    SwTOXDescription*   CreateTOXDescFromTOXBase(const SwTOXBase*pCurTOX);

    DECL_LINK(CreateExample_Hdl, void*);
    DECL_LINK(ShowPreviewHdl, void*);

public:
    SwMultiTOXTabDialog(Window* pParent, const SfxItemSet& rSet,
                        SwWrtShell &rShell,
                        SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                        sal_Bool bGlobal = sal_False);
    ~SwMultiTOXTabDialog();

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

    SwForm*             GetForm(CurTOXType eType);

    CurTOXType          GetCurrentTOXType() const { return eCurrentTOXType;}
    void                SetCurrentTOXType(CurTOXType    eSet)
                                {
                                    eCurrentTOXType = eSet;
                                }

    void                UpdateExample();
    sal_Bool                IsTOXEditMode() const { return bEditTOX;}

    SwWrtShell&         GetWrtShell() {return rSh;}

    SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes);
    void                CreateOrUpdateExample(
                            TOXTypes nTOXIndex, sal_uInt16 nPage = 0, sal_uInt16 nCurLevel = USHRT_MAX);

    static sal_Bool IsNoNum(SwWrtShell& rSh, const String& rName);
};

class IndexEntryResource;
class IndexEntrySupplierWrapper;

class SwTOXSelectTabPage : public SfxTabPage
{
    FixedLine       aTypeTitleFL;
    FixedText       aTitleFT;
    Edit            aTitleED;
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    CheckBox        aReadOnlyCB;

    FixedLine       aAreaFL;
    FixedText       aAreaFT;
    ListBox         aAreaLB;
    FixedText       aLevelFT;   //content, user
    NumericField    aLevelNF;   //content, user

    //content
    FixedLine       aCreateFromFL;  // content, user, illustration
    CheckBox        aFromHeadingsCB;
    CheckBox        aAddStylesCB;
    PushButton      aAddStylesPB;
    Point           aAddStylesPosDef;
    Point           aAddStylesPosUser;
    //user
    CheckBox        aFromTablesCB;
    CheckBox        aFromFramesCB;
    CheckBox        aFromGraphicsCB;
    CheckBox        aFromOLECB;
    CheckBox        aLevelFromChapterCB;

    //illustration + table
    RadioButton     aFromCaptionsRB;
    RadioButton     aFromObjectNamesRB;

    //illustration and tables
    FixedText       aCaptionSequenceFT;
    ListBox         aCaptionSequenceLB;
    FixedText       aDisplayTypeFT;
    ListBox         aDisplayTypeLB;

    //all but illustration and table
    CheckBox        aTOXMarksCB;

    //

    //index only
    FixedLine       aIdxOptionsFL;
    CheckBox        aCollectSameCB;
    CheckBox        aUseFFCB;
    CheckBox        aUseDashCB;
    CheckBox        aCaseSensitiveCB;
    CheckBox        aInitialCapsCB;
    CheckBox        aKeyAsEntryCB;
    CheckBox        aFromFileCB;
    MenuButton      aAutoMarkPB;

    // object only
    SwOLENames      aFromNames;
    SvxCheckListBox aFromObjCLB;
    FixedLine       aFromObjFL;

    CheckBox        aSequenceCB;
    FixedText       aBracketFT;
    ListBox         aBracketLB;
    FixedLine       aAuthorityFormatFL;

    //all
    FixedLine       aSortOptionsFL;
    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;
    FixedText       aSortAlgorithmFT;
    ListBox         aSortAlgorithmLB;

    IndexEntryResource* pIndexRes;

    Point           aCBLeftPos1;
    Point           aCBLeftPos2;
    Point           aCBLeftPos3;

    String          aStyleArr[MAXLEVEL];
    String          sAutoMarkURL;
    String          sAutoMarkType;
    String          sAddStyleUser;
    String          sAddStyleContent;

    const IndexEntrySupplierWrapper* pIndexEntryWrapper;

    sal_Bool            bFirstCall;

    DECL_LINK(TOXTypeHdl,   ListBox* );
    DECL_LINK(AddStylesHdl, PushButton* );
    DECL_LINK(MenuEnableHdl, Menu*);
    DECL_LINK(MenuExecuteHdl, Menu*);
    DECL_LINK(LanguageHdl, ListBox*);

    DECL_LINK(CheckBoxHdl,  CheckBox*   );
    DECL_LINK(RadioButtonHdl, void *);
    DECL_LINK(ModifyHdl, void*);

      void  ApplyTOXDescription();
    void    FillTOXDescription();

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXSelectTabPage(Window* pParent, const SfxItemSet& rAttrSet);
    ~SwTOXSelectTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SelectType(TOXTypes eSet);  //preset TOXType, GlobalDoc
    void                SetWrtShell(SwWrtShell& rSh);
};

class SwTOXEdit;
class SwTOXButton;
class SwTOXEntryTabPage;

class SwTokenWindow : public Window
{
    typedef std::vector<Control*>::iterator ctrl_iterator;
    typedef std::vector<Control*>::const_iterator ctrl_const_iterator;
    typedef std::vector<Control*>::reverse_iterator ctrl_reverse_iterator;
    typedef std::vector<Control*>::const_reverse_iterator ctrl_const_reverse_iterator;

    ImageButton     aLeftScrollWin;
    Window          aCtrlParentWin;
    ImageButton     aRightScrollWin;
    std::vector<Control*>   aControlList;
    SwForm*         pForm;
    sal_uInt16          nLevel;
    sal_Bool            bValid;
    String          aButtonTexts[TOKEN_END]; // Text of the buttons
    String          aButtonHelpTexts[TOKEN_END]; // QuickHelpText of the buttons
    String          sCharStyle;
    Link            aButtonSelectedHdl;
    Control*        pActiveCtrl;
    Link            aModifyHdl;

    SwTOXEntryTabPage*  m_pParent;

    DECL_LINK(EditResize, Edit*);
    DECL_LINK(NextItemHdl, SwTOXEdit* );
    DECL_LINK(TbxFocusHdl, SwTOXEdit* );
    DECL_LINK(NextItemBtnHdl, SwTOXButton* );
    DECL_LINK(TbxFocusBtnHdl, SwTOXButton* );
    DECL_LINK(ScrollHdl, ImageButton* );

    void    SetActiveControl(Control* pSet);

    Control*    InsertItem(const String& rText, const SwFormToken& aToken);
    void        AdjustPositions();
    void        AdjustScrolling();
    void        MoveControls(long nOffset);

public:
    SwTokenWindow(SwTOXEntryTabPage* pParent, const ResId& rResId);
    ~SwTokenWindow();

    void        SetForm(SwForm& rForm, sal_uInt16 nLevel);
    sal_uInt16      GetLastLevel()const {return nLevel;};

    sal_Bool        IsValid() const {return bValid;}

    void        SetInvalid() {bValid = sal_False;}

    String      GetPattern() const;

    void        SetButtonSelectedHdl(const Link& rLink)
                { aButtonSelectedHdl = rLink;}

    void        SetModifyHdl(const Link& rLink){aModifyHdl = rLink;}

    Control*    GetActiveControl()
                    { return pActiveCtrl;}

    void        InsertAtSelection(const String& rText, const SwFormToken& aToken);
    void        RemoveControl(SwTOXButton* pDel, sal_Bool bInternalCall = sal_False);

    sal_Bool        Contains(FormTokenType) const;

    sal_Bool        DetermineLinkStart();

    //helper for pattern buttons and edits
    sal_Bool        CreateQuickHelp(Control* pCtrl,
                    const SwFormToken& rToken, const HelpEvent& );

    virtual void        Resize();
    virtual void        GetFocus();
};

class SwTOXEntryTabPage;

class SwIdxTreeListBox : public SvTreeListBox
{
    SwTOXEntryTabPage* pParent;

    virtual void    RequestHelp( const HelpEvent& rHEvt );
public:
    SwIdxTreeListBox(SwTOXEntryTabPage* pPar, const ResId& rResId);
};

class SwTOXEntryTabPage : public SfxTabPage
{
    FixedText           aLevelFT;
    SwIdxTreeListBox    aLevelLB;

    FixedLine       aEntryFL;
    FixedText       aTokenFT;
    SwTokenWindow   aTokenWIN;
    PushButton      aAllLevelsPB;

    PushButton      aEntryNoPB;
    PushButton      aEntryPB;
    PushButton      aTabPB;
    PushButton      aChapterInfoPB;
    PushButton      aPageNoPB;
    PushButton      aHyperLinkPB;

    ListBox         aAuthFieldsLB;
    PushButton      aAuthInsertPB;
    PushButton      aAuthRemovePB;

    FixedText       aCharStyleFT;
    ListBox         aCharStyleLB;       // character style of the current token
    PushButton      aEditStylePB;

    FixedText       aChapterEntryFT;
    ListBox         aChapterEntryLB;    // type of chapter info

    FixedText       aNumberFormatFT;
    ListBox         aNumberFormatLB;    //!< format for numbering (E#)

    FixedText       aEntryOutlineLevelFT;    //!< Fixed text, for i53420
    NumericField    aEntryOutlineLevelNF;   //!< level to evaluate outline level to, for i53420
    FixedText       aFillCharFT;
    ComboBox        aFillCharCB;        // fill char for tab stop
    FixedText       aTabPosFT;
    MetricField     aTabPosMF;          // tab stop position
    CheckBox        aAutoRightCB;
    FixedLine       aFormatFL;

    CheckBox        aRelToStyleCB;      // position relative to the right margin of the para style
    FixedText       aMainEntryStyleFT;
    ListBox         aMainEntryStyleLB;  // character style of main entries in indexes
    CheckBox        aAlphaDelimCB;
    CheckBox        aCommaSeparatedCB;

    RadioButton     aSortDocPosRB;
    RadioButton     aSortContentRB;
    FixedLine       aSortingFL;

    FixedText           aFirstKeyFT;
    ListBox             aFirstKeyLB;
    ImageRadioButton    aFirstSortUpRB;
    ImageRadioButton    aFirstSortDownRB;

    FixedText           aSecondKeyFT;
    ListBox             aSecondKeyLB;
    ImageRadioButton    aSecondSortUpRB;
    ImageRadioButton    aSecondSortDownRB;

    FixedText           aThirdKeyFT;
    ListBox             aThirdKeyLB;
    ImageRadioButton    aThirdSortUpRB;
    ImageRadioButton    aThirdSortDownRB;

    FixedLine       aSortKeyFL;

    String          sDelimStr;
    String          sLevelStr;
    String          sAuthTypeStr;

    String          sNoCharStyle;
    String          sNoCharSortKey;
    Point           aButtonPositions[5];
    SwForm*         m_pCurrentForm;

    Point           aRelToStylePos;
    Point           aRelToStyleIdxPos;
    Size            aLevelFLSize;

    CurTOXType      aLastTOXType;
    sal_Bool            bInLevelHdl;

    Point           aChapterEntryFTPosition; //!< holds position of ChapterEntryFT control,
                                             //to be used in moving the element among different tokens
    Point           aEntryOutlineLevelFTPosition;//!< holds position ofrEntryOutlineLevelFT control
    sal_Int32       nBiasToEntryPoint;

    DECL_LINK(StyleSelectHdl, ListBox*);
    DECL_LINK(EditStyleHdl, PushButton*);
    DECL_LINK(InsertTokenHdl, PushButton*);
    DECL_LINK(LevelHdl, SvTreeListBox*);
    DECL_LINK(AutoRightHdl, CheckBox*);
    DECL_LINK(TokenSelectedHdl, SwFormToken*);
    DECL_LINK(TabPosHdl, MetricField*);
    DECL_LINK(FillCharHdl, ComboBox*);
    DECL_LINK(RemoveInsertAuthHdl, PushButton*);
    DECL_LINK(SortKeyHdl, RadioButton*);
    DECL_LINK(ChapterInfoHdl, ListBox*);
    DECL_LINK(ChapterInfoOutlineHdl, NumericField*);
    DECL_LINK(NumberFormatHdl, ListBox*);

    DECL_LINK(AllLevelsHdl, void *);

    void            EnableButtons();
    void            WriteBackLevel();
    void            UpdateDescriptor();
    DECL_LINK(ModifyHdl, void*);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXEntryTabPage(Window* pParent, const SfxItemSet& rAttrSet);
    ~SwTOXEntryTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
    void                SetWrtShell(SwWrtShell& rSh);

    String              GetLevelHelp(sal_uInt16 nLevel) const;

    void                PreTokenButtonRemoved(const SwFormToken& rToken);
};

class SwTOXStylesTabPage : public SfxTabPage
{
    ListBox*        m_pLevelLB;
    PushButton*     m_pAssignBT;
    ListBox*        m_pParaLayLB;
    PushButton*     m_pStdBT;
    PushButton*     m_pEditStyleBT;

    SwForm*         m_pCurrentForm;

    DECL_LINK( EditStyleHdl, Button *);
    DECL_LINK(StdHdl, void *);
    DECL_LINK(EnableSelectHdl, void *);
    DECL_LINK(DoubleClickHdl, void *);
    DECL_LINK(AssignHdl, void *);
    void Modify();

    SwForm&     GetForm()
        {
            SwMultiTOXTabDialog* pDlg = (SwMultiTOXTabDialog*)GetTabDialog();
            return *pDlg->GetForm(pDlg->GetCurrentTOXType());
        }

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwTOXStylesTabPage(Window* pParent, const SfxItemSet& rAttrSet);
    ~SwTOXStylesTabPage();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );

    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

};

#endif // _SWUI_CNTTAB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
