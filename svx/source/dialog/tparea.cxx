/*************************************************************************
 *
 *  $RCSfile: tparea.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 14:54:00 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#pragma hdrstop

#define _SVX_TPAREA_CXX

#if defined (MAC) || defined (UNX) || defined (WTC) || defined (ICC) || defined(WNT)
#include <stdlib.h>
#endif

#include "xpool.hxx"
#include "dialogs.hrc"
#include "tabarea.hrc"
#include "dlgname.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST

#include "xflbckit.hxx"

#include "svdattr.hxx"
#include "xoutx.hxx"
#include "xtable.hxx"
#include "xlineit0.hxx"

#include "drawitem.hxx"
#include "tabarea.hxx"
#include "dlgname.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#define DLGWIN this->GetParent()->GetParent()

// static ----------------------------------------------------------------

static USHORT pAreaRanges[] =
{
    XATTR_GRADIENTSTEPCOUNT,
    XATTR_GRADIENTSTEPCOUNT,
    SID_ATTR_FILL_STYLE,
    SID_ATTR_FILL_BITMAP,
    0
};

static USHORT pTransparenceRanges[] =
{
    XATTR_FILLTRANSPARENCE,
    XATTR_FILLTRANSPARENCE,
    SDRATTR_SHADOWTRANSPARENCE,
    SDRATTR_SHADOWTRANSPARENCE,
    XATTR_FILLFLOATTRANSPARENCE,
    XATTR_FILLFLOATTRANSPARENCE,
    0
};

/*************************************************************************
|*
|*  Dialog for transparence
|*
\************************************************************************/

IMPL_LINK(SvxTransparenceTabPage, ClickTransOffHdl_Impl, void *, EMPTYARG)
{
    // disable all other controls
    ActivateLinear(FALSE);
    ActivateGradient(FALSE);

    // Preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );

    InvalidatePreview( FALSE );

    return( 0L );
}

IMPL_LINK(SvxTransparenceTabPage, ClickTransLinearHdl_Impl, void *, EMPTYARG)
{
    // enable linear, disable other
    ActivateLinear(TRUE);
    ActivateGradient(FALSE);

    // preview
    rXFSet.ClearItem (XATTR_FILLFLOATTRANSPARENCE);
    ModifyTransparentHdl_Impl (NULL);

    return( 0L );
}

IMPL_LINK(SvxTransparenceTabPage, ClickTransGradientHdl_Impl, void *, EMPTYARG)
{
    // enable gradient, disable other
    ActivateLinear(FALSE);
    ActivateGradient(TRUE);

    // preview
    rXFSet.ClearItem (XATTR_FILLTRANSPARENCE);
    ModifiedTrgrHdl_Impl (NULL);

    return( 0L );
}

void SvxTransparenceTabPage::ActivateLinear(BOOL bActivate)
{
    aMtrTransparent.Enable(bActivate);
    aGrpTransparent.Enable(bActivate);
}

IMPL_LINK(SvxTransparenceTabPage, ModifyTransparentHdl_Impl, void*, EMPTYARG)
{
    UINT16 nPos = (UINT16)aMtrTransparent.GetValue();
    XFillTransparenceItem aItem(nPos);
    rXFSet.Put(XFillTransparenceItem(aItem));

    // preview
    InvalidatePreview();

    return 0L;
}

IMPL_LINK(SvxTransparenceTabPage, ModifiedTrgrHdl_Impl, void *, pControl)
{
    if(pControl == &aLbTrgrGradientType || pControl == this)
    {
        XGradientStyle eXGS = (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos();
        SetControlState_Impl( eXGS );
    }

    // preview
    UINT8 nStartCol = (UINT8)(((UINT16)aMtrTrgrStartValue.GetValue() * 255) / 100);
    UINT8 nEndCol = (UINT8)(((UINT16)aMtrTrgrEndValue.GetValue() * 255) / 100);
    XGradient aTmpGradient(
                Color(nStartCol, nStartCol, nStartCol),
                Color(nEndCol, nEndCol, nEndCol),
                (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos(),
                (UINT16)aMtrTrgrAngle.GetValue() * 10,
                (UINT16)aMtrTrgrCenterX.GetValue(),
                (UINT16)aMtrTrgrCenterY.GetValue(),
                (UINT16)aMtrTrgrBorder.GetValue(),
                100, 100);

    String aString;
    XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aTmpGradient);
    rXFSet.Put ( aItem );

    InvalidatePreview();

    return( 0L );
}

void SvxTransparenceTabPage::ActivateGradient(BOOL bActivate)
{
    aFtTrgrType.Enable(bActivate);
    aLbTrgrGradientType.Enable(bActivate);
    aFtTrgrCenterX.Enable(bActivate);
    aMtrTrgrCenterX.Enable(bActivate);
    aFtTrgrCenterY.Enable(bActivate);
    aMtrTrgrCenterY.Enable(bActivate);
    aFtTrgrAngle.Enable(bActivate);
    aMtrTrgrAngle.Enable(bActivate);
    aFtTrgrBorder.Enable(bActivate);
    aMtrTrgrBorder.Enable(bActivate);
    aFtTrgrStartValue.Enable(bActivate);
    aMtrTrgrStartValue.Enable(bActivate);
    aFtTrgrEndValue.Enable(bActivate);
    aMtrTrgrEndValue.Enable(bActivate);
    aGrpTransGradient.Enable(bActivate);

    if(bActivate)
    {
        XGradientStyle eXGS = (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos();
        SetControlState_Impl( eXGS );
    }
}

IMPL_LINK(SvxTransparenceTabPage, ChangeTrgrTypeHdl_Impl, void *, EMPTYARG)
{
    return( 0L );
}

void SvxTransparenceTabPage::SetControlState_Impl(XGradientStyle eXGS)
{
    switch(eXGS)
    {
        case XGRAD_LINEAR:
        case XGRAD_AXIAL:
            aFtTrgrCenterX.Disable();
            aMtrTrgrCenterX.Disable();
            aFtTrgrCenterY.Disable();
            aMtrTrgrCenterY.Disable();
            aFtTrgrAngle.Enable();
            aMtrTrgrAngle.Enable();
            break;

        case XGRAD_RADIAL:
            aFtTrgrCenterX.Enable();
            aMtrTrgrCenterX.Enable();
            aFtTrgrCenterY.Enable();
            aMtrTrgrCenterY.Enable();
            aFtTrgrAngle.Disable();
            aMtrTrgrAngle.Disable();
            break;

        case XGRAD_ELLIPTICAL:
            aFtTrgrCenterX.Enable();
            aMtrTrgrCenterX.Enable();
            aFtTrgrCenterY.Enable();
            aMtrTrgrCenterY.Enable();
            aFtTrgrAngle.Enable();
            aMtrTrgrAngle.Enable();
            break;

        case XGRAD_SQUARE:
        case XGRAD_RECT:
            aFtTrgrCenterX.Enable();
            aMtrTrgrCenterX.Enable();
            aFtTrgrCenterY.Enable();
            aMtrTrgrCenterY.Enable();
            aFtTrgrAngle.Enable();
            aMtrTrgrAngle.Enable();
            break;
    }
}

SvxTransparenceTabPage::SvxTransparenceTabPage(Window* pParent, const SfxItemSet& rInAttrs)
:   SvxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_TRANSPARENCE ), rInAttrs),
    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFillAttr          ( pXPool ),
    rOutAttrs           ( rInAttrs ),
    rXFSet              ( aXFillAttr.GetItemSet() ),

    aRbtTransOff        ( this, ResId( RBT_TRANS_OFF ) ),
    aRbtTransLinear     ( this, ResId( RBT_TRANS_LINEAR ) ),
    aRbtTransGradient   ( this, ResId( RBT_TRANS_GRADIENT ) ),
    aGrpTransMode       ( this, ResId( GRP_TRANS_MODE ) ),

    aMtrTransparent     ( this, ResId( MTR_TRANSPARENT ) ),
    aGrpTransparent     ( this, ResId( GRP_TRANSPARENT ) ),

    aFtTrgrType         ( this, ResId( FT_TRGR_TYPE ) ),
    aLbTrgrGradientType ( this, ResId( LB_TRGR_GRADIENT_TYPES ) ),
    aFtTrgrCenterX      ( this, ResId( FT_TRGR_CENTER_X ) ),
    aMtrTrgrCenterX     ( this, ResId( MTR_TRGR_CENTER_X ) ),
    aFtTrgrCenterY      ( this, ResId( FT_TRGR_CENTER_Y ) ),
    aMtrTrgrCenterY     ( this, ResId( MTR_TRGR_CENTER_Y ) ),
    aFtTrgrAngle        ( this, ResId( FT_TRGR_ANGLE ) ),
    aMtrTrgrAngle       ( this, ResId( MTR_TRGR_ANGLE ) ),
    aFtTrgrBorder       ( this, ResId( FT_TRGR_BORDER ) ),
    aMtrTrgrBorder      ( this, ResId( MTR_TRGR_BORDER ) ),
    aFtTrgrStartValue   ( this, ResId( FT_TRGR_START_VALUE ) ),
    aMtrTrgrStartValue  ( this, ResId( MTR_TRGR_START_VALUE ) ),
    aFtTrgrEndValue     ( this, ResId( FT_TRGR_END_VALUE ) ),
    aMtrTrgrEndValue    ( this, ResId( MTR_TRGR_END_VALUE ) ),
    aGrpTransGradient   ( this, ResId( GRP_TRANS_GRADIENT ) ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    XOut                ( &aCtlXRectPreview ),
    XOutBmp             ( &aCtlBitmapPreview ),
    aCtlBitmapPreview   ( this, ResId( CTL_BITMAP_PREVIEW ), &XOutBmp ),
    aCtlXRectPreview    ( this, ResId( CTL_TRANS_PREVIEW ), &XOut ),
    bBitmap             ( FALSE )
{
    FreeResource();

    // main selection
    aRbtTransOff.SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransOffHdl_Impl));
    aRbtTransLinear.SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransLinearHdl_Impl));
    aRbtTransGradient.SetClickHdl(LINK(this, SvxTransparenceTabPage, ClickTransGradientHdl_Impl));

    // linear transparency
    aMtrTransparent.SetValue( 50 );
    aMtrTransparent.SetModifyHdl(LINK(this, SvxTransparenceTabPage, ModifyTransparentHdl_Impl));

    // gradient transparency
    aMtrTrgrEndValue.SetValue( 100 );
    aMtrTrgrStartValue.SetValue( 0 );
    aLbTrgrGradientType.SetSelectHdl(LINK(this, SvxTransparenceTabPage, ChangeTrgrTypeHdl_Impl));
    Link aLink = LINK( this, SvxTransparenceTabPage, ModifiedTrgrHdl_Impl);
    aLbTrgrGradientType.SetSelectHdl( aLink );
    aMtrTrgrCenterX.SetModifyHdl( aLink );
    aMtrTrgrCenterY.SetModifyHdl( aLink );
    aMtrTrgrAngle.SetModifyHdl( aLink );
    aMtrTrgrBorder.SetModifyHdl( aLink );
    aMtrTrgrStartValue.SetModifyHdl( aLink );
    aMtrTrgrEndValue.SetModifyHdl( aLink );

    // this page needs ExchangeSupport
    SetExchangeSupport();
}

