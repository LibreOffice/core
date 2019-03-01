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
#include <tools/helpers.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>

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

    const Sequence< OUString >  aNames( GetPropertyNames() );
    const Sequence< Any >       aValues = mpCfgItem->GetProperties( aNames );

    if( aNames.getLength() && ( aValues.getLength() == aNames.getLength() ) )
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

    if( aNames.getLength() && ( aValues.getLength() == aNames.getLength() ) )
    {
        if( WriteData( aValues.getArray() ) )
            rCfgItem.PutProperties( aNames, aValues );
        else
        {
            OSL_FAIL( "PutProperties failed" );
        }
    }
}

Sequence< OUString > SdOptionsGeneric::GetPropertyNames() const
{
    sal_uLong           nCount;
    const char**    ppPropNames;

    GetPropNameArray( ppPropNames, nCount );

    Sequence< OUString > aNames( nCount );
    OUString*            pNames = aNames.getArray();

    for( sal_uLong i = 0; i < nCount; i++ )
        pNames[ i ] = OUString::createFromAscii( ppPropNames[ i ] );

    return aNames;
}

void SdOptionsGeneric::Store()
{
    if( mpCfgItem )
        mpCfgItem->Commit();
}

bool SdOptionsGeneric::isMetricSystem()
{
    SvtSysLocale aSysLocale;
    MeasurementSystem eSys = aSysLocale.GetLocaleDataPtr()->getMeasurementSystemEnum();

    return ( eSys == MeasurementSystem::Metric );
}

/*************************************************************************
|*
|* SdOptionsLayout
|*
\************************************************************************/

SdOptionsLayout::SdOptionsLayout(bool bImpress, bool bUseConfig) :
    SdOptionsGeneric( bImpress, bUseConfig ?
                      ( bImpress ?
                        OUString( "Office.Impress/Layout" ) :
                        OUString( "Office.Draw/Layout" ) ) :
                      OUString() ),
    bRuler( true ),
    bMoveOutline( true ),
    bDragStripes( false ),
    bHandlesBezier( false ),
    bHelplines( true ),
    nMetric(static_cast<sal_uInt16>(isMetricSystem() ? FieldUnit::CM : FieldUnit::INCH)),
    nDefTab( 1250 )
{
    EnableModify( true );
}

bool SdOptionsLayout::operator==( const SdOptionsLayout& rOpt ) const
{
    return( IsRulerVisible() == rOpt.IsRulerVisible() &&
            IsMoveOutline() == rOpt.IsMoveOutline() &&
            IsDragStripes() == rOpt.IsDragStripes() &&
            IsHandlesBezier() == rOpt.IsHandlesBezier() &&
            IsHelplines() == rOpt.IsHelplines() &&
            GetMetric() == rOpt.GetMetric() &&
            GetDefTab() == rOpt.GetDefTab() );
}

void SdOptionsLayout::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    if( isMetricSystem() )
    {
        static const char* aPropNamesMetric[] =
        {
            "Display/Ruler",
            "Display/Bezier",
            "Display/Contour",
            "Display/Guide",
            "Display/Helpline",
            "Other/MeasureUnit/Metric",
            "Other/TabStop/Metric"
        };
        ppNames = aPropNamesMetric;
        rCount = SAL_N_ELEMENTS(aPropNamesMetric);
    }
    else
    {
        static const char* aPropNamesNonMetric[] =
        {
            "Display/Ruler",
            "Display/Bezier",
            "Display/Contour",
            "Display/Guide",
            "Display/Helpline",
            "Other/MeasureUnit/NonMetric",
            "Other/TabStop/NonMetric"
        };
        ppNames = aPropNamesNonMetric;
        rCount = SAL_N_ELEMENTS(aPropNamesNonMetric);
    }
}

bool SdOptionsLayout::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetRulerVisible( *o3tl::doAccess<bool>(pValues[ 0 ]) );
    if( pValues[1].hasValue() ) SetHandlesBezier( *o3tl::doAccess<bool>(pValues[ 1 ]) );
    if( pValues[2].hasValue() ) SetMoveOutline( *o3tl::doAccess<bool>(pValues[ 2 ]) );
    if( pValues[3].hasValue() ) SetDragStripes( *o3tl::doAccess<bool>(pValues[ 3 ]) );
    if( pValues[4].hasValue() ) SetHelplines( *o3tl::doAccess<bool>(pValues[ 4 ]) );
    if( pValues[5].hasValue() ) SetMetric( static_cast<sal_uInt16>(*o3tl::doAccess<sal_Int32>(pValues[ 5 ])) );
    if( pValues[6].hasValue() ) SetDefTab( static_cast<sal_uInt16>(*o3tl::doAccess<sal_Int32>(pValues[ 6 ])) );

    return true;
}

