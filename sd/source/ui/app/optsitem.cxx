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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svxids.hrc>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <osl/diagnose.h>

#include <optsitem.hxx>
#include <FrameView.hxx>
#include <sdattr.hrc>

using namespace ::utl;
using namespace ::com::sun::star::uno;

template< class T > static T getSafeValue( const Any& rAny )
{
    T value = T();
    bool bOk = (rAny >>= value);

    DBG_ASSERT( bOk, "SdOptionsItem, wrong type from configuration!" );

    return value;
}


SdOptionsItem::SdOptionsItem( const SdOptionsGeneric& rParent, const OUString& rSubTree ) :
    ConfigItem  ( rSubTree ),
    mrParent    ( rParent )
{
}

SdOptionsItem::~SdOptionsItem()
{
}

void SdOptionsItem::ImplCommit()
{
    if( IsModified() )
        mrParent.Commit( *this );
};

void SdOptionsItem::Notify( const css::uno::Sequence<OUString>& )
{}

Sequence< Any > SdOptionsItem::GetProperties( const Sequence< OUString >& rNames )
{
    return ConfigItem::GetProperties( rNames );
}

bool SdOptionsItem::PutProperties( const Sequence< OUString >& rNames, const Sequence< Any>& rValues )
{
    return ConfigItem::PutProperties( rNames, rValues );
}

SdOptionsGeneric::SdOptionsGeneric(bool bImpress, const OUString& rSubTree)
    : maSubTree(rSubTree)
    , mbImpress(bImpress)
    , mbInit(rSubTree.isEmpty())
    , mbEnableModify(false)
{
}

SdOptionsGeneric::SdOptionsGeneric(SdOptionsGeneric const & rSource)
{
    operator=(rSource);
}

SdOptionsGeneric& SdOptionsGeneric::operator=(SdOptionsGeneric const & rSource)
{
    if (this != &rSource)
    {
        maSubTree = rSource.maSubTree;
        mpCfgItem.reset(rSource.mpCfgItem ? new SdOptionsItem(*rSource.mpCfgItem) : nullptr );
        mbImpress = rSource.mbImpress;
        mbInit = rSource.mbInit;
        mbEnableModify = rSource.mbEnableModify;
    }
    return *this;
}

void SdOptionsGeneric::Init() const
{
    if( mbInit )
        return;

    SdOptionsGeneric* pThis = const_cast<SdOptionsGeneric*>(this);

    if( !mpCfgItem )
        pThis->mpCfgItem.reset( new SdOptionsItem( *this, maSubTree ) );
    assert(mpCfgItem && "mpCfgItem is set by now");

    const Sequence< OUString >  aNames( GetPropertyNames() );
    const Sequence< Any >       aValues = mpCfgItem->GetProperties( aNames );

    if( aNames.hasElements() && ( aValues.getLength() == aNames.getLength() ) )
    {
        const Any* pValues = aValues.getConstArray();

        pThis->EnableModify( false );
        pThis->mbInit = pThis->ReadData( pValues );
        pThis->EnableModify( true );
    }
    else
        pThis->mbInit = true;
}

SdOptionsGeneric::~SdOptionsGeneric()
{
}

void SdOptionsGeneric::Commit( SdOptionsItem& rCfgItem ) const
{
    const Sequence< OUString >  aNames( GetPropertyNames() );
    Sequence< Any >             aValues( aNames.getLength() );

    if( aNames.hasElements() )
    {
        if( WriteData( aValues.getArray() ) )
            rCfgItem.PutProperties( aNames, aValues );
        else
        {
            OSL_FAIL( "PutProperties failed" );
        }
    }
}

void SdOptionsGeneric::Store()
{
    if( mpCfgItem )
        mpCfgItem->Commit();
}

bool SdOptionsGeneric::isMetricSystem()
{
    SvtSysLocale aSysLocale;
    MeasurementSystem eSys = aSysLocale.GetLocaleData().getMeasurementSystemEnum();

    return ( eSys == MeasurementSystem::Metric );
}

/*************************************************************************
|*
|* SdOptionsMisc
|*
\************************************************************************/