void SvxTransparenceTabPage::Construct()
{
}

SfxTabPage* SvxTransparenceTabPage::Create(Window* pWindow, const SfxItemSet& rAttrs)
{
    return(new SvxTransparenceTabPage(pWindow, rAttrs));
}

UINT16* SvxTransparenceTabPage::GetRanges()
{
    return(pTransparenceRanges);
}

BOOL SvxTransparenceTabPage::FillItemSet(SfxItemSet& rAttrs)
{
    const SfxPoolItem* pGradientItem = NULL;
    const SfxPoolItem* pLinearItem = NULL;
    SfxItemState eStateGradient(rOutAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, TRUE, &pGradientItem));
    SfxItemState eStateLinear(rOutAttrs.GetItemState(XATTR_FILLTRANSPARENCE, TRUE, &pLinearItem));
    BOOL bGradActive = (eStateGradient == SFX_ITEM_SET && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled());
    BOOL bLinearActive = (eStateLinear == SFX_ITEM_SET && ((XFillTransparenceItem*)pLinearItem)->GetValue() != 0);

    BOOL bModified(FALSE);
    BOOL bSwitchOffLinear(FALSE);
    BOOL bSwitchOffGradient(FALSE);

    if(aMtrTransparent.IsEnabled())
    {
        // linear transparence
        UINT16 nPos = (UINT16)aMtrTransparent.GetValue();
        if(nPos != (UINT16)aMtrTransparent.GetSavedValue().ToInt32() || !bLinearActive)
        {
            XFillTransparenceItem aItem(nPos);
            SdrShadowTransparenceItem aShadowItem(nPos);
            const SfxPoolItem* pOld = GetOldItem(rAttrs, XATTR_FILLTRANSPARENCE);
            if(!pOld || !(*(const XFillTransparenceItem*)pOld == aItem) || !bLinearActive)
            {
                rAttrs.Put(aItem);
                rAttrs.Put(aShadowItem);
                bModified = TRUE;
                bSwitchOffGradient = TRUE;
            }
        }
    }
    else if(aLbTrgrGradientType.IsEnabled())
    {
        // transparence gradient, fill ItemSet from values
        if(!bGradActive
            || (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos() != (XGradientStyle)aLbTrgrGradientType.GetSavedValue()
            || (UINT16)aMtrTrgrAngle.GetValue() != (UINT16)aMtrTrgrAngle.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrCenterX.GetValue() != (UINT16)aMtrTrgrCenterX.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrCenterY.GetValue() != (UINT16)aMtrTrgrCenterY.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrBorder.GetValue() != (UINT16)aMtrTrgrBorder.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrStartValue.GetValue() != (UINT16)aMtrTrgrStartValue.GetSavedValue().ToInt32()
            || (UINT16)aMtrTrgrEndValue.GetValue() != (UINT16)aMtrTrgrEndValue.GetSavedValue().ToInt32() )
        {
            UINT8 nStartCol = (UINT8)(((UINT16)aMtrTrgrStartValue.GetValue() * 255) / 100);
            UINT8 nEndCol = (UINT8)(((UINT16)aMtrTrgrEndValue.GetValue() * 255) / 100);
            XGradient aTmpGradient(
                        Color(nStartCol, nStartCol, nStartCol),
                        Color(nEndCol, nEndCol, nEndCol),
                        (XGradientStyle)aLbTrgrGradientType.GetSelectEntryPos(),
                        (UINT16)aMtrTrgrAngle.GetValue() * 10,
                        (UINT16)aMtrTrgrCenterX.GetValue(),
                        (UINT16)aMtrTrgrCenterY.GetValue(),
                        (UINT16)aMtrTrgrBorder.GetValue(),
                        100, 100);

            String aString;
            XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aTmpGradient);
            const SfxPoolItem* pOld = GetOldItem(rAttrs, XATTR_FILLFLOATTRANSPARENCE);

            if(!pOld || !(*(const XFillFloatTransparenceItem*)pOld == aItem) || !bGradActive)
            {
                rAttrs.Put(aItem);
                bModified = TRUE;
                bSwitchOffLinear = TRUE;
            }
        }
    }
    else
    {
        // no transparence
        bSwitchOffGradient = TRUE;
        bSwitchOffLinear = TRUE;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffGradient && bGradActive)
    {
        Color aColor(COL_BLACK);
        XGradient aGrad(aColor, Color(COL_WHITE));
        aGrad.SetStartIntens(100);
        aGrad.SetEndIntens(100);
        String aString;
        XFillFloatTransparenceItem aItem( rXFSet.GetPool()/*aString*/, aGrad);
        aItem.SetEnabled(FALSE);
        rAttrs.Put(aItem);
        bModified = TRUE;
    }

    // disable unused XFillFloatTransparenceItem
    if(bSwitchOffLinear && bLinearActive)
    {
        XFillTransparenceItem aItem(0);
        SdrShadowTransparenceItem aShadowItem(0);
        rAttrs.Put(aItem);
        rAttrs.Put(aShadowItem);
        bModified = TRUE;
    }

    return bModified;
}

void SvxTransparenceTabPage::Reset(const SfxItemSet& rAttrs)
{
    const SfxPoolItem* pGradientItem = NULL;
    SfxItemState eStateGradient(rAttrs.GetItemState(XATTR_FILLFLOATTRANSPARENCE, TRUE, &pGradientItem));
    if(!pGradientItem)
        pGradientItem = &rAttrs.Get(XATTR_FILLFLOATTRANSPARENCE);
    BOOL bGradActive = (eStateGradient == SFX_ITEM_SET && ((XFillFloatTransparenceItem*)pGradientItem)->IsEnabled());

    const SfxPoolItem* pLinearItem = NULL;
    SfxItemState eStateLinear(rAttrs.GetItemState(XATTR_FILLTRANSPARENCE, TRUE, &pLinearItem));
    if(!pLinearItem)
        pLinearItem = &rAttrs.Get(XATTR_FILLTRANSPARENCE);
    BOOL bLinearActive = (eStateLinear == SFX_ITEM_SET && ((XFillTransparenceItem*)pLinearItem)->GetValue() != 0);

    // transparence gradient
    const XGradient& rGradient = ((XFillFloatTransparenceItem*)pGradientItem)->GetValue();
    XGradientStyle eXGS(rGradient.GetGradientStyle());
    aLbTrgrGradientType.SelectEntryPos(eXGS);
    aMtrTrgrAngle.SetValue(rGradient.GetAngle() / 10);
    aMtrTrgrBorder.SetValue(rGradient.GetBorder());
    aMtrTrgrCenterX.SetValue(rGradient.GetXOffset());
    aMtrTrgrCenterY.SetValue(rGradient.GetYOffset());
    aMtrTrgrStartValue.SetValue((UINT16)((((UINT16)rGradient.GetStartColor().GetRed() + 1) * 100) / 255));
    aMtrTrgrEndValue.SetValue((UINT16)((((UINT16)rGradient.GetEndColor().GetRed() + 1) * 100) / 255));

    // linear transparence
    UINT16 nTransp = ((XFillTransparenceItem*)pLinearItem)->GetValue();
    aMtrTransparent.SetValue(bLinearActive ? nTransp : 50);
    ModifyTransparentHdl_Impl(NULL);

    // select the correct radio button
    if(bGradActive)
    {
        // transparence gradient, set controls appropriate to item
        aRbtTransGradient.Check();
        ClickTransGradientHdl_Impl(NULL);
    }
    else if(bLinearActive)
    {
        // linear transparence
        aRbtTransLinear.Check();
        ClickTransLinearHdl_Impl(NULL);
    }
    else
    {
        // no transparence
        aRbtTransOff.Check();
        ClickTransOffHdl_Impl(NULL);
        ModifiedTrgrHdl_Impl(NULL);
    }

    // save values
    aMtrTransparent.SaveValue();
    aLbTrgrGradientType.SaveValue();
    aMtrTrgrCenterX.SaveValue();
    aMtrTrgrCenterY.SaveValue();
    aMtrTrgrAngle.SaveValue();
    aMtrTrgrBorder.SaveValue();
    aMtrTrgrStartValue.SaveValue();
    aMtrTrgrEndValue.SaveValue();

    BOOL bActive = InitPreview ( rAttrs );
    InvalidatePreview ( bActive );
}

void SvxTransparenceTabPage::ActivatePage(const SfxItemSet& rSet)
{
    if(*pDlgType == 0) // Flaechen-Dialog
        *pPageType = PT_TRANSPARENCE;

    InitPreview ( rSet );
}

int SvxTransparenceTabPage::DeactivatePage(SfxItemSet* pSet)
{
    FillItemSet(*pSet);
    return(LEAVE_PAGE);
}