bool SdOptionsLayout::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsRulerVisible();
    pValues[ 1 ] <<= IsHandlesBezier();
    pValues[ 2 ] <<= IsMoveOutline();
    pValues[ 3 ] <<= IsDragStripes();
    pValues[ 4 ] <<= IsHelplines();
    pValues[ 5 ] <<= static_cast<sal_Int32>(GetMetric());
    pValues[ 6 ] <<= static_cast<sal_Int32>(GetDefTab());

    return true;
}

/*************************************************************************
|*
|* SdOptionsLayoutItem
|*
\************************************************************************/

SdOptionsLayoutItem::SdOptionsLayoutItem()
:   SfxPoolItem     ( ATTR_OPTIONS_LAYOUT )
,   maOptionsLayout ( false, false )
{
}

SdOptionsLayoutItem::SdOptionsLayoutItem( SdOptions const * pOpts, ::sd::FrameView const * pView )
:   SfxPoolItem     ( ATTR_OPTIONS_LAYOUT )
,   maOptionsLayout ( false, false )
{
    if( pOpts )
    {
        maOptionsLayout.SetMetric( pOpts->GetMetric() );
        maOptionsLayout.SetDefTab( pOpts->GetDefTab() );
    }

    if( pView )
    {
        maOptionsLayout.SetRulerVisible( pView->HasRuler() );
        maOptionsLayout.SetMoveOutline( !pView->IsNoDragXorPolys() );
        maOptionsLayout.SetDragStripes( pView->IsDragStripes() );
        maOptionsLayout.SetHandlesBezier( pView->IsPlusHandlesAlwaysVisible() );
        maOptionsLayout.SetHelplines( pView->IsHlplVisible() );
    }
    else if( pOpts )
    {
        maOptionsLayout.SetRulerVisible( pOpts->IsRulerVisible() );
        maOptionsLayout.SetMoveOutline( pOpts->IsMoveOutline() );
        maOptionsLayout.SetDragStripes( pOpts->IsDragStripes() );
        maOptionsLayout.SetHandlesBezier( pOpts->IsHandlesBezier() );
        maOptionsLayout.SetHelplines( pOpts->IsHelplines() );
    }
}

SfxPoolItem* SdOptionsLayoutItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsLayoutItem( *this );
}

bool SdOptionsLayoutItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return maOptionsLayout == static_cast<const SdOptionsLayoutItem&>(rAttr).maOptionsLayout;
}

void SdOptionsLayoutItem::SetOptions( SdOptions* pOpts ) const
{
    if( pOpts )
    {
        pOpts->SetRulerVisible( maOptionsLayout.IsRulerVisible() );
        pOpts->SetMoveOutline( maOptionsLayout.IsMoveOutline() );
        pOpts->SetDragStripes( maOptionsLayout.IsDragStripes() );
        pOpts->SetHandlesBezier( maOptionsLayout.IsHandlesBezier() );
        pOpts->SetHelplines( maOptionsLayout.IsHelplines() );
        pOpts->SetMetric( maOptionsLayout.GetMetric() );
        pOpts->SetDefTab( maOptionsLayout.GetDefTab() );
    }
}

/*************************************************************************
|*
|* SdOptionsContents
|*
\************************************************************************/

SdOptionsContents::SdOptionsContents(bool bImpress) :
    SdOptionsGeneric( bImpress, bImpress ?
                        OUString( "Office.Impress/Content" ) :
                        OUString( "Office.Draw/Content" ) )
{
    EnableModify( true );
}

bool SdOptionsContents::operator==(const SdOptionsContents&) const
{
    return true;
}

void SdOptionsContents::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Display/PicturePlaceholder",
        "Display/ContourMode",
        "Display/LineContour",
        "Display/TextPlaceholder"
    };

    rCount = SAL_N_ELEMENTS(aPropNames);
    ppNames = aPropNames;
}

bool SdOptionsContents::ReadData(const Any*)
{
    return true;
}

bool SdOptionsContents::WriteData( Any* pValues ) const
{
    //#i80528# no draft anymore
    pValues[ 0 ] <<= false;
    pValues[ 1 ] <<= false;
    pValues[ 2 ] <<= false;
    pValues[ 3 ] <<= false;

    return true;
}
/*************************************************************************
|*
|* SdOptionsMisc
|*
\************************************************************************/

