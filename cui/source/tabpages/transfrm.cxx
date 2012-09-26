/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <svx/svdview.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdotext.hxx>
#include <svx/sderitm.hxx>
#include <svx/dialogs.hrc>
#include <cuires.hrc>
#include "transfrm.hrc"
#include <editeng/sizeitem.hxx>

#include "transfrm.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <editeng/svxenum.hxx>
#include "svx/anchorid.hxx"
#include <sfx2/module.hxx>
#include <svl/rectitem.hxx>
#include <svl/aeitem.hxx>
#include <swpossizetabpage.hxx>

// static ----------------------------------------------------------------

static sal_uInt16 pPosSizeRanges[] =
{
    SID_ATTR_TRANSFORM_POS_X,
    SID_ATTR_TRANSFORM_POS_Y,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_ANCHOR,
    SID_ATTR_TRANSFORM_VERT_ORIENT,
    SID_ATTR_TRANSFORM_WIDTH,
    SID_ATTR_TRANSFORM_SIZE_POINT,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_AUTOWIDTH,
    SID_ATTR_TRANSFORM_AUTOHEIGHT,
    0
};

static sal_uInt16 pAngleRanges[] =
{
    SID_ATTR_TRANSFORM_ROT_X,
    SID_ATTR_TRANSFORM_ANGLE,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    0
};

static sal_uInt16 pSlantRanges[] =
{
    SDRATTR_ECKENRADIUS,
    SDRATTR_ECKENRADIUS,
    SID_ATTR_TRANSFORM_SHEAR,
    SID_ATTR_TRANSFORM_SHEAR_VERTICAL,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    0
};

void lcl_ConvertRect(basegfx::B2DRange& rRange, const sal_uInt16 nDigits, const MapUnit ePoolUnit, const FieldUnit eDlgUnit)
{
    const basegfx::B2DPoint aTopLeft(
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinX()), nDigits, ePoolUnit, eDlgUnit),
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinY()), nDigits, ePoolUnit, eDlgUnit));
    const basegfx::B2DPoint aBottomRight(
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxX()), nDigits, ePoolUnit, eDlgUnit),
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxY()), nDigits, ePoolUnit, eDlgUnit));

    rRange = basegfx::B2DRange(aTopLeft, aBottomRight);
}

void lcl_ScaleRect(basegfx::B2DRange& rRange, const Fraction aUIScale)
{
    const double fFactor(1.0 / double(aUIScale));
    rRange = basegfx::B2DRange(rRange.getMinimum() * fFactor, rRange.getMaximum() * fFactor);
}

/*************************************************************************
|*
|* constructor of the tab dialog: adds the pages to the dialog
|*
\************************************************************************/

SvxTransformTabDialog::SvxTransformTabDialog( Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pSdrView, sal_uInt16 nAnchorTypes ) :
    SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_TRANSFORM ), pAttr ),
    pView       ( pSdrView ),
    nAnchorCtrls(nAnchorTypes)
{
    DBG_ASSERT(pView, "no valid view (!)");
    FreeResource();

    //different positioning page in Writer
    if(nAnchorCtrls & 0x00ff)
    {
        AddTabPage(RID_SVXPAGE_SWPOSSIZE, SvxSwPosSizeTabPage::Create, SvxSwPosSizeTabPage::GetRanges);
        RemoveTabPage(RID_SVXPAGE_POSITION_SIZE);
    }
    else
    {
        AddTabPage(RID_SVXPAGE_POSITION_SIZE, SvxPositionSizeTabPage::Create, SvxPositionSizeTabPage::GetRanges);
        RemoveTabPage(RID_SVXPAGE_SWPOSSIZE);
    }

    AddTabPage(RID_SVXPAGE_ANGLE, SvxAngleTabPage::Create, SvxAngleTabPage::GetRanges);
    AddTabPage(RID_SVXPAGE_SLANT, SvxSlantTabPage::Create, SvxSlantTabPage::GetRanges);
}

// -----------------------------------------------------------------------

SvxTransformTabDialog::~SvxTransformTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxTransformTabDialog::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    switch(nId)
    {
        case RID_SVXPAGE_POSITION_SIZE:
        {
            SvxPositionSizeTabPage& rSvxPos =  static_cast<SvxPositionSizeTabPage&>(rPage);
            rSvxPos.SetView(pView);
            rSvxPos.Construct();

            if(nAnchorCtrls & SVX_OBJ_NORESIZE)
            {
                rSvxPos.DisableResize();
            }

            if(nAnchorCtrls & SVX_OBJ_NOPROTECT)
            {
                rSvxPos.DisableProtect();
                rSvxPos.UpdateControlStates();
            }

            break;
        }
        case RID_SVXPAGE_SWPOSSIZE :
        {
            SvxSwPosSizeTabPage& rSwPos =  static_cast<SvxSwPosSizeTabPage&>(rPage);

            rSwPos.EnableAnchorTypes(nAnchorCtrls);
            rSwPos.SetValidateFramePosLink(aValidateLink);
            rSwPos.SetView(pView);

            break;
        }

        case RID_SVXPAGE_ANGLE:
        {
            SvxAngleTabPage& rSvxAng =  static_cast<SvxAngleTabPage&>(rPage);

            rSvxAng.SetView( pView );
            rSvxAng.Construct();

            break;
        }

        case RID_SVXPAGE_SLANT:
        {
            SvxSlantTabPage& rSvxSlnt =  static_cast<SvxSlantTabPage&>(rPage);

            rSvxSlnt.SetView( pView );
            rSvxSlnt.Construct();

            break;
        }
    }
}

// -----------------------------------------------------------------------

