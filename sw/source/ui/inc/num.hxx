/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _NUM_HXX
#define _NUM_HXX


#include <sfx2/tabdlg.hxx>
#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <svx/stddlg.hxx>
#include <numprevw.hxx>
#include "numrule.hxx"

class SwWrtShell;
class SvxBrushItem;
class SwOutlineTabDialog;


/*-----------------13.02.97 14.02-------------------

--------------------------------------------------*/

struct SwBmpItemInfo
{
    SvxBrushItem*   pBrushItem;
    sal_uInt16          nItemId;
};


/*-----------------07.02.97 15.37-------------------

--------------------------------------------------*/
#define NUM_PAGETYPE_BULLET         0
#define NUM_PAGETYPE_SINGLENUM      1
#define NUM_PAGETYPE_NUM            2
#define NUM_PAGETYPE_BMP            3
#define PAGETYPE_USER_START         10

/*-----------------03.12.97 10:18-------------------

--------------------------------------------------*/
class SwNumPositionTabPage : public SfxTabPage
{
        FixedLine       aLevelFL;
    MultiListBox    aLevelLB;

    // --> OD 2008-02-01 #newlistlevelattrs#
    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
    FixedLine       aPositionFL;
    FixedText           aDistBorderFT;
    MetricField         aDistBorderMF;
    CheckBox            aRelativeCB;
    FixedText           aIndentFT;
    MetricField         aIndentMF;
    FixedText           aDistNumFT;
    MetricField         aDistNumMF;
    FixedText           aAlignFT;
    ListBox             aAlignLB;
    // <--

    // --> OD 2008-02-01 #newlistlevelattrs#
    // new set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_ALIGNMENT
    FixedText           aLabelFollowedByFT;
    ListBox             aLabelFollowedByLB;
    FixedText           aListtabFT;
    MetricField         aListtabMF;
    FixedText           aAlign2FT;
    ListBox             aAlign2LB;
    FixedText           aAlignedAtFT;
    MetricField         aAlignedAtMF;
    FixedText           aIndentAtFT;
    MetricField         aIndentAtMF;
    // <--

    PushButton          aStandardPB;

    NumberingPreview    aPreviewWIN;

    SwNumRule*          pActNum;
    SwNumRule*          pSaveNum;
    SwWrtShell*         pWrtSh;

    SwOutlineTabDialog* pOutlineDlg;
    sal_uInt16              nActNumLvl;

    sal_Bool                bModified           : 1;
    sal_Bool                bPreset             : 1;
    sal_Bool                bInInintControl     : 1;  //Modify-Fehler umgehen, soll ab 391 behoben sein
    // --> OD 2008-02-01 #newlistlevelattrs#
    bool                bLabelAlignmentPosAndSpaceModeActive;
    // <--

    void                InitControls();

    DECL_LINK( LevelHdl, ListBox * );
    DECL_LINK( EditModifyHdl, Edit*);
    DECL_LINK( DistanceHdl, MetricField * );
    DECL_LINK( RelativeHdl, CheckBox * );
    DECL_LINK( StandardHdl, PushButton * );

    // --> OD 2008-02-01 #newlistlevelattrs#
    void InitPosAndSpaceMode();
    void ShowControlsDependingOnPosAndSpaceMode();

    DECL_LINK( LabelFollowedByHdl_Impl, ListBox* );
    DECL_LINK( ListtabPosHdl_Impl, MetricField* );
    DECL_LINK( AlignAtHdl_Impl, MetricField* );
    DECL_LINK( IndentAtHdl_Impl, MetricField* );
    // <--

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet);
    ~SwNumPositionTabPage();

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetOutlineTabDialog(SwOutlineTabDialog* pDlg){pOutlineDlg = pDlg;}
    void                SetWrtShell(SwWrtShell* pSh);
#if OSL_DEBUG_LEVEL > 1
    void                SetModified(sal_Bool bRepaint = sal_True);
#else
    void                SetModified(sal_Bool bRepaint = sal_True)
                            {   bModified = sal_True;
                                if(bRepaint)
                                {
                                    aPreviewWIN.SetLevel(nActNumLvl);
                                    aPreviewWIN.Invalidate();
                                }
                            }
#endif
};

class SwSvxNumBulletTabDialog : public SfxTabDialog
{
    SwWrtShell&         rWrtSh;

    String              sRemoveText;
    int                 nRetOptionsDialog;

    protected:
        virtual short   Ok();
        virtual void    PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage);
        DECL_LINK(RemoveNumberingHdl, PushButton*);
    public:
        SwSvxNumBulletTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
        ~SwSvxNumBulletTabDialog();
};
#endif // _NUM_CXX

