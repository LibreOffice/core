/*************************************************************************
 *
 *  $RCSfile: num.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
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
#ifndef _SVX_NUMPAGES_HXX
#include <svx/numpages.hxx>
#endif
#include "numrule.hxx"
#include "bmpwin.hxx" // BmpWindow

class SwWrtShell;
class SwDocShell;
class SwUiNumRules;
class SwCharFmt;
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



/*-----------------08.02.97 10.48-------------------
    struct aktuelle Numerierung, wird in einem PointerItem
    uebertragen und von allen Tabpages geshared
--------------------------------------------------*/

class SwNumValueSet;
class ValueSet;
class SwBmpNumValueSet;


/*-----------------03.12.97 10:18-------------------

--------------------------------------------------*/
class SwNumPositionTabPage : public SfxTabPage
{
    GroupBox        aLevelGB;
    MultiListBox    aLevelLB;

    GroupBox            aPositionGB;
    FixedText           aAlignFT;
    ListBox             aAlignLB;
    FixedText           aDistBorderFT;
    CheckBox            aRelativeCB;
    MetricField         aDistBorderMF;
    FixedText           aIndentFT;
    MetricField         aIndentMF;
    FixedText           aDistNumFT;
    MetricField         aDistNumMF;
    PushButton          aStandardPB;

    GroupBox            aPreviewGB;
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

//  void                SetMinDist();
    void                InitControls();

    DECL_LINK( LevelHdl, ListBox * );
    DECL_LINK( EditModifyHdl, Edit*);
    DECL_LINK( DistanceHdl, MetricField * );
    DECL_LINK( RelativeHdl, CheckBox * );
    DECL_LINK( StandardHdl, PushButton * );

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
#ifdef DEBUG
    void                SetModified(BOOL bRepaint = TRUE);
#else
    void                SetModified(BOOL bRepaint = TRUE)
                            {bModified = TRUE;
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