void SvxTransformTabDialog::SetValidateFramePosLink(const Link& rLink)
{
    aValidateLink = rLink;
}

/*************************************************************************
|*
|*      dialog for changing the positions of the rotation
|*      angle and the rotation angle of the graphic objects
|*
\************************************************************************/

SvxAngleTabPage::SvxAngleTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage              ( pParent, CUI_RES( RID_SVXPAGE_ANGLE ), rInAttrs ),
    aFlPosition             ( this, CUI_RES( FL_POSITION ) ),
    aFtPosX                 ( this, CUI_RES( FT_POS_X ) ),
    aMtrPosX                ( this, CUI_RES( MTR_FLD_POS_X ) ),
    aFtPosY                 ( this, CUI_RES( FT_POS_Y ) ),
    aMtrPosY                ( this, CUI_RES( MTR_FLD_POS_Y ) ),
    aFtPosPresets           ( this, CUI_RES(FT_POSPRESETS) ),
    aCtlRect                ( this, CUI_RES( CTL_RECT ) ),

    aFlAngle                ( this, CUI_RES( FL_ANGLE ) ),
    aFtAngle                ( this, CUI_RES( FT_ANGLE ) ),
    maNfAngle               ( this, CUI_RES( NF_ANGLE ) ),
    aFtAnglePresets         ( this, CUI_RES(FT_ANGLEPRESETS) ),
    aCtlAngle               ( this, CUI_RES( CTL_ANGLE ) ),
    rOutAttrs               ( rInAttrs )
{
    FreeResource();

    // calculate PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    ePoolUnit = pPool->GetMetric(SID_ATTR_TRANSFORM_POS_X);

    aCtlRect.SetAccessibleRelationLabeledBy(&aFtPosPresets);
    aCtlRect.SetAccessibleRelationMemberOf(&aFlPosition);
    aCtlAngle.SetAccessibleRelationLabeledBy(&aFtAnglePresets);
    aCtlAngle.SetAccessibleRelationMemberOf(&aFlAngle);
    aCtlAngle.SetLinkedField( &maNfAngle );
}

// -----------------------------------------------------------------------

void SvxAngleTabPage::Construct()
{
    DBG_ASSERT(pView, "No valid view (!)");
    eDlgUnit = GetModuleFieldUnit(GetItemSet());
    SetFieldUnit(aMtrPosX, eDlgUnit, sal_True);
    SetFieldUnit(aMtrPosY, eDlgUnit, sal_True);

    if(FUNIT_MILE == eDlgUnit || FUNIT_KM == eDlgUnit)
    {
        aMtrPosX.SetDecimalDigits( 3 );
        aMtrPosY.SetDecimalDigits( 3 );
    }

    { // #i75273#
        Rectangle aTempRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    // Take anchor into account (Writer)
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if(rMarkList.GetMarkCount())
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        maAnchor = basegfx::B2DPoint(pObj->GetAnchorPos().X(), pObj->GetAnchorPos().Y());

        if(!maAnchor.equalZero()) // -> Writer
        {
            maRange = basegfx::B2DRange(maRange.getMinimum() - maAnchor, maRange.getMaximum() - maAnchor);
        }
    }

    // take scale into account
    const Fraction aUIScale(pView->GetModel()->GetUIScale());
    lcl_ScaleRect(maRange, aUIScale);

    // take UI units into account
    sal_uInt16 nDigits(aMtrPosX.GetDecimalDigits());
    lcl_ConvertRect(maRange, nDigits, (MapUnit)ePoolUnit, eDlgUnit);

    if(!pView->IsRotateAllowed())
    {
        aFlPosition.Disable();
        aFtPosX.Disable();
        aMtrPosX.Disable();
        aFtPosY.Disable();
        aMtrPosY.Disable();
        aFtPosPresets.Disable();
        aCtlRect.Disable();
        aFlAngle.Disable();
        aFtAngle.Disable();
        maNfAngle.Disable();
        aFtAnglePresets.Disable();
        aCtlAngle.Disable();
    }
}

sal_Bool SvxAngleTabPage::FillItemSet(SfxItemSet& rSet)
{
    sal_Bool bModified = sal_False;

    if(aCtlAngle.IsValueModified() || aMtrPosX.IsValueModified() || aMtrPosY.IsValueModified())
    {
        const double fUIScale(double(pView->GetModel()->GetUIScale()));
        const double fTmpX((GetCoreValue(aMtrPosX, ePoolUnit) + maAnchor.getX()) * fUIScale);
        const double fTmpY((GetCoreValue(aMtrPosY, ePoolUnit) + maAnchor.getY()) * fUIScale);

        rSet.Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ANGLE), aCtlAngle.GetRotation()));
        rSet.Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ROT_X), basegfx::fround(fTmpX)));
        rSet.Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ROT_Y), basegfx::fround(fTmpY)));

        bModified = sal_True;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxAngleTabPage::Reset(const SfxItemSet& rAttrs)
{
    const double fUIScale(double(pView->GetModel()->GetUIScale()));

    const SfxPoolItem* pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_ROT_X );
    if(pItem)
    {
        const double fTmp(((double)((const SfxInt32Item*)pItem)->GetValue() - maAnchor.getX()) / fUIScale);
        SetMetricValue(aMtrPosX, basegfx::fround(fTmp), ePoolUnit);
    }
    else
    {
        aMtrPosX.SetText( String() );
    }

    pItem = GetItem(rAttrs, SID_ATTR_TRANSFORM_ROT_Y);
    if(pItem)
    {
        const double fTmp(((double)((const SfxInt32Item*)pItem)->GetValue() - maAnchor.getY()) / fUIScale);
        SetMetricValue(aMtrPosY, basegfx::fround(fTmp), ePoolUnit);
    }
    else
    {
        aMtrPosX.SetText( String() );
    }

    pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_ANGLE );
    if(pItem)
    {
        aCtlAngle.SetRotation(((const SfxInt32Item*)pItem)->GetValue());
    }
    else
    {
        aCtlAngle.SetRotation(0);
    }
    aCtlAngle.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* SvxAngleTabPage::Create( Window* pWindow, const SfxItemSet& rSet)
{
    return(new SvxAngleTabPage(pWindow, rSet));
}

