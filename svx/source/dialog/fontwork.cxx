/*************************************************************************
 *
 *  $RCSfile: fontwork.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:08 $
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

// include ---------------------------------------------------------------

#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#pragma hdrstop

#define _SVX_FONTWORK_CXX

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif
#ifndef _SVDOPATH_HXX
#include "svdopath.hxx"
#endif
#ifndef _SVDVIEW_HXX
#include "svdview.hxx"
#endif
#ifndef _SVDOCIRC_HXX
#include "svdocirc.hxx"
#endif
#ifndef _SVX_XTEXTITEM_HXX //autogen
#include "xtextit.hxx"
#endif

#include "dialmgr.hxx"
#include "dlgutil.hxx"

#include "dialogs.hrc"
#include "fontwork.hrc"
#include "fontwork.hxx"
#include "outlobj.hxx"


SFX_IMPL_DOCKINGWINDOW( SvxFontWorkChildWindow, SID_FONTWORK );

/*************************************************************************
|*
|* ControllerItem fuer Fontwork
|*
\************************************************************************/

SvxFontWorkControllerItem::SvxFontWorkControllerItem
(
    USHORT nId,
    SvxFontWorkDialog& rDlg,
    SfxBindings& rBindings
) :

    SfxControllerItem( nId, rBindings ),

    rFontWorkDlg( rDlg )
{
}

/*************************************************************************
|*
|* StateChanged-Methode fuer FontWork-Items
|*
\************************************************************************/