SdOptionsMisc::SdOptionsMisc( bool bImpress, bool bUseConfig ) :
    SdOptionsGeneric( bImpress, bUseConfig ?
                      ( bImpress ?
                        OUString( "Office.Impress/Misc" ) :
                        OUString( "Office.Draw/Misc" ) ) :
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
    bEnableSdremote( false ),
    bEnablePresenterScreen( true),
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
    mnPrinterIndependentLayout (1)
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
            IsEnableSdremote() == rOpt.IsEnableSdremote() &&
            IsEnablePresenterScreen() == rOpt.IsEnablePresenterScreen()&&
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
            GetPresentationPenWidth() == rOpt.GetPresentationPenWidth()
        );
}

void SdOptionsMisc::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    static const char* aPropNames[] =
    {
        "ObjectMoveable",
        "NoDistort",
        "TextObject/QuickEditing",
        "BackgroundCache",
        "CopyWhileMoving",
        "TextObject/Selectable",
        "DclickTextedit",
        "RotateClick",
        "Preview",
        "ModifyWithAttributes",
        "DefaultObjectSize/Width",
        "DefaultObjectSize/Height",

        "Compatibility/PrinterIndependentLayout",

        "ShowComments",

        // just for impress
        "NewDoc/AutoPilot",
        "Compatibility/AddBetween",
        "ShowUndoDeleteWarning",
        "SlideshowRespectZOrder",

        "PreviewNewEffects",
        "PreviewChangedEffects",
        "PreviewTransitions",

        "Display",

        "PenColor",
        "PenWidth",
        "Start/EnableSdremote",
        "Start/EnablePresenterScreen",
        "TabBarVisible"
    };

    rCount = ( IsImpress() ? SAL_N_ELEMENTS(aPropNames) : 14 );
    ppNames = aPropNames;
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
    if( pValues[10].hasValue() ) SetDefaultObjectSizeWidth( *o3tl::doAccess<sal_uInt32>(pValues[ 10 ]) );
    if( pValues[11].hasValue() ) SetDefaultObjectSizeHeight( *o3tl::doAccess<sal_uInt32>(pValues[ 11 ]) );
    if( pValues[12].hasValue() ) SetPrinterIndependentLayout( *o3tl::doAccess<sal_uInt16>(pValues[ 12 ]) );

    if( pValues[13].hasValue() )
        SetShowComments(  *o3tl::doAccess<bool>(pValues[ 13 ]) );

    // just for Impress
    if (IsImpress())
    {
        if( pValues[14].hasValue() )
            SetStartWithTemplate( *o3tl::doAccess<bool>(pValues[ 14 ]) );
        if( pValues[15].hasValue() )
            SetSummationOfParagraphs( *o3tl::doAccess<bool>(pValues[ 15 ]) );
        if( pValues[16].hasValue() )
            SetShowUndoDeleteWarning( *o3tl::doAccess<bool>(pValues[ 16 ]) );

        if( pValues[17].hasValue() )
            SetSlideshowRespectZOrder(*o3tl::doAccess<bool>(pValues[ 17 ]));

        if( pValues[18].hasValue() )
            SetPreviewNewEffects(*o3tl::doAccess<bool>(pValues[ 18 ]));

        if( pValues[19].hasValue() )
            SetPreviewChangedEffects(*o3tl::doAccess<bool>(pValues[ 19 ]));

        if( pValues[20].hasValue() )
            SetPreviewTransitions(*o3tl::doAccess<bool>(pValues[ 20 ]));

        if( pValues[21].hasValue() )
            SetDisplay(*o3tl::doAccess<sal_Int32>(pValues[ 21 ]));

        if( pValues[22].hasValue() )
            SetPresentationPenColor( getSafeValue< sal_Int32 >( pValues[ 22 ] ) );

        if( pValues[23].hasValue() )
            SetPresentationPenWidth( getSafeValue< double >( pValues[ 23 ] ) );

        if( pValues[24].hasValue() )
            SetEnableSdremote( *o3tl::doAccess<bool>(pValues[ 24 ]) );

        if( pValues[25].hasValue() )
            SetEnablePresenterScreen( *o3tl::doAccess<bool>(pValues[ 25 ]) );

        if( pValues[26].hasValue() ) {
            SetTabBarVisible( *o3tl::doAccess<bool>(pValues[ 26 ]) );
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

    // just for Impress
    if (IsImpress())
    {
        pValues[ 14 ] <<= IsStartWithTemplate();
        pValues[ 15 ] <<= IsSummationOfParagraphs();
        pValues[ 16 ] <<= IsShowUndoDeleteWarning();
        pValues[ 17 ] <<= IsSlideshowRespectZOrder();

        pValues[ 18 ] <<= IsPreviewNewEffects();
        pValues[ 19 ] <<= IsPreviewChangedEffects();
        pValues[ 20 ] <<= IsPreviewTransitions();

        pValues[ 21 ] <<= GetDisplay();

        pValues[ 22 ] <<= GetPresentationPenColor();
        pValues[ 23 ] <<= GetPresentationPenWidth();
        pValues[ 24 ] <<= IsEnableSdremote();
        pValues[ 25 ] <<= IsEnablePresenterScreen();
        pValues[ 26 ] <<= IsTabBarVisible();
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
        maOptionsMisc.SetEnableSdremote( pOpts->IsEnableSdremote() );
        maOptionsMisc.SetEnablePresenterScreen( pOpts->IsEnablePresenterScreen() );
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
        maOptionsMisc.SetPickThrough( pView->GetModel()->IsPickThroughTransparentTextFrames() );
        maOptionsMisc.SetDoubleClickTextEdit( pView->IsDoubleClickTextEdit() );
        maOptionsMisc.SetClickChangeRotation( pView->IsClickChangeRotation() );
        maOptionsMisc.SetSolidDragging( pView->IsSolidDragging() );
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
    }
}

SfxPoolItem* SdOptionsMiscItem::Clone( SfxItemPool* ) const
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

    pOpts->SetStartWithTemplate( maOptionsMisc.IsStartWithTemplate() );
    pOpts->SetMarkedHitMovesAlways( maOptionsMisc.IsMarkedHitMovesAlways() );
    pOpts->SetMoveOnlyDragging( maOptionsMisc.IsMoveOnlyDragging() );
    pOpts->SetCrookNoContortion( maOptionsMisc.IsCrookNoContortion() );
    pOpts->SetQuickEdit( maOptionsMisc.IsQuickEdit() );
    pOpts->SetMasterPagePaintCaching( maOptionsMisc.IsMasterPagePaintCaching() );
    pOpts->SetDragWithCopy( maOptionsMisc.IsDragWithCopy() );
    pOpts->SetPickThrough( maOptionsMisc.IsPickThrough() );
    pOpts->SetDoubleClickTextEdit( maOptionsMisc.IsDoubleClickTextEdit() );
    pOpts->SetClickChangeRotation( maOptionsMisc.IsClickChangeRotation() );
    pOpts->SetEnableSdremote( maOptionsMisc.IsEnableSdremote() );
    pOpts->SetEnablePresenterScreen( maOptionsMisc.IsEnablePresenterScreen() );
    pOpts->SetSummationOfParagraphs( maOptionsMisc.IsSummationOfParagraphs() );
    pOpts->SetTabBarVisible( maOptionsMisc.IsTabBarVisible() );

    pOpts->SetSolidDragging( maOptionsMisc.IsSolidDragging() );
    pOpts->SetShowUndoDeleteWarning( maOptionsMisc.IsShowUndoDeleteWarning() );
    pOpts->SetPrinterIndependentLayout( maOptionsMisc.GetPrinterIndependentLayout() );
    pOpts->SetShowComments( maOptionsMisc.IsShowComments() );
    pOpts->SetDefaultObjectSizeWidth( maOptionsMisc.GetDefaultObjectSizeWidth() );
    pOpts->SetDefaultObjectSizeHeight( maOptionsMisc.GetDefaultObjectSizeHeight() );

    pOpts->SetPreviewNewEffects( maOptionsMisc.IsPreviewNewEffects() );
    pOpts->SetPreviewChangedEffects( maOptionsMisc.IsPreviewChangedEffects() );
    pOpts->SetPreviewTransitions( maOptionsMisc.IsPreviewTransitions() );

    pOpts->SetDisplay( maOptionsMisc.GetDisplay() );

    pOpts->SetPresentationPenColor( maOptionsMisc.GetPresentationPenColor() );
    pOpts->SetPresentationPenWidth( maOptionsMisc.GetPresentationPenWidth() );
}