//------------------------------------------------------------------------

sal_uInt16* SvxAngleTabPage::GetRanges()
{
    return(pAngleRanges);
}

// -----------------------------------------------------------------------

void SvxAngleTabPage::ActivatePage(const SfxItemSet& /*rSet*/)
{
}

// -----------------------------------------------------------------------

int SvxAngleTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(*_pSet);
    }

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

void SvxAngleTabPage::PointChanged(Window* pWindow, RECT_POINT eRP)
{
    if(pWindow == &aCtlRect)
    {
        switch(eRP)
        {
            case RP_LT:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RP_MT:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RP_RT:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RP_LM:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RP_MM:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RP_RM:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RP_LB:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RP_MB:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RP_RB:
            {
                aMtrPosX.SetUserValue( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                aMtrPosY.SetUserValue( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
        }
    }
}

/*************************************************************************
|*
|*      dialog for changing slant and corner radius
|*
\************************************************************************/

SvxSlantTabPage::SvxSlantTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage              ( pParent, CUI_RES( RID_SVXPAGE_SLANT ), rInAttrs ),

    aFlRadius               ( this, CUI_RES( FL_RADIUS ) ),
    aFtRadius               ( this, CUI_RES( FT_RADIUS ) ),
    aMtrRadius              ( this, CUI_RES( MTR_FLD_RADIUS ) ),
    aFlAngle                ( this, CUI_RES( FL_SLANT ) ),
    aFtAngle                ( this, CUI_RES( FT_ANGLE ) ),
    aMtrAngle               ( this, CUI_RES( MTR_FLD_ANGLE ) ),
    rOutAttrs               ( rInAttrs )
{
    FreeResource();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );
}

// -----------------------------------------------------------------------

void SvxSlantTabPage::Construct()
{
    // get the range
    DBG_ASSERT(pView, "no valid view (!)");
    eDlgUnit = GetModuleFieldUnit(GetItemSet());
    SetFieldUnit(aMtrRadius, eDlgUnit, sal_True);

    { // #i75273#
        Rectangle aTempRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }
}

// -----------------------------------------------------------------------

sal_Bool SvxSlantTabPage::FillItemSet(SfxItemSet& rAttrs)
{
    sal_Bool  bModified = sal_False;
    String aStr = aMtrRadius.GetText();

    if( aStr != aMtrRadius.GetSavedValue() )
    {
        Fraction aUIScale = pView->GetModel()->GetUIScale();
        long nTmp = GetCoreValue( aMtrRadius, ePoolUnit );
        nTmp = Fraction( nTmp ) * aUIScale;

        rAttrs.Put( SdrEckenradiusItem( nTmp ) );
        bModified = sal_True;
    }

    aStr = aMtrAngle.GetText();

    if( aStr != aMtrAngle.GetSavedValue() )
    {
        sal_Int32 nValue = static_cast<sal_Int32>(aMtrAngle.GetValue());
        rAttrs.Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR, nValue ) );
        bModified = sal_True;
    }

    if( bModified )
    {
        // set reference points
        // #75897#
        Rectangle aObjectRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aObjectRect);
        Point aPt = aObjectRect.Center();

        rAttrs.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X, aPt.X()));
        rAttrs.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y, aPt.Y()));
        rAttrs.Put( SfxBoolItem( SID_ATTR_TRANSFORM_SHEAR_VERTICAL, sal_False ) );
    }

    return( bModified );
}

// -----------------------------------------------------------------------

void SvxSlantTabPage::Reset(const SfxItemSet& rAttrs)
{
    // if the view has selected objects, items with SFX_ITEM_DEFAULT need to be disabled
    const SfxPoolItem* pItem;

    // corner radius
    if(!pView->IsEdgeRadiusAllowed())
    {
        aFlRadius.Disable();
        aFtRadius.Disable();
        aMtrRadius.Disable();
        aMtrRadius.SetText( String() );
    }
    else
    {
        pItem = GetItem( rAttrs, SDRATTR_ECKENRADIUS );

        if( pItem )
        {
            const double fUIScale(double(pView->GetModel()->GetUIScale()));
            const double fTmp((double)((const SdrEckenradiusItem*)pItem)->GetValue() / fUIScale);
            SetMetricValue(aMtrRadius, basegfx::fround(fTmp), ePoolUnit);
        }
        else
        {
            aMtrRadius.SetText( String() );
        }
    }

    aMtrRadius.SaveValue();

    // slant: angle
    if( !pView->IsShearAllowed() )
    {
        aFlAngle.Disable();
        aFtAngle.Disable();
        aMtrAngle.Disable();
        aMtrAngle.SetText( String() );
    }
    else
    {
        pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_SHEAR );

        if( pItem )
        {
            aMtrAngle.SetValue( ( (const SfxInt32Item*)pItem )->GetValue() );
        }
        else
        {
            aMtrAngle.SetText( String() );
        }
    }

    aMtrAngle.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* SvxSlantTabPage::Create( Window* pWindow, const SfxItemSet& rOutAttrs )
{
    return( new SvxSlantTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

sal_uInt16* SvxSlantTabPage::GetRanges()
{
    return( pSlantRanges );
}

// -----------------------------------------------------------------------

void SvxSlantTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem* pRectItem = NULL;

    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , sal_False, (const SfxPoolItem**) &pRectItem ) )
    {
        const Rectangle aTempRect(pRectItem->GetValue());
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }
}

