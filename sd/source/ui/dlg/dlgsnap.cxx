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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif


#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svl/itempool.hxx>
#include <svl/aeitem.hxx>

#include "dlgsnap.hrc"
#include "sdattr.hxx"
#include "View.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "dlgsnap.hxx"


/*************************************************************************
|*
|* Dialog zum Einstellen des Rasters (wohl kaum ESO!)
|*
\************************************************************************/

SdSnapLineDlg::SdSnapLineDlg(
    ::Window* pWindow,
    const SfxItemSet& rInAttrs,
    ::sd::View* pView)
    : ModalDialog (pWindow, SdResId(DLG_SNAPLINE)),
      aFlPos      (this, SdResId(FL_POSITION)),
      aFtX        (this, SdResId(FT_X)),
      aMtrFldX    (this, SdResId(MTR_FLD_X)),
      aFtY        (this, SdResId(FT_Y)),
      aMtrFldY    (this, SdResId(MTR_FLD_Y)),
      aFlDir      (this, SdResId(FL_DIRECTION)),
      aRbPoint    (this, SdResId(RB_POINT)),
      aRbVert     (this, SdResId(RB_VERTICAL)),
      aRbHorz     (this, SdResId(RB_HORIZONTAL)),
      aBtnOK      (this, SdResId(BTN_OK)),
      aBtnCancel  (this, SdResId(BTN_CANCEL)),
      aBtnHelp    (this, SdResId(BTN_HELP)),
      aBtnDelete  (this, SdResId(BTN_DELETE)),
      eUIUnit(pView->GetDoc()->GetUIUnit()),
      aUIScale(pView->GetDoc()->GetUIScale())
{
    aRbPoint.SetModeRadioImage( Image( SdResId( BMP_POINT_H ) ), BMP_COLOR_HIGHCONTRAST );
    aRbVert.SetModeRadioImage( Image( SdResId( BMP_VERTICAL_H ) ), BMP_COLOR_HIGHCONTRAST );
    aRbHorz.SetModeRadioImage( Image( SdResId( BMP_HORIZONTAL_H ) ), BMP_COLOR_HIGHCONTRAST );

    FreeResource();

    aRbHorz.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    aRbVert.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    aRbPoint.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    aBtnDelete.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    SetFieldUnit( aMtrFldX, eUIUnit, sal_True );
    SetFieldUnit( aMtrFldY, eUIUnit, sal_True );

    // WorkArea holen
    Rectangle aWorkArea = pView->GetWorkArea();

    // PoolUnit ermitteln
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    SfxMapUnit ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // #i48497# Consider page origin
    SdrPageView* pPV = pView->GetSdrPageView();
    Point aLeftTop(aWorkArea.Left()+1, aWorkArea.Top()+1);
    pPV->LogicToPagePos(aLeftTop);
    Point aRightBottom(aWorkArea.Right()-2, aWorkArea.Bottom()-2);
    pPV->LogicToPagePos(aRightBottom);

    // Hier werden die Max- und MinWerte in Abhaengigkeit von der
    // WorkArea, PoolUnit und der FieldUnit:
    SetMetricValue( aMtrFldX, aLeftTop.X(), ePoolUnit );

    long nValue = static_cast<long>(aMtrFldX.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMin( nValue );
    aMtrFldX.SetFirst( nValue );

    SetMetricValue( aMtrFldX, aRightBottom.X(), ePoolUnit );
    nValue = static_cast<long>(aMtrFldX.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMax( nValue );
    aMtrFldX.SetLast( nValue );

    SetMetricValue( aMtrFldY, aLeftTop.Y(), ePoolUnit );
    nValue = static_cast<long>(aMtrFldY.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldY.SetMin( nValue );
    aMtrFldY.SetFirst( nValue );

    SetMetricValue( aMtrFldY, aRightBottom.Y(), ePoolUnit );
    nValue = static_cast<long>(aMtrFldY.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldY.SetMax( nValue );
    aMtrFldY.SetLast( nValue );

    // Setzen der Werte
    nXValue = ((const SfxUInt32Item&) rInAttrs.Get(ATTR_SNAPLINE_X)).GetValue();
    nYValue = ((const SfxUInt32Item&) rInAttrs.Get(ATTR_SNAPLINE_Y)).GetValue();
    nXValue = Fraction(nXValue) / aUIScale;
    nYValue = Fraction(nYValue) / aUIScale;
    SetMetricValue( aMtrFldX, nXValue, SFX_MAPUNIT_100TH_MM);
    SetMetricValue( aMtrFldY, nYValue, SFX_MAPUNIT_100TH_MM);

    aRbPoint.Check();
}

/*************************************************************************
|*
|* Fuellt uebergebenes Item-Set mit Dialogbox-Attributen
|*
\************************************************************************/

IMPL_LINK( SdSnapLineDlg, ClickHdl, Button *, pBtn )
{
    if ( pBtn == &aRbPoint )        SetInputFields(sal_True, sal_True);
    else if ( pBtn == &aRbHorz )    SetInputFields(sal_False, sal_True);
    else if ( pBtn == &aRbVert )    SetInputFields(sal_True, sal_False);
    else if ( pBtn == &aBtnDelete ) EndDialog(RET_SNAP_DELETE);

    return 0;
}

/*************************************************************************
|*
|* Fuellt uebergebenes Item-Set mit Dialogbox-Attributen
|*
\************************************************************************/

void SdSnapLineDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    SnapKind eKind;

    if ( aRbHorz.IsChecked() )      eKind = SK_HORIZONTAL;
    else if ( aRbVert.IsChecked() ) eKind = SK_VERTICAL;
    else                            eKind = SK_POINT;

    nXValue = Fraction( GetCoreValue( aMtrFldX, SFX_MAPUNIT_100TH_MM) ) * aUIScale;
    nYValue = Fraction( GetCoreValue( aMtrFldY, SFX_MAPUNIT_100TH_MM) ) * aUIScale;

    rOutAttrs.Put(SfxAllEnumItem(ATTR_SNAPLINE_KIND, (sal_uInt16)eKind));
    rOutAttrs.Put(SfxUInt32Item(ATTR_SNAPLINE_X, nXValue));
    rOutAttrs.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, nYValue));
}

/*************************************************************************
|*
|* Gruppe mit Radiobutton verstecken
|*
\************************************************************************/

void SdSnapLineDlg::HideRadioGroup()
{
    aFlDir.Hide();
    aRbHorz.Hide();
    aRbVert.Hide();
    aRbPoint.Hide();
}

/*************************************************************************
|*
|* X- bzw. Y-Inputfeld disablen
|*
\************************************************************************/

void SdSnapLineDlg::SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY)
{
    if ( bEnableX )
    {
        if ( !aMtrFldX.IsEnabled() )
            aMtrFldX.SetValue(nXValue);
        aMtrFldX.Enable();
        aFtX.Enable();
    }
    else if ( aMtrFldX.IsEnabled() )
    {
        nXValue = static_cast<long>(aMtrFldX.GetValue());
        aMtrFldX.SetText(String());
        aMtrFldX.Disable();
        aFtX.Disable();
    }
    if ( bEnableY )
    {
        if ( !aMtrFldY.IsEnabled() )
            aMtrFldY.SetValue(nYValue);
        aMtrFldY.Enable();
        aFtY.Enable();
    }
    else if ( aMtrFldY.IsEnabled() )
    {
        nYValue = static_cast<long>(aMtrFldY.GetValue());
        aMtrFldY.SetText(String());
        aMtrFldY.Disable();
        aFtY.Disable();
    }
}