void SvxFontWorkControllerItem::StateChanged( USHORT nSID, SfxItemState eState,
                                              const SfxPoolItem* pItem )
{
    switch ( GetId() )
    {
        case SID_FORMTEXT_STYLE:
        {
            const XFormTextStyleItem* pStateItem =
                                PTR_CAST(XFormTextStyleItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStyleItem erwartet");
            rFontWorkDlg.SetStyle_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_ADJUST:
        {
            const XFormTextAdjustItem* pStateItem =
                                PTR_CAST(XFormTextAdjustItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextAdjustItem erwartet");
            rFontWorkDlg.SetAdjust_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_DISTANCE:
        {
            const XFormTextDistanceItem* pStateItem =
                                PTR_CAST(XFormTextDistanceItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextDistanceItem erwartet");
            rFontWorkDlg.SetDistance_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_START:
        {
            const XFormTextStartItem* pStateItem =
                                PTR_CAST(XFormTextStartItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStartItem erwartet");
            rFontWorkDlg.SetStart_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_MIRROR:
        {
            const XFormTextMirrorItem* pStateItem =
                                PTR_CAST(XFormTextMirrorItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextMirrorItem erwartet");
            rFontWorkDlg.SetMirror_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_STDFORM:
        {
            const XFormTextStdFormItem* pStateItem =
                                PTR_CAST(XFormTextStdFormItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextStdFormItem erwartet");
            rFontWorkDlg.SetStdForm_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_HIDEFORM:
        {
            const XFormTextHideFormItem* pStateItem =
                                PTR_CAST(XFormTextHideFormItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextHideFormItem erwartet");
            rFontWorkDlg.SetShowForm_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_OUTLINE:
        {
            const XFormTextOutlineItem* pStateItem =
                                PTR_CAST(XFormTextOutlineItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextOutlineItem erwartet");
            rFontWorkDlg.SetOutline_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHADOW:
        {
            const XFormTextShadowItem* pStateItem =
                                PTR_CAST(XFormTextShadowItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowItem erwartet");
            rFontWorkDlg.SetShadow_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWCOLOR:
        {
            const XFormTextShadowColorItem* pStateItem =
                                PTR_CAST(XFormTextShadowColorItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowColorItem erwartet");
            rFontWorkDlg.SetShadowColor_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWXVAL:
        {
            const XFormTextShadowXValItem* pStateItem =
                                PTR_CAST(XFormTextShadowXValItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowXValItem erwartet");
            rFontWorkDlg.SetShadowXVal_Impl(pStateItem);
            break;
        }
        case SID_FORMTEXT_SHDWYVAL:
        {
            const XFormTextShadowYValItem* pStateItem =
                                PTR_CAST(XFormTextShadowYValItem, pItem);
            DBG_ASSERT(pStateItem || pItem == 0, "XFormTextShadowYValItem erwartet");
            rFontWorkDlg.SetShadowYVal_Impl(pStateItem);
            break;
        }
    }
}

/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Fontwork-Dialog
|*
\************************************************************************/

SvxFontWorkChildWindow::SvxFontWorkChildWindow
(
    Window* pParent,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo
) :

    SfxChildWindow( pParent, nId )

{
    pWindow = new SvxFontWorkDialog( pBindings, this, pParent,
                                     SVX_RES( RID_SVXDLG_FONTWORK ) );
    SvxFontWorkDialog* pDlg = (SvxFontWorkDialog*) pWindow;

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pDlg->Initialize( pInfo );
}

/*************************************************************************
|*
|* Floating Window zur Attributierung von Texteffekten
|*
\************************************************************************/

// pMgr wird unten angelegt
#define FW_RESID(nId) ResId(nId, pMgr)

SvxFontWorkDialog::SvxFontWorkDialog( SfxBindings *pBindinx,
                                      SfxChildWindow *pCW,
                                      Window* pParent,
                                      const ResId& rResId ) :
    SfxDockingWindow( pBindinx, pCW, pParent, rResId ),

    aFormSet        (this, ResId(VS_FORMS)),

    aTbxStyle       (this, ResId(TBX_STYLE)),
    aTbxAdjust      (this, ResId(TBX_ADJUST)),

    aFbDistance     (this, ResId(FB_DISTANCE)),
    aMtrFldDistance (this, ResId(MTR_FLD_DISTANCE)),
    aFbTextStart    (this, ResId(FB_TEXTSTART)),
    aMtrFldTextStart(this, ResId(MTR_FLD_TEXTSTART)),

    aTbxShadow      (this, ResId(TBX_SHADOW)),

    aFbShadowX      (this, ResId(FB_SHADOW_X)),
    aMtrFldShadowX  (this, ResId(MTR_FLD_SHADOW_X)),
    aFbShadowY      (this, ResId(FB_SHADOW_Y)),
    aMtrFldShadowY  (this, ResId(MTR_FLD_SHADOW_Y)),

    aShadowColorLB  (this, ResId(CLB_SHADOW_COLOR)),

    rBindings       (*pBindinx),

    nLastStyleTbxId(0),
    nLastAdjustTbxId(0),
    nLastShadowTbxId(0),
    nSaveShadowX    (0),
    nSaveShadowY    (0),
    nSaveShadowAngle(450),
    nSaveShadowSize (100),

    pColorTable     (NULL)
{
    FreeResource();

    pCtrlItems[0] = new SvxFontWorkControllerItem(SID_FORMTEXT_STYLE, *this, rBindings);
    pCtrlItems[1] = new SvxFontWorkControllerItem(SID_FORMTEXT_ADJUST, *this, rBindings);
    pCtrlItems[2] = new SvxFontWorkControllerItem(SID_FORMTEXT_DISTANCE, *this, rBindings);
    pCtrlItems[3] = new SvxFontWorkControllerItem(SID_FORMTEXT_START, *this, rBindings);
    pCtrlItems[4] = new SvxFontWorkControllerItem(SID_FORMTEXT_MIRROR, *this, rBindings);
    pCtrlItems[5] = new SvxFontWorkControllerItem(SID_FORMTEXT_STDFORM, *this, rBindings);
    pCtrlItems[6] = new SvxFontWorkControllerItem(SID_FORMTEXT_HIDEFORM, *this, rBindings);
    pCtrlItems[7] = new SvxFontWorkControllerItem(SID_FORMTEXT_OUTLINE, *this, rBindings);
    pCtrlItems[8] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHADOW, *this, rBindings);
    pCtrlItems[9] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWCOLOR, *this, rBindings);
    pCtrlItems[10] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWXVAL, *this, rBindings);
    pCtrlItems[11] = new SvxFontWorkControllerItem(SID_FORMTEXT_SHDWYVAL, *this, rBindings);

    WinBits aNewStyle = ( aFormSet.GetStyle() | WB_VSCROLL | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aFormSet.SetStyle( aNewStyle );

    Size aSize = aTbxStyle.CalcWindowSizePixel();
    aTbxStyle.SetSizePixel(aSize);
    aTbxStyle.SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectStyleHdl_Impl) );

    aTbxAdjust.SetSizePixel(aSize);
    aTbxAdjust.SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectAdjustHdl_Impl) );

    aTbxShadow.SetSizePixel(aSize);
    aTbxShadow.SetSelectHdl( LINK(this, SvxFontWorkDialog, SelectShadowHdl_Impl) );

    ResMgr* pMgr = DIALOG_MGR();
    aFbShadowX.SetBitmap(Bitmap(FW_RESID(RID_SVXBMP_SHADOW_XDIST)));
    aFbShadowY.SetBitmap(Bitmap(FW_RESID(RID_SVXBMP_SHADOW_YDIST)));

    Link aLink = LINK(this, SvxFontWorkDialog, ModifyInputHdl_Impl);
    aMtrFldDistance.SetModifyHdl( aLink );
    aMtrFldTextStart.SetModifyHdl( aLink );
    aMtrFldShadowX.SetModifyHdl( aLink );
    aMtrFldShadowY.SetModifyHdl( aLink );

    // System-Metrik setzen
    FieldUnit eDlgUnit;
    GET_MODULE_FIELDUNIT( eDlgUnit );
    SetFieldUnit( aMtrFldDistance, eDlgUnit, TRUE );
    SetFieldUnit( aMtrFldTextStart, eDlgUnit, TRUE );
    SetFieldUnit( aMtrFldShadowX, eDlgUnit, TRUE );
    SetFieldUnit( aMtrFldShadowY, eDlgUnit, TRUE );
    if( eDlgUnit == FUNIT_MM )
    {
        aMtrFldDistance.SetSpinSize( 50 );
        aMtrFldTextStart.SetSpinSize( 50 );
        aMtrFldShadowX.SetSpinSize( 50 );
        aMtrFldShadowY.SetSpinSize( 50 );
    }
    else
    {
        aMtrFldDistance.SetSpinSize( 10 );
        aMtrFldTextStart.SetSpinSize( 10 );
        aMtrFldShadowX.SetSpinSize( 10 );
        aMtrFldShadowY.SetSpinSize( 10 );
    }

    aShadowColorLB.SetSelectHdl( LINK(this, SvxFontWorkDialog, ColorSelectHdl_Impl) );

    aInputTimer.SetTimeout(500);
    aInputTimer.SetTimeoutHdl(LINK(this, SvxFontWorkDialog, InputTimoutHdl_Impl));

    aFormSet.SetSelectHdl( LINK(this, SvxFontWorkDialog, FormSelectHdl_Impl) );
    aFormSet.SetColCount(4);
    aFormSet.SetLineCount(2);

    Bitmap aBmp(FW_RESID(RID_SVXBMP_FONTWORK_FORM1));
    aSize.Height() = aFormSet.CalcWindowSizePixel(aBmp.GetSizePixel()).Height() + 2;
    aFormSet.SetSizePixel(aSize);

    aFormSet.InsertItem( 1, aBmp, String(FW_RESID(RID_SVXSTR_FONTWORK_FORM1)));
    aFormSet.InsertItem( 2, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM2)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM2)));
    aFormSet.InsertItem( 3, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM3)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM3)));
    aFormSet.InsertItem( 4, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM4)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM4)));
    aFormSet.InsertItem( 5, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM5)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM5)));
    aFormSet.InsertItem( 6, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM6)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM6)));
    aFormSet.InsertItem( 7, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM7)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM7)));
    aFormSet.InsertItem( 8, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM8)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM8)));
    aFormSet.InsertItem( 9, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM9)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM9)));
    aFormSet.InsertItem(10, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM10)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM10)));
    aFormSet.InsertItem(11, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM11)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM11)));
    aFormSet.InsertItem(12, Bitmap(FW_RESID(RID_SVXBMP_FONTWORK_FORM12)),
                            String(FW_RESID(RID_SVXSTR_FONTWORK_FORM12)));
}