// -----------------------------------------------------------------------

int SvxSlantTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(*_pSet);
    }

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

void SvxSlantTabPage::PointChanged( Window* , RECT_POINT  )
{
}

/*************************************************************************
|*
|*      Dialog for changing position and size of graphic objects
|*
\************************************************************************/

SvxPositionSizeTabPage::SvxPositionSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage      ( pParent, CUI_RES( RID_SVXPAGE_POSITION_SIZE ), rInAttrs ),
    maFlPosition        ( this, CUI_RES( FL_POSITION ) ),
    maFtPosX            ( this, CUI_RES( FT_POS_X ) ),
    maMtrPosX           ( this, CUI_RES( MTR_FLD_POS_X ) ),
    maFtPosY            ( this, CUI_RES( FT_POS_Y ) ),
    maMtrPosY           ( this, CUI_RES( MTR_FLD_POS_Y ) ),
    maFtPosReference    ( this, CUI_RES( FT_POSREFERENCE ) ),
    maCtlPos            ( this, CUI_RES( CTL_POSRECT ), RP_LT ),

    maFlSize                         ( this, CUI_RES( FL_SIZE ) ),
    maFtWidth                        ( this, CUI_RES( FT_WIDTH ) ),
    maMtrWidth                       ( this, CUI_RES( MTR_FLD_WIDTH ) ),
    maFtHeight                       ( this, CUI_RES( FT_HEIGHT ) ),
    maMtrHeight                      ( this, CUI_RES( MTR_FLD_HEIGHT ) ),
    maCbxScale                       ( this, CUI_RES( CBX_SCALE ) ),
    maFtSizeReference                ( this, CUI_RES( FT_SIZEREFERENCE) ),
    maCtlSize                        ( this, CUI_RES( CTL_SIZERECT ), RP_LT ),

    maFlProtect         ( this, CUI_RES( FL_PROTECT) ),
    maTsbPosProtect     ( this, CUI_RES( TSB_POSPROTECT ) ),
    maTsbSizeProtect                 ( this, CUI_RES( TSB_SIZEPROTECT ) ),


    maFlAdjust                       ( this, CUI_RES( FL_ADJUST ) ),
    maTsbAutoGrowWidth              ( this, CUI_RES( TSB_AUTOGROW_WIDTH ) ),
    maTsbAutoGrowHeight             ( this, CUI_RES( TSB_AUTOGROW_HEIGHT ) ),

    maFlDivider                     (this, CUI_RES( FL_DIVIDER ) ),

    mrOutAttrs       ( rInAttrs ),
    mnProtectSizeState( STATE_NOCHECK ),
    mbPageDisabled   ( sal_False ),
    mbProtectDisabled( false ),
    mbSizeDisabled( false ),
    mbAdjustDisabled( true )
{
    FreeResource();

    // this pege needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = mrOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    meRP = RP_LT; // see above

    maMtrWidth.SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeWidthHdl ) );
    maMtrHeight.SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeHeightHdl ) );
    maCbxScale.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickAutoHdl ) );

    maTsbAutoGrowWidth.Disable();
    maTsbAutoGrowHeight.Disable();
    maFlAdjust.Disable();

    // #i2379# disable controls when protected
    maTsbPosProtect.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangePosProtectHdl ) );
    maTsbSizeProtect.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangeSizeProtectHdl ) );

    maCtlPos.SetAccessibleRelationMemberOf( &maFlPosition );
    maCtlSize.SetAccessibleRelationMemberOf( &maFlSize );
    maCtlPos.SetAccessibleRelationLabeledBy( &maFtPosReference );
    maCtlSize.SetAccessibleRelationLabeledBy( &maFtSizeReference );
}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::Construct()
{
    // get range and work area
    DBG_ASSERT( mpView, "no valid view (!)" );
    meDlgUnit = GetModuleFieldUnit( GetItemSet() );
    SetFieldUnit( maMtrPosX, meDlgUnit, sal_True );
    SetFieldUnit( maMtrPosY, meDlgUnit, sal_True );
    SetFieldUnit( maMtrWidth, meDlgUnit, sal_True );
    SetFieldUnit( maMtrHeight, meDlgUnit, sal_True );

    if(FUNIT_MILE == meDlgUnit || FUNIT_KM == meDlgUnit)
    {
        maMtrPosX.SetDecimalDigits( 3 );
        maMtrPosY.SetDecimalDigits( 3 );
        maMtrWidth.SetDecimalDigits( 3 );
        maMtrHeight.SetDecimalDigits( 3 );
    }

    { // #i75273#
        Rectangle aTempRect(mpView->GetAllMarkedRect());
        mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    { // #i75273#
        Rectangle aTempRect(mpView->GetWorkArea());
        mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maWorkRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    // take anchor into account (Writer)
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if(rMarkList.GetMarkCount())
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        maAnchor = basegfx::B2DPoint(pObj->GetAnchorPos().X(), pObj->GetAnchorPos().Y());

        if(!maAnchor.equalZero()) // -> Writer
        {
            for(sal_uInt16 i(1); i < rMarkList.GetMarkCount(); i++)
            {
                pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                if(maAnchor != basegfx::B2DPoint(pObj->GetAnchorPos().X(), pObj->GetAnchorPos().Y()))
                {
                    // diferent anchor positions
                    maMtrPosX.SetText( String() );
                    maMtrPosY.SetText( String() );
                    mbPageDisabled = sal_True;
                    return;
                }
            }

            // translate ranges about anchor
            maRange = basegfx::B2DRange(maRange.getMinimum() - maAnchor, maRange.getMaximum() - maAnchor);
            maWorkRange = basegfx::B2DRange(maWorkRange.getMinimum() - maAnchor, maWorkRange.getMaximum() - maAnchor);
        }
    }

    // this should happen via SID_ATTR_TRANSFORM_AUTOSIZE
    if(1 == rMarkList.GetMarkCount())
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        const SdrObjKind eKind((SdrObjKind)pObj->GetObjIdentifier());

        if((pObj->GetObjInventor() == SdrInventor) && (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind) && ((SdrTextObj*)pObj)->HasText())
        {
            mbAdjustDisabled = false;
            maFlAdjust.Enable();
            maTsbAutoGrowWidth.Enable();
            maTsbAutoGrowHeight.Enable();
            maTsbAutoGrowWidth.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );
            maTsbAutoGrowHeight.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );

            // is used as flag to evaluate if its selectable
            maTsbAutoGrowWidth.EnableTriState( sal_False );
            maTsbAutoGrowHeight.EnableTriState( sal_False );
        }
    }

    // take scale into account
    const Fraction aUIScale(mpView->GetModel()->GetUIScale());
    lcl_ScaleRect( maWorkRange, aUIScale );
    lcl_ScaleRect( maRange, aUIScale );

    // take UI units into account
    const sal_uInt16 nDigits(maMtrPosX.GetDecimalDigits());
    lcl_ConvertRect( maWorkRange, nDigits, (MapUnit) mePoolUnit, meDlgUnit );
    lcl_ConvertRect( maRange, nDigits, (MapUnit) mePoolUnit, meDlgUnit );

    SetMinMaxPosition();
}

