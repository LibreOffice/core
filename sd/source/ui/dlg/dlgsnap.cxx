/*************************************************************************
 *
 *  $RCSfile: dlgsnap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:32 $
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

#pragma hdrstop

#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif

#include "dlgsnap.hrc"
#include "sdattr.hxx"
#include "sdview.hxx"
#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "dlgsnap.hxx"


/*************************************************************************
|*
|* Dialog zum Einstellen des Rasters (wohl kaum ESO!)
|*
\************************************************************************/

SdSnapLineDlg::SdSnapLineDlg(Window* pWindow, const SfxItemSet& rInAttrs,
                             SdView* pView) :
    ModalDialog (pWindow, SdResId(DLG_SNAPLINE)),
    aFtX        (this, SdResId(FT_X)),
    aFtY        (this, SdResId(FT_Y)),
    aMtrFldX    (this, SdResId(MTR_FLD_X)),
    aMtrFldY    (this, SdResId(MTR_FLD_Y)),
    aGrpPos     (this, SdResId(GRP_POSITION)),
    aGrpDir     (this, SdResId(GRP_DIRECTION)),
    aRbHorz     (this, SdResId(RB_HORIZONTAL)),
    aRbVert     (this, SdResId(RB_VERTICAL)),
    aRbPoint    (this, SdResId(RB_POINT)),
    aBtnOK      (this, SdResId(BTN_OK)),
    aBtnCancel  (this, SdResId(BTN_CANCEL)),
    aBtnHelp    (this, SdResId(BTN_HELP)),
    aBtnDelete  (this, SdResId(BTN_DELETE)),
    eUIUnit(pView->GetDoc()->GetUIUnit()),
    aUIScale(pView->GetDoc()->GetUIScale())
{
    FreeResource();

    aRbHorz.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    aRbVert.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));
    aRbPoint.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    aBtnDelete.SetClickHdl(LINK(this, SdSnapLineDlg, ClickHdl));

    SetFieldUnit( aMtrFldX, eUIUnit, TRUE );
    SetFieldUnit( aMtrFldY, eUIUnit, TRUE );

    // WorkArea holen
    Rectangle aWorkArea = pView->GetWorkArea();

    // PoolUnit ermitteln
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    SfxMapUnit ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Hier werden die Max- und MinWerte in Ahaengigkeit von der
    // WorkArea, PoolUnit und der FieldUnit:
    SetMetricValue( aMtrFldX, aWorkArea.Left(), ePoolUnit );
    long nValue = aMtrFldX.GetValue();
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMin( nValue );
    aMtrFldX.SetFirst( nValue );

    SetMetricValue( aMtrFldX, aWorkArea.Right()+1, ePoolUnit );
    nValue = aMtrFldX.GetValue();
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldX.SetMax( nValue );
    aMtrFldX.SetLast( nValue );

    SetMetricValue( aMtrFldY, aWorkArea.Top(), ePoolUnit );
    nValue = aMtrFldY.GetValue();
    nValue = Fraction( nValue ) / aUIScale;
    aMtrFldY.SetMin( nValue );
    aMtrFldY.SetFirst( nValue );

    SetMetricValue( aMtrFldY, aWorkArea.Bottom()+1, ePoolUnit );
    nValue = aMtrFldY.GetValue();
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
    if ( pBtn == &aRbPoint )        SetInputFields(TRUE, TRUE);
    else if ( pBtn == &aRbHorz )    SetInputFields(FALSE, TRUE);
    else if ( pBtn == &aRbVert )    SetInputFields(TRUE, FALSE);
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

    rOutAttrs.Put(SfxAllEnumItem(ATTR_SNAPLINE_KIND, eKind));
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
    aGrpDir.Hide();
    aRbHorz.Hide();
    aRbVert.Hide();
    aRbPoint.Hide();
}

/*************************************************************************
|*
|* X- bzw. Y-Inputfeld disablen
|*
\************************************************************************/

void SdSnapLineDlg::SetInputFields(BOOL bEnableX, BOOL bEnableY)
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
        nXValue = aMtrFldX.GetValue();
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
        nYValue = aMtrFldY.GetValue();
        aMtrFldY.SetText(String());
        aMtrFldY.Disable();
        aFtY.Disable();
    }
}