#undef FW_RESID

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SvxFontWorkDialog::~SvxFontWorkDialog()
{
    for (USHORT i = 0; i < CONTROLLER_COUNT; i++)
        DELETEZ(pCtrlItems[i]);
}

/*************************************************************************
|*
\************************************************************************/

void SvxFontWorkDialog::Zoom()
{
    SfxDockingWindow::Roll();
}


/*************************************************************************
|*
|*
|*
\************************************************************************/

SfxChildAlignment SvxFontWorkDialog::CheckAlignment( SfxChildAlignment eActAlign,
                                                     SfxChildAlignment eAlign )
{
    SfxChildAlignment eAlignment;

    switch ( eAlign )
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
        {
            eAlignment = eActAlign;
        }
        break;

        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
        {
            eAlignment = eAlign;
        }
        break;

        default:
        {
            eAlignment = eAlign;
        }
        break;
    }

    return eAlignment;
}


/*************************************************************************
|*
|* Style-Buttons setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetStyle_Impl(const XFormTextStyleItem* pItem)
{
    if ( pItem )
    {
        USHORT nId = TBI_STYLE_OFF;

        switch ( pItem->GetValue() )
        {
            case XFT_ROTATE : nId = TBI_STYLE_ROTATE;   break;
            case XFT_UPRIGHT: nId = TBI_STYLE_UPRIGHT;  break;
            case XFT_SLANTX : nId = TBI_STYLE_SLANTX;   break;
            case XFT_SLANTY : nId = TBI_STYLE_SLANTY;   break;
        }
        aTbxStyle.Enable();

        if ( pItem->GetValue() == XFT_NONE )
        {
            aTbxStyle.CheckItem(TBI_STYLE_ROTATE, FALSE);
            aTbxStyle.CheckItem(TBI_STYLE_UPRIGHT, FALSE);
            aTbxStyle.CheckItem(TBI_STYLE_SLANTX, FALSE);
            aTbxStyle.CheckItem(TBI_STYLE_SLANTY, FALSE);
        }
        else
            aTbxStyle.CheckItem(TBI_STYLE_OFF, FALSE);

        aTbxStyle.CheckItem(nId);
        nLastStyleTbxId = nId;
    }
    else
        aTbxStyle.Disable();
}

/*************************************************************************
|*
|* Adjust-Buttons setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetAdjust_Impl(const XFormTextAdjustItem* pItem)
{
    if ( pItem )
    {
        USHORT nId;

        aTbxAdjust.Enable();
        aMtrFldDistance.Enable();

        if ( pItem->GetValue() == XFT_LEFT || pItem->GetValue() == XFT_RIGHT )
        {
            if ( pItem->GetValue() == XFT_LEFT )    nId = TBI_ADJUST_LEFT;
            else                                    nId = TBI_ADJUST_RIGHT;
            aMtrFldTextStart.Enable();
        }
        else
        {
            if ( pItem->GetValue() == XFT_CENTER )  nId = TBI_ADJUST_CENTER;
            else                                    nId = TBI_ADJUST_AUTOSIZE;
            aMtrFldTextStart.Disable();
        }

        if ( !aTbxAdjust.IsItemChecked(nId) )
        {
            aTbxAdjust.CheckItem(nId);
        }
        nLastAdjustTbxId = nId;
    }
    else
    {
        aTbxAdjust.Disable();
        aMtrFldTextStart.Disable();
        aMtrFldDistance.Disable();
    }
}

/*************************************************************************
|*
|* Abstand-Wert in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetDistance_Impl(const XFormTextDistanceItem* pItem)
{
    if ( pItem && !aMtrFldDistance.HasFocus() )
    {
        SetMetricValue( aMtrFldDistance, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
    }
}

/*************************************************************************
|*
|* Einzug-Wert in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetStart_Impl(const XFormTextStartItem* pItem)
{
    if ( pItem && !aMtrFldTextStart.HasFocus() )
    {
        SetMetricValue( aMtrFldTextStart, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
    }
}

/*************************************************************************
|*
|* Button fuer Umkehrung der Textrichtung setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetMirror_Impl(const XFormTextMirrorItem* pItem)
{
    if ( pItem )
        aTbxAdjust.CheckItem(TBI_ADJUST_MIRROR, pItem->GetValue());
}

/*************************************************************************
|*
|* Standardform im ValueSet anzeigen
|*
\************************************************************************/