SdOptionsMisc::SdOptionsMisc( bool bImpress, bool bUseConfig ) :
    SdOptionsGeneric( bImpress, bUseConfig ?
                      ( bImpress ?
                        u"Office.Impress/Misc"_ustr :
                        u"Office.Draw/Misc"_ustr ) :
                      OUString() ),
    nDefaultObjectSizeWidth(8000),
    nDefaultObjectSizeHeight(5000),
    bStartWithTemplate( false ),
    bMarkedHitMovesAlways( true ),
    bMoveOnlyDragging( false ),
    bCrookNoContortion( false ),
    bQuickEdit( IsImpress() ),
    bMasterPageCache( true ),
    bDragWithCopy( false ),
    bPickThrough( true ),
    bDoubleClickTextEdit( true ),
    bClickChangeRotation( false ),
    bSolidDragging( true ),
    bSummationOfParagraphs( false ),
    bTabBarVisible( true ),
    bShowUndoDeleteWarning( true ),
    bSlideshowRespectZOrder( true ),
    bShowComments( true ),
    bPreviewNewEffects( true ),
    bPreviewChangedEffects( false ),
    bPreviewTransitions( true ),
    mnDisplay( 0 ),
    mnPenColor( 0xff0000 ),
    mnPenWidth( 150.0 ),

    // The default for 6.1-and-above documents is to use printer-independent
    // formatting.
    mnPrinterIndependentLayout (1),
    mnDragThresholdPixels(6)
{
    EnableModify( true );
}

bool SdOptionsMisc::operator==( const SdOptionsMisc& rOpt ) const
{
    return( IsStartWithTemplate() == rOpt.IsStartWithTemplate() &&
            IsMarkedHitMovesAlways() == rOpt.IsMarkedHitMovesAlways() &&
            IsMoveOnlyDragging() == rOpt.IsMoveOnlyDragging() &&
            IsCrookNoContortion() == rOpt.IsCrookNoContortion() &&
            IsQuickEdit() == rOpt.IsQuickEdit() &&
            IsMasterPagePaintCaching() == rOpt.IsMasterPagePaintCaching() &&
            IsDragWithCopy() == rOpt.IsDragWithCopy() &&
            IsPickThrough() == rOpt.IsPickThrough() &&
            IsDoubleClickTextEdit() == rOpt.IsDoubleClickTextEdit() &&
            IsClickChangeRotation() == rOpt.IsClickChangeRotation() &&
            IsSummationOfParagraphs() == rOpt.IsSummationOfParagraphs() &&
            IsTabBarVisible() == rOpt.IsTabBarVisible() &&
            IsSolidDragging() == rOpt.IsSolidDragging() &&
            IsShowUndoDeleteWarning() == rOpt.IsShowUndoDeleteWarning() &&
            IsSlideshowRespectZOrder() == rOpt.IsSlideshowRespectZOrder() &&
            GetPrinterIndependentLayout() == rOpt.GetPrinterIndependentLayout() &&
            GetDefaultObjectSizeWidth() == rOpt.GetDefaultObjectSizeWidth() &&
            GetDefaultObjectSizeHeight() == rOpt.GetDefaultObjectSizeHeight() &&

            IsPreviewNewEffects() == rOpt.IsPreviewNewEffects() &&
            IsPreviewChangedEffects() == rOpt.IsPreviewChangedEffects() &&
            IsPreviewTransitions() == rOpt.IsPreviewTransitions() &&
            GetDisplay() == rOpt.GetDisplay() &&
            IsShowComments() == rOpt.IsShowComments() &&
            GetPresentationPenColor() == rOpt.GetPresentationPenColor() &&
            GetPresentationPenWidth() == rOpt.GetPresentationPenWidth() &&
            GetDragThresholdPixels() == rOpt.GetDragThresholdPixels()
        );
}

Sequence< OUString > SdOptionsMisc::GetPropertyNames() const
{
    static constexpr OUString aPropNames[] =
    {
        u"ObjectMoveable"_ustr,
        u"NoDistort"_ustr,
        u"TextObject/QuickEditing"_ustr,
        u"BackgroundCache"_ustr,
        u"CopyWhileMoving"_ustr,
        u"TextObject/Selectable"_ustr,
        u"DclickTextedit"_ustr,
        u"RotateClick"_ustr,
        u"Preview"_ustr,
        u"ModifyWithAttributes"_ustr,
        u"DefaultObjectSize/Width"_ustr,
        u"DefaultObjectSize/Height"_ustr,

        u"Compatibility/PrinterIndependentLayout"_ustr,

        u"ShowComments"_ustr,
        u"DragThresholdPixels"_ustr,

        // just for impress
        u"NewDoc/AutoPilot"_ustr,
        u"Compatibility/AddBetween"_ustr,
        u"ShowUndoDeleteWarning"_ustr,
        u"SlideshowRespectZOrder"_ustr,

        u"PreviewNewEffects"_ustr,
        u"PreviewChangedEffects"_ustr,
        u"PreviewTransitions"_ustr,

        u"Display"_ustr,

        u"PenColor"_ustr,
        u"PenWidth"_ustr,
        u"TabBarVisible"_ustr
    };

    size_t len = ( IsImpress() ? std::size(aPropNames) : 15 );
    return Sequence<OUString>(aPropNames, len);
}

