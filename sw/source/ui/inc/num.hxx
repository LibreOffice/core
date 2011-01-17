/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    FixedLine       aPositionFL;
    FixedLine       aLevelFL;
    MultiListBox    aLevelLB;

    // --> OD 2008-02-01 #newlistlevelattrs#
    // former set of controls shown for numbering rules containing list level
    // attributes in SvxNumberFormat::SvxNumPositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
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