void SvxFontWorkDialog::SetStdForm_Impl(const XFormTextStdFormItem* pItem)
{
    if ( pItem )
    {
        aFormSet.Enable();
        aFormSet.SetNoSelection();

        if ( pItem->GetValue() != XFTFORM_NONE )
            aFormSet.SelectItem(pItem->GetValue());
    }
    else
        aFormSet.Disable();
}

/*************************************************************************
|*
|* Button fuer Konturanzeige setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShowForm_Impl(const XFormTextHideFormItem* pItem)
{
    if ( pItem )
        aTbxShadow.CheckItem(TBI_SHOWFORM, !pItem->GetValue());
}

/*************************************************************************
|*
|* Button fuer Zeichenumrandung setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetOutline_Impl(const XFormTextOutlineItem* pItem)
{
    if ( pItem )
        aTbxShadow.CheckItem(TBI_OUTLINE, pItem->GetValue());
}

/*************************************************************************
|*
|* Shadow-Buttons setzen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadow_Impl(const XFormTextShadowItem* pItem,
                                        BOOL bRestoreValues)
{
    if ( pItem )
    {
        USHORT nId;

        aTbxShadow.Enable();

        if ( pItem->GetValue() == XFTSHADOW_NONE )
        {
            nId = TBI_SHADOW_OFF;
            aFbShadowX.Hide();
            aFbShadowY.Hide();
            aMtrFldShadowX.Disable();
            aMtrFldShadowY.Disable();
            aShadowColorLB.Disable();
        }
        else
        {
            ResMgr* pMgr = DIALOG_MGR();
            aFbShadowX.Show();
            aFbShadowY.Show();
            aMtrFldShadowX.Enable();
            aMtrFldShadowY.Enable();
            aShadowColorLB.Enable();

            if ( pItem->GetValue() == XFTSHADOW_NORMAL )
            {
                nId = TBI_SHADOW_NORMAL;
                FieldUnit eDlgUnit;
                GET_MODULE_FIELDUNIT( eDlgUnit );

                aFbShadowX.SetBitmap(
                    Bitmap( ResId(RID_SVXBMP_SHADOW_XDIST, pMgr ) ) );
                //aMtrFldShadowX.SetUnit(FUNIT_MM);
                aMtrFldShadowX.SetUnit( eDlgUnit );
                aMtrFldShadowX.SetDecimalDigits(2);
                aMtrFldShadowX.SetMin(LONG_MIN);
                aMtrFldShadowX.SetMax(LONG_MAX);
                if( eDlgUnit == FUNIT_MM )
                    aMtrFldShadowX.SetSpinSize( 50 );
                else
                    aMtrFldShadowX.SetSpinSize( 10 );

                aFbShadowY.SetBitmap(
                    Bitmap( ResId( RID_SVXBMP_SHADOW_YDIST, pMgr ) ) );
                //aMtrFldShadowY.SetUnit(FUNIT_MM);
                aMtrFldShadowY.SetUnit( eDlgUnit );
                aMtrFldShadowY.SetDecimalDigits(2);
                aMtrFldShadowY.SetMin(LONG_MIN);
                aMtrFldShadowY.SetMax(LONG_MAX);
                if( eDlgUnit == FUNIT_MM )
                    aMtrFldShadowY.SetSpinSize( 50 );
                else
                    aMtrFldShadowY.SetSpinSize( 10 );

                if ( bRestoreValues )
                {
                    SetMetricValue( aMtrFldShadowX, nSaveShadowX, SFX_MAPUNIT_100TH_MM );
                    SetMetricValue( aMtrFldShadowY, nSaveShadowY, SFX_MAPUNIT_100TH_MM );

                    XFormTextShadowXValItem aXItem( nSaveShadowX );
                    XFormTextShadowYValItem aYItem( nSaveShadowY );

                    GetBindings().GetDispatcher()->Execute(
                        SID_FORMTEXT_SHDWXVAL, SFX_CALLMODE_RECORD, &aXItem, &aYItem, 0L );
                }
            }
            else
            {
                nId = TBI_SHADOW_SLANT;

                aFbShadowX.SetBitmap(
                    Bitmap( ResId( RID_SVXBMP_SHADOW_ANGLE, pMgr ) ) );
                aMtrFldShadowX.SetUnit(FUNIT_CUSTOM);
                aMtrFldShadowX.SetDecimalDigits(1);
                aMtrFldShadowX.SetMin(-1800);
                aMtrFldShadowX.SetMax( 1800);
                aMtrFldShadowX.SetSpinSize(10);

                aFbShadowY.SetBitmap(
                    Bitmap( ResId( RID_SVXBMP_SHADOW_SIZE, pMgr ) ) );
                aMtrFldShadowY.SetUnit(FUNIT_CUSTOM);
                aMtrFldShadowY.SetDecimalDigits(0);
                aMtrFldShadowY.SetMin(-999);
                aMtrFldShadowY.SetMax( 999);
                aMtrFldShadowY.SetSpinSize(10);

                if ( bRestoreValues )
                {
                    aMtrFldShadowX.SetValue(nSaveShadowAngle);
                    aMtrFldShadowY.SetValue(nSaveShadowSize);
                    XFormTextShadowXValItem aXItem(nSaveShadowAngle);
                    XFormTextShadowYValItem aYItem(nSaveShadowSize);
                    GetBindings().GetDispatcher()->Execute(
                        SID_FORMTEXT_SHDWXVAL, SFX_CALLMODE_RECORD, &aXItem, &aYItem, 0L );
                }
            }
        }

        if ( !aTbxShadow.IsItemChecked(nId) )
        {
            aTbxShadow.CheckItem(nId);
        }
        nLastShadowTbxId = nId;
    }
    else
    {
        aTbxShadow.Disable();
        aMtrFldShadowX.Disable();
        aMtrFldShadowY.Disable();
        aShadowColorLB.Disable();
    }
}

/*************************************************************************
|*
|* Schattenfarbe in Listbox eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadowColor_Impl(const XFormTextShadowColorItem* pItem)
{
    if ( pItem )
        aShadowColorLB.SelectEntry(pItem->GetValue());
}

/*************************************************************************
|*
|* X-Wert fuer Schatten in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadowXVal_Impl(const XFormTextShadowXValItem* pItem)
{
    if ( pItem && !aMtrFldShadowX.HasFocus() )
    {
        INT32 nValue = pItem->GetValue();
        nValue = nValue - ( int( float( nValue ) / 360.0 ) * 360 );
        SetMetricValue( aMtrFldShadowX, nValue/*pItem->GetValue()*/, SFX_MAPUNIT_100TH_MM );
    }
}