bool SdOptionsMisc::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetMarkedHitMovesAlways( *o3tl::doAccess<bool>(pValues[ 0 ]) );
    if( pValues[1].hasValue() ) SetCrookNoContortion( *o3tl::doAccess<bool>(pValues[ 1 ]) );
    if( pValues[2].hasValue() ) SetQuickEdit( *o3tl::doAccess<bool>(pValues[ 2 ]) );
    if( pValues[3].hasValue() ) SetMasterPagePaintCaching( *o3tl::doAccess<bool>(pValues[ 3 ]) );
    if( pValues[4].hasValue() ) SetDragWithCopy( *o3tl::doAccess<bool>(pValues[ 4 ]) );
    if( pValues[5].hasValue() ) SetPickThrough( *o3tl::doAccess<bool>(pValues[ 5 ]) );
    if( pValues[6].hasValue() ) SetDoubleClickTextEdit( *o3tl::doAccess<bool>(pValues[ 6 ]) );
    if( pValues[7].hasValue() ) SetClickChangeRotation( *o3tl::doAccess<bool>(pValues[ 7 ]) );
    if( pValues[9].hasValue() ) SetSolidDragging( *o3tl::doAccess<bool>(pValues[ 9 ]) );
    if( pValues[10].hasValue() ) SetDefaultObjectSizeWidth( *o3tl::doAccess<sal_Int32>(pValues[ 10 ]) );
    if( pValues[11].hasValue() ) SetDefaultObjectSizeHeight( *o3tl::doAccess<sal_Int32>(pValues[ 11 ]) );
    if( pValues[12].hasValue() ) SetPrinterIndependentLayout( *o3tl::doAccess<sal_uInt16>(pValues[ 12 ]) );

    if( pValues[13].hasValue() )
        SetShowComments(  *o3tl::doAccess<bool>(pValues[ 13 ]) );
    if (pValues[14].hasValue())
        SetDragThreshold(*o3tl::doAccess<sal_Int32>(pValues[ 14 ]));

    // just for Impress
    if (IsImpress())
    {
        if( pValues[15].hasValue() )
            SetStartWithTemplate( *o3tl::doAccess<bool>(pValues[ 15 ]) );
        if( pValues[16].hasValue() )
            SetSummationOfParagraphs( *o3tl::doAccess<bool>(pValues[ 16 ]) );
        if( pValues[17].hasValue() )
            SetShowUndoDeleteWarning( *o3tl::doAccess<bool>(pValues[ 17 ]) );

        if( pValues[18].hasValue() )
            SetSlideshowRespectZOrder(*o3tl::doAccess<bool>(pValues[ 18 ]));

        if( pValues[19].hasValue() )
            SetPreviewNewEffects(*o3tl::doAccess<bool>(pValues[ 19 ]));

        if( pValues[20].hasValue() )
            SetPreviewChangedEffects(*o3tl::doAccess<bool>(pValues[ 20 ]));

        if( pValues[21].hasValue() )
            SetPreviewTransitions(*o3tl::doAccess<bool>(pValues[ 21 ]));

        if( pValues[22].hasValue() )
            SetDisplay(*o3tl::doAccess<sal_Int32>(pValues[ 22 ]));

        if( pValues[23].hasValue() )
            SetPresentationPenColor( getSafeValue< sal_Int32 >( pValues[ 23 ] ) );

        if( pValues[24].hasValue() )
            SetPresentationPenWidth( getSafeValue< double >( pValues[ 24 ] ) );

        if( pValues[25].hasValue() ) {
            SetTabBarVisible( *o3tl::doAccess<bool>(pValues[ 25 ]) );
        }
    }

    return true;
}

bool SdOptionsMisc::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsMarkedHitMovesAlways();
    pValues[ 1 ] <<= IsCrookNoContortion();
    pValues[ 2 ] <<= IsQuickEdit();
    pValues[ 3 ] <<= IsMasterPagePaintCaching();
    pValues[ 4 ] <<= IsDragWithCopy();
    pValues[ 5 ] <<= IsPickThrough();
    pValues[ 6 ] <<= IsDoubleClickTextEdit();
    pValues[ 7 ] <<= IsClickChangeRotation();
    // The preview is not supported anymore.  Use a dummy value.
    pValues[ 8 ] <<= double(0);// GetPreviewQuality();
    pValues[ 9 ] <<= IsSolidDragging();
    pValues[ 10 ] <<= GetDefaultObjectSizeWidth();
    pValues[ 11 ] <<= GetDefaultObjectSizeHeight();
    pValues[ 12 ] <<= GetPrinterIndependentLayout();
    pValues[ 13 ] <<= IsShowComments();
    pValues[ 14 ] <<= GetDragThresholdPixels();

    // just for Impress
    if (IsImpress())
    {
        pValues[ 15 ] <<= IsStartWithTemplate();
        pValues[ 16 ] <<= IsSummationOfParagraphs();
        pValues[ 17 ] <<= IsShowUndoDeleteWarning();
        pValues[ 18 ] <<= IsSlideshowRespectZOrder();

        pValues[ 19 ] <<= IsPreviewNewEffects();
        pValues[ 20 ] <<= IsPreviewChangedEffects();
        pValues[ 21 ] <<= IsPreviewTransitions();

        pValues[ 22 ] <<= GetDisplay();

        pValues[ 23 ] <<= GetPresentationPenColor();
        pValues[ 24 ] <<= GetPresentationPenWidth();
        pValues[ 25 ] <<= IsTabBarVisible();
    }

    return true;
}

