/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: num.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:23:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _NUM_HXX
#define _NUM_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif
#ifndef _NUMPREVW_HXX
#include <numprevw.hxx>
#endif
#include "numrule.hxx"

class SwWrtShell;
class SvxBrushItem;
class SwOutlineTabDialog;


/*-----------------13.02.97 14.02-------------------

--------------------------------------------------*/

struct SwBmpItemInfo
{
    SvxBrushItem*   pBrushItem;
    USHORT          nItemId;
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
    USHORT              nActNumLvl;

    BOOL                bModified           : 1;
    BOOL                bHasChild           : 1;
    BOOL                bPreset             : 1;
    BOOL                bInInintControl     : 1;  //Modify-Fehler umgehen, soll ab 391 behoben sein
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

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    SwNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet);
    ~SwNumPositionTabPage();

    virtual void        ActivatePage(const SfxItemSet& rSet);
    virtual int         DeactivatePage(SfxItemSet *pSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    void                SetOutlineTabDialog(SwOutlineTabDialog* pDlg){pOutlineDlg = pDlg;}
    void                SetWrtShell(SwWrtShell* pSh);
#if OSL_DEBUG_LEVEL > 1
    void                SetModified(BOOL bRepaint = TRUE);
#else
    void                SetModified(BOOL bRepaint = TRUE)
                            {   bModified = TRUE;
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
        virtual void    PageCreated(USHORT nPageId, SfxTabPage& rPage);
        DECL_LINK(RemoveNumberingHdl, PushButton*);
    public:
        SwSvxNumBulletTabDialog(Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
        ~SwSvxNumBulletTabDialog();
};
#endif // _NUM_CXX