// -----------------------------------------------------------------------

sal_Bool SvxPositionSizeTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    sal_Bool bModified(sal_False);

    if ( maMtrWidth.HasFocus() )
    {
        ChangeWidthHdl( this );
    }

    if ( maMtrHeight.HasFocus() )
    {
        ChangeHeightHdl( this );
    }

    if( !mbPageDisabled )
    {
        if ( maMtrPosX.IsValueModified() || maMtrPosY.IsValueModified() )
        {
            const double fUIScale(double(mpView->GetModel()->GetUIScale()));
            double fX((GetCoreValue( maMtrPosX, mePoolUnit ) + maAnchor.getX()) * fUIScale);
            double fY((GetCoreValue( maMtrPosY, mePoolUnit ) + maAnchor.getY()) * fUIScale);

            { // #i75273#
                Rectangle aTempRect(mpView->GetAllMarkedRect());
                mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
                maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
            }

            // #101581# GetTopLeftPosition(...) needs coordinates after UI scaling, in real PagePositions
            GetTopLeftPosition(fX, fY, maRange);

            rOutAttrs.Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_POS_X), basegfx::fround(fX)));
            rOutAttrs.Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_POS_Y), basegfx::fround(fY)));

            bModified |= sal_True;
        }

        if ( maTsbPosProtect.GetState() != maTsbPosProtect.GetSavedValue() )
        {
            if( maTsbPosProtect.GetState() == STATE_DONTKNOW )
            {
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
            }
            else
            {
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                    maTsbPosProtect.GetState() == STATE_CHECK ? sal_True : sal_False ) );
            }

            bModified |= sal_True;
        }
    }

    if ( maMtrWidth.IsValueModified() || maMtrHeight.IsValueModified() )
    {
        Fraction aUIScale = mpView->GetModel()->GetUIScale();

        // get Width
        double nWidth = static_cast<double>(maMtrWidth.GetValue( meDlgUnit ));
        nWidth = MetricField::ConvertDoubleValue( nWidth, maMtrWidth.GetBaseValue(), maMtrWidth.GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lWidth = long(nWidth * (double)aUIScale);
        lWidth = OutputDevice::LogicToLogic( lWidth, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lWidth = static_cast<long>(maMtrWidth.Denormalize( lWidth ));

        // get Height
        double nHeight = static_cast<double>(maMtrHeight.GetValue( meDlgUnit ));
        nHeight = MetricField::ConvertDoubleValue( nHeight, maMtrHeight.GetBaseValue(), maMtrHeight.GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lHeight = long(nHeight * (double)aUIScale);
        lHeight = OutputDevice::LogicToLogic( lHeight, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lHeight = static_cast<long>(maMtrWidth.Denormalize( lHeight ));

        // put Width & Height to itemset
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ),
                        (sal_uInt32) lWidth ) );
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ),
                        (sal_uInt32) lHeight ) );
        rOutAttrs.Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), sal::static_int_cast< sal_uInt16 >( meRP ) ) );
        bModified |= sal_True;
    }

    if ( maTsbSizeProtect.GetState() != maTsbSizeProtect.GetSavedValue() )
    {
        if ( maTsbSizeProtect.GetState() == STATE_DONTKNOW )
            rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rOutAttrs.Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                maTsbSizeProtect.GetState() == STATE_CHECK ? sal_True : sal_False ) );
        bModified |= sal_True;
    }

    if ( maTsbAutoGrowWidth.GetState() != maTsbAutoGrowWidth.GetSavedValue() )
    {
        if ( !maTsbAutoGrowWidth.IsTriStateEnabled() )
        {
            if( maTsbAutoGrowWidth.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOWIDTH );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOWIDTH ),
                    maTsbAutoGrowWidth.GetState() == STATE_CHECK ? sal_True : sal_False ) );
        }
        bModified |= sal_True;
    }

    if ( maTsbAutoGrowHeight.GetState() != maTsbAutoGrowHeight.GetSavedValue() )
    {
        if ( !maTsbAutoGrowHeight.IsTriStateEnabled() )
        {
            if( maTsbAutoGrowHeight.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOHEIGHT );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOHEIGHT ),
                    maTsbAutoGrowHeight.GetState() == STATE_CHECK ? sal_True : sal_False ) );
        }
        bModified |= sal_True;
    }


    return bModified;
}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::Reset( const SfxItemSet&  )
{
    const SfxPoolItem* pItem;
    const double fUIScale(double(mpView->GetModel()->GetUIScale()));

    if ( !mbPageDisabled )
    {
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_X );
        if ( pItem )
        {
            const double fTmp((((const SfxInt32Item*)pItem)->GetValue() - maAnchor.getX()) / fUIScale);
            SetMetricValue(maMtrPosX, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_Y );
        if ( pItem )
        {
            const double fTmp((((const SfxInt32Item*)pItem)->GetValue() - maAnchor.getY()) / fUIScale);
            SetMetricValue(maMtrPosY, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_POS );
        if ( pItem )
        {
            sal_Bool bProtected = ( ( const SfxBoolItem* )pItem )->GetValue();
            maTsbPosProtect.SetState( bProtected ? STATE_CHECK : STATE_NOCHECK );
            maTsbPosProtect.EnableTriState( sal_False );
        }
        else
        {
            maTsbPosProtect.SetState( STATE_DONTKNOW );
        }

        maTsbPosProtect.SaveValue();
        maCtlPos.Reset();

        // #i2379# Disable controls for protected objects
        ChangePosProtectHdl( this );
    }

    { // #i75273# set width
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_WIDTH );
        mfOldWidth = std::max( pItem ? (double)((const SfxUInt32Item*)pItem)->GetValue() : 0.0, 1.0 );
        double fTmpWidth((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldWidth), (MapUnit)mePoolUnit, MAP_100TH_MM)) / fUIScale);

        if(maMtrWidth.GetDecimalDigits())
            fTmpWidth *= pow(10.0, maMtrWidth.GetDecimalDigits());

        fTmpWidth = MetricField::ConvertDoubleValue(fTmpWidth, maMtrWidth.GetBaseValue(), maMtrWidth.GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
        maMtrWidth.SetValue(static_cast<sal_Int64>(fTmpWidth), meDlgUnit);
    }

    { // #i75273# set height
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_HEIGHT );
        mfOldHeight = std::max( pItem ? (double)((const SfxUInt32Item*)pItem)->GetValue() : 0.0, 1.0 );
        double fTmpHeight((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldHeight), (MapUnit)mePoolUnit, MAP_100TH_MM)) / fUIScale);

        if(maMtrHeight.GetDecimalDigits())
            fTmpHeight *= pow(10.0, maMtrHeight.GetDecimalDigits());

        fTmpHeight = MetricField::ConvertDoubleValue(fTmpHeight, maMtrHeight.GetBaseValue(), maMtrHeight.GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
        maMtrHeight.SetValue(static_cast<sal_Int64>(fTmpHeight), meDlgUnit);
    }

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_SIZE );
    if ( pItem )
    {
        maTsbSizeProtect.SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                              ? STATE_CHECK : STATE_NOCHECK );
        maTsbSizeProtect.EnableTriState( sal_False );
    }
    else
        maTsbSizeProtect.SetState( STATE_DONTKNOW );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOWIDTH );
    if ( pItem )
    {
        maTsbAutoGrowWidth.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
    }
    else
        maTsbAutoGrowWidth.SetState( STATE_DONTKNOW );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOHEIGHT );
    if ( pItem )
    {
        maTsbAutoGrowHeight.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
    }
    else
        maTsbAutoGrowHeight.SetState( STATE_DONTKNOW );

    // Is matching set?
    String aStr = GetUserData();
    maCbxScale.Check( (sal_Bool)aStr.ToInt32() );

    maTsbSizeProtect.SaveValue();
    maTsbAutoGrowWidth.SaveValue();
    maTsbAutoGrowHeight.SaveValue();
    ClickSizeProtectHdl( NULL );

    // #i2379# Disable controls for protected objects
    ChangeSizeProtectHdl( this );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxPositionSizeTabPage::Create( Window* pWindow, const SfxItemSet& rOutAttrs )
{
    return( new SvxPositionSizeTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

sal_uInt16* SvxPositionSizeTabPage::GetRanges()
{
    return( pPosSizeRanges );
}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem* pRectItem = NULL;

    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , sal_False, (const SfxPoolItem**) &pRectItem ) )
    {
        { // #i75273#
            const Rectangle aTempRect(pRectItem->GetValue());
            maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
        }

        SetMinMaxPosition();
    }
}

