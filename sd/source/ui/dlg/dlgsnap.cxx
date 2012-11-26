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

    SetFieldUnit( aMtrFldX, eUIUnit, true );
    SetFieldUnit( aMtrFldY, eUIUnit, true );

    // PoolUnit ermitteln
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    SfxMapUnit ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // get WorkArea, consider page origin
    basegfx::B2DRange aWorkArea(pView->GetWorkArea());
    SdrPageView* pPV = pView->GetSdrPageView();

    if(pPV)
    {
        aWorkArea.transform(basegfx::tools::createTranslateB2DHomMatrix(pPV->GetPageOrigin()));
    }

    // Hier werden die Max- und MinWerte in Abhaengigkeit von der
    // WorkArea, PoolUnit und der FieldUnit:
    SetMetricValue( aMtrFldX, basegfx::fround(aWorkArea.getMinX()), ePoolUnit );

    long nValue = static_cast<long>(aMtrFldX.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMin( nValue );
    aMtrFldX.SetFirst( nValue );

    SetMetricValue( aMtrFldX, basegfx::fround(aWorkArea.getMaxX()), ePoolUnit );
    nValue = static_cast<long>(aMtrFldX.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMax( nValue );
    aMtrFldX.SetLast( nValue );

    SetMetricValue( aMtrFldY, basegfx::fround(aWorkArea.getMinY()), ePoolUnit );
    nValue = static_cast<long>(aMtrFldY.GetValue());
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldY.SetMin( nValue );
    aMtrFldY.SetFirst( nValue );

    SetMetricValue( aMtrFldY, basegfx::fround(aWorkArea.getMaxY()), ePoolUnit );
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
    if ( pBtn == &aRbPoint )        SetInputFields(true, true);
    else if ( pBtn == &aRbHorz )    SetInputFields(false, true);
    else if ( pBtn == &aRbVert )    SetInputFields(true, false);
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

void SdSnapLineDlg::SetInputFields(bool bEnableX, bool bEnableY)
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