/*************************************************************************
|*
|* Y-Wert fuer Schatten in Editfeld eintragen
|*
\************************************************************************/

void SvxFontWorkDialog::SetShadowYVal_Impl(const XFormTextShadowYValItem* pItem)
{
    if ( pItem && !aMtrFldShadowY.HasFocus() )
    {
        SetMetricValue( aMtrFldShadowY, pItem->GetValue(), SFX_MAPUNIT_100TH_MM );
    }
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, SelectStyleHdl_Impl, void *, EMPTYARG )
{
    USHORT nId = aTbxStyle.GetCurItemId();

    if ( nId != nLastStyleTbxId )
    {
        XFormTextStyle eStyle = XFT_NONE;

        switch ( nId )
        {
            case TBI_STYLE_ROTATE   : eStyle = XFT_ROTATE;  break;
            case TBI_STYLE_UPRIGHT  : eStyle = XFT_UPRIGHT; break;
            case TBI_STYLE_SLANTX   : eStyle = XFT_SLANTX;  break;
            case TBI_STYLE_SLANTY   : eStyle = XFT_SLANTY;  break;
        }
        XFormTextStyleItem aItem( eStyle );
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_STYLE, SFX_CALLMODE_RECORD, &aItem, 0L );
        SetStyle_Impl( &aItem );
        nLastStyleTbxId = nId;
    }
    return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, SelectAdjustHdl_Impl, void *, EMPTYARG )
{
    USHORT nId = aTbxAdjust.GetCurItemId();

    if ( nId == TBI_ADJUST_MIRROR )
    {
        XFormTextMirrorItem aItem(aTbxAdjust.IsItemChecked(nId));
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_MIRROR, SFX_CALLMODE_SLOT, &aItem, 0L );
    }
    else if ( nId != nLastAdjustTbxId )
    {
        XFormTextAdjust eAdjust = XFT_AUTOSIZE;

        switch ( nId )
        {
            case TBI_ADJUST_LEFT    : eAdjust = XFT_LEFT;   break;
            case TBI_ADJUST_CENTER  : eAdjust = XFT_CENTER; break;
            case TBI_ADJUST_RIGHT   : eAdjust = XFT_RIGHT;  break;
        }
        XFormTextAdjustItem aItem(eAdjust);
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_ADJUST, SFX_CALLMODE_RECORD, &aItem, 0L );
        SetAdjust_Impl(&aItem);
        nLastAdjustTbxId = nId;
    }
    return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, SelectShadowHdl_Impl, void *, EMPTYARG )
{
    USHORT nId = aTbxShadow.GetCurItemId();

    if ( nId == TBI_SHOWFORM )
    {
        XFormTextHideFormItem aItem(!aTbxShadow.IsItemChecked(nId));
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_HIDEFORM, SFX_CALLMODE_RECORD, &aItem, 0L );
    }
    else if ( nId == TBI_OUTLINE )
    {
        XFormTextOutlineItem aItem(aTbxShadow.IsItemChecked(nId));
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_OUTLINE, SFX_CALLMODE_RECORD, &aItem, 0L );
    }
    else if ( nId != nLastShadowTbxId )
    {
        XFormTextShadow eShadow = XFTSHADOW_NONE;

        if ( nLastShadowTbxId == TBI_SHADOW_NORMAL )
        {
            nSaveShadowX = GetCoreValue( aMtrFldShadowX, SFX_MAPUNIT_100TH_MM );
            nSaveShadowY = GetCoreValue( aMtrFldShadowY, SFX_MAPUNIT_100TH_MM );
        }
        else if ( nLastShadowTbxId == TBI_SHADOW_SLANT )
        {
            nSaveShadowAngle = aMtrFldShadowX.GetValue();
            nSaveShadowSize  = aMtrFldShadowY.GetValue();
        }
        nLastShadowTbxId = nId;

        if ( nId == TBI_SHADOW_NORMAL )     eShadow = XFTSHADOW_NORMAL;
        else if ( nId == TBI_SHADOW_SLANT ) eShadow = XFTSHADOW_SLANT;

        XFormTextShadowItem aItem(eShadow);
        GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_SHADOW, SFX_CALLMODE_RECORD, &aItem, 0L );
        SetShadow_Impl(&aItem, TRUE);
    }
    return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK_INLINE_START( SvxFontWorkDialog, ModifyInputHdl_Impl, void *, EMPTYARG )
{
    aInputTimer.Start();
    return 0;
}
IMPL_LINK_INLINE_END( SvxFontWorkDialog, ModifyInputHdl_Impl, void *, EMPTYARG )

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, InputTimoutHdl_Impl, void *, EMPTYARG )
{
    // System-Metrik evtl. neu setzen
    // Dieses sollte mal als Listener passieren, ist aber aus
    // inkompatibilitaetsgruenden z.Z. nicht moeglich
    FieldUnit eDlgUnit;
    GET_MODULE_FIELDUNIT( eDlgUnit );
    if( eDlgUnit != aMtrFldDistance.GetUnit() )
    {
        SetFieldUnit( aMtrFldDistance, eDlgUnit, TRUE );
        SetFieldUnit( aMtrFldTextStart, eDlgUnit, TRUE );
        aMtrFldDistance.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
        aMtrFldTextStart.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
    }
    if( eDlgUnit != aMtrFldShadowX.GetUnit() &&
        aTbxShadow.IsItemChecked( TBI_SHADOW_NORMAL ) )
    {
        SetFieldUnit( aMtrFldShadowX, eDlgUnit, TRUE );
        SetFieldUnit( aMtrFldShadowY, eDlgUnit, TRUE );
        aMtrFldShadowX.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
        aMtrFldShadowY.SetSpinSize( eDlgUnit == FUNIT_MM ? 50 : 10 );
    }

    long nValue = GetCoreValue( aMtrFldDistance, SFX_MAPUNIT_100TH_MM );
    XFormTextDistanceItem aDistItem( nValue );
    nValue = GetCoreValue( aMtrFldTextStart, SFX_MAPUNIT_100TH_MM );
    XFormTextStartItem aStartItem( nValue );

    nValue = GetCoreValue( aMtrFldShadowX, SFX_MAPUNIT_100TH_MM );
    XFormTextShadowXValItem aShadowXItem( nValue );
    nValue = GetCoreValue( aMtrFldShadowY, SFX_MAPUNIT_100TH_MM );
    XFormTextShadowYValItem aShadowYItem( nValue );

    // Slot-ID ist egal, die Exec-Methode wertet das gesamte ItemSet aus
    GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_DISTANCE, SFX_CALLMODE_RECORD, &aDistItem,
                                            &aStartItem, &aShadowXItem, &aShadowYItem, 0L );
    return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, FormSelectHdl_Impl, void *, EMPTYARG )
{
    XFormTextStdFormItem aItem;

    if ( aFormSet.IsNoSelection() )
        aItem.SetValue(XFTFORM_NONE);
    else
        aItem.SetValue(((XFormTextStdForm)(aFormSet.GetSelectItemId())));
    GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_STDFORM, SFX_CALLMODE_RECORD, &aItem, 0L );
    aFormSet.SetNoSelection();
    return 0;
}