/*************************************************************************
|*
|* SdOptionsMiscItem
|*
\************************************************************************/

SdOptionsMiscItem::SdOptionsMiscItem()
:   SfxPoolItem     ( ATTR_OPTIONS_MISC )
,   maOptionsMisc   ( false, false )
{
}

SdOptionsMiscItem::SdOptionsMiscItem( SdOptions const * pOpts, ::sd::FrameView const * pView )
:   SfxPoolItem     ( ATTR_OPTIONS_MISC )
,   maOptionsMisc   ( false, false )
{
    if( pOpts )
    {
        maOptionsMisc.SetStartWithTemplate( pOpts->IsStartWithTemplate() );
        maOptionsMisc.SetSummationOfParagraphs( pOpts->IsSummationOfParagraphs() );
        maOptionsMisc.SetTabBarVisible( pOpts->IsTabBarVisible() );
        maOptionsMisc.SetShowUndoDeleteWarning( pOpts->IsShowUndoDeleteWarning() );
        maOptionsMisc.SetPrinterIndependentLayout( pOpts->GetPrinterIndependentLayout() );
        maOptionsMisc.SetDefaultObjectSizeWidth( pOpts->GetDefaultObjectSizeWidth() );
        maOptionsMisc.SetDefaultObjectSizeHeight( pOpts->GetDefaultObjectSizeHeight() );

        maOptionsMisc.SetPreviewNewEffects(pOpts->IsPreviewNewEffects());
        maOptionsMisc.SetPreviewChangedEffects(pOpts->IsPreviewChangedEffects());
        maOptionsMisc.SetPreviewTransitions(pOpts->IsPreviewTransitions());

        maOptionsMisc.SetDisplay(pOpts->GetDisplay());
        maOptionsMisc.SetShowComments( pOpts->IsShowComments() );

        maOptionsMisc.SetPresentationPenColor(pOpts->GetPresentationPenColor() );
        maOptionsMisc.SetPresentationPenWidth(pOpts->GetPresentationPenWidth() );
    }

    if( pView )
    {
        maOptionsMisc.SetMarkedHitMovesAlways( pView->IsMarkedHitMovesAlways() );
        maOptionsMisc.SetMoveOnlyDragging( pView->IsMoveOnlyDragging() );
        maOptionsMisc.SetCrookNoContortion( pView->IsCrookNoContortion() );
        maOptionsMisc.SetQuickEdit( pView->IsQuickEdit() );

        // #i26631#
        maOptionsMisc.SetMasterPagePaintCaching( pView->IsMasterPagePaintCaching() );

        maOptionsMisc.SetDragWithCopy( pView->IsDragWithCopy() );
        maOptionsMisc.SetPickThrough(pView->GetModel().IsPickThroughTransparentTextFrames());
        maOptionsMisc.SetDoubleClickTextEdit( pView->IsDoubleClickTextEdit() );
        maOptionsMisc.SetClickChangeRotation( pView->IsClickChangeRotation() );
        maOptionsMisc.SetSolidDragging( pView->IsSolidDragging() );
        maOptionsMisc.SetDragThreshold(pView->GetDragThresholdPixels());
    }
    else if( pOpts )
    {
        maOptionsMisc.SetMarkedHitMovesAlways( pOpts->IsMarkedHitMovesAlways() );
        maOptionsMisc.SetMoveOnlyDragging( pOpts->IsMoveOnlyDragging() );
        maOptionsMisc.SetCrookNoContortion( pOpts->IsCrookNoContortion() );
        maOptionsMisc.SetQuickEdit( pOpts->IsQuickEdit() );
        maOptionsMisc.SetMasterPagePaintCaching( pOpts->IsMasterPagePaintCaching() );
        maOptionsMisc.SetDragWithCopy( pOpts->IsDragWithCopy() );
        maOptionsMisc.SetPickThrough( pOpts->IsPickThrough() );
        maOptionsMisc.SetDoubleClickTextEdit( pOpts->IsDoubleClickTextEdit() );
        maOptionsMisc.SetClickChangeRotation( pOpts->IsClickChangeRotation() );
        maOptionsMisc.SetSolidDragging( pOpts->IsSolidDragging() );
        maOptionsMisc.SetDragThreshold(pOpts->GetDragThresholdPixels());
    }
}

SdOptionsMiscItem* SdOptionsMiscItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsMiscItem( *this );
}