// -----------------------------------------------------------------------

int SvxPositionSizeTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
    {
        double fX((double)maMtrPosX.GetValue());
        double fY((double)maMtrPosY.GetValue());

        GetTopLeftPosition(fX, fY, maRange);
        const Rectangle aOutRectangle(
            basegfx::fround(fX), basegfx::fround(fY),
            basegfx::fround(fX + maRange.getWidth()), basegfx::fround(fY + maRange.getHeight()));
        _pSet->Put(SfxRectangleItem(SID_ATTR_TRANSFORM_INTERN, aOutRectangle));

        FillItemSet(*_pSet);
    }

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangePosProtectHdl)
{
    // #106572# Remember user's last choice
    maTsbSizeProtect.SetState( maTsbPosProtect.GetState() == STATE_CHECK ?  STATE_CHECK : mnProtectSizeState );
    UpdateControlStates();
    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::UpdateControlStates()
{
    const bool bPosProtect =  maTsbPosProtect.GetState() == STATE_CHECK;
    const bool bSizeProtect = maTsbSizeProtect.GetState() == STATE_CHECK;
    const bool bHeightChecked = !maTsbAutoGrowHeight.IsTriStateEnabled() && (maTsbAutoGrowHeight.GetState() == STATE_CHECK);
    const bool bWidthChecked = !maTsbAutoGrowWidth.IsTriStateEnabled() && (maTsbAutoGrowWidth.GetState() == STATE_CHECK);

    maFlPosition.Enable( !bPosProtect && !mbPageDisabled );
    maFtPosX.Enable( !bPosProtect && !mbPageDisabled );
    maMtrPosX.Enable( !bPosProtect && !mbPageDisabled );
    maFtPosY.Enable( !bPosProtect && !mbPageDisabled );
    maMtrPosY.Enable( !bPosProtect && !mbPageDisabled );
    maFtPosReference.Enable( !bPosProtect && !mbPageDisabled );
    maCtlPos.Enable( !bPosProtect );
    maTsbPosProtect.Enable( !mbProtectDisabled && !mbPageDisabled );

    maFlSize.Enable( !mbSizeDisabled && !bSizeProtect );
    maCtlSize.Enable( !mbSizeDisabled && !bSizeProtect && (!bHeightChecked || !bWidthChecked) );
    maFtWidth.Enable( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );
    maMtrWidth.Enable( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );
    maFtHeight.Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );
    maMtrHeight.Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );
    maCbxScale.Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked && !bWidthChecked );
    maFtSizeReference.Enable( !mbSizeDisabled && !bSizeProtect );
    maFlProtect.Enable( !mbProtectDisabled );
    maTsbSizeProtect.Enable( !mbProtectDisabled && !bPosProtect );

    maFlAdjust.Enable( !mbSizeDisabled && !bSizeProtect && !mbAdjustDisabled );
    maTsbAutoGrowWidth.Enable( !mbSizeDisabled && !bSizeProtect && !mbAdjustDisabled );
    maTsbAutoGrowHeight.Enable( !mbSizeDisabled && !bSizeProtect && !mbAdjustDisabled );

    maCtlSize.Invalidate();
    maCtlPos.Invalidate();

}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeSizeProtectHdl)
{
    if( maTsbSizeProtect.IsEnabled() )
    {
        // #106572# Remember user's last choice

        // Note: this works only as long as the dialog is open.  When
        // the user closes the dialog, there is no way to remember
        // whether size was enabled or disabled befor pos protect was
        // clicked. Thus, if pos protect is selected, the dialog is
        // closed and reopened again, unchecking pos protect will
        // always uncheck size protect, too. That's life.
        mnProtectSizeState = maTsbSizeProtect.GetState();
    }

    UpdateControlStates();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::SetMinMaxPosition()
{
    // position
    double fLeft(maWorkRange.getMinX());
    double fTop(maWorkRange.getMinY());
    double fRight(maWorkRange.getMaxX());
    double fBottom(maWorkRange.getMaxY());

    switch ( maCtlPos.GetActualRP() )
    {
        case RP_LT:
        {
            fRight  -= maRange.getWidth();
            fBottom -= maRange.getHeight();
            break;
        }
        case RP_MT:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fBottom -= maRange.getHeight();
            break;
        }
        case RP_RT:
        {
            fLeft   += maRange.getWidth();
            fBottom -= maRange.getHeight();
            break;
        }
        case RP_LM:
        {
            fRight  -= maRange.getWidth();
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RP_MM:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RP_RM:
        {
            fLeft   += maRange.getWidth();
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RP_LB:
        {
            fRight  -= maRange.getWidth();
            fTop    += maRange.getHeight();
            break;
        }
        case RP_MB:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fTop    += maRange.getHeight();
            break;
        }
        case RP_RB:
        {
            fLeft   += maRange.getWidth();
            fTop    += maRange.getHeight();
            break;
        }
    }

    const double fMaxLong((double)(MetricField::ConvertValue( LONG_MAX, 0, MAP_100TH_MM, meDlgUnit ) - 1L));
    fLeft = (fLeft > fMaxLong) ? fMaxLong : (fLeft < -fMaxLong) ? -fMaxLong : fLeft;
    fRight = (fRight > fMaxLong) ? fMaxLong : (fRight < -fMaxLong) ? -fMaxLong : fRight;
    fTop = (fTop > fMaxLong) ? fMaxLong : (fTop < -fMaxLong) ? -fMaxLong : fTop;
    fBottom = (fBottom > fMaxLong) ? fMaxLong : (fBottom < -fMaxLong) ? -fMaxLong : fBottom;

    // #i75273# normalizing when setting the min/max values was wrong, removed
    maMtrPosX.SetMin(basegfx::fround64(fLeft));
    maMtrPosX.SetFirst(basegfx::fround64(fLeft));
    maMtrPosX.SetMax(basegfx::fround64(fRight));
    maMtrPosX.SetLast(basegfx::fround64(fRight));
    maMtrPosY.SetMin(basegfx::fround64(fTop));
    maMtrPosY.SetFirst(basegfx::fround64(fTop));
    maMtrPosY.SetMax(basegfx::fround64(fBottom));
    maMtrPosY.SetLast(basegfx::fround64(fBottom));

    // size
    fLeft = maWorkRange.getMinX();
    fTop = maWorkRange.getMinY();
    fRight = maWorkRange.getMaxX();
    fBottom = maWorkRange.getMaxY();
    double fNewX(0);
    double fNewY(0);

    switch ( maCtlSize.GetActualRP() )
    {
        case RP_LT:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RP_MT:
        {
            fNewX = std::min( maRange.getCenter().getX() - fLeft, fRight - maRange.getCenter().getX() ) * 2.0;
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RP_RT:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RP_LM:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = std::min( maRange.getCenter().getY() - fTop, fBottom - maRange.getCenter().getY() ) * 2.0;
            break;
        }
        case RP_MM:
        {
            const double f1(maRange.getCenter().getX() - fLeft);
            const double f2(fRight - maRange.getCenter().getX());
            const double f3(std::min(f1, f2));
            const double f4(maRange.getCenter().getY() - fTop);
            const double f5(fBottom - maRange.getCenter().getY());
            const double f6(std::min(f4, f5));

            fNewX = f3 * 2.0;
            fNewY = f6 * 3.0;

            break;
        }
        case RP_RM:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = std::min( maRange.getCenter().getY() - fTop, fBottom - maRange.getCenter().getY() ) * 2.0;
            break;
        }
        case RP_LB:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = maWorkRange.getHeight() - ( fBottom - maRange.getMaxY() );
            break;
        }
        case RP_MB:
        {
            fNewX = std::min( maRange.getCenter().getX() - fLeft, fRight - maRange.getCenter().getX() ) * 2.0;
            fNewY = maWorkRange.getHeight() - ( maRange.getMaxY() - fBottom );
            break;
        }
        case RP_RB:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = maWorkRange.getHeight() - ( fBottom - maRange.getMaxY() );
            break;
        }
    }

    // #i75273# normalizing when setting the min/max values was wrong, removed
    maMtrWidth.SetMax(basegfx::fround64(fNewX));
    maMtrWidth.SetLast(basegfx::fround64(fNewX));
    maMtrHeight.SetMax(basegfx::fround64(fNewY));
    maMtrHeight.SetLast(basegfx::fround64(fNewY));
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange)
{
    switch (maCtlPos.GetActualRP())
    {
        case RP_LT:
        {
            break;
        }
        case RP_MT:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            break;
        }
        case RP_RT:
        {
            rfX -= rRange.getWidth();
            break;
        }
        case RP_LM:
        {
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RP_MM:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RP_RM:
        {
            rfX -= rRange.getWidth();
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RP_LB:
        {
            rfY -= rRange.getHeight();
            break;
        }
        case RP_MB:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            rfY -= rRange.getHeight();
            break;
        }
        case RP_RB:
        {
            rfX -= rRange.getWidth();
            rfY -= rRange.getHeight();
            break;
        }
    }
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    if( pWindow == &maCtlPos )
    {
        SetMinMaxPosition();
        switch( eRP )
        {
            case RP_LT:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getMinX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_MT:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_RT:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getMaxX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_LM:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getMinX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_MM:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_RM:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getMaxX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_LB:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getMinX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
            case RP_MB:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
            case RP_RB:
            {
                maMtrPosX.SetValue( basegfx::fround64(maRange.getMaxX()) );
                maMtrPosY.SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
        }
    }
    else
    {
        meRP = eRP;
        SetMinMaxPosition();
    }
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::DisableResize()
{
    mbSizeDisabled = true;
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::DisableProtect()
{
    mbProtectDisabled = true;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeWidthHdl)
{
    if( maCbxScale.IsChecked() && maCbxScale.IsEnabled() )
    {
        sal_Int64 nHeight(basegfx::fround64((mfOldHeight * (double)maMtrWidth.GetValue()) / mfOldWidth));

        if(nHeight <= maMtrHeight.GetMax(FUNIT_NONE))
        {
            maMtrHeight.SetUserValue(nHeight, FUNIT_NONE);
        }
        else
        {
            nHeight = maMtrHeight.GetMax(FUNIT_NONE);
            maMtrHeight.SetUserValue(nHeight);

            const sal_Int64 nWidth(basegfx::fround64((mfOldWidth * (double)nHeight) / mfOldHeight));
            maMtrWidth.SetUserValue(nWidth, FUNIT_NONE);
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeHeightHdl)
{
    if( maCbxScale.IsChecked() && maCbxScale.IsEnabled() )
    {
        sal_Int64 nWidth(basegfx::fround64((mfOldWidth * (double)maMtrHeight.GetValue()) / mfOldHeight));

        if(nWidth <= maMtrWidth.GetMax(FUNIT_NONE))
        {
            maMtrWidth.SetUserValue(nWidth, FUNIT_NONE);
        }
        else
        {
            nWidth = maMtrWidth.GetMax(FUNIT_NONE);
            maMtrWidth.SetUserValue(nWidth);

            const sal_Int64 nHeight(basegfx::fround64((mfOldHeight * (double)nWidth) / mfOldWidth));
            maMtrHeight.SetUserValue(nHeight, FUNIT_NONE);
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ClickSizeProtectHdl)
{
    UpdateControlStates();
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ClickAutoHdl)
{
    if( maCbxScale.IsChecked() )
    {
        mfOldWidth  = std::max( (double)GetCoreValue( maMtrWidth,  mePoolUnit ), 1.0 );
        mfOldHeight = std::max( (double)GetCoreValue( maMtrHeight, mePoolUnit ), 1.0 );
    }

    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::FillUserData()
{
    // matching is saved in the Ini-file
    UniString aStr = UniString::CreateFromInt32( (sal_Int32) maCbxScale.IsChecked() );
    SetUserData( aStr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