void SvxTransparenceTabPage::PointChanged(Window* pWindow, RECT_POINT eRcPt)
{
    eRP = eRcPt;
}

//
// Preview-Methods
//
BOOL SvxTransparenceTabPage::InitPreview ( const SfxItemSet& rSet )
{
    // set transparencetyp for preview
    if ( aRbtTransOff.IsChecked() )
    {
        ClickTransOffHdl_Impl(NULL);
    } else if ( aRbtTransLinear.IsChecked() )
    {
        ClickTransLinearHdl_Impl(NULL);
    } else if ( aRbtTransGradient.IsChecked() )
    {
        ClickTransGradientHdl_Impl(NULL);
    }

    // Get fillstyle for preview
    rXFSet.Put ( ( XFillStyleItem& )    rSet.Get(XATTR_FILLSTYLE) );
    rXFSet.Put ( ( XFillColorItem& )    rSet.Get(XATTR_FILLCOLOR) );
    rXFSet.Put ( ( XFillGradientItem& ) rSet.Get(XATTR_FILLGRADIENT) );
    rXFSet.Put ( ( XFillHatchItem& )    rSet.Get(XATTR_FILLHATCH) );
    rXFSet.Put ( ( XFillBackgroundItem&)rSet.Get(XATTR_FILLBACKGROUND) );
    rXFSet.Put ( ( XFillBitmapItem& )   rSet.Get(XATTR_FILLBITMAP) );

    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    XOutBmp.SetFillAttr( aXFillAttr.GetItemSet() );

    bBitmap = ( ( ( XFillStyleItem& )rSet.Get(XATTR_FILLSTYLE) ).GetValue() == XFILL_BITMAP );

    // show the right preview window
    if ( bBitmap )
    {
        aCtlBitmapPreview.Show();
        aCtlXRectPreview.Hide();
    }
    else
    {
        aCtlBitmapPreview.Hide();
        aCtlXRectPreview.Show();
    }

    return !aRbtTransOff.IsChecked();
}

void SvxTransparenceTabPage::InvalidatePreview (BOOL bEnable)
{
    if ( bBitmap )
    {
        if ( bEnable )
        {
            aCtlBitmapPreview.Enable();
            XOutBmp.SetFillAttr( aXFillAttr.GetItemSet() );
        }
        else
            aCtlBitmapPreview.Disable();
        aCtlBitmapPreview.Invalidate();
    }
    else
    {
        if ( bEnable )
        {
            aCtlXRectPreview.Enable();
            XOut.SetFillAttr( aXFillAttr.GetItemSet() );
        }
        else
            aCtlXRectPreview.Disable();
        aCtlXRectPreview.Invalidate();
    }
}


/*************************************************************************
|*
|*  Dialog to modify fill-attributes
|*
\************************************************************************/

SvxAreaTabPage::SvxAreaTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_AREA ), rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlXRectPreview ),
    XOutBmp             ( &aCtlBitmapPreview ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() ),

    aRbtInvisible       ( this, ResId( RBT_INVISIBLE ) ),
    aRbtColor           ( this, ResId( RBT_COLOR ) ),
    aRbtGradient        ( this, ResId( RBT_GRADIENT ) ),
    aRbtHatching        ( this, ResId( RBT_HATCHING ) ),
    aRbtBitmap          ( this, ResId( RBT_BITMAP ) ),

    aLbColor            ( this, ResId( LB_COLOR ) ),
    aLbGradient         ( this, ResId( LB_GRADIENT ) ),
    aLbHatching         ( this, ResId( LB_HATCHING ) ),
    aLbBitmap           ( this, ResId( LB_BITMAP ) ),
    aCtlBitmapPreview   ( this, ResId( CTL_BITMAP_PREVIEW ), &XOutBmp ),
    aGrpFill            ( this, ResId( GRP_FILL ) ),

    aGrpHatchBckgrd     ( this, ResId( GRP_HATCHBCKGRD ) ),
    aCbxHatchBckgrd     ( this, ResId( CB_HATCHBCKGRD ) ),
    aLbHatchBckgrdColor ( this, ResId( LB_HATCHBCKGRDCOLOR ) ),

    aTsbStepCount       ( this, ResId( TSB_STEPCOUNT ) ),
    aNumFldStepCount    ( this, ResId( NUM_FLD_STEPCOUNT ) ),
    aGrpStepCount       ( this, ResId( GRP_STEPCOUNT ) ),

    aTsbTile            ( this, ResId( TSB_TILE ) ),
    aTsbStretch         ( this, ResId( TSB_STRETCH ) ),
    aTsbScale           ( this, ResId( TSB_SCALE ) ),
    aTsbOriginal        ( this, ResId( TSB_ORIGINAL ) ),
    aFtXSize            ( this, ResId( FT_X_SIZE ) ),
    aMtrFldXSize        ( this, ResId( MTR_FLD_X_SIZE ) ),
    aFtYSize            ( this, ResId( FT_Y_SIZE ) ),
    aMtrFldYSize        ( this, ResId( MTR_FLD_Y_SIZE ) ),
    aGrpSize            ( this, ResId( GRP_SIZE ) ),
    aRbtRow             ( this, ResId( RBT_ROW ) ),
    aRbtColumn          ( this, ResId( RBT_COLUMN ) ),
    aMtrFldOffset       ( this, ResId( MTR_FLD_OFFSET ) ),
    aGrpOffset          ( this, ResId( GRP_OFFSET ) ),
    aCtlPosition        ( this, ResId( CTL_POSITION ),
                                    RP_RM, 110, 80, CS_RECT ),
    aFtXOffset          ( this, ResId( FT_X_OFFSET ) ),
    aMtrFldXOffset      ( this, ResId( MTR_FLD_X_OFFSET ) ),
    aFtYOffset          ( this, ResId( FT_Y_OFFSET ) ),
    aMtrFldYOffset      ( this, ResId( MTR_FLD_Y_OFFSET ) ),
    aGrpPosition        ( this, ResId( GRP_POSITION ) ),

    aCtlXRectPreview    ( this, ResId( CTL_COLOR_PREVIEW ), &XOut ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    rOutAttrs           ( rInAttrs )

{
    FreeResource();

    // Gruppen, die sich ueberlagern
    aLbBitmap.Hide();
    aCtlBitmapPreview.Hide();

    aGrpStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aGrpSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aGrpOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aGrpPosition.Hide();
    // Controls for Hatch-Background
    aGrpHatchBckgrd.Hide();
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();


    aTsbOriginal.EnableTriState( FALSE );


    // this page needs ExchangeSupport
    SetExchangeSupport();

    // set Metrics
    eFUnit = GetModuleFieldUnit( &rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
    }
    SetFieldUnit( aMtrFldXSize, eFUnit, TRUE );
    SetFieldUnit( aMtrFldYSize, eFUnit, TRUE );

    // get PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( XATTR_FILLBMP_SIZEX );

    // Setzen Output-Devices
    rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
    rXFSet.Put( XFillColorItem( String(), COL_BLACK ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    XOutBmp.SetFillAttr( aXFillAttr.GetItemSet() );

    // Set line to None at the OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    XOut.SetLineAttr( aXLineAttr.GetItemSet() );
    XOutBmp.SetLineAttr( aXLineAttr.GetItemSet() );

    aLbColor.SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyColorHdl_Impl ) );
    aLbHatchBckgrdColor.SetSelectHdl( LINK( this, SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl ) );
    aCbxHatchBckgrd.SetToggleHdl( LINK( this, SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl ) );

    aLbGradient.SetSelectHdl(
        LINK( this, SvxAreaTabPage, ModifyGradientHdl_Impl ) );
    aLbHatching.SetSelectHdl(
        LINK( this, SvxAreaTabPage, ModifyHatchingHdl_Impl ) );
    aLbBitmap.SetSelectHdl(
        LINK( this, SvxAreaTabPage, ModifyBitmapHdl_Impl ) );

    aTsbStepCount.SetClickHdl(
        LINK( this, SvxAreaTabPage, ModifyStepCountHdl_Impl ) );
    aNumFldStepCount.SetModifyHdl(
        LINK( this, SvxAreaTabPage, ModifyStepCountHdl_Impl ) );

    Link aLink( LINK( this, SvxAreaTabPage, ModifyTileHdl_Impl ) );
    aTsbTile.SetClickHdl( aLink );
    aTsbStretch.SetClickHdl( aLink );
    aTsbOriginal.SetClickHdl( aLink );
    aMtrFldXSize.SetModifyHdl( aLink );
    aMtrFldYSize.SetModifyHdl( aLink );
    aRbtRow.SetClickHdl( aLink );
    aRbtColumn.SetClickHdl( aLink );
    aMtrFldOffset.SetModifyHdl( aLink );
    aMtrFldXOffset.SetModifyHdl( aLink );
    aMtrFldYOffset.SetModifyHdl( aLink );
    aTsbScale.SetClickHdl( LINK( this, SvxAreaTabPage, ClickScaleHdl_Impl ) );

    aRbtInvisible.SetClickHdl(
        LINK( this, SvxAreaTabPage, ClickInvisibleHdl_Impl ) );
    aRbtColor.SetClickHdl( LINK( this, SvxAreaTabPage, ClickColorHdl_Impl ) );
    aRbtGradient.SetClickHdl(
        LINK( this, SvxAreaTabPage, ClickGradientHdl_Impl ) );
    aRbtHatching.SetClickHdl(
        LINK( this, SvxAreaTabPage, ClickHatchingHdl_Impl ) );
    aRbtBitmap.SetClickHdl(
        LINK( this, SvxAreaTabPage, ClickBitmapHdl_Impl ) );

    pColorTab = NULL;
    pGradientList = NULL;
    pHatchingList = NULL;
    pBitmapList = NULL;
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::Construct()
{
    // fill colortables / lists
    aLbColor.Fill( pColorTab );
    aLbHatchBckgrdColor.Fill ( pColorTab );

    aLbGradient.Fill( pGradientList );
    aLbHatching.Fill( pHatchingList );
    aLbBitmap.Fill( pBitmapList );
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::ActivatePage( const SfxItemSet& rSet )
{
    int nPos;
    int nCount;

    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        *pbAreaTP = TRUE;

        if( pColorTab )
        {
            // Bitmapliste
            if( *pnBitmapListState )
            {
                if( *pnBitmapListState & CT_CHANGED )
                    pBitmapList = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewBitmapList();

                nPos = aLbBitmap.GetSelectEntryPos();

                aLbBitmap.Clear();
                aLbBitmap.Fill( pBitmapList );
                nCount = aLbBitmap.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nPos )
                    aLbBitmap.SelectEntryPos( 0 );
                else
                    aLbBitmap.SelectEntryPos( nPos );
                ModifyBitmapHdl_Impl( this );
            }
            // hatch-liste
            if( *pnHatchingListState )
            {
                if( *pnHatchingListState & CT_CHANGED )
                    pHatchingList = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewHatchingList();

                nPos = aLbHatching.GetSelectEntryPos();

                aLbHatching.Clear();
                aLbHatching.Fill( pHatchingList );
                nCount = aLbHatching.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nPos )
                    aLbHatching.SelectEntryPos( 0 );
                else
                    aLbHatching.SelectEntryPos( nPos );
                ModifyHatchingHdl_Impl( this );

                ModifyHatchBckgrdColorHdl_Impl( this );
            }
            // gradient-liste
            if( *pnGradientListState )
            {
                if( *pnGradientListState & CT_CHANGED )
                    pGradientList = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewGradientList();

                nPos = aLbGradient.GetSelectEntryPos();

                aLbGradient.Clear();
                aLbGradient.Fill( pGradientList );
                nCount = aLbGradient.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nPos )
                    aLbGradient.SelectEntryPos( 0 );
                else
                    aLbGradient.SelectEntryPos( nPos );
                ModifyGradientHdl_Impl( this );
            }
            // ColorTable
            if( *pnColorTableState )
            {
                if( *pnColorTableState & CT_CHANGED )
                    pColorTab = ( (SvxAreaTabDialog*) DLGWIN )->
                                            GetNewColorTable();
                // aLbColor
                nPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorTab );
                nCount = aLbColor.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nPos )
                    aLbColor.SelectEntryPos( 0 );
                else
                    aLbColor.SelectEntryPos( nPos );

                ModifyColorHdl_Impl( this );

                // Backgroundcolor of hatch
                nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
                aLbHatchBckgrdColor.Clear();
                aLbHatchBckgrdColor.Fill( pColorTab );
                nCount = aLbHatchBckgrdColor.GetEntryCount();
                if( nCount == 0 )
                    ; // This case should never occur
                else if( nCount <= nPos )
                    aLbHatchBckgrdColor.SelectEntryPos( 0 );
                else
                    aLbHatchBckgrdColor.SelectEntryPos( nPos );

                ModifyHatchBckgrdColorHdl_Impl( this );
            }

            // evaluate if any other Tabpage set another filltype
            if( !aRbtInvisible.IsChecked() )
            {
                switch( *pPageType )
                {
                    case PT_GRADIENT:
                        aRbtGradient.Check();
                        aLbGradient.SelectEntryPos( *pPos );
                        ClickGradientHdl_Impl( this );
                    break;

                    case PT_HATCH:
                        aRbtHatching.Check();
                        aLbHatching.SelectEntryPos( *pPos );
                        ClickHatchingHdl_Impl( this );
                    break;

                    case PT_BITMAP:
                        aRbtBitmap.Check();
                        aLbBitmap.SelectEntryPos( *pPos );
                        ClickBitmapHdl_Impl( this );
                    break;

                    case PT_COLOR:
                        aRbtColor.Check();
                        aLbColor.SelectEntryPos( *pPos );
                        aLbHatchBckgrdColor.SelectEntryPos( *pPos );
                        ClickColorHdl_Impl( this );
                    break;
                }
            }
            *pPageType = PT_AREA;
        }
    }
}