bool SdOptionsMiscItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return maOptionsMisc == static_cast<const SdOptionsMiscItem&>(rAttr).maOptionsMisc;
}

void SdOptionsMiscItem::SetOptions( SdOptions* pOpts ) const
{
    if( !pOpts )
        return;
    //this needs to reflect all settings from SdTpOptionsMisc::FillItemSet()
    pOpts->SetStartWithTemplate( maOptionsMisc.IsStartWithTemplate() );
    pOpts->SetMarkedHitMovesAlways( maOptionsMisc.IsMarkedHitMovesAlways() );
    pOpts->SetQuickEdit(maOptionsMisc.IsQuickEdit());
    pOpts->SetPickThrough( maOptionsMisc.IsPickThrough() );
    pOpts->SetMasterPagePaintCaching( maOptionsMisc.IsMasterPagePaintCaching() );
    pOpts->SetDragWithCopy( maOptionsMisc.IsDragWithCopy() );
    pOpts->SetSummationOfParagraphs( maOptionsMisc.IsSummationOfParagraphs() );
    pOpts->SetCrookNoContortion( maOptionsMisc.IsCrookNoContortion() );

}

/*************************************************************************
|*
|* SdOptionsGrid
|*
\************************************************************************/

SdOptionsGrid::SdOptionsGrid(bool bImpress) :
    SdOptionsGeneric( bImpress,
                      bImpress ?
                        u"Office.Impress/Grid"_ustr :
                        u"Office.Draw/Grid"_ustr
                    )
{
    EnableModify( false );
    SetDefaults();
    EnableModify( true );
}

SdOptionsGrid::~SdOptionsGrid()
{
}

void SdOptionsGrid::SetDefaults()
{
    const sal_uInt32 nVal = 1000;

    SetFieldDivisionX( nVal );
    SetFieldDivisionY( nVal );
    SetFieldDrawX( nVal );
    SetFieldDrawY( nVal );
    SetUseGridSnap( false );
    SetSynchronize( true );
    SetGridVisible( false );
}

Sequence< OUString > SdOptionsGrid::GetPropertyNames() const
{
    if( isMetricSystem() )
    {
        return Sequence({
            u"Resolution/XAxis/Metric"_ustr,
            u"Resolution/YAxis/Metric"_ustr,
            u"Subdivision/XAxis"_ustr,
            u"Subdivision/YAxis"_ustr,
            u"Option/SnapToGrid"_ustr,
            u"Option/Synchronize"_ustr,
            u"Option/VisibleGrid"_ustr
        });
    }
    else
    {
        return Sequence({
            u"Resolution/XAxis/NonMetric"_ustr,
            u"Resolution/YAxis/NonMetric"_ustr,
            u"Subdivision/XAxis"_ustr,
            u"Subdivision/YAxis"_ustr,
            u"Option/SnapToGrid"_ustr,
            u"Option/Synchronize"_ustr,
            u"Option/VisibleGrid"_ustr
        });
    }
}

static sal_uInt32 getDivisor(const Any& rValue)
{
    if (rValue.hasValue())
    {
        sal_uInt32 nDiv = basegfx::fround<sal_uInt32>(*o3tl::doAccess<double>(rValue));
        return nDiv + 1;
    }
    return 0;
}

bool SdOptionsGrid::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetFieldDrawX( *o3tl::doAccess<sal_Int32>(pValues[ 0 ]) );
    if( pValues[1].hasValue() ) SetFieldDrawY( *o3tl::doAccess<sal_Int32>(pValues[ 1 ]) );

    if (const sal_uInt32 nDivX = getDivisor(pValues[2]))
        SetFieldDivisionX(SvxOptionsGrid::GetFieldDrawX() / nDivX);

    if (const sal_uInt32 nDivY = getDivisor(pValues[3]))
        SetFieldDivisionY(SvxOptionsGrid::GetFieldDrawY() / nDivY);

    if( pValues[4].hasValue() ) SetUseGridSnap( *o3tl::doAccess<bool>(pValues[ 4 ]) );
    if( pValues[5].hasValue() ) SetSynchronize( *o3tl::doAccess<bool>(pValues[ 5 ]) );
    if( pValues[6].hasValue() ) SetGridVisible( *o3tl::doAccess<bool>(pValues[ 6 ]) );

    return true;
}

bool SdOptionsGrid::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= static_cast<sal_Int32>(GetFieldDrawX());
    pValues[ 1 ] <<= static_cast<sal_Int32>(GetFieldDrawY());
    pValues[ 2 ] <<= ( GetFieldDivisionX() ? ( static_cast<double>(GetFieldDrawX()) / GetFieldDivisionX() - 1.0 ) : double(0) );
    pValues[ 3 ] <<= ( GetFieldDivisionY() ? ( static_cast<double>(GetFieldDrawY()) / GetFieldDivisionY() - 1.0 ) : double(0) );
    pValues[ 4 ] <<= IsUseGridSnap();
    pValues[ 5 ] <<= IsSynchronize();
    pValues[ 6 ] <<= IsGridVisible();

    return true;
}