/*************************************************************************
|*
|* SdOptionsSnap
|*
\************************************************************************/

SdOptionsSnap::SdOptionsSnap( bool bImpress, bool bUseConfig ) :
    SdOptionsGeneric( bImpress, bUseConfig ?
                      ( bImpress ?
                        OUString( "Office.Impress/Snap" ) :
                        OUString( "Office.Draw/Snap" ) ) :
                      OUString() ),
    bSnapHelplines( true ),
    bSnapBorder( true ),
    bSnapFrame( false ),
    bSnapPoints( false ),
    bOrtho( false ),
    bBigOrtho( true ),
    bRotate( false ),
    nSnapArea( 5 ),
    nAngle( 1500 ),
    nBezAngle( 1500 )

{
    EnableModify( true );
}

bool SdOptionsSnap::operator==( const SdOptionsSnap& rOpt ) const
{
    return( IsSnapHelplines() == rOpt.IsSnapHelplines() &&
            IsSnapBorder() == rOpt.IsSnapBorder() &&
            IsSnapFrame() == rOpt.IsSnapFrame() &&
            IsSnapPoints() == rOpt.IsSnapPoints() &&
            IsOrtho() == rOpt.IsOrtho() &&
            IsBigOrtho() == rOpt.IsBigOrtho() &&
            IsRotate() == rOpt.IsRotate() &&
            GetSnapArea() == rOpt.GetSnapArea() &&
            GetAngle() == rOpt.GetAngle() &&
            GetEliminatePolyPointLimitAngle() == rOpt.GetEliminatePolyPointLimitAngle() );
}

