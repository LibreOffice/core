/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

static void lcl_ConvertRect(basegfx::B2DRange& rRange, const sal_uInt16 nDigits, const MapUnit ePoolUnit, const FieldUnit eDlgUnit)
{
    const basegfx::B2DPoint aTopLeft(
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinX()), nDigits, ePoolUnit, eDlgUnit),
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMinY()), nDigits, ePoolUnit, eDlgUnit));
    const basegfx::B2DPoint aBottomRight(
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxX()), nDigits, ePoolUnit, eDlgUnit),
        (double)MetricField::ConvertValue(basegfx::fround(rRange.getMaxY()), nDigits, ePoolUnit, eDlgUnit));

    rRange = basegfx::B2DRange(aTopLeft, aBottomRight);
}

static void lcl_ScaleRect(basegfx::B2DRange& rRange, const Fraction aUIScale)
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
    aCtlAngle.SetLinkedField( &maNfAngle, 2 );
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
        aMtrPosY.SetText( String() );
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
    SvxTabPage      ( pParent
                     ,"PositionAndSize"
                     ,"cui/ui/possizetabpage.ui"
                     , rInAttrs ),

    mrOutAttrs       ( rInAttrs ),
    mnProtectSizeState( STATE_NOCHECK ),
    mbPageDisabled   ( sal_False ),
    mbProtectDisabled( false ),
    mbSizeDisabled( false ),
    mbAdjustDisabled( true )
{

    get(m_pFlPosition, "FL_POSITION");
    get(m_pMtrPosX, "MTR_FLD_POS_X");
    get(m_pMtrPosY, "MTR_FLD_POS_Y");
    get(m_pCtlPos, "CTL_POSRECT" );

    get(m_pFlSize, "FL_SIZE");
    get(m_pFtWidth, "FT_WIDTH");
    get(m_pMtrWidth, "MTR_FLD_WIDTH");
    get(m_pFtHeight, "FT_HEIGHT");
    get(m_pMtrHeight, "MTR_FLD_HEIGHT");
    get(m_pCbxScale, "CBX_SCALE");
    get(m_pCtlSize, "CTL_SIZERECT" );

    get(m_pFlProtect, "FL_PROTECT");
    get(m_pTsbPosProtect, "TSB_POSPROTECT");
    get(m_pTsbSizeProtect, "TSB_SIZEPROTECT");

    get(m_pFlAdjust, "FL_ADJUST");
    get(m_pTsbAutoGrowWidth, "TSB_AUTOGROW_WIDTH");
    get(m_pTsbAutoGrowHeight, "TSB_AUTOGROW_HEIGHT");

    // this pege needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = mrOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    m_pCtlPos->SetActualRP(RP_LT);
    m_pCtlSize->SetActualRP(RP_LT);
    meRP = RP_LT; // see above

    m_pMtrWidth->SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeWidthHdl ) );
    m_pMtrHeight->SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeHeightHdl ) );
    m_pCbxScale->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickAutoHdl ) );

    m_pTsbAutoGrowWidth->Disable();
    m_pTsbAutoGrowHeight->Disable();
    m_pFlAdjust->Disable();

    // #i2379# disable controls when protected
    m_pTsbPosProtect->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangePosProtectHdl ) );
    m_pTsbSizeProtect->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangeSizeProtectHdl ) );

}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::Construct()
{
    // get range and work area
    DBG_ASSERT( mpView, "no valid view (!)" );
    meDlgUnit = GetModuleFieldUnit( GetItemSet() );
    SetFieldUnit( *m_pMtrPosX, meDlgUnit, sal_True );
    SetFieldUnit( *m_pMtrPosY, meDlgUnit, sal_True );
    SetFieldUnit( *m_pMtrWidth, meDlgUnit, sal_True );
    SetFieldUnit( *m_pMtrHeight, meDlgUnit, sal_True );

    if(FUNIT_MILE == meDlgUnit || FUNIT_KM == meDlgUnit)
    {
        m_pMtrPosX->SetDecimalDigits( 3 );
        m_pMtrPosY->SetDecimalDigits( 3 );
        m_pMtrWidth->SetDecimalDigits( 3 );
        m_pMtrHeight->SetDecimalDigits( 3 );
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
                    m_pMtrPosX->SetText( "" );
                    m_pMtrPosY->SetText( "" );
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

        if((pObj->GetObjInventor() == SdrInventor) &&
            (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind) &&
            pObj->HasText())
        {
            mbAdjustDisabled = false;

            m_pFlAdjust->Enable();

            m_pTsbAutoGrowWidth->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );
            m_pTsbAutoGrowHeight->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );

            // is used as flag to evaluate if its selectable
            m_pTsbAutoGrowWidth->EnableTriState( sal_False );
            m_pTsbAutoGrowHeight->EnableTriState( sal_False );
        }
    }

    // take scale into account
    const Fraction aUIScale(mpView->GetModel()->GetUIScale());
    lcl_ScaleRect( maWorkRange, aUIScale );
    lcl_ScaleRect( maRange, aUIScale );

    // take UI units into account
    const sal_uInt16 nDigits(m_pMtrPosX->GetDecimalDigits());
    lcl_ConvertRect( maWorkRange, nDigits, (MapUnit) mePoolUnit, meDlgUnit );
    lcl_ConvertRect( maRange, nDigits, (MapUnit) mePoolUnit, meDlgUnit );

    SetMinMaxPosition();
}