/*************************************************************************
|*
|* SdOptionsGridItem
|*
\************************************************************************/

SdOptionsGridItem::SdOptionsGridItem( SdOptions const * pOpts ) :
    SvxGridItem( SID_ATTR_GRID_OPTIONS )
{
    SetSynchronize( pOpts->IsSynchronize() );

    SetFieldDrawX( pOpts->GetFieldDrawX() );
    SetFieldDrawY( pOpts->GetFieldDrawY() );
    SetFieldDivisionX( pOpts->GetFieldDivisionX() ? ( pOpts->GetFieldDrawX() / pOpts->GetFieldDivisionX() - 1 ) : 0 );
    SetFieldDivisionY( pOpts->GetFieldDivisionY() ? ( pOpts->GetFieldDrawY() / pOpts->GetFieldDivisionY() - 1 ) : 0 );
    SetUseGridSnap( pOpts->IsUseGridSnap() );
    SetGridVisible( pOpts->IsGridVisible() );
}

void SdOptionsGridItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetFieldDrawX( GetFieldDrawX() );
    pOpts->SetFieldDivisionX( GetFieldDrawX() / ( GetFieldDivisionX() + 1 ) );
    pOpts->SetFieldDrawY( GetFieldDrawY() );
    pOpts->SetFieldDivisionY( GetFieldDrawY() / ( GetFieldDivisionY() + 1 ) );
    pOpts->SetUseGridSnap( GetUseGridSnap() );
    pOpts->SetSynchronize( GetSynchronize() );
    pOpts->SetGridVisible( GetGridVisible() );
}

/*************************************************************************
|*
|* SdOptionsPrint
|*
\************************************************************************/

SdOptionsPrint::SdOptionsPrint( bool bImpress, bool bUseConfig ) :
    SdOptionsGeneric( bImpress, bUseConfig ?
                      ( bImpress ?
                        u"Office.Impress/Print"_ustr :
                        u"Office.Draw/Print"_ustr ) :
                      OUString() ),
    bDraw( true ),
    bNotes( false ),
    bHandout( false ),
    bOutline( false ),
    bDate( false ),
    bTime( false ),
    bPagename( false ),
    bHiddenPages( true ),
    bPagesize( false ),
    bPagetile( false ),
    bWarningPrinter( true ),
    bWarningSize( false ),
    bWarningOrientation( false ),
    bBooklet( false ),
    bFront( true ),
    bBack( true ),
    bCutPage( false ),
    bPaperbin( false ),
    mbHandoutHorizontal( true ),
    mnHandoutPages( 6 ),
    nQuality( 0 )
{
    EnableModify( true );
}

bool SdOptionsPrint::operator==( const SdOptionsPrint& rOpt ) const
{
    return( IsDraw() == rOpt.IsDraw() &&
            IsNotes() == rOpt.IsNotes() &&
            IsHandout() == rOpt.IsHandout() &&
            IsOutline() == rOpt.IsOutline() &&
            IsDate() == rOpt.IsDate() &&
            IsTime() == rOpt.IsTime() &&
            IsPagename() == rOpt.IsPagename() &&
            IsHiddenPages() == rOpt.IsHiddenPages() &&
            IsPagesize() == rOpt.IsPagesize() &&
            IsPagetile() == rOpt.IsPagetile() &&
            IsWarningPrinter() == rOpt.IsWarningPrinter() &&
            IsWarningSize() == rOpt.IsWarningSize() &&
            IsWarningOrientation() == rOpt.IsWarningOrientation() &&
            IsBooklet() == rOpt.IsBooklet() &&
            IsFrontPage() == rOpt.IsFrontPage() &&
            IsBackPage() == rOpt.IsBackPage() &&
            IsCutPage() == rOpt.IsCutPage() &&
            IsPaperbin() == rOpt.IsPaperbin() &&
            GetOutputQuality() == rOpt.GetOutputQuality() &&
            IsHandoutHorizontal() == rOpt.IsHandoutHorizontal() &&
            GetHandoutPages() == rOpt.GetHandoutPages() );
}