void SdOptionsSnap::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Object/SnapLine",
        "Object/PageMargin",
        "Object/ObjectFrame",
        "Object/ObjectPoint",
        "Position/CreatingMoving",
        "Position/ExtendEdges",
        "Position/Rotating",
        "Object/Range",
        "Position/RotatingValue",
        "Position/PointReduction"
    };

    rCount = SAL_N_ELEMENTS(aPropNames);
    ppNames = aPropNames;
}

bool SdOptionsSnap::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetSnapHelplines( *o3tl::doAccess<bool>(pValues[ 0 ]) );
    if( pValues[1].hasValue() ) SetSnapBorder( *o3tl::doAccess<bool>(pValues[ 1 ]) );
    if( pValues[2].hasValue() ) SetSnapFrame( *o3tl::doAccess<bool>(pValues[ 2 ]) );
    if( pValues[3].hasValue() ) SetSnapPoints( *o3tl::doAccess<bool>(pValues[ 3 ]) );
    if( pValues[4].hasValue() ) SetOrtho( *o3tl::doAccess<bool>(pValues[ 4 ]) );
    if( pValues[5].hasValue() ) SetBigOrtho( *o3tl::doAccess<bool>(pValues[ 5 ]) );
    if( pValues[6].hasValue() ) SetRotate( *o3tl::doAccess<bool>(pValues[ 6 ]) );
    if( pValues[7].hasValue() ) SetSnapArea( static_cast<sal_Int16>(*o3tl::doAccess<sal_Int32>(pValues[ 7 ])) );
    if( pValues[8].hasValue() ) SetAngle( static_cast<sal_Int16>(*o3tl::doAccess<sal_Int32>(pValues[ 8 ])) );
    if( pValues[9].hasValue() ) SetEliminatePolyPointLimitAngle( static_cast<sal_Int16>(*o3tl::doAccess<sal_Int32>(pValues[ 9 ])) );

    return true;
}

bool SdOptionsSnap::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsSnapHelplines();
    pValues[ 1 ] <<= IsSnapBorder();
    pValues[ 2 ] <<= IsSnapFrame();
    pValues[ 3 ] <<= IsSnapPoints();
    pValues[ 4 ] <<= IsOrtho();
    pValues[ 5 ] <<= IsBigOrtho();
    pValues[ 6 ] <<= IsRotate();
    pValues[ 7 ] <<= static_cast<sal_Int32>(GetSnapArea());
    pValues[ 8 ] <<= static_cast<sal_Int32>(GetAngle());
    pValues[ 9 ] <<= static_cast<sal_Int32>(GetEliminatePolyPointLimitAngle());

    return true;
}

/*************************************************************************
|*
|* SdOptionsSnapItem
|*
\************************************************************************/

SdOptionsSnapItem::SdOptionsSnapItem()
:   SfxPoolItem     ( ATTR_OPTIONS_SNAP )
,   maOptionsSnap   ( false, false )
{
}

