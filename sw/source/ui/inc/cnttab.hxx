/*************************************************************************
 *
 *  $RCSfile: cnttab.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-03 11:25:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CNTTAB_HXX
#define _CNTTAB_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#include "tox.hxx"
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOXMGR_HXX
#include <toxmgr.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX //autogen wg. SvxCheckListBox
#include <svx/checklbx.hxx>
#endif
#ifndef _SV_RESARY_HXX //autogen wg. ResStringArray
#include <vcl/resary.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_MENUBTN_HXX
#include <vcl/menubtn.hxx>
#endif

class SwWrtShell;
class SwTOXMgr;
/* -----------------14.06.99 12:12-------------------

 --------------------------------------------------*/
struct CurTOXType
{
    TOXTypes    eType;
    USHORT      nIndex; //for TOX_USER only

    BOOL operator==(const CurTOXType aCmp)
       {
        return eType == aCmp.eType && nIndex == aCmp.nIndex;
       }
    USHORT GetFlatIndex() const;

};
//-----------------------------------------------------------------------------
#if 0
class IdxExampleResource : public Resource
{
    ResStringArray      aTextArray;

public:
    IdxExampleResource(const ResId& rResId);

    ResStringArray& GetTextArray() {return aTextArray;}
};
#endif

//-----------------------------------------------------------------------------
class SwOneExampleFrame;
struct SwIndexSections_Impl;

class SwMultiTOXTabDialog : public SfxTabDialog
{
    Window                  aExampleContainerWIN;
    Window                  aExampleWIN;
    CheckBox                aShowExampleCB;
    SwTOXMgr*               pMgr;
    SwWrtShell&             rSh;

    SwOneExampleFrame*      pExampleFrame;

    SwTOXDescription**      pDescArr; //
    SwForm**                pFormArr; //
    SwIndexSections_Impl**  pxIndexSectionsArr;

    SwTOXBase*              pParamTOXBase;

    CurTOXType              eCurrentTOXType;

    String                  sUserDefinedIndex;
    USHORT                  nTypeCount;
    USHORT                  nInitialTOXType;

    BOOL                    bEditTOX;
    BOOL                    bExampleCreated;
    BOOL                    bGlobalFlag;

    virtual short       Ok();
    SwTOXDescription*   CreateTOXDescFromTOXBase(const SwTOXBase*pCurTOX);

    DECL_LINK(CreateExample_Hdl, void* );
    DECL_LINK(ShowPreviewHdl, CheckBox*);

public:
    SwMultiTOXTabDialog(Window* pParent, const SfxItemSet& rSet,
                        SwWrtShell &rShell,
                        SwTOXBase* pCurTOX, USHORT nToxType = USHRT_MAX,
                        BOOL bGlobal = FALSE);
    ~SwMultiTOXTabDialog();

    virtual void        PageCreated( USHORT nId, SfxTabPage &rPage );

    SwForm*             GetForm(CurTOXType eType);

    CurTOXType          GetCurrentTOXType() const { return eCurrentTOXType;}
    void                SetCurrentTOXType(CurTOXType    eSet)
                                {
                                    eCurrentTOXType = eSet;
                                }

    void                UpdateExample();
    BOOL                IsTOXEditMode() const { return bEditTOX;}
//  const SwTOXBase*    GetCurTOX() const;

    SwWrtShell&         GetWrtShell() {return rSh;}

    SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes);
    void                CreateOrUpdateExample(
                            USHORT nTOXIndex, USHORT nPage = 0, USHORT nCurLevel = USHRT_MAX);

    static BOOL IsNoNum(SwWrtShell& rSh, const String& rName);
};

/* -----------------14.06.99 12:17-------------------

 --------------------------------------------------*/
class SwOLENames : public Resource
{
    ResStringArray      aNamesAry;
public:
    SwOLENames(const ResId& rResId) :
        Resource(rResId),
        aNamesAry(ResId(1)){FreeResource();}

    ResStringArray&     GetNames() { return aNamesAry;}

};
/* -----------------14.07.99 12:17-------------------

 --------------------------------------------------*/