Sequence< OUString > SdOptionsPrint::GetPropertyNames() const
{
    if (IsImpress())
    {
        return Sequence({

            u"Other/Date"_ustr,
            u"Other/Time"_ustr,
            u"Other/PageName"_ustr,
            u"Other/HiddenPage"_ustr,
            u"Page/PageSize"_ustr,
            u"Page/PageTile"_ustr,
            // bWarningPrinter
            // bWarningSize
            // bWarningOrientation
            u"Page/Booklet"_ustr,
            u"Page/BookletFront"_ustr,
            u"Page/BookletBack"_ustr,
            // bCutPage
            u"Other/FromPrinterSetup"_ustr,
            u"Other/Quality"_ustr,
            u"Content/Presentation"_ustr,
            u"Content/Note"_ustr,
            u"Content/Handout"_ustr,
            u"Content/Outline"_ustr,
            u"Other/HandoutHorizontal"_ustr,
            u"Other/PagesPerHandout"_ustr
        });
    }
    else
    {
        return Sequence({
            u"Other/Date"_ustr,
            u"Other/Time"_ustr,
            u"Other/PageName"_ustr,
            u"Other/HiddenPage"_ustr,
            u"Page/PageSize"_ustr,
            u"Page/PageTile"_ustr,
            // bWarningPrinter
            // bWarningSize
            // bWarningOrientation
            u"Page/Booklet"_ustr,
            u"Page/BookletFront"_ustr,
            u"Page/BookletBack"_ustr,
            // bCutPage
            u"Other/FromPrinterSetup"_ustr,
            u"Other/Quality"_ustr,
            u"Content/Drawing"_ustr,
        });
    }
}

bool SdOptionsPrint::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetDate( *o3tl::doAccess<bool>(pValues[ 0 ]) );
    if( pValues[1].hasValue() ) SetTime( *o3tl::doAccess<bool>(pValues[ 1 ]) );
    if( pValues[2].hasValue() ) SetPagename( *o3tl::doAccess<bool>(pValues[ 2 ]) );
    if( pValues[3].hasValue() ) SetHiddenPages( *o3tl::doAccess<bool>(pValues[ 3 ]) );
    if( pValues[4].hasValue() ) SetPagesize( *o3tl::doAccess<bool>(pValues[ 4 ]) );
    if( pValues[5].hasValue() ) SetPagetile( *o3tl::doAccess<bool>(pValues[ 5 ]) );
    if( pValues[6].hasValue() ) SetBooklet( *o3tl::doAccess<bool>(pValues[ 6 ]) );
    if( pValues[7].hasValue() ) SetFrontPage( *o3tl::doAccess<bool>(pValues[ 7 ]) );
    if( pValues[8].hasValue() ) SetBackPage( *o3tl::doAccess<bool>(pValues[ 8 ]) );
    if( pValues[9].hasValue() ) SetPaperbin( *o3tl::doAccess<bool>(pValues[ 9 ]) );
    if( pValues[10].hasValue() ) SetOutputQuality( static_cast<sal_uInt16>(*o3tl::doAccess<sal_Int32>(pValues[ 10 ])) );
    if( pValues[11].hasValue() ) SetDraw( *o3tl::doAccess<bool>(pValues[ 11 ]) );

    // just for impress
    if (IsImpress())
    {
        if( pValues[12].hasValue() ) SetNotes( *o3tl::doAccess<bool>(pValues[ 12 ]) );
        if( pValues[13].hasValue() ) SetHandout( *o3tl::doAccess<bool>(pValues[ 13 ]) );
        if( pValues[14].hasValue() ) SetOutline( *o3tl::doAccess<bool>(pValues[ 14 ]) );
        if( pValues[15].hasValue() ) SetHandoutHorizontal( *o3tl::doAccess<bool>(pValues[15]) );
        if( pValues[16].hasValue() ) SetHandoutPages( static_cast<sal_uInt16>(*o3tl::doAccess<sal_Int32>(pValues[16])) );
    }

    return true;
}

bool SdOptionsPrint::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsDate();
    pValues[ 1 ] <<= IsTime();
    pValues[ 2 ] <<= IsPagename();
    pValues[ 3 ] <<= IsHiddenPages();
    pValues[ 4 ] <<= IsPagesize();
    pValues[ 5 ] <<= IsPagetile();
    pValues[ 6 ] <<= IsBooklet();
    pValues[ 7 ] <<= IsFrontPage();
    pValues[ 8 ] <<= IsBackPage();
    pValues[ 9 ] <<= IsPaperbin();
    pValues[ 10 ] <<= static_cast<sal_Int32>(GetOutputQuality());
    pValues[ 11 ] <<= IsDraw();

    // just for impress
    if (IsImpress())
    {
        pValues[ 12 ] <<= IsNotes();
        pValues[ 13 ] <<= IsHandout();
        pValues[ 14 ] <<= IsOutline();
        pValues[ 15 ] <<= IsHandoutHorizontal();
        pValues[ 16 ] <<= GetHandoutPages();
    }

    return true;
}

/*************************************************************************
|*
|* SdOptionsPrintItem
|*
\************************************************************************/

SdOptionsPrintItem::SdOptionsPrintItem()
:   SfxPoolItem     ( ATTR_OPTIONS_PRINT )
,   maOptionsPrint  ( false, false )
{
}