SdOptionsSnapItem::SdOptionsSnapItem( SdOptions const * pOpts, ::sd::FrameView const * pView )
:   SfxPoolItem     ( ATTR_OPTIONS_SNAP )
,   maOptionsSnap   ( false, false )
{
    if( pView )
    {
        maOptionsSnap.SetSnapHelplines( pView->IsHlplSnap() );
        maOptionsSnap.SetSnapBorder( pView->IsBordSnap() );
        maOptionsSnap.SetSnapFrame( pView->IsOFrmSnap() );
        maOptionsSnap.SetSnapPoints( pView->IsOPntSnap() );
        maOptionsSnap.SetOrtho( pView->IsOrtho() );
        maOptionsSnap.SetBigOrtho( pView->IsBigOrtho() );
        maOptionsSnap.SetRotate( pView->IsAngleSnapEnabled() );
        maOptionsSnap.SetSnapArea( pView->GetSnapMagneticPixel() );
        maOptionsSnap.SetAngle( static_cast<sal_Int16>(pView->GetSnapAngle()) );
        maOptionsSnap.SetEliminatePolyPointLimitAngle( static_cast<sal_Int16>(pView->GetEliminatePolyPointLimitAngle()) );
    }
    else if( pOpts )
    {
        maOptionsSnap.SetSnapHelplines( pOpts->IsSnapHelplines() );
        maOptionsSnap.SetSnapBorder( pOpts->IsSnapBorder() );
        maOptionsSnap.SetSnapFrame( pOpts->IsSnapFrame() );
        maOptionsSnap.SetSnapPoints( pOpts->IsSnapPoints() );
        maOptionsSnap.SetOrtho( pOpts->IsOrtho() );
        maOptionsSnap.SetBigOrtho( pOpts->IsBigOrtho() );
        maOptionsSnap.SetRotate( pOpts->IsRotate() );
        maOptionsSnap.SetSnapArea( pOpts->GetSnapArea() );
        maOptionsSnap.SetAngle( pOpts->GetAngle() );
        maOptionsSnap.SetEliminatePolyPointLimitAngle( pOpts->GetEliminatePolyPointLimitAngle() );
    }
}

SfxPoolItem* SdOptionsSnapItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsSnapItem( *this );
}

bool SdOptionsSnapItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return maOptionsSnap == static_cast<const SdOptionsSnapItem&>(rAttr).maOptionsSnap;
}

void SdOptionsSnapItem::SetOptions( SdOptions* pOpts ) const
{
    if( !pOpts )
        return;

    pOpts->SetSnapHelplines( maOptionsSnap.IsSnapHelplines() );
    pOpts->SetSnapBorder( maOptionsSnap.IsSnapBorder() );
    pOpts->SetSnapFrame( maOptionsSnap.IsSnapFrame() );
    pOpts->SetSnapPoints( maOptionsSnap.IsSnapPoints() );
    pOpts->SetOrtho( maOptionsSnap.IsOrtho() );
    pOpts->SetBigOrtho( maOptionsSnap.IsBigOrtho() );
    pOpts->SetRotate( maOptionsSnap.IsRotate() );
    pOpts->SetSnapArea( maOptionsSnap.GetSnapArea() );
    pOpts->SetAngle( maOptionsSnap.GetAngle() );
    pOpts->SetEliminatePolyPointLimitAngle( maOptionsSnap.GetEliminatePolyPointLimitAngle() );
}

/*************************************************************************
|*
|* SdOptionsZoom
|*
\************************************************************************/

SdOptionsZoom::SdOptionsZoom( bool bImpress ) :
    SdOptionsGeneric( bImpress, bImpress ?
                                 OUString() :
                                 OUString("Office.Draw/Zoom") ),
    nX( 1 ),
    nY( 1 )

{
    EnableModify( true );
}

void SdOptionsZoom::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    static const char* aPropNames[] =
    {
        "ScaleX",
        "ScaleY"
    };

    rCount = !IsImpress() ? SAL_N_ELEMENTS(aPropNames) : 0;
    ppNames = aPropNames;
}

bool SdOptionsZoom::ReadData( const Any* pValues )
{
    sal_Int32 x = 1, y = 1;

    if( pValues[0].hasValue() ) x = *o3tl::doAccess<sal_Int32>(pValues[ 0 ]);
    if( pValues[1].hasValue() ) y = *o3tl::doAccess<sal_Int32>(pValues[ 1 ]);

    SetScale( x, y );

    return true;
}

bool SdOptionsZoom::WriteData( Any* pValues ) const
{
    sal_Int32 x, y;

    GetScale( x, y );

    pValues[ 0 ] <<= x;
    pValues[ 1 ] <<= y;

    return true;
}