class SwTOXSelectTabPage : public SfxTabPage
{
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    CheckBox        aReadOnlyCB;
    FixedText       aTitleFT;
    Edit            aTitleED;
    GroupBox        aTypeTitleGB;

    ListBox         aAreaLB;
    FixedText       aLevelFT;   //content, user
    NumericField    aLevelNF;   //content, user
    GroupBox        aAreaGB;

    //content
    CheckBox        aFromHeadingsCB;
    PushButton      aChapterDlgPB;
    CheckBox        aAddStylesCB;
    PushButton      aAddStylesPB;
    //user
//  CheckBox        aFromStylesCB; -> aAddStylesCB;
//  PushButton      aFromStylesPB; -> aAddStylesPB;
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
    GroupBox        aCreateFromGB;  // content, user, illustration

    //index only
    CheckBox        aCollectSameCB;
    CheckBox        aUseFFCB;
    CheckBox        aUseDashCB;
    CheckBox        aCaseSensitiveCB;
    CheckBox        aInitialCapsCB;
    CheckBox        aKeyAsEntryCB;
    CheckBox        aFromFileCB;
    MenuButton      aAutoMarkPB;
//  PushButton      aCreateAutoMarkPB;
//  PushButton      aEditAutoMarkPB;
    GroupBox        aIdxOptionsGB; // index only

    // object only
    SwOLENames      aFromNames;
    SvxCheckListBox aFromObjCLB;
    GroupBox        aFromObjGB;

    CheckBox        aSequenceCB;
    FixedText       aBracketFT;
    ListBox         aBracketLB;
    GroupBox        aAuthorityFormatGB;

    Point           aCBLeftPos1;
    Point           aCBLeftPos2;
    Point           aCBLeftPos3;

    String          aStyleArr[MAXLEVEL];
    String          sAutoMarkURL;
    String          sAutoMarkType;
    String          sAddStyleUser;
    String          sAddStyleContent;

    BOOL            bFirstCall;

    DECL_LINK(TOXTypeHdl,   ListBox* );
    DECL_LINK(TOXAreaHdl,   ListBox* );
    DECL_LINK(ChapterHdl,   PushButton* );
    DECL_LINK(AddStylesHdl, PushButton* );
//  DECL_LINK(AutoMarkHdl,  PushButton* );
//  DECL_LINK(CreateEditAutoMarkHdl,PushButton* );
    DECL_LINK(MenuEnableHdl, Menu*);
    DECL_LINK(MenuExecuteHdl, Menu*);

    DECL_LINK(CheckBoxHdl,  CheckBox*   );
    DECL_LINK(RadioButtonHdl, RadioButton* );
    DECL_LINK(ModifyHdl, void*);

      void  ApplyTOXDescription();
    void    FillTOXDescription();
public:
    SwTOXSelectTabPage(Window* pParent, const SfxItemSet& rAttrSet);
    ~SwTOXSelectTabPage();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SelectType(TOXTypes eSet);  //preset TOXType, GlobalDoc
    void                SetWrtShell(SwWrtShell& rSh);
};
/* -----------------16.06.99 08:33-------------------

 --------------------------------------------------*/

DECLARE_LIST(TOXControlList, Control*);
class SwTOXEdit;
class SwTOXButton;
class SwTOXEntryTabPage;
class SwTokenWindow : public Window
{
    ImageButton     aLeftScrollWin;
    Window          aCtrlParentWin;
    ImageButton     aRightScrollWin;
    TOXControlList  aControlList;
    SwForm*         pForm;
    USHORT          nLevel;
    BOOL            bValid;
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

    void        SetForm(SwForm& rForm, USHORT nLevel);
    USHORT      GetLastLevel()const {return nLevel;};

    BOOL        IsValid() const {return bValid;}
    void        Invalidate() {bValid = FALSE;}

    String      GetPattern() const;

    void        SetButtonSelectedHdl(const Link& rLink)
                { aButtonSelectedHdl = rLink;}

    void        SetModifyHdl(const Link& rLink){aModifyHdl = rLink;}

    Control*    GetActiveControl()
                    { return pActiveCtrl;}

    void        InsertAtSelection(const String& rText, const SwFormToken& aToken);
    void        RemoveControl(SwTOXButton* pDel, BOOL bInternalCall = FALSE);