// -----------------------------------------------------------------------

int SvxAreaTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        if( aRbtGradient.IsChecked() )
        {
            *pPageType = PT_GRADIENT;
            *pPos = aLbGradient.GetSelectEntryPos();
        }
        if( aRbtHatching.IsChecked() )
        {
            *pPageType = PT_HATCH;
            *pPos = aLbHatching.GetSelectEntryPos();
        }
        if( aRbtBitmap.IsChecked() )
        {
            *pPageType = PT_BITMAP;
            *pPos = aLbBitmap.GetSelectEntryPos();
        }
        if( aRbtColor.IsChecked() )
        {
            *pPageType = PT_COLOR;
            *pPos = aLbColor.GetSelectEntryPos();
        }
    }

    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

BOOL SvxAreaTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    const SfxPoolItem* pOld = NULL;
    USHORT  nPos;
    BOOL    bModified = FALSE;

    if( *pDlgType != 0 || *pbAreaTP )
    {
        if( aRbtInvisible.IsChecked() )
        {
            if( !aRbtInvisible.GetSavedValue() )
            {
                XFillStyleItem aStyleItem( XFILL_NONE );
                pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                {
                    rAttrs.Put( aStyleItem );
                    bModified = TRUE;
                }
            }
        }
        else if( aRbtColor.IsChecked() )
        {
            nPos = aLbColor.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                nPos != aLbColor.GetSavedValue() )
            {
                XFillColorItem aItem( aLbColor.GetSelectEntry(),
                                        aLbColor.GetSelectEntryColor() );
                pOld = GetOldItem( rAttrs, XATTR_FILLCOLOR );
                if ( !pOld || !( *(const XFillColorItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
            // NEU
            if( !aRbtColor.GetSavedValue() &&
                ( bModified ||
                  SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE ) ) )
            {
                XFillStyleItem aStyleItem( XFILL_SOLID );
                pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                {
                    rAttrs.Put( aStyleItem );
                    bModified = TRUE;
                }
            }
        }
        else if( aRbtGradient.IsChecked() )
        {
            nPos = aLbGradient.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                nPos != aLbGradient.GetSavedValue() )
            {
                XGradient aGradient = pGradientList->Get( nPos )->GetGradient();
                String aString = aLbGradient.GetSelectEntry();
                XFillGradientItem aItem( aString, aGradient );
                pOld = GetOldItem( rAttrs, XATTR_FILLGRADIENT );
                if ( !pOld || !( *(const XFillGradientItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
            // NEU
            if( !aRbtGradient.GetSavedValue() &&
                ( bModified ||
                  SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), TRUE ) ) )
            {
                XFillStyleItem aStyleItem( XFILL_GRADIENT );
                pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                {
                    rAttrs.Put( aStyleItem );
                    bModified = TRUE;
                }
            }
        }
        else if( aRbtHatching.IsChecked() )
        {
            nPos = aLbHatching.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                nPos != aLbHatching.GetSavedValue() )
            {
                XHatch aHatching = pHatchingList->Get( nPos )->GetHatch();
                String aString = aLbHatching.GetSelectEntry();
                XFillHatchItem aItem( aString, aHatching );
                pOld = GetOldItem( rAttrs, XATTR_FILLHATCH );
                if ( !pOld || !( *(const XFillHatchItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }

            XFillBackgroundItem aItem ( aCbxHatchBckgrd.IsChecked() );
            rAttrs.Put( aItem );

            nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                nPos != aLbHatchBckgrdColor.GetSavedValue() )
            {
                XFillColorItem aItem( aLbHatchBckgrdColor.GetSelectEntry(),
                                      aLbHatchBckgrdColor.GetSelectEntryColor() );
                pOld = GetOldItem( rAttrs, XATTR_FILLCOLOR );
                if ( !pOld || !( *(const XFillColorItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
            // NEU
            if( !aRbtHatching.GetSavedValue() &&
                ( bModified ||
                  SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), TRUE ) ) )
            {
                XFillStyleItem aStyleItem( XFILL_HATCH );
                pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                {
                    rAttrs.Put( aStyleItem );
                    bModified = TRUE;
                }
            }
        }
        else if( aRbtBitmap.IsChecked() )
        {
            nPos = aLbBitmap.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND &&
                nPos != aLbBitmap.GetSavedValue() )
            {
                XOBitmap aXOBitmap = pBitmapList->Get( nPos )->GetXBitmap();
                String aString = aLbBitmap.GetSelectEntry();
                XFillBitmapItem aItem( aString, aXOBitmap );
                pOld = GetOldItem( rAttrs, XATTR_FILLBITMAP );
                if ( !pOld || !( *(const XFillBitmapItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
            // NEU
            if( !aRbtBitmap.GetSavedValue() &&
                ( bModified ||
                  SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), TRUE ) ) )
            {
                XFillStyleItem aStyleItem( XFILL_BITMAP );
                pOld = GetOldItem( rAttrs, XATTR_FILLSTYLE );
                if ( !pOld || !( *(const XFillStyleItem*)pOld == aStyleItem ) )
                {
                    rAttrs.Put( aStyleItem );
                    bModified = TRUE;
                }
            }
        }

        // Schrittweite
        if( aTsbStepCount.IsEnabled() )
        {
            UINT16 nValue = 0;
            BOOL   bValueModified = FALSE;
            TriState eState = aTsbStepCount.GetState();
            if( eState == STATE_CHECK )
            {
                if( eState != aTsbStepCount.GetSavedValue() )
                    bValueModified = TRUE;
            }
            else
            {
                // Zustand != Disabled ?
                if( aNumFldStepCount.GetText().Len() > 0 )
                {
                    nValue = (UINT16) aNumFldStepCount.GetValue();
                    if( nValue != (UINT16) aNumFldStepCount.GetSavedValue().ToInt32() )
                        bValueModified = TRUE;
                }
            }
            if( bValueModified )
            {
                XGradientStepCountItem aItem( nValue );
                pOld = GetOldItem( rAttrs, XATTR_GRADIENTSTEPCOUNT );
                if ( !pOld || !( *(const XGradientStepCountItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
        }

        // Kacheln
        if( aTsbTile.IsEnabled() )
        {
            TriState eState = aTsbTile.GetState();
            if( eState != aTsbTile.GetSavedValue() )
            {
                XFillBmpTileItem aItem( eState );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILE );
                if ( !pOld || !( *(const XFillBmpTileItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
        }
        // Stretchen
        if( aTsbStretch.IsEnabled() )
        {
            TriState eState = aTsbStretch.GetState();
            if( eState != aTsbStretch.GetSavedValue() )
            {
                XFillBmpStretchItem aItem( eState );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_STRETCH );
                if ( !pOld || !( *(const XFillBmpStretchItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
        }

        // Originalgroesse (im UI) wird wie folgt benutzt:
        // Controls sind disabled, muessen aber gesetzt werden.
        // SizeX = 0; SizeY = 0; Log = TRUE

        //aTsbScale
        TriState eState = aTsbScale.GetState();
        if( eState != aTsbScale.GetSavedValue() ||
            ( !aTsbScale.IsEnabled() &&
              aTsbOriginal.IsEnabled() &&
              aTsbScale.GetSavedValue() != STATE_CHECK ) )
        {
            XFillBmpSizeLogItem* pItem = NULL;
            if( aTsbScale.IsEnabled() )
                pItem = new XFillBmpSizeLogItem( eState == STATE_NOCHECK );
            else if( aTsbOriginal.IsEnabled() && aTsbOriginal.GetState() == STATE_CHECK )
                pItem = new XFillBmpSizeLogItem( TRUE );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZELOG );
                if ( !pOld || !( *(const XFillBmpSizeLogItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = TRUE;
                }
                delete pItem;
            }
        }

        //aMtrFldXSize
        String aStr = aMtrFldXSize.GetText();
        {
            XFillBmpSizeXItem* pItem = NULL;
            TriState eState = aTsbScale.GetState();

            if( aMtrFldXSize.IsEnabled() &&
                aStr.Len() > 0  &&
                aStr != aMtrFldXSize.GetSavedValue() )
            {
                if( eState == STATE_NOCHECK )
                    pItem = new XFillBmpSizeXItem( GetCoreValue( aMtrFldXSize, ePoolUnit ) );
                else
                {
                    // Prozentwerte werden negativ gesetzt, damit
                    // diese nicht skaliert werden; dieses wird
                    // im Item beruecksichtigt ( KA05.11.96 )
                    pItem = new XFillBmpSizeXItem( -labs( aMtrFldXSize.GetValue() ) );
                }
            }
            else if( aTsbOriginal.IsEnabled() &&
                     aTsbOriginal.GetState() == STATE_CHECK &&
                     aMtrFldXSize.GetSavedValue().Len() > 0 )
                pItem = new XFillBmpSizeXItem( 0 );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZEX );
                if ( !pOld || !( *(const XFillBmpSizeXItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = TRUE;
                }
                delete pItem;
            }
        }

        //aMtrFldYSize
        aStr = aMtrFldYSize.GetText();
        {
            XFillBmpSizeYItem* pItem = NULL;
            TriState eState = aTsbScale.GetState();

            if( aMtrFldYSize.IsEnabled() &&
                aStr.Len() > 0  &&
                aStr != aMtrFldYSize.GetSavedValue() )
            {
                if( eState == STATE_NOCHECK )
                    pItem = new XFillBmpSizeYItem( GetCoreValue( aMtrFldYSize, ePoolUnit ) );
                else
                {
                    // Prozentwerte werden negativ gesetzt, damit
                    // diese vom MetricItem nicht skaliert werden;
                    // dieses wird im Item beruecksichtigt ( KA05.11.96 )
                    pItem = new XFillBmpSizeYItem( -labs( aMtrFldYSize.GetValue() ) );
                }
            }
            else if( aTsbOriginal.IsEnabled() &&
                     aTsbOriginal.GetState() == STATE_CHECK &&
                     aMtrFldYSize.GetSavedValue().Len() > 0 )
                pItem = new XFillBmpSizeYItem( 0 );

            if( pItem )
            {
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_SIZEY );
                if ( !pOld || !( *(const XFillBmpSizeYItem*)pOld == *pItem ) )
                {
                    rAttrs.Put( *pItem );
                    bModified = TRUE;
                }
                delete pItem;
            }
        }

        //aRbtRow
        //aRbtColumn
        //aMtrFldOffset
        if( aMtrFldOffset.IsEnabled() )
        {
            String aStr = aMtrFldOffset.GetText();
            if( ( aStr.Len() > 0  &&
                  aStr != aMtrFldOffset.GetSavedValue() ) ||
                  aRbtRow.GetSavedValue() != aRbtRow.IsChecked() ||
                  aRbtColumn.GetSavedValue() != aRbtColumn.IsChecked() )
            {
                if( aRbtRow.IsChecked() )
                {
                    XFillBmpTileOffsetXItem aItem( (UINT16) aMtrFldOffset.GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETX );
                    if ( !pOld || !( *(const XFillBmpTileOffsetXItem*)pOld == aItem ) )
                    {
                        rAttrs.Put( aItem );
                        rAttrs.Put( XFillBmpTileOffsetYItem( 0 ) );
                        bModified = TRUE;
                    }
                }
                else if( aRbtColumn.IsChecked() )
                {
                    XFillBmpTileOffsetYItem aItem( (UINT16) aMtrFldOffset.GetValue() );
                    pOld = GetOldItem( rAttrs, XATTR_FILLBMP_TILEOFFSETY );
                    if ( !pOld || !( *(const XFillBmpTileOffsetYItem*)pOld == aItem ) )
                    {
                        rAttrs.Put( aItem );
                        rAttrs.Put( XFillBmpTileOffsetXItem( 0 ) );
                        bModified = TRUE;
                    }
                }
            }
        }

        //aCtlPosition
        if( aCtlPosition.IsEnabled() )
        {
            BOOL bPut = FALSE;
            RECT_POINT eRP = aCtlPosition.GetActualRP();

            if( SFX_ITEM_DONTCARE == rOutAttrs.GetItemState( XATTR_FILLBMP_POS ) )
                bPut = TRUE;
            else
            {
                RECT_POINT eValue = ( ( const XFillBmpPosItem& ) rOutAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
                if( eValue != eRP )
                    bPut = TRUE;
            }
            if( bPut )
            {
                XFillBmpPosItem aItem( eRP );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POS );
                if ( !pOld || !( *(const XFillBmpPosItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
        }

        //aMtrFldXOffset
        if( aMtrFldXOffset.IsEnabled() )
        {
            String aStr = aMtrFldXOffset.GetText();
            if( aStr.Len() > 0  &&
                aStr != aMtrFldXOffset.GetSavedValue() )
            {
                XFillBmpPosOffsetXItem aItem( (UINT16) aMtrFldXOffset.GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETX );
                if ( !pOld || !( *(const XFillBmpPosOffsetXItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
        }

        //aMtrFldYOffset
        if( aMtrFldYOffset.IsEnabled() )
        {
            String aStr = aMtrFldYOffset.GetText();
            if( aStr.Len() > 0  &&
                aStr != aMtrFldYOffset.GetSavedValue() )
            {
                XFillBmpPosOffsetYItem aItem( (UINT16) aMtrFldYOffset.GetValue() );
                pOld = GetOldItem( rAttrs, XATTR_FILLBMP_POSOFFSETY );
                if ( !pOld || !( *(const XFillBmpPosOffsetYItem*)pOld == aItem ) )
                {
                    rAttrs.Put( aItem );
                    bModified = TRUE;
                }
            }
        }
    }

    return( bModified );
}

// -----------------------------------------------------------------------

void SvxAreaTabPage::Reset( const SfxItemSet& rAttrs )
{
    //const SfxPoolItem *pPoolItem = NULL;

    XFillStyle eXFS;
    if( rAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXFS = (XFillStyle) ( ( ( const XFillStyleItem& ) rAttrs.
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
        switch( eXFS )
        {
            case XFILL_NONE:
                aRbtInvisible.Check();
                ClickInvisibleHdl_Impl( this );
            break;

            case XFILL_SOLID:
                aRbtColor.Check();
                //if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rAttrs.Get( XATTR_FILLCOLOR ) );

                    aLbColor.SelectEntry( aColorItem.GetValue() );
                    aLbHatchBckgrdColor.SelectEntry( aColorItem.GetValue() );
                }
                ClickColorHdl_Impl( this );
            break;

            case XFILL_GRADIENT:
                aRbtGradient.Check();
                //if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), TRUE, &pPoolItem ) )
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLGRADIENT ) )
                {
                    XFillGradientItem aGradientItem( ( ( const XFillGradientItem& )
                                            rAttrs.Get( XATTR_FILLGRADIENT ) ) );
                    String    aString( aGradientItem.GetName() );
                    XGradient aGradient( aGradientItem.GetValue() );

                    aLbGradient.SelectEntryByList( pGradientList, aString, aGradient );
                }
                ClickGradientHdl_Impl( this );
            break;

            case XFILL_HATCH:
                aRbtHatching.Check();
                //if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), TRUE, &pPoolItem ) )
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    aLbHatching.SelectEntry( ( ( const XFillHatchItem& )
                                    rAttrs.Get( XATTR_FILLHATCH ) ).GetName() );
                }
                ClickHatchingHdl_Impl( this );
                if ( SFX_ITEM_DONTCARE != rAttrs.GetItemState ( XATTR_FILLBACKGROUND ) )
                {
                    aCbxHatchBckgrd.Check ( ( ( const XFillBackgroundItem& ) rAttrs.Get ( XATTR_FILLBACKGROUND ) ).GetValue() );
                }
                ToggleHatchBckgrdColorHdl_Impl( this );
                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rAttrs.Get( XATTR_FILLCOLOR ) );

                    aLbColor.SelectEntry( aColorItem.GetValue() );
                    aLbHatchBckgrdColor.SelectEntry( aColorItem.GetValue() );
                }
            break;

            case XFILL_BITMAP:
            {
                aRbtBitmap.Check();

                if( SFX_ITEM_DONTCARE != rAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    XFillBitmapItem aBitmapItem( ( const XFillBitmapItem& )
                                        rAttrs.Get( XATTR_FILLBITMAP ) );

                    String aString( aBitmapItem.GetName() );
                    aLbBitmap.SelectEntry( aString );
                }
            }
            break;

            default:
                //aLbColor.SelectEntryPos(0);
                //aRbtColor.Check();
            break;
        }
    }
    else
    {
        // Alle LBs nicht zug"anglich machen
        aLbColor.Hide();
        aLbGradient.Hide();
        aLbHatching.Hide();
        aLbBitmap.Hide();
        aCtlBitmapPreview.Hide();
        aLbColor.Disable();
        aLbColor.Show();

        // Damit Reset() auch mit Zurueck richtig funktioniert
        aRbtInvisible.Check( FALSE );
        aRbtColor.Check( FALSE );
        aRbtGradient.Check( FALSE );
        aRbtHatching.Check( FALSE );
        aRbtBitmap.Check( FALSE );
    }

    // Schrittweite
    if( ( rAttrs.GetItemState( XATTR_GRADIENTSTEPCOUNT ) != SFX_ITEM_DONTCARE ) ||
        ( rAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE ) )
    {
        aTsbStepCount.EnableTriState( FALSE );
        UINT16 nValue = ( ( const XGradientStepCountItem& ) rAttrs.Get( XATTR_GRADIENTSTEPCOUNT ) ).GetValue();
        if( nValue == 0 )
        {
            aTsbStepCount.SetState( STATE_CHECK );
            aNumFldStepCount.SetText( String() );
        }
        else
        {
            aTsbStepCount.SetState( STATE_NOCHECK );
            aNumFldStepCount.SetValue( nValue );
        }
        ModifyStepCountHdl_Impl( &aTsbStepCount );
    }
    else
    {
        aTsbStepCount.SetState( STATE_DONTKNOW );
        aNumFldStepCount.SetText( String() );
    }

    // Attribute fuer die Bitmap-Fuellung

    // Ist Kacheln gesetzt?
    if( rAttrs.GetItemState( XATTR_FILLBMP_TILE ) != SFX_ITEM_DONTCARE )
    {
        aTsbTile.EnableTriState( FALSE );

        if( ( ( const XFillBmpTileItem& ) rAttrs.Get( XATTR_FILLBMP_TILE ) ).GetValue() )
            aTsbTile.SetState( STATE_CHECK );
        else
            aTsbTile.SetState( STATE_NOCHECK );
    }
    else
        aTsbTile.SetState( STATE_DONTKNOW );

    // Ist Stretchen gesetzt?
    if( rAttrs.GetItemState( XATTR_FILLBMP_STRETCH ) != SFX_ITEM_DONTCARE )
    {
        aTsbStretch.EnableTriState( FALSE );

        if( ( ( const XFillBmpStretchItem& ) rAttrs.Get( XATTR_FILLBMP_STRETCH ) ).GetValue() )
            aTsbStretch.SetState( STATE_CHECK );
        else
            aTsbStretch.SetState( STATE_NOCHECK );
    }
    else
        aTsbStretch.SetState( STATE_DONTKNOW );


    //aTsbScale
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZELOG ) != SFX_ITEM_DONTCARE )
    {
        aTsbScale.EnableTriState( FALSE );

        if( ( ( const XFillBmpSizeLogItem& ) rAttrs.Get( XATTR_FILLBMP_SIZELOG ) ).GetValue() )
            aTsbScale.SetState( STATE_NOCHECK );
        else
            aTsbScale.SetState( STATE_CHECK );

        ClickScaleHdl_Impl( NULL );
    }
    else
        aTsbScale.SetState( STATE_DONTKNOW );


    // Status fuer Originalgroesse ermitteln
    TriState eOriginal = STATE_NOCHECK;

    //aMtrFldXSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEX ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpSizeXItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEX ) ).GetValue();
        if( aTsbScale.GetState() == STATE_CHECK )
        {
            // Wenn im Item eine Prozentangabe steckt,
            // so ist diese wegen des MetricItems negativ
            aMtrFldXSize.SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( aMtrFldXSize, nValue, ePoolUnit );
        aMtrFldXSize.SaveValue();

        if( nValue == 0 )
        {
            eOriginal = STATE_CHECK;
            // Wert ist beim Ausschalten von Originalgroesse sonst zu klein
            // (Performance-Problem)
            aMtrFldXSize.SetValue( 100 );
        }
    }
    else
    {
        aMtrFldXSize.SetText( String() );
        aMtrFldXSize.SaveValue();
    }

    //aMtrFldYSize
    if( rAttrs.GetItemState( XATTR_FILLBMP_SIZEY ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpSizeYItem& ) rAttrs.Get( XATTR_FILLBMP_SIZEY ) ).GetValue();
        if( aTsbScale.GetState() == STATE_CHECK )
        {
            // Wenn im Item eine Prozentangabe steckt,
            // so ist diese wegen des MetricItems negativ
            aMtrFldYSize.SetValue( labs( nValue ) );
        }
        else
            SetMetricValue( aMtrFldYSize, nValue, ePoolUnit );
        aMtrFldYSize.SaveValue();

        if( nValue == 0 )
            aMtrFldYSize.SetValue( 100 ); //s.o.
        else
            eOriginal = STATE_NOCHECK;
    }
    else
    {
        aMtrFldYSize.SetText( String() );
        aMtrFldYSize.SaveValue();
        eOriginal = STATE_NOCHECK;
    }

    // aTsbOriginal
    aTsbOriginal.SetState( eOriginal );

    //aRbtRow
    //aRbtColumn
    //aMtrFldOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_TILEOFFSETX ) != SFX_ITEM_DONTCARE )
    {
        UINT16 nValue = ( ( const XFillBmpTileOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_TILEOFFSETX ) ).GetValue();
        if( nValue > 0 )
        {
            aMtrFldOffset.SetValue( nValue );
            aRbtRow.Check();
        }
        else if( rAttrs.GetItemState( XATTR_FILLBMP_TILEOFFSETY ) != SFX_ITEM_DONTCARE )
        {
            nValue = ( ( const XFillBmpTileOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_TILEOFFSETY ) ).GetValue();
            if( nValue > 0 )
            {
                aMtrFldOffset.SetValue( nValue );
                aRbtColumn.Check();
            }
        }
        else
            aMtrFldOffset.SetValue( 0 );
    }
    else
        aMtrFldOffset.SetText( String() );


    //aCtlPosition
    if( rAttrs.GetItemState( XATTR_FILLBMP_POS ) != SFX_ITEM_DONTCARE )
    {
        RECT_POINT eValue = ( ( const XFillBmpPosItem& ) rAttrs.Get( XATTR_FILLBMP_POS ) ).GetValue();
        aCtlPosition.SetActualRP( eValue );
    }
    else
        aCtlPosition.Reset();

    //aMtrFldXOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETX ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpPosOffsetXItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETX ) ).GetValue();
        aMtrFldXOffset.SetValue( nValue );
    }
    else
        aMtrFldXOffset.SetText( String() );

    //aMtrFldYOffset
    if( rAttrs.GetItemState( XATTR_FILLBMP_POSOFFSETY ) != SFX_ITEM_DONTCARE )
    {
        INT32 nValue = ( ( const XFillBmpPosOffsetYItem& ) rAttrs.Get( XATTR_FILLBMP_POSOFFSETY ) ).GetValue();
        aMtrFldYOffset.SetValue( nValue );
    }
    else
        aMtrFldYOffset.SetText( String() );

    // Erst hier, damit Tile und Stretch mit beruecksichtigt wird
    if( aRbtBitmap.IsChecked() )
        ClickBitmapHdl_Impl( NULL );

    // Werte sichern
    aRbtInvisible.SaveValue();
    aRbtColor.SaveValue();
    aRbtGradient.SaveValue();
    aRbtHatching.SaveValue();
    aRbtBitmap.SaveValue();
    aLbColor.SaveValue();
    aLbGradient.SaveValue();
    aLbHatching.SaveValue();
    //aCbxHatchBckgrd.SaveValue();
    aLbHatchBckgrdColor.SaveValue();
    aLbBitmap.SaveValue();
//  aLbTransparent.SaveValue();
    aTsbStepCount.SaveValue();
    aNumFldStepCount.SaveValue();
    aTsbTile.SaveValue();
    aTsbStretch.SaveValue();
    aTsbScale.SaveValue();
    aRbtRow.SaveValue();
    aRbtColumn.SaveValue();
    aMtrFldOffset.SaveValue();
    aMtrFldXOffset.SaveValue();
    aMtrFldYOffset.SaveValue();
    //aMtrFldXSize.SaveValue(); <- wird oben behandelt
    //aMtrFldYSize.SaveValue(); <- wird oben behandelt
}

// -----------------------------------------------------------------------

SfxTabPage* SvxAreaTabPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxAreaTabPage( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxAreaTabPage::GetRanges()
{
    return( pAreaRanges );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickInvisibleHdl_Impl, void *, EMPTYARG )
{
    aLbColor.Disable();
    aLbGradient.Disable();
    aLbHatching.Disable();
    aLbBitmap.Disable();
    aCtlBitmapPreview.Disable();
    aCtlXRectPreview.Disable();

//  aGrpTransparent.Disable();
//  aLbTransparent.Disable();
    aGrpStepCount.Disable();
    aTsbStepCount.Disable();
    aNumFldStepCount.Disable();
    aTsbTile.Disable();
    aTsbStretch.Disable();
    aTsbScale.Disable();
    aTsbOriginal.Disable();
    aFtXSize.Hide();
    aMtrFldXSize.Disable();
    aFtYSize.Hide();
    aMtrFldYSize.Disable();
    aGrpSize.Disable();
    aRbtRow.Disable();
    aRbtColumn.Disable();
    aMtrFldOffset.Disable();
    aGrpOffset.Disable();
    aCtlPosition.Disable();
    aFtXOffset.Disable();
    aMtrFldXOffset.Disable();
    aFtYOffset.Disable();
    aMtrFldYOffset.Disable();
    aGrpPosition.Disable();
    aCtlPosition.Invalidate();

    // Controls for Hatch-Background
    aGrpHatchBckgrd.Disable();
    aCbxHatchBckgrd.Disable();
    aLbHatchBckgrdColor.Disable();

    rXFSet.Put( XFillStyleItem( XFILL_NONE ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    XOutBmp.SetFillAttr( aXFillAttr.GetItemSet() );

    aCtlXRectPreview.Invalidate();
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickColorHdl_Impl, void *, EMPTYARG )
{
    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aGrpSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aGrpOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aGrpPosition.Hide();

    aLbColor.Enable();
    aLbColor.Show();
    aLbGradient.Hide();
    aLbHatching.Hide();
    aLbBitmap.Hide();
    aCtlXRectPreview.Enable();
    aCtlXRectPreview.Show();
    aCtlBitmapPreview.Hide();
    aGrpPreview.Enable();
    aGrpPreview.Show();

    aGrpStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    // Controls for Hatch-Background
    aGrpHatchBckgrd.Hide();
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    // Text der Tabelle setzen
    String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pColorTab->GetPath() );

    aURL.Append( pColorTab->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().Len() > 18 )
    {
        aString += aURL.getBase().Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += aURL.getBase();

    aGrpFill.SetText( aString );
    ModifyColorHdl_Impl( this );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyColorHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nPos = aLbColor.GetSelectEntryPos();
    aLbHatchBckgrdColor.SelectEntryPos( nPos );
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        rXFSet.Put( XFillColorItem( String(),
                                    aLbColor.GetSelectEntryColor() ) );
    }
    // NEU
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        Color aColor( ( ( const XFillColorItem* ) pPoolItem )->GetValue() );
        rXFSet.Put( XFillColorItem( String(), aColor ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}
//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickGradientHdl_Impl, void *, EMPTYARG )
{
    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aGrpSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aGrpOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aGrpPosition.Hide();

    aLbColor.Hide();
    aLbGradient.Enable();
    aLbGradient.Show();
    aLbHatching.Hide();
    aLbBitmap.Hide();
    aCtlXRectPreview.Enable();
    aCtlXRectPreview.Show();
    aCtlBitmapPreview.Hide();
    aGrpPreview.Enable();
    aGrpPreview.Show();

    aGrpStepCount.Enable();
    aGrpStepCount.Show();
    aTsbStepCount.Enable();
    aTsbStepCount.Show();
    aNumFldStepCount.Show();

    // Controls for Hatch-Background
    aGrpHatchBckgrd.Hide();
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    // Text der Tabelle setzen
    String          aString( SVX_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pGradientList->GetPath() );

    aURL.Append( pGradientList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().Len() > 18 )
    {
        aString += aURL.getBase().Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += aURL.getBase();

    aGrpFill.SetText( aString );
    ModifyGradientHdl_Impl( this );
    ModifyStepCountHdl_Impl( &aTsbStepCount );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyGradientHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nPos = aLbGradient.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // ItemSet fuellen und an XOut weiterleiten
        XGradientEntry* pEntry = pGradientList->Get( nPos );

        rXFSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rXFSet.Put( XFillGradientItem( String(),
                                       pEntry->GetGradient() ) );
    }
    // NEU
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_GRADIENT ) );
        rXFSet.Put( XFillGradientItem( String(), ( ( const XFillGradientItem* ) pPoolItem )->GetValue() ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickHatchingHdl_Impl, void *, EMPTYARG )
{
    aLbColor.Hide();
    aLbGradient.Hide();
    aLbHatching.Enable();
    aLbHatching.Show();
    aLbBitmap.Hide();
    aCtlXRectPreview.Enable();
    aCtlXRectPreview.Show();
    aCtlBitmapPreview.Hide();
    aGrpPreview.Enable();
    aGrpPreview.Show();

//  aGrpTransparent.Hide();
//  aLbTransparent.Hide();
    aGrpStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    aTsbTile.Hide();
    aTsbStretch.Hide();
    aTsbScale.Hide();
    aTsbOriginal.Hide();
    aFtXSize.Hide();
    aMtrFldXSize.Hide();
    aFtYSize.Hide();
    aMtrFldYSize.Hide();
    aGrpSize.Hide();
    aRbtRow.Hide();
    aRbtColumn.Hide();
    aMtrFldOffset.Hide();
    aGrpOffset.Hide();
    aCtlPosition.Hide();
    aFtXOffset.Hide();
    aMtrFldXOffset.Hide();
    aFtYOffset.Hide();
    aMtrFldYOffset.Hide();
    aGrpPosition.Hide();

    // Controls for Hatch-Background
    aGrpHatchBckgrd.Show();
    aCbxHatchBckgrd.Show();
    aLbHatchBckgrdColor.Show();
    aGrpHatchBckgrd.Enable();
    aCbxHatchBckgrd.Enable();
    aLbHatchBckgrdColor.Enable();

    // Text der Tabelle setzen
    String          aString( SVX_RES( RID_SVXSTR_TABLE ) );     aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pHatchingList->GetPath() );

    aURL.Append( pHatchingList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().Len() > 18 )
    {
        aString += aURL.getBase().Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += aURL.getBase();

    aGrpFill.SetText( aString );
    ModifyHatchingHdl_Impl( this );
    ModifyHatchBckgrdColorHdl_Impl( this );
    ToggleHatchBckgrdColorHdl_Impl( this );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyHatchingHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nPos = aLbHatching.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // ItemSet fuellen und an XOut weiterleiten
        XHatchEntry* pEntry = pHatchingList->Get( nPos );

        rXFSet.Put( XFillStyleItem( XFILL_HATCH ) );
        rXFSet.Put( XFillHatchItem( String(), pEntry->GetHatch() ) );
    }
    // NEU
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_HATCH ) );
        rXFSet.Put( XFillHatchItem( String(), ( ( const XFillHatchItem* ) pPoolItem )->GetValue() ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyHatchBckgrdColorHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nPos = aLbHatchBckgrdColor.GetSelectEntryPos();
    aLbColor.SelectEntryPos( nPos );
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
//      rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        rXFSet.Put( XFillColorItem( String(),
                                    aLbHatchBckgrdColor.GetSelectEntryColor() ) );
    }
    // NEU
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), TRUE, &pPoolItem ) )
    {
//      rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
        Color aColor( ( ( const XFillColorItem* ) pPoolItem )->GetValue() );
        rXFSet.Put( XFillColorItem( String(), aColor ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ToggleHatchBckgrdColorHdl_Impl, void *, EMPTYARG )
{
    // switch on/off backgroundcolor for hatches
    aLbHatchBckgrdColor.Enable( aCbxHatchBckgrd.IsChecked() );

    XFillBackgroundItem aItem( aCbxHatchBckgrd.IsChecked() );
    rXFSet.Put ( aItem, XATTR_FILLBACKGROUND );

    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    if( aLbHatchBckgrdColor.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
    {
        if ( SFX_ITEM_SET == rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )//>= SFX_ITEM_DEFAULT )
        {
            XFillColorItem aColorItem( (const XFillColorItem&)rOutAttrs.Get( XATTR_FILLCOLOR ) );
            aLbHatchBckgrdColor.SelectEntry( aColorItem.GetValue() );
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickBitmapHdl_Impl, void *, EMPTYARG )
{
    aLbColor.Hide();
    aLbGradient.Hide();
    aLbHatching.Hide();
    aLbBitmap.Enable();
    aLbBitmap.Show();
    aCtlBitmapPreview.Enable();
    aCtlBitmapPreview.Show();
    aCtlXRectPreview.Hide();
    aGrpPreview.Hide();
//  aGrpTransparent.Hide();
//  aLbTransparent.Hide();
    aGrpStepCount.Hide();
    aTsbStepCount.Hide();
    aNumFldStepCount.Hide();

    aTsbTile.Enable();
    aTsbStretch.Enable();
    aTsbScale.Enable();
    aTsbOriginal.Enable();
    aFtXSize.Enable();
    aMtrFldXSize.Enable();
    aFtYSize.Enable();
    aMtrFldYSize.Enable();
    aGrpSize.Enable();
    aCtlPosition.Enable();
    aFtXOffset.Enable();
    aMtrFldXOffset.Enable();
    aFtYOffset.Enable();
    aMtrFldYOffset.Enable();
    aGrpPosition.Enable();
    aRbtRow.Enable();
    aRbtColumn.Enable();
    aMtrFldOffset.Enable();
    aGrpOffset.Enable();

    // Controls for Hatch-Background
    aGrpHatchBckgrd.Hide();
    aCbxHatchBckgrd.Hide();
    aLbHatchBckgrdColor.Hide();

    aTsbTile.Show();
    aTsbStretch.Show();
    aTsbScale.Show();
    aTsbOriginal.Show();
    aFtXSize.Show();
    aMtrFldXSize.Show();
    aFtYSize.Show();
    aMtrFldYSize.Show();
    aGrpSize.Show();
    aCtlPosition.Show();
    aFtXOffset.Show();
    aMtrFldXOffset.Show();
    aFtYOffset.Show();
    aMtrFldYOffset.Show();
    aGrpPosition.Show();
    aRbtRow.Show();
    aRbtColumn.Show();
    aMtrFldOffset.Show();
    aGrpOffset.Show();

    // Text der Tabelle setzen
    String          aString( SVX_RES( RID_SVXSTR_TABLE ) );     aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
    INetURLObject   aURL( pBitmapList->GetPath() );

    aURL.Append( pBitmapList->GetName() );
    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( aURL.getBase().Len() > 18 )
    {
        aString += aURL.getBase().Copy( 0, 15 );
        aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
    }
    else
        aString += aURL.getBase();

    aGrpFill.SetText( aString );
    ModifyBitmapHdl_Impl( this );
    ModifyTileHdl_Impl( &aTsbOriginal );
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyBitmapHdl_Impl, void *, EMPTYARG )
{
    const SfxPoolItem* pPoolItem = NULL;
    USHORT nPos = aLbBitmap.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        // ItemSet fuellen und an XOut weiterleiten
        XBitmapEntry* pEntry = pBitmapList->Get( nPos );

        rXFSet.Put( XFillStyleItem( XFILL_BITMAP ) );
        rXFSet.Put( XFillBitmapItem( String(), pEntry->GetXBitmap() ) );
    }
    // NEU
    else if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), TRUE, &pPoolItem ) )
    {
        rXFSet.Put( XFillStyleItem( XFILL_BITMAP ) );
        rXFSet.Put( XFillBitmapItem( String(), ( ( const XFillBitmapItem* ) pPoolItem )->GetValue() ) );
    }
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    XOutBmp.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

//IMPL_LINK( SvxAreaTabPage, ModifyTransparentHdl_Impl, void *, EMPTYARG )
//{
//  USHORT nPos = aLbTransparent.GetSelectEntryPos();
//  if( nPos != LISTBOX_ENTRY_NOTFOUND )
//  {
//      XFillTransparenceItem aItem( nPos * 25 );
//
//      rXFSet.Put( XFillTransparenceItem( aItem ) );
//      XOut.SetFillAttr( aXFillAttr );
//
//      aCtlXRectPreview.Invalidate();
//  }
//
//  return( 0L );
//}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyStepCountHdl_Impl, void *, p )
{
    if( p == &aTsbStepCount )
    {
        if( aTsbStepCount.GetState() == STATE_NOCHECK )
            aNumFldStepCount.Enable();
        else
            aNumFldStepCount.Disable();
    }

    UINT16 nValue = 0;
    if( aTsbStepCount.GetState() != STATE_CHECK )
    {
        // Zustand != Disabled ?
        if( aNumFldStepCount.GetText().Len() > 0 )
            nValue = (UINT16) aNumFldStepCount.GetValue();
    }
    rXFSet.Put( XGradientStepCountItem( nValue ) );
    XOut.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ModifyTileHdl_Impl, void *, p )
{
    TriState eState = aTsbTile.GetState();
    if( eState == STATE_CHECK )
    {
        aTsbStretch.Disable();
        aRbtRow.Enable();
        aRbtColumn.Enable();
        aMtrFldOffset.Enable();
        aGrpOffset.Enable();

        aCtlPosition.Enable();
        aCtlPosition.Invalidate();
        aFtXOffset.Enable();
        aMtrFldXOffset.Enable();
        aFtYOffset.Enable();
        aMtrFldYOffset.Enable();
        aGrpPosition.Enable();

        aTsbScale.Enable();
        aTsbOriginal.Enable();
        aFtXSize.Enable();
        aMtrFldXSize.Enable();
        aFtYSize.Enable();
        aMtrFldYSize.Enable();
        aGrpSize.Enable();
    }
    else if( eState == STATE_NOCHECK )
    {
        aTsbStretch.Enable();
        aRbtRow.Disable();
        aRbtColumn.Disable();
        aMtrFldOffset.Disable();
        aGrpOffset.Disable();

        aCtlPosition.Disable();
        aCtlPosition.Invalidate();
        aFtXOffset.Disable();
        aMtrFldXOffset.Disable();
        aFtYOffset.Disable();
        aMtrFldYOffset.Disable();
        aGrpPosition.Disable();

        if( aTsbStretch.GetState() != STATE_NOCHECK )
        {
            aTsbScale.Disable();
            aTsbOriginal.Disable();
            aFtXSize.Disable();
            aMtrFldXSize.Disable();
            aFtYSize.Disable();
            aMtrFldYSize.Disable();
            aGrpSize.Disable();
        }
        else
        {
            aTsbScale.Enable();
            aTsbOriginal.Enable();
            aFtXSize.Enable();
            aMtrFldXSize.Enable();
            aFtYSize.Enable();
            aMtrFldYSize.Enable();
            aGrpSize.Enable();
        }
    }
    else
    {
        aTsbStretch.Disable();
        aRbtRow.Disable();
        aRbtColumn.Disable();
        aMtrFldOffset.Disable();
        aGrpOffset.Disable();

        aCtlPosition.Disable();
        aCtlPosition.Invalidate();
        aFtXOffset.Disable();
        aMtrFldXOffset.Disable();
        aFtYOffset.Disable();
        aMtrFldYOffset.Disable();
        aGrpPosition.Disable();

        aTsbScale.Disable();
        aTsbOriginal.Disable();
        aFtXSize.Disable();
        aMtrFldXSize.Disable();
        aFtYSize.Disable();
        aMtrFldYSize.Disable();
        aGrpSize.Disable();
    }

    if( aTsbOriginal.GetState() == STATE_CHECK )
    {
        aMtrFldXSize.SetText( String() );
        aMtrFldYSize.SetText( String() );
        aFtXSize.Disable();
        aFtYSize.Disable();
        aMtrFldXSize.Disable();
        aMtrFldYSize.Disable();
        aTsbScale.Disable();
    }
    else
    {
        aMtrFldXSize.SetValue( aMtrFldXSize.GetValue() );
        aMtrFldYSize.SetValue( aMtrFldYSize.GetValue() );
        /*
        if( eState == STATE_CHECK )
        {
            aFtXSize.Enable();
            aFtYSize.Enable();
            aMtrFldXSize.Enable();
            aMtrFldYSize.Enable();
        }
        */
    }

    rXFSet.Put( XFillBmpTileItem( eState ) );

    if( aTsbStretch.IsEnabled() )
        rXFSet.Put( XFillBmpStretchItem( aTsbStretch.GetState() ) );

    if( aTsbScale.IsEnabled() )
        rXFSet.Put( XFillBmpSizeLogItem( aTsbScale.GetState() == STATE_NOCHECK ) );

    if( aMtrFldXSize.IsEnabled() )
    {
        XFillBmpSizeXItem* pItem = NULL;
        TriState eState = aTsbScale.GetState();

        if( eState == STATE_NOCHECK )
            pItem = new XFillBmpSizeXItem( GetCoreValue( aMtrFldXSize, ePoolUnit ) );
        else
            pItem = new XFillBmpSizeXItem( -labs( aMtrFldXSize.GetValue() ) );

        rXFSet.Put( *pItem );

        delete pItem;
    }
    else if( aTsbOriginal.IsEnabled() && aTsbOriginal.GetState() == STATE_CHECK )
    {
        // Originalgroesse -> Size == 0
        rXFSet.Put( XFillBmpSizeXItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( TRUE ) );
    }

    if( aMtrFldYSize.IsEnabled() )
    {
        XFillBmpSizeYItem* pItem = NULL;
        TriState eState = aTsbScale.GetState();

        if( eState == STATE_NOCHECK )
            pItem = new XFillBmpSizeYItem( GetCoreValue( aMtrFldYSize, ePoolUnit ) );
        else
            pItem = new XFillBmpSizeYItem( -labs( aMtrFldYSize.GetValue() ) );

        rXFSet.Put( *pItem );

        delete pItem;
    }
    else if( aTsbOriginal.IsEnabled() && aTsbOriginal.GetState() == STATE_CHECK )
    {
        // Originalgroesse -> Size == 0
        rXFSet.Put( XFillBmpSizeYItem( 0 ) );
        rXFSet.Put( XFillBmpSizeLogItem( TRUE ) );
    }

    if( aMtrFldOffset.IsEnabled() )
    {
        if( aRbtRow.IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (UINT16) aMtrFldOffset.GetValue() ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (UINT16) 0 ) );
        }
        else if( aRbtColumn.IsChecked() )
        {
            rXFSet.Put( XFillBmpTileOffsetXItem( (UINT16) 0 ) );
            rXFSet.Put( XFillBmpTileOffsetYItem( (UINT16) aMtrFldOffset.GetValue() ) );
        }
    }

    if( aCtlPosition.IsEnabled() )
        rXFSet.Put( XFillBmpPosItem( aCtlPosition.GetActualRP() ) );

    if( aMtrFldXOffset.IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetXItem( (UINT16) aMtrFldXOffset.GetValue() ) );

    if( aMtrFldYOffset.IsEnabled() )
        rXFSet.Put( XFillBmpPosOffsetYItem( (UINT16) aMtrFldYOffset.GetValue() ) );


    XOutBmp.SetFillAttr( aXFillAttr.GetItemSet() );
    aCtlBitmapPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAreaTabPage, ClickScaleHdl_Impl, void *, p )
{
    if( aTsbScale.GetState() == STATE_CHECK )
    {
        aMtrFldXSize.SetDecimalDigits( 0 );
        aMtrFldXSize.SetUnit( FUNIT_CUSTOM );
        aMtrFldXSize.SetValue( 100 );
        aMtrFldXSize.SetMax( 100 );
        aMtrFldXSize.SetLast( 100 );

        aMtrFldYSize.SetDecimalDigits( 0 );
        aMtrFldYSize.SetUnit( FUNIT_CUSTOM );
        aMtrFldYSize.SetValue( 100 );
        aMtrFldYSize.SetMax( 100 );
        aMtrFldYSize.SetLast( 100 );
    }
    else
    {
        aMtrFldXSize.SetDecimalDigits( 2 );
        aMtrFldXSize.SetUnit( eFUnit );
        aMtrFldXSize.SetValue( 100 );
        aMtrFldXSize.SetMax( 999900 );
        aMtrFldXSize.SetLast( 100000 );

        aMtrFldYSize.SetDecimalDigits( 2 );
        aMtrFldYSize.SetUnit( eFUnit );
        aMtrFldYSize.SetValue( 100 );
        aMtrFldYSize.SetMax( 999900 );
        aMtrFldYSize.SetLast( 100000 );
    }

    ModifyTileHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

void SvxAreaTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    eRP = eRcPt;

    // Ausrichtung der Bitmapfuellung
    ModifyTileHdl_Impl( pWindow );
}