/*************************************************************************
|*
|* SdOptionsGrid
|*
\************************************************************************/

SdOptionsGrid::SdOptionsGrid(bool bImpress) :
    SdOptionsGeneric( bImpress,
                      bImpress ?
                        OUString( "Office.Impress/Grid" ) :
                        OUString( "Office.Draw/Grid" )
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
    SetFieldSnapX( nVal );
    SetFieldSnapY( nVal );
    SetUseGridSnap( false );
    SetSynchronize( true );
    SetGridVisible( false );
    SetEqualGrid( true );
}

void SdOptionsGrid::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    if( isMetricSystem() )
    {
        static const char* aPropNamesMetric[] =
        {
            "Resolution/XAxis/Metric",
            "Resolution/YAxis/Metric",
            "Subdivision/XAxis",
            "Subdivision/YAxis",
            "SnapGrid/XAxis/Metric",
            "SnapGrid/YAxis/Metric",
            "Option/SnapToGrid",
            "Option/Synchronize",
            "Option/VisibleGrid",
            "SnapGrid/Size"
        };
        ppNames = aPropNamesMetric;
        rCount = SAL_N_ELEMENTS(aPropNamesMetric);
    }
    else
    {
        static const char* aPropNamesNonMetric[] =
        {
            "Resolution/XAxis/NonMetric",
            "Resolution/YAxis/NonMetric",
            "Subdivision/XAxis",
            "Subdivision/YAxis",
            "SnapGrid/XAxis/NonMetric",
            "SnapGrid/YAxis/NonMetric",
            "Option/SnapToGrid",
            "Option/Synchronize",
            "Option/VisibleGrid",
            "SnapGrid/Size"
        };
        ppNames = aPropNamesNonMetric;
        rCount = SAL_N_ELEMENTS(aPropNamesNonMetric);
    }
}

bool SdOptionsGrid::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetFieldDrawX( *o3tl::doAccess<sal_Int32>(pValues[ 0 ]) );
    if( pValues[1].hasValue() ) SetFieldDrawY( *o3tl::doAccess<sal_Int32>(pValues[ 1 ]) );

    if( pValues[2].hasValue() )
    {
        const sal_uInt32 nDivX = FRound( *o3tl::doAccess<double>(pValues[ 2 ]) );
        SetFieldDivisionX( SvxOptionsGrid::GetFieldDrawX() / ( nDivX + 1 ) );
    }

    if( pValues[3].hasValue() )
    {
        const sal_uInt32 nDivY = FRound( *o3tl::doAccess<double>(pValues[ 3 ]) );
        SetFieldDivisionY( SvxOptionsGrid::GetFieldDrawY() / ( nDivY + 1 ) );
    }

    if( pValues[4].hasValue() ) SetFieldSnapX( *o3tl::doAccess<sal_Int32>(pValues[ 4 ]) );
    if( pValues[5].hasValue() ) SetFieldSnapY( *o3tl::doAccess<sal_Int32>(pValues[ 5 ]) );
    if( pValues[6].hasValue() ) SetUseGridSnap( *o3tl::doAccess<bool>(pValues[ 6 ]) );
    if( pValues[7].hasValue() ) SetSynchronize( *o3tl::doAccess<bool>(pValues[ 7 ]) );
    if( pValues[8].hasValue() ) SetGridVisible( *o3tl::doAccess<bool>(pValues[ 8 ]) );
    if( pValues[9].hasValue() ) SetEqualGrid( *o3tl::doAccess<bool>(pValues[ 9 ]) );

    return true;
}