SdOptionsPrintItem::SdOptionsPrintItem( SdOptions const * pOpts )
:   SfxPoolItem     ( ATTR_OPTIONS_PRINT )
,   maOptionsPrint  ( false, false )
{
    if( !pOpts )
        return;

    maOptionsPrint.SetDraw( pOpts->IsDraw() );
    maOptionsPrint.SetNotes( pOpts->IsNotes() );
    maOptionsPrint.SetHandout( pOpts->IsHandout() );
    maOptionsPrint.SetOutline( pOpts->IsOutline() );
    maOptionsPrint.SetDate( pOpts->IsDate() );
    maOptionsPrint.SetTime( pOpts->IsTime() );
    maOptionsPrint.SetPagename( pOpts->IsPagename() );
    maOptionsPrint.SetHiddenPages( pOpts->IsHiddenPages() );
    maOptionsPrint.SetPagesize( pOpts->IsPagesize() );
    maOptionsPrint.SetPagetile( pOpts->IsPagetile() );
    maOptionsPrint.SetWarningPrinter( pOpts->IsWarningPrinter() );
    maOptionsPrint.SetWarningSize( pOpts->IsWarningSize() );
    maOptionsPrint.SetWarningOrientation( pOpts->IsWarningOrientation() );
    maOptionsPrint.SetBooklet( pOpts->IsBooklet() );
    maOptionsPrint.SetFrontPage( pOpts->IsFrontPage() );
    maOptionsPrint.SetBackPage( pOpts->IsBackPage() );
    maOptionsPrint.SetCutPage( pOpts->IsCutPage() );
    maOptionsPrint.SetPaperbin( pOpts->IsPaperbin() );
    maOptionsPrint.SetOutputQuality( pOpts->GetOutputQuality() );
}

SdOptionsPrintItem* SdOptionsPrintItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsPrintItem( *this );
}

bool SdOptionsPrintItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return maOptionsPrint == static_cast<const SdOptionsPrintItem&>(rAttr).maOptionsPrint;
}

void SdOptionsPrintItem::SetOptions( SdOptions* pOpts ) const
{
    if( !pOpts )
        return;

    pOpts->SetDraw( maOptionsPrint.IsDraw() );
    pOpts->SetNotes( maOptionsPrint.IsNotes() );
    pOpts->SetHandout( maOptionsPrint.IsHandout() );
    pOpts->SetOutline( maOptionsPrint.IsOutline() );
    pOpts->SetDate( maOptionsPrint.IsDate() );
    pOpts->SetTime( maOptionsPrint.IsTime() );
    pOpts->SetPagename( maOptionsPrint.IsPagename() );
    pOpts->SetHiddenPages( maOptionsPrint.IsHiddenPages() );
    pOpts->SetPagesize( maOptionsPrint.IsPagesize() );
    pOpts->SetPagetile( maOptionsPrint.IsPagetile() );
    pOpts->SetWarningPrinter( maOptionsPrint.IsWarningPrinter() );
    pOpts->SetWarningSize( maOptionsPrint.IsWarningSize() );
    pOpts->SetWarningOrientation( maOptionsPrint.IsWarningOrientation() );
    pOpts->SetBooklet( maOptionsPrint.IsBooklet() );
    pOpts->SetFrontPage( maOptionsPrint.IsFrontPage() );
    pOpts->SetBackPage( maOptionsPrint.IsBackPage() );
    pOpts->SetCutPage( maOptionsPrint.IsCutPage() );
    pOpts->SetPaperbin( maOptionsPrint.IsPaperbin() );
    pOpts->SetOutputQuality( maOptionsPrint.GetOutputQuality() );
}

/*************************************************************************
|*
|* SdOptions
|*
\************************************************************************/

SdOptions::SdOptions(bool bImpress) :
    SdOptionsMisc( bImpress, true ),
    SdOptionsGrid( bImpress ),
    SdOptionsPrint( bImpress, true )
{
}

SdOptions::~SdOptions()
{
}

void SdOptions::StoreConfig()
{
    SdOptionsMisc::Store();
    SdOptionsGrid::Store();
    SdOptionsPrint::Store();
}

sal_Int32 SdOptionsMisc::GetDragThresholdPixels() const
{
    Init();
    return mnDragThresholdPixels;
}

void SdOptionsMisc::SetDragThreshold(sal_Int32 nDragThresholdPixels)
{
    if (mnDragThresholdPixels != nDragThresholdPixels)
    {
        OptionsChanged();
        mnDragThresholdPixels = nDragThresholdPixels;
    }
}

sal_Int32 SdOptionsMisc::GetDisplay() const
{
    Init();
    return mnDisplay;
}

void SdOptionsMisc::SetDisplay( sal_Int32 nDisplay )
{
    if( mnDisplay != nDisplay )
    {
        OptionsChanged();
        mnDisplay = nDisplay;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