/*************************************************************************
|*
\************************************************************************/

IMPL_LINK( SvxFontWorkDialog, ColorSelectHdl_Impl, void *, EMPTYARG )
{
// Changed by obo. Linux-Compiler can't parse commented lines
    XFormTextShadowColorItem aItem( (const String &) String(),
                                    (const Color &) aShadowColorLB.GetSelectEntryColor() );
//  XFormTextShadowColorItem aItem( String(),
//                                  aShadowColorLB.GetSelectEntryColor() );
    GetBindings().GetDispatcher()->Execute( SID_FORMTEXT_SHDWCOLOR, SFX_CALLMODE_RECORD, &aItem, 0L );
    return 0;
}

/*************************************************************************
|*
\************************************************************************/

void SvxFontWorkDialog::SetColorTable(const XColorTable* pTable)
{
    if ( pTable && pTable != pColorTable )
    {
        pColorTable = pTable;
        aShadowColorLB.Clear();
        aShadowColorLB.Fill(pColorTable);
    }
}

/*************************************************************************
|*
\************************************************************************/

void SvxFontWorkDialog::SetActive(BOOL bActivate)
{
}

/*************************************************************************
|*
|* Standard-FontWork-Objekt erzeugen
|*
\************************************************************************/

void SvxFontWorkDialog::CreateStdFormObj(SdrView& rView, SdrPageView& rPV,
                                         const SfxItemSet& rAttr,
                                         SdrObject& rOldObj,
                                         XFormTextStdForm eForm)
{
    SfxItemSet  aAttr(*rAttr.GetPool(), XATTR_FORMTXTSTYLE,
                                        XATTR_FORMTXTHIDEFORM);
    SdrObject*  pNewObj = NULL;
    Rectangle   aRect;
    XFormTextAdjust eAdjust = XFT_AUTOSIZE;

    rOldObj.TakeAttributes(aAttr, TRUE, FALSE);
    const XFormTextStdFormItem& rOldForm = (const XFormTextStdFormItem&)
                                            aAttr.Get(XATTR_FORMTXTSTDFORM);

    aRect = rOldObj.GetSnapRect();

    if ( !rOldObj.ISA(SdrPathObj) )
    {
        Point aPos = aRect.TopLeft();
        aRect = rOldObj.GetLogicRect();
        aRect.SetPos(aPos);
    }

    aAttr.Put(rAttr);

    if ( rOldForm.GetValue() == XFTFORM_NONE )
    {
        long nW = aRect.GetWidth();
        aRect.SetSize(Size(nW, nW));
        aAttr.Put(XFormTextStyleItem(XFT_ROTATE));
    }

    Point aCenter = aRect.Center();

    switch ( eForm )
    {
        case XFTFORM_TOPCIRC:
        case XFTFORM_BOTCIRC:
        case XFTFORM_LFTCIRC:
        case XFTFORM_RGTCIRC:
        case XFTFORM_TOPARC:
        case XFTFORM_BOTARC:
        case XFTFORM_LFTARC:
        case XFTFORM_RGTARC:
        {
            long nBeg, nEnd;

            switch ( eForm )
            {
                case XFTFORM_TOPCIRC:
                    nBeg = 0;
                    nEnd = 18000;
                    break;
                case XFTFORM_BOTCIRC:
                    nBeg = 18000;
                    nEnd = 36000;
                    break;
                case XFTFORM_LFTCIRC:
                    nBeg =  9000;
                    nEnd = 27000;
                    break;
                case XFTFORM_RGTCIRC:
                    nBeg = 27000;
                    nEnd =  9000;
                    break;
                case XFTFORM_TOPARC:
                    nBeg =  4500;
                    nEnd = 13500;
                    break;
                case XFTFORM_BOTARC:
                    nBeg = 22500;
                    nEnd = 31500;
                    break;
                case XFTFORM_LFTARC:
                    nBeg = 13500;
                    nEnd = 22500;
                    break;
                case XFTFORM_RGTARC:
                    nBeg = 31500;
                    nEnd =  4500;
                    break;
            }
            pNewObj = new SdrCircObj(OBJ_CARC, aRect, nBeg, nEnd);
            break;
        }
        case XFTFORM_BUTTON1:
        {
            XPolyPolygon aXPP;
            XPolygon aLine(3);
            long nR = aRect.GetWidth() / 2;

            XPolygon aTopArc(aCenter, -nR, nR, 50, 1750, FALSE);
            XPolygon aBottomArc(aCenter, -nR, nR, 1850, 3550, FALSE);

            // Polygone schliessen
            aTopArc[aTopArc.GetPointCount()] = aTopArc[0];
            aBottomArc[aBottomArc.GetPointCount()] = aBottomArc[0];
            aXPP.Insert(aTopArc);
            aLine[0] = aBottomArc[aBottomArc.GetPointCount()-2];
            aLine[2] = aLine[0];
            aLine[1] = aBottomArc[0];
            aXPP.Insert(aLine);
            aXPP.Insert(aBottomArc);

            pNewObj = new SdrPathObj(OBJ_PATHFILL, aXPP);
            eAdjust = XFT_CENTER;
            break;
        }
        case XFTFORM_BUTTON2:
        case XFTFORM_BUTTON3:
        case XFTFORM_BUTTON4:
        {
            XPolyPolygon aXPP;
            XPolygon aLine(3);
            long nR = aRect.GetWidth() / 2;
            long nWDiff = nR / 5;
            long nHDiff;

            if ( eForm == XFTFORM_BUTTON4 )
            {
                aXPP.Insert(XPolygon(aCenter, -nR, nR, 950, 2650, FALSE));
                // Polygon schliessen
                aXPP[0][aXPP[0].GetPointCount()] = aXPP[0][0];
                eAdjust = XFT_CENTER;
            }
            else
                aXPP.Insert(XPolygon(aCenter, -nR, nR, 2700, 2700));

            if ( eForm == XFTFORM_BUTTON3 )
                nHDiff = -aRect.GetHeight() / 10;
            else
                nHDiff = aRect.GetHeight() / 20;

            aLine[0] = aRect.LeftCenter();
            aLine[0].X() += nWDiff;
            aLine[0].Y() += nHDiff;
            aLine[2] = aLine[0];
            aLine[1] = aRect.RightCenter();
            aLine[1].X() -= nWDiff;
            aLine[1].Y() += nHDiff;
            aXPP.Insert(aLine);

            if ( eForm == XFTFORM_BUTTON4 )
            {
                aXPP.Insert(XPolygon(aCenter, -nR, nR, 2750, 850, FALSE));
                aXPP[2][aXPP[2].GetPointCount()] = aXPP[2][0];
            }

            if ( eForm == XFTFORM_BUTTON3 )
            {
                nHDiff += nHDiff;
                aLine[0].Y() -= nHDiff;
                aLine[1].Y() -= nHDiff;
                aLine[2].Y() -= nHDiff;
                aXPP.Insert(aLine);
            }

            pNewObj = new SdrPathObj(OBJ_PATHFILL, aXPP);
            break;
        }
    }
    if ( pNewObj )
    {
        Size aSize;
        Rectangle aSnap = pNewObj->GetSnapRect();

        aSize.Width() = aRect.Left() - aSnap.Left();
        aSize.Height() = aRect.Top() - aSnap.Top();
        pNewObj->NbcMove(aSize);

        rView.BegUndo( SVX_RESSTR( RID_SVXSTR_FONTWORK_UNDOCREATE ) );
        OutlinerParaObject* pPara = rOldObj.GetOutlinerParaObject();
        BOOL bHide = TRUE;

        if ( pPara != NULL )
        {
            pPara = new OutlinerParaObject(*pPara);
            pNewObj->SetOutlinerParaObject(pPara);
        }
        else
            bHide = FALSE;

        rView.ReplaceObject(&rOldObj, rPV, pNewObj, TRUE);
        pNewObj->SetLayer(rOldObj.GetLayer());
        aAttr.Put(XFormTextHideFormItem(bHide));
        aAttr.Put(XFormTextAdjustItem(eAdjust));

        XFormTextShadow eShadow = XFTSHADOW_NONE;

        if ( nLastShadowTbxId == TBI_SHADOW_NORMAL )
        {
            eShadow = XFTSHADOW_NORMAL;
            aAttr.Put(XFormTextShadowXValItem(nSaveShadowX));
            aAttr.Put(XFormTextShadowYValItem(nSaveShadowY));
        }
        else if ( nLastShadowTbxId == TBI_SHADOW_SLANT )
        {
            eShadow = XFTSHADOW_SLANT;
            aAttr.Put(XFormTextShadowXValItem(nSaveShadowAngle));
            aAttr.Put(XFormTextShadowYValItem(nSaveShadowSize));
        }

        aAttr.Put(XFormTextShadowItem(eShadow));

        rView.SetAttributes(aAttr);
        rView.EndUndo();
    }
}