bool SdOptionsGrid::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= static_cast<sal_Int32>(GetFieldDrawX());
    pValues[ 1 ] <<= static_cast<sal_Int32>(GetFieldDrawY());
    pValues[ 2 ] <<= ( GetFieldDivisionX() ? ( static_cast<double>(GetFieldDrawX()) / GetFieldDivisionX() - 1.0 ) : double(0) );
    pValues[ 3 ] <<= ( GetFieldDivisionY() ? ( static_cast<double>(GetFieldDrawY()) / GetFieldDivisionY() - 1.0 ) : double(0) );
    pValues[ 4 ] <<= static_cast<sal_Int32>(GetFieldSnapX());
    pValues[ 5 ] <<= static_cast<sal_Int32>(GetFieldSnapY());
    pValues[ 6 ] <<= IsUseGridSnap();
    pValues[ 7 ] <<= IsSynchronize();
    pValues[ 8 ] <<= IsGridVisible();
    pValues[ 9 ] <<= IsEqualGrid();

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
    SetEqualGrid( pOpts->IsEqualGrid() );

    SetFieldDrawX( pOpts->GetFieldDrawX() );
    SetFieldDrawY( pOpts->GetFieldDrawY() );
    SetFieldDivisionX( pOpts->GetFieldDivisionX() ? ( pOpts->GetFieldDrawX() / pOpts->GetFieldDivisionX() - 1 ) : 0 );
    SetFieldDivisionY( pOpts->GetFieldDivisionY() ? ( pOpts->GetFieldDrawY() / pOpts->GetFieldDivisionY() - 1 ) : 0 );
    SetFieldSnapX( pOpts->GetFieldSnapX() );
    SetFieldSnapY( pOpts->GetFieldSnapY() );
    SetUseGridSnap( pOpts->IsUseGridSnap() );
    SetGridVisible( pOpts->IsGridVisible() );
}

void SdOptionsGridItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetFieldDrawX( GetFieldDrawX() );
    pOpts->SetFieldDivisionX( GetFieldDrawX() / ( GetFieldDivisionX() + 1 ) );
    pOpts->SetFieldDrawY( GetFieldDrawY() );
    pOpts->SetFieldDivisionY( GetFieldDrawY() / ( GetFieldDivisionY() + 1 ) );
    pOpts->SetFieldSnapX( GetFieldSnapX() );
    pOpts->SetFieldSnapY( GetFieldSnapY() );
    pOpts->SetUseGridSnap( GetUseGridSnap() );
    pOpts->SetSynchronize( GetSynchronize() );
    pOpts->SetGridVisible( GetGridVisible() );
    pOpts->SetEqualGrid( GetEqualGrid() );
}

/*************************************************************************
|*
|* SdOptionsPrint
|*
\************************************************************************/

SdOptionsPrint::SdOptionsPrint( bool bImpress, bool bUseConfig ) :
    SdOptionsGeneric( bImpress, bUseConfig ?
                      ( bImpress ?
                        OUString( "Office.Impress/Print" ) :
                        OUString( "Office.Draw/Print" ) ) :
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

void SdOptionsPrint::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    if (IsImpress())
    {
        static const char* aImpressPropNames[] =
        {
            "Other/Date",
            "Other/Time",
            "Other/PageName",
            "Other/HiddenPage",
            "Page/PageSize",
            "Page/PageTile",
            // bWarningPrinter
            // bWarningSize
            // bWarningOrientation
            "Page/Booklet",
            "Page/BookletFront",
            "Page/BookletBack",
            // bCutPage
            "Other/FromPrinterSetup",
            "Other/Quality",
            "Content/Presentation",
            "Content/Note",
            "Content/Handout",
            "Content/Outline",
            "Other/HandoutHorizontal",
            "Other/PagesPerHandout"
        };
        rCount = SAL_N_ELEMENTS(aImpressPropNames);
        ppNames = aImpressPropNames;
    }
    else
    {
        static const char* aDrawPropNames[] =
        {
            "Other/Date",
            "Other/Time",
            "Other/PageName",
            "Other/HiddenPage",
            "Page/PageSize",
            "Page/PageTile",
            // bWarningPrinter
            // bWarningSize
            // bWarningOrientation
            "Page/Booklet",
            "Page/BookletFront",
            "Page/BookletBack",
            // bCutPage
            "Other/FromPrinterSetup",
            "Other/Quality",
            "Content/Drawing",
        };
        rCount = SAL_N_ELEMENTS(aDrawPropNames);
        ppNames = aDrawPropNames;
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

SfxPoolItem* SdOptionsPrintItem::Clone( SfxItemPool* ) const
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
    SdOptionsLayout( bImpress, true ),
    SdOptionsContents( bImpress ),
    SdOptionsMisc( bImpress, true ),
    SdOptionsSnap( bImpress, true ),
    SdOptionsZoom( bImpress ),
    SdOptionsGrid( bImpress ),
    SdOptionsPrint( bImpress, true )
{
}

SdOptions::~SdOptions()
{
}

void SdOptions::StoreConfig()
{
    SdOptionsLayout::Store();
    SdOptionsContents::Store();
    SdOptionsMisc::Store();
    SdOptionsSnap::Store();
    SdOptionsZoom::Store();
    SdOptionsGrid::Store();
    SdOptionsPrint::Store();
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