// -----------------------------------------------------------------------

sal_Bool SvxPositionSizeTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    sal_Bool bModified(sal_False);

    if ( m_pMtrWidth->HasFocus() )
    {
        ChangeWidthHdl( this );
    }

    if ( m_pMtrHeight->HasFocus() )
    {
        ChangeHeightHdl( this );
    }

    if( !mbPageDisabled )
    {
        if ( m_pMtrPosX->IsValueModified() || m_pMtrPosY->IsValueModified() )
        {
            const double fUIScale(double(mpView->GetModel()->GetUIScale()));
            double fX((GetCoreValue( *m_pMtrPosX, mePoolUnit ) + maAnchor.getX()) * fUIScale);
            double fY((GetCoreValue( *m_pMtrPosY, mePoolUnit ) + maAnchor.getY()) * fUIScale);

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

        if ( m_pTsbPosProtect->GetState() != m_pTsbPosProtect->GetSavedValue() )
        {
            if( m_pTsbPosProtect->GetState() == STATE_DONTKNOW )
            {
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
            }
            else
            {
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                    m_pTsbPosProtect->GetState() == STATE_CHECK ? sal_True : sal_False ) );
            }

            bModified |= sal_True;
        }
    }

    if ( m_pMtrWidth->IsValueModified() || m_pMtrHeight->IsValueModified() )
    {
        Fraction aUIScale = mpView->GetModel()->GetUIScale();

        // get Width
        double nWidth = static_cast<double>(m_pMtrWidth->GetValue( meDlgUnit ));
        nWidth = MetricField::ConvertDoubleValue( nWidth, m_pMtrWidth->GetBaseValue(), m_pMtrWidth->GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lWidth = long(nWidth * (double)aUIScale);
        lWidth = OutputDevice::LogicToLogic( lWidth, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lWidth = static_cast<long>(m_pMtrWidth->Denormalize( lWidth ));

        // get Height
        double nHeight = static_cast<double>(m_pMtrHeight->GetValue( meDlgUnit ));
        nHeight = MetricField::ConvertDoubleValue( nHeight, m_pMtrHeight->GetBaseValue(), m_pMtrHeight->GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lHeight = long(nHeight * (double)aUIScale);
        lHeight = OutputDevice::LogicToLogic( lHeight, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lHeight = static_cast<long>(m_pMtrHeight->Denormalize( lHeight ));

        // put Width & Height to itemset
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ), (sal_uInt32) lWidth ) );
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ), (sal_uInt32) lHeight ) );
        rOutAttrs.Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), sal::static_int_cast< sal_uInt16 >( meRP ) ) );
        bModified |= sal_True;
    }

    if ( m_pTsbSizeProtect->GetState() != m_pTsbSizeProtect->GetSavedValue() )
    {
        if ( m_pTsbSizeProtect->GetState() == STATE_DONTKNOW )
            rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rOutAttrs.Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_pTsbSizeProtect->GetState() == STATE_CHECK ? sal_True : sal_False ) );
        bModified |= sal_True;
    }

    if ( m_pTsbAutoGrowWidth->GetState() != m_pTsbAutoGrowWidth->GetSavedValue() )
    {
        if ( !m_pTsbAutoGrowWidth->IsTriStateEnabled() )
        {
            if( m_pTsbAutoGrowWidth->GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOWIDTH );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOWIDTH ),
                    m_pTsbAutoGrowWidth->GetState() == STATE_CHECK ? sal_True : sal_False ) );
        }
        bModified |= sal_True;
    }

    if ( m_pTsbAutoGrowHeight->GetState() != m_pTsbAutoGrowHeight->GetSavedValue() )
    {
        if ( !m_pTsbAutoGrowHeight->IsTriStateEnabled() )
        {
            if( m_pTsbAutoGrowHeight->GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOHEIGHT );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOHEIGHT ),
                    m_pTsbAutoGrowHeight->GetState() == STATE_CHECK ? sal_True : sal_False ) );
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
            SetMetricValue(*m_pMtrPosX, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_Y );
        if ( pItem )
        {
            const double fTmp((((const SfxInt32Item*)pItem)->GetValue() - maAnchor.getY()) / fUIScale);
            SetMetricValue(*m_pMtrPosY, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_POS );
        if ( pItem )
        {
            sal_Bool bProtected = ( ( const SfxBoolItem* )pItem )->GetValue();
            m_pTsbPosProtect->SetState( bProtected ? STATE_CHECK : STATE_NOCHECK );
            m_pTsbPosProtect->EnableTriState( sal_False );
        }
        else
        {
            m_pTsbPosProtect->SetState( STATE_DONTKNOW );
        }

        m_pTsbPosProtect->SaveValue();
        m_pCtlPos->Reset();

        // #i2379# Disable controls for protected objects
        ChangePosProtectHdl( this );
    }

    { // #i75273# set width
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_WIDTH );
        mfOldWidth = std::max( pItem ? (double)((const SfxUInt32Item*)pItem)->GetValue() : 0.0, 1.0 );
        double fTmpWidth((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldWidth), (MapUnit)mePoolUnit, MAP_100TH_MM)) / fUIScale);

        if(m_pMtrWidth->GetDecimalDigits())
            fTmpWidth *= pow(10.0, m_pMtrWidth->GetDecimalDigits());

        fTmpWidth = MetricField::ConvertDoubleValue(fTmpWidth, m_pMtrWidth->GetBaseValue(), m_pMtrWidth->GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
        m_pMtrWidth->SetValue(static_cast<sal_Int64>(fTmpWidth), meDlgUnit);
    }

    { // #i75273# set height
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_HEIGHT );
        mfOldHeight = std::max( pItem ? (double)((const SfxUInt32Item*)pItem)->GetValue() : 0.0, 1.0 );
        double fTmpHeight((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldHeight), (MapUnit)mePoolUnit, MAP_100TH_MM)) / fUIScale);

        if(m_pMtrHeight->GetDecimalDigits())
            fTmpHeight *= pow(10.0, m_pMtrHeight->GetDecimalDigits());

        fTmpHeight = MetricField::ConvertDoubleValue(fTmpHeight, m_pMtrHeight->GetBaseValue(), m_pMtrHeight->GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
        m_pMtrHeight->SetValue(static_cast<sal_Int64>(fTmpHeight), meDlgUnit);
    }

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_SIZE );
    if ( pItem )
    {
        m_pTsbSizeProtect->SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                              ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbSizeProtect->EnableTriState( sal_False );
    }
    else
        m_pTsbSizeProtect->SetState( STATE_DONTKNOW );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOWIDTH );
    if ( pItem )
    {
        m_pTsbAutoGrowWidth->SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
    }
    else
        m_pTsbAutoGrowWidth->SetState( STATE_DONTKNOW );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOHEIGHT );
    if ( pItem )
    {
        m_pTsbAutoGrowHeight->SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
    }
    else
        m_pTsbAutoGrowHeight->SetState( STATE_DONTKNOW );

    // Is matching set?
    String aStr = GetUserData();
    m_pCbxScale->Check( (sal_Bool)aStr.ToInt32() );

    m_pTsbSizeProtect->SaveValue();
    m_pTsbAutoGrowWidth->SaveValue();
    m_pTsbAutoGrowHeight->SaveValue();
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
        double fX((double)m_pMtrPosX->GetValue());
        double fY((double)m_pMtrPosY->GetValue());

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
    m_pTsbSizeProtect->SetState( m_pTsbPosProtect->GetState() == STATE_CHECK ?  STATE_CHECK : mnProtectSizeState );
    UpdateControlStates();
    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::UpdateControlStates()
{
    const bool bPosProtect =  m_pTsbPosProtect->GetState() == STATE_CHECK;
    const bool bSizeProtect = m_pTsbSizeProtect->GetState() == STATE_CHECK;
    const bool bHeightChecked = !m_pTsbAutoGrowHeight->IsTriStateEnabled() && (m_pTsbAutoGrowHeight->GetState() == STATE_CHECK);
    const bool bWidthChecked = !m_pTsbAutoGrowWidth->IsTriStateEnabled() && (m_pTsbAutoGrowWidth->GetState() == STATE_CHECK);

    m_pFlPosition->Enable( !bPosProtect && !mbPageDisabled );

    m_pTsbPosProtect->Enable( !mbProtectDisabled && !mbPageDisabled );

    m_pFlSize->Enable( !mbSizeDisabled && !bSizeProtect );

    m_pFtWidth->Enable( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );
    m_pMtrWidth->Enable( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );

    m_pFtHeight->Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );
    m_pMtrHeight->Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );

    m_pCbxScale->Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked && !bWidthChecked );
    m_pCtlSize->Enable( !mbSizeDisabled && !bSizeProtect && (!bHeightChecked || !bWidthChecked) );

    m_pFlProtect->Enable( !mbProtectDisabled );
    m_pTsbSizeProtect->Enable( !mbProtectDisabled && !bPosProtect );

    m_pFlAdjust->Enable( !mbSizeDisabled && !bSizeProtect && !mbAdjustDisabled );

    m_pCtlSize->Invalidate();
    m_pCtlPos->Invalidate();

}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeSizeProtectHdl)
{
    if( m_pTsbSizeProtect->IsEnabled() )
    {
        // #106572# Remember user's last choice

        // Note: this works only as long as the dialog is open.  When
        // the user closes the dialog, there is no way to remember
        // whether size was enabled or disabled befor pos protect was
        // clicked. Thus, if pos protect is selected, the dialog is
        // closed and reopened again, unchecking pos protect will
        // always uncheck size protect, too. That's life.
        mnProtectSizeState = m_pTsbSizeProtect->GetState();
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

    switch ( m_pCtlPos->GetActualRP() )
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
    m_pMtrPosX->SetMin(basegfx::fround64(fLeft));
    m_pMtrPosX->SetFirst(basegfx::fround64(fLeft));
    m_pMtrPosX->SetMax(basegfx::fround64(fRight));
    m_pMtrPosX->SetLast(basegfx::fround64(fRight));
    m_pMtrPosY->SetMin(basegfx::fround64(fTop));
    m_pMtrPosY->SetFirst(basegfx::fround64(fTop));
    m_pMtrPosY->SetMax(basegfx::fround64(fBottom));
    m_pMtrPosY->SetLast(basegfx::fround64(fBottom));

    // size
    fLeft = maWorkRange.getMinX();
    fTop = maWorkRange.getMinY();
    fRight = maWorkRange.getMaxX();
    fBottom = maWorkRange.getMaxY();
    double fNewX(0);
    double fNewY(0);

    switch ( m_pCtlSize->GetActualRP() )
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
    m_pMtrWidth->SetMax(basegfx::fround64(fNewX));
    m_pMtrWidth->SetLast(basegfx::fround64(fNewX));
    m_pMtrHeight->SetMax(basegfx::fround64(fNewY));
    m_pMtrHeight->SetLast(basegfx::fround64(fNewY));
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange)
{
    switch (m_pCtlPos->GetActualRP())
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
    if( pWindow == m_pCtlPos )
    {
        SetMinMaxPosition();
        switch( eRP )
        {
            case RP_LT:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMinX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_MT:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_RT:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMaxX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_LM:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMinX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_MM:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_RM:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMaxX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_LB:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMinX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
            case RP_MB:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
            case RP_RB:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMaxX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMaxY()) );
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
    if( m_pCbxScale->IsChecked() && m_pCbxScale->IsEnabled() )
    {
        sal_Int64 nHeight(basegfx::fround64((mfOldHeight * (double)m_pMtrWidth->GetValue()) / mfOldWidth));

        if(nHeight <= m_pMtrHeight->GetMax(FUNIT_NONE))
        {
            m_pMtrHeight->SetUserValue(nHeight, FUNIT_NONE);
        }
        else
        {
            nHeight = m_pMtrHeight->GetMax(FUNIT_NONE);
            m_pMtrHeight->SetUserValue(nHeight);

            const sal_Int64 nWidth(basegfx::fround64((mfOldWidth * (double)nHeight) / mfOldHeight));
            m_pMtrWidth->SetUserValue(nWidth, FUNIT_NONE);
        }
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeHeightHdl)
{
    if( m_pCbxScale->IsChecked() && m_pCbxScale->IsEnabled() )
    {
        sal_Int64 nWidth(basegfx::fround64((mfOldWidth * (double)m_pMtrHeight->GetValue()) / mfOldHeight));

        if(nWidth <= m_pMtrWidth->GetMax(FUNIT_NONE))
        {
            m_pMtrWidth->SetUserValue(nWidth, FUNIT_NONE);
        }
        else
        {
            nWidth = m_pMtrWidth->GetMax(FUNIT_NONE);
            m_pMtrWidth->SetUserValue(nWidth);

            const sal_Int64 nHeight(basegfx::fround64((mfOldHeight * (double)nWidth) / mfOldWidth));
            m_pMtrHeight->SetUserValue(nHeight, FUNIT_NONE);
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
    if( m_pCbxScale->IsChecked() )
    {
        mfOldWidth  = std::max( (double)GetCoreValue( *m_pMtrWidth,  mePoolUnit ), 1.0 );
        mfOldHeight = std::max( (double)GetCoreValue( *m_pMtrHeight, mePoolUnit ), 1.0 );
    }

    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::FillUserData()
{
    // matching is saved in the Ini-file
    OUString aStr = m_pCbxScale->IsChecked() ? OUString("1") : OUString("0");
    SetUserData( aStr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