    BOOL        Contains(FormTokenType) const;

    BOOL        DetermineLinkStart();

    //helper for pattern buttons and edits
    BOOL        CreateQuickHelp(Control* pCtrl,
                    const SwFormToken& rToken, const HelpEvent& );

    virtual void        Resize();
};
/* -----------------------------23.12.99 14:16--------------------------------

 ---------------------------------------------------------------------------*/
class SwTOXEntryTabPage;
class SwIdxTreeListBox : public SvTreeListBox
{
    SwTOXEntryTabPage* pParent;

    virtual void    RequestHelp( const HelpEvent& rHEvt );
public:
    SwIdxTreeListBox(SwTOXEntryTabPage* pPar, const ResId& rResId);
};

/* -----------------16.06.99 12:49-------------------

 --------------------------------------------------*/
class SwTOXEntryTabPage : public SfxTabPage
{
    SwIdxTreeListBox    aLevelLB;
    FixedText           aLevelFT;

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
//  FixedText       aSortingFT;
//  ListBox         aSortingLB;

    FixedText       aFillCharFT;
    ComboBox        aFillCharCB;        // fill char for tab stop
    FixedText       aTabPosFT;
    MetricField     aTabPosMF;          // tab stop position
    CheckBox        aAutoRightCB;
    GroupBox        aEntryGB;

    CheckBox        aRelToStyleCB;      // position relative to the right margin of the para style
    FixedText       aMainEntryStyleFT;
    ListBox         aMainEntryStyleLB;  // character style of main entries in indexes
    CheckBox        aAlphaDelimCB;
    CheckBox        aCommaSeparatedCB;
    GroupBox        aFormatGB;

    RadioButton     aSortDocPosRB;
    RadioButton     aSortContentRB;
    GroupBox        aSortingGB;

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

    GroupBox        aSortKeyGB;

    String          sDelimStr;
    String          sLevelStr;
    String          sAuthTypeStr;

    String          sNoCharStyle;
    String          sNoCharSortKey;
    Point           aButtonPositions[5];
    SwForm*         pCurrentForm;

    Point           aRelToStylePos;
    Point           aRelToStyleIdxPos;
    Size            aLevelGBSize;

    CurTOXType      aLastTOXType;
    BOOL            bInLevelHdl;

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
    DECL_LINK(AllLevelsHdl, PushButton*);

    void            EnableButtons();
    void            WriteBackLevel();
    void            UpdateDescriptor();
    DECL_LINK(ModifyHdl, void*);

public:
    SwTOXEntryTabPage(Window* pParent, const SfxItemSet& rAttrSet);
    ~SwTOXEntryTabPage();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
    void                SetWrtShell(SwWrtShell& rSh);

    String              GetLevelHelp(USHORT nLevel) const;

    void                PreTokenButtonRemoved(const SwFormToken& rToken);
};
/* -----------------05.07.99 13:00-------------------

 --------------------------------------------------*/
class SwTOXStylesTabPage : public SfxTabPage
{
    FixedText       aLevelFT2;
    ListBox         aLevelLB;
    FixedText       aTemplateFT;
    ListBox         aParaLayLB;
    PushButton      aStdBT;
    PushButton      aAssignBT;
    PushButton      aEditStyleBT;
    GroupBox        aFormatGB;

    SwForm*         pCurrentForm;
//  void            UpdatePattern();

    DECL_LINK( EditStyleHdl, Button *);
    DECL_LINK( StdHdl, Button * );
    DECL_LINK( EnableSelectHdl, ListBox * );
    DECL_LINK( DoubleClickHdl, Button * );
    DECL_LINK( AssignHdl, Button * );
    DECL_LINK( ModifyHdl, void*);

    SwForm&     GetForm()
        {
            SwMultiTOXTabDialog* pDlg = (SwMultiTOXTabDialog*)GetTabDialog();
            return *pDlg->GetForm(pDlg->GetCurrentTOXType());
        }
public:
    SwTOXStylesTabPage(Window* pParent, const SfxItemSet& rAttrSet);
    ~SwTOXStylesTabPage();

    virtual BOOL        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet& );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

};

#endif // _CNTTAB_HXX

