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

#include <svx/svdmodel.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <tools/helpers.hxx>
#include <unotools/syslocale.hxx>

#include "sdmod.hxx"
#include "optsitem.hxx"
#include "cfgids.hxx"
#include "FrameView.hxx"

using namespace ::utl;
using namespace ::com::sun::star::uno;

template< class T > T getSafeValue( const Any& rAny )
{
    T value = T();
    bool bOk = (rAny >>= value);

    DBG_ASSERT( bOk, "SdOptionsItem, wrong type from configuration!" );
    (void)bOk;

    return value;
}

// - SdOptionsItem -

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

void SdOptionsItem::SetModified()
{
    ConfigItem::SetModified();
}

// - SdOptionsGeneric -
SdOptionsGeneric::SdOptionsGeneric(sal_uInt16 nConfigId, const OUString& rSubTree)
    : maSubTree(rSubTree)
    , mpCfgItem( nullptr)
    , mnConfigId(nConfigId)
    , mbInit(rSubTree.isEmpty())
    , mbEnableModify(false)
{
}

void SdOptionsGeneric::Init() const
{
    if( !mbInit )
    {
        SdOptionsGeneric* pThis = const_cast<SdOptionsGeneric*>(this);

        if( !mpCfgItem )
            pThis->mpCfgItem = new SdOptionsItem( *this, maSubTree );

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
}

SdOptionsGeneric::~SdOptionsGeneric()
{
    delete mpCfgItem;
    mpCfgItem = nullptr;
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

    return ( eSys == MEASURE_METRIC );
}

/*************************************************************************
|*
|* SdOptionsLayout
|*
\************************************************************************/

SdOptionsLayout::SdOptionsLayout(  sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        OUString( "Office.Draw/Layout" ) :
                        OUString( "Office.Impress/Layout" ) ) :
                      OUString() ),
    bRuler( true ),
    bMoveOutline( true ),
    bDragStripes( false ),
    bHandlesBezier( false ),
    bHelplines( true ),
    nMetric((sal_uInt16)(isMetricSystem() ? FUNIT_CM : FUNIT_INCH)),
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
    if( pValues[0].hasValue() ) SetRulerVisible( *static_cast<sal_Bool const *>(pValues[ 0 ].getValue()) );
    if( pValues[1].hasValue() ) SetHandlesBezier( *static_cast<sal_Bool const *>(pValues[ 1 ].getValue()) );
    if( pValues[2].hasValue() ) SetMoveOutline( *static_cast<sal_Bool const *>(pValues[ 2 ].getValue()) );
    if( pValues[3].hasValue() ) SetDragStripes( *static_cast<sal_Bool const *>(pValues[ 3 ].getValue()) );
    if( pValues[4].hasValue() ) SetHelplines( *static_cast<sal_Bool const *>(pValues[ 4 ].getValue()) );
    if( pValues[5].hasValue() ) SetMetric( (sal_uInt16) *static_cast<sal_Int32 const *>(pValues[ 5 ].getValue()) );
    if( pValues[6].hasValue() ) SetDefTab( (sal_uInt16) *static_cast<sal_Int32 const *>(pValues[ 6 ].getValue()) );

    return true;
}

bool SdOptionsLayout::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsRulerVisible();
    pValues[ 1 ] <<= IsHandlesBezier();
    pValues[ 2 ] <<= IsMoveOutline();
    pValues[ 3 ] <<= IsDragStripes();
    pValues[ 4 ] <<= IsHelplines();
    pValues[ 5 ] <<= (sal_Int32) GetMetric();
    pValues[ 6 ] <<= (sal_Int32) GetDefTab();

    return true;
}

/*************************************************************************
|*
|* SdOptionsLayoutItem
|*
\************************************************************************/

SdOptionsLayoutItem::SdOptionsLayoutItem( sal_uInt16 _nWhich )
:   SfxPoolItem     ( _nWhich )
,   maOptionsLayout ( 0, false )
{
}

SdOptionsLayoutItem::SdOptionsLayoutItem( sal_uInt16 _nWhich, SdOptions* pOpts, ::sd::FrameView* pView )
:   SfxPoolItem     ( _nWhich )
,   maOptionsLayout ( 0, false )
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
    const bool bSameType = SfxPoolItem::operator==( rAttr );
    DBG_ASSERT( bSameType, "SdOptionsLayoutItem::operator==(), different pool item type!" );
    return bSameType && ( maOptionsLayout == static_cast< const SdOptionsLayoutItem& >( rAttr ).maOptionsLayout );
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

SdOptionsContents::SdOptionsContents( sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        OUString( "Office.Draw/Content" ) :
                        OUString( "Office.Impress/Content" ) ) :
                      OUString() )
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
|* SdOptionsContentsItem
|*
\************************************************************************/

SdOptionsContentsItem::SdOptionsContentsItem(sal_uInt16 _nWhich, SdOptions*, ::sd::FrameView*)
:   SfxPoolItem         ( _nWhich )
,   maOptionsContents   ( 0, false )
{
}

SfxPoolItem* SdOptionsContentsItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsContentsItem( *this );
}

bool SdOptionsContentsItem::operator==( const SfxPoolItem& rAttr ) const
{
    const bool bSameType = SfxPoolItem::operator==(rAttr);
    DBG_ASSERT( bSameType, "SdOptionsContentsItem::operator==(), different pool item type!" );
    return bSameType && ( maOptionsContents == static_cast<const SdOptionsContentsItem&>( rAttr ).maOptionsContents );
}

/*************************************************************************
|*
|* SdOptionsMisc
|*
\************************************************************************/

SdOptionsMisc::SdOptionsMisc( sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        OUString( "Office.Draw/Misc" ) :
                        OUString( "Office.Impress/Misc" ) ) :
                      OUString() ),
    nDefaultObjectSizeWidth(8000),
    nDefaultObjectSizeHeight(5000),
    bStartWithTemplate( false ),
    bMarkedHitMovesAlways( true ),
    bMoveOnlyDragging( false ),
    bCrookNoContortion( false ),
    bQuickEdit( GetConfigId() != SDCFG_DRAW ),
    bMasterPageCache( true ),
    bDragWithCopy( false ),
    bPickThrough( true ),
    bDoubleClickTextEdit( true ),
    bClickChangeRotation( false ),
    bEnableSdremote( false ),
    bEnablePresenterScreen( true),
    bSolidDragging( true ),
    bSummationOfParagraphs( false ),
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
        "Start/EnablePresenterScreen"
    };

    rCount = ( ( GetConfigId() == SDCFG_IMPRESS ) ? SAL_N_ELEMENTS(aPropNames) : 14 );
    ppNames = aPropNames;
}

bool SdOptionsMisc::ReadData( const Any* pValues )
{
    if( pValues[0].hasValue() ) SetMarkedHitMovesAlways( *static_cast<sal_Bool const *>(pValues[ 0 ].getValue()) );
    if( pValues[1].hasValue() ) SetCrookNoContortion( *static_cast<sal_Bool const *>(pValues[ 1 ].getValue()) );
    if( pValues[2].hasValue() ) SetQuickEdit( *static_cast<sal_Bool const *>(pValues[ 2 ].getValue()) );
    if( pValues[3].hasValue() ) SetMasterPagePaintCaching( *static_cast<sal_Bool const *>(pValues[ 3 ].getValue()) );
    if( pValues[4].hasValue() ) SetDragWithCopy( *static_cast<sal_Bool const *>(pValues[ 4 ].getValue()) );
    if( pValues[5].hasValue() ) SetPickThrough( *static_cast<sal_Bool const *>(pValues[ 5 ].getValue()) );
    if( pValues[6].hasValue() ) SetDoubleClickTextEdit( *static_cast<sal_Bool const *>(pValues[ 6 ].getValue()) );
    if( pValues[7].hasValue() ) SetClickChangeRotation( *static_cast<sal_Bool const *>(pValues[ 7 ].getValue()) );
    if( pValues[9].hasValue() ) SetSolidDragging( *static_cast<sal_Bool const *>(pValues[ 9 ].getValue()) );
    if( pValues[10].hasValue() ) SetDefaultObjectSizeWidth( *static_cast<sal_uInt32 const *>(pValues[ 10 ].getValue()) );
    if( pValues[11].hasValue() ) SetDefaultObjectSizeHeight( *static_cast<sal_uInt32 const *>(pValues[ 11 ].getValue()) );
    if( pValues[12].hasValue() ) SetPrinterIndependentLayout( *static_cast<sal_uInt16 const *>(pValues[ 12 ].getValue()) );

    if( pValues[13].hasValue() )
        SetShowComments(  *static_cast<sal_Bool const *>(pValues[ 13 ].getValue()) );

    // just for Impress
    if( GetConfigId() == SDCFG_IMPRESS )
    {
        if( pValues[14].hasValue() )
            SetStartWithTemplate( *static_cast<sal_Bool const *>(pValues[ 14 ].getValue()) );
        if( pValues[15].hasValue() )
            SetSummationOfParagraphs( *static_cast<sal_Bool const *>(pValues[ 15 ].getValue()) );
        if( pValues[16].hasValue() )
            SetShowUndoDeleteWarning( *static_cast<sal_Bool const *>(pValues[ 16 ].getValue()) );

        if( pValues[17].hasValue() )
            SetSlideshowRespectZOrder(*static_cast<sal_Bool const *>(pValues[ 17 ].getValue()));

        if( pValues[18].hasValue() )
            SetPreviewNewEffects(*static_cast<sal_Bool const *>(pValues[ 18 ].getValue()));

        if( pValues[19].hasValue() )
            SetPreviewChangedEffects(*static_cast<sal_Bool const *>(pValues[ 19 ].getValue()));

        if( pValues[20].hasValue() )
            SetPreviewTransitions(*static_cast<sal_Bool const *>(pValues[ 20 ].getValue()));

        if( pValues[21].hasValue() )
            SetDisplay(*static_cast<sal_Int32 const *>(pValues[ 21 ].getValue()));

        if( pValues[22].hasValue() )
            SetPresentationPenColor( getSafeValue< sal_Int32 >( pValues[ 22 ] ) );

        if( pValues[23].hasValue() )
            SetPresentationPenWidth( getSafeValue< double >( pValues[ 23 ] ) );

        if( pValues[24].hasValue() )
            SetEnableSdremote( *static_cast<sal_Bool const *>(pValues[ 24 ].getValue()) );

        if( pValues[25].hasValue() )
            SetEnablePresenterScreen( *static_cast<sal_Bool const *>(pValues[ 25 ].getValue()) );
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
    pValues[ 8 ] <<= (double)0;// GetPreviewQuality();
    pValues[ 9 ] <<= IsSolidDragging();
    pValues[ 10 ] <<= GetDefaultObjectSizeWidth();
    pValues[ 11 ] <<= GetDefaultObjectSizeHeight();
    pValues[ 12 ] <<= GetPrinterIndependentLayout();
    pValues[ 13 ] <<= IsShowComments();

    // just for Impress
    if( GetConfigId() == SDCFG_IMPRESS )
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
    }

    return true;
}

/*************************************************************************
|*
|* SdOptionsMiscItem
|*
\************************************************************************/

SdOptionsMiscItem::SdOptionsMiscItem( sal_uInt16 _nWhich )
:   SfxPoolItem     ( _nWhich )
,   maOptionsMisc   ( 0, false )
{
}

SdOptionsMiscItem::SdOptionsMiscItem( sal_uInt16 _nWhich, SdOptions* pOpts, ::sd::FrameView* pView )
:   SfxPoolItem     ( _nWhich )
,   maOptionsMisc   ( 0, false )
{
    if( pOpts )
    {
        maOptionsMisc.SetStartWithTemplate( pOpts->IsStartWithTemplate() );
        maOptionsMisc.SetEnableSdremote( pOpts->IsEnableSdremote() );
        maOptionsMisc.SetEnablePresenterScreen( pOpts->IsEnablePresenterScreen() );
        maOptionsMisc.SetSummationOfParagraphs( pOpts->IsSummationOfParagraphs() );
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
    const bool bSameType = SfxPoolItem::operator==(rAttr);
    DBG_ASSERT( bSameType, "SdOptionsMiscItem::operator==(), different pool item type!" );
    return bSameType && ( maOptionsMisc == static_cast< const SdOptionsMiscItem& >(rAttr).maOptionsMisc );
}

void SdOptionsMiscItem::SetOptions( SdOptions* pOpts ) const
{
    if( pOpts )
    {
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
}

/*************************************************************************
|*
|* SdOptionsSnap
|*
\************************************************************************/

SdOptionsSnap::SdOptionsSnap( sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        OUString( "Office.Draw/Snap" ) :
                        OUString( "Office.Impress/Snap" ) ) :
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
    if( pValues[0].hasValue() ) SetSnapHelplines( *static_cast<sal_Bool const *>(pValues[ 0 ].getValue()) );
    if( pValues[1].hasValue() ) SetSnapBorder( *static_cast<sal_Bool const *>(pValues[ 1 ].getValue()) );
    if( pValues[2].hasValue() ) SetSnapFrame( *static_cast<sal_Bool const *>(pValues[ 2 ].getValue()) );
    if( pValues[3].hasValue() ) SetSnapPoints( *static_cast<sal_Bool const *>(pValues[ 3 ].getValue()) );
    if( pValues[4].hasValue() ) SetOrtho( *static_cast<sal_Bool const *>(pValues[ 4 ].getValue()) );
    if( pValues[5].hasValue() ) SetBigOrtho( *static_cast<sal_Bool const *>(pValues[ 5 ].getValue()) );
    if( pValues[6].hasValue() ) SetRotate( *static_cast<sal_Bool const *>(pValues[ 6 ].getValue()) );
    if( pValues[7].hasValue() ) SetSnapArea( (sal_Int16) *static_cast<sal_Int32 const *>(pValues[ 7 ].getValue()) );
    if( pValues[8].hasValue() ) SetAngle( (sal_Int16) *static_cast<sal_Int32 const *>(pValues[ 8 ].getValue()) );
    if( pValues[9].hasValue() ) SetEliminatePolyPointLimitAngle( (sal_Int16) *static_cast<sal_Int32 const *>(pValues[ 9 ].getValue()) );

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
    pValues[ 7 ] <<= (sal_Int32) GetSnapArea();
    pValues[ 8 ] <<= (sal_Int32) GetAngle();
    pValues[ 9 ] <<= (sal_Int32) GetEliminatePolyPointLimitAngle();

    return true;
}

/*************************************************************************
|*
|* SdOptionsSnapItem
|*
\************************************************************************/

SdOptionsSnapItem::SdOptionsSnapItem( sal_uInt16 _nWhich )
:   SfxPoolItem     ( _nWhich )
,   maOptionsSnap   ( 0, false )
{
}

SdOptionsSnapItem::SdOptionsSnapItem( sal_uInt16 _nWhich, SdOptions* pOpts, ::sd::FrameView* pView )
:   SfxPoolItem     ( _nWhich )
,   maOptionsSnap   ( 0, false )
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
        maOptionsSnap.SetAngle( (sal_Int16) pView->GetSnapAngle() );
        maOptionsSnap.SetEliminatePolyPointLimitAngle( (sal_Int16) pView->GetEliminatePolyPointLimitAngle() );
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
    const bool bSameType = SfxPoolItem::operator==(rAttr);
    DBG_ASSERT( bSameType, "SdOptionsSnapItem::operator==(), different pool item type!" );
    return bSameType && ( maOptionsSnap == static_cast< const SdOptionsSnapItem& >(rAttr).maOptionsSnap );
}

void SdOptionsSnapItem::SetOptions( SdOptions* pOpts ) const
{
    if( pOpts )
    {
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
}

/*************************************************************************
|*
|* SdOptionsZoom
|*
\************************************************************************/

SdOptionsZoom::SdOptionsZoom( sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, ( bUseConfig &&  ( SDCFG_DRAW == nConfigId ) ) ?
                                 OUString( "Office.Draw/Zoom" ) :
                                 OUString() ),
    nX( 1 ),
    nY( 1 )

{
    EnableModify( true );
}

bool SdOptionsZoom::operator==( const SdOptionsZoom& rOpt ) const
{
    sal_Int32 nX1, nX2, nY1, nY2;

    GetScale( nX1, nY1 );
    rOpt.GetScale( nX2, nY2 );

    return( ( nX1 == nX2 ) &&
            ( nY1 == nY2 ) );
}

void SdOptionsZoom::GetPropNameArray( const char**& ppNames, sal_uLong& rCount ) const
{
    static const char* aPropNames[] =
    {
        "ScaleX",
        "ScaleY"
    };

    rCount = ( GetConfigId() == SDCFG_DRAW ) ? SAL_N_ELEMENTS(aPropNames) : 0;
    ppNames = aPropNames;
}

bool SdOptionsZoom::ReadData( const Any* pValues )
{
    sal_Int32 x = 1, y = 1;

    if( pValues[0].hasValue() ) x = ( *static_cast<sal_Int32 const *>(pValues[ 0 ].getValue()) );
    if( pValues[1].hasValue() ) y = ( *static_cast<sal_Int32 const *>(pValues[ 1 ].getValue()) );

    SetScale( x, y );

    return true;
}

bool SdOptionsZoom::WriteData( Any* pValues ) const
{
    sal_Int32 x, y;

    GetScale( x, y );

    pValues[ 0 ] <<= (sal_Int32) x;
    pValues[ 1 ] <<= (sal_Int32) y;

    return true;
}

/*************************************************************************
|*
|* SdOptionsGrid
|*
\************************************************************************/

SdOptionsGrid::SdOptionsGrid( sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        OUString( "Office.Draw/Grid" ) :
                        OUString( "Office.Impress/Grid" ) ) :
                      OUString() )
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

bool SdOptionsGrid::operator==( const SdOptionsGrid& rOpt ) const
{
    return( GetFieldDrawX() == rOpt.GetFieldDrawX() &&
            GetFieldDivisionX() == rOpt.GetFieldDivisionX() &&
            GetFieldDrawY() == rOpt.GetFieldDrawY() &&
            GetFieldDivisionY() == rOpt.GetFieldDivisionY() &&
            GetFieldSnapX() == rOpt.GetFieldSnapX() &&
            GetFieldSnapY() == rOpt.GetFieldSnapY() &&
            IsUseGridSnap() == rOpt.IsUseGridSnap() &&
            IsSynchronize() == rOpt.IsSynchronize() &&
            IsGridVisible() == rOpt.IsGridVisible() &&
            IsEqualGrid() == rOpt.IsEqualGrid() );
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
    if( pValues[0].hasValue() ) SetFieldDrawX( *static_cast<sal_Int32 const *>(pValues[ 0 ].getValue()) );
    if( pValues[1].hasValue() ) SetFieldDrawY( *static_cast<sal_Int32 const *>(pValues[ 1 ].getValue()) );

    if( pValues[2].hasValue() )
    {
        const sal_uInt32 nDivX = FRound( *static_cast<double const *>(pValues[ 2 ].getValue()) );
        SetFieldDivisionX( SvxOptionsGrid::GetFieldDrawX() / ( nDivX + 1 ) );
    }

    if( pValues[3].hasValue() )
    {
        const sal_uInt32 nDivY = FRound( *static_cast<double const *>(pValues[ 3 ].getValue()) );
        SetFieldDivisionY( SvxOptionsGrid::GetFieldDrawY() / ( nDivY + 1 ) );
    }

    if( pValues[4].hasValue() ) SetFieldSnapX( *static_cast<sal_Int32 const *>(pValues[ 4 ].getValue()) );
    if( pValues[5].hasValue() ) SetFieldSnapY( *static_cast<sal_Int32 const *>(pValues[ 5 ].getValue()) );
    if( pValues[6].hasValue() ) SetUseGridSnap( *static_cast<sal_Bool const *>(pValues[ 6 ].getValue()) );
    if( pValues[7].hasValue() ) SetSynchronize( *static_cast<sal_Bool const *>(pValues[ 7 ].getValue()) );
    if( pValues[8].hasValue() ) SetGridVisible( *static_cast<sal_Bool const *>(pValues[ 8 ].getValue()) );
    if( pValues[9].hasValue() ) SetEqualGrid( *static_cast<sal_Bool const *>(pValues[ 9 ].getValue()) );

    return true;
}

bool SdOptionsGrid::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= (sal_Int32) GetFieldDrawX();
    pValues[ 1 ] <<= (sal_Int32) GetFieldDrawY();
    pValues[ 2 ] <<= ( GetFieldDivisionX() ? ( (double) GetFieldDrawX() / GetFieldDivisionX() - 1.0 ) : (double) 0 );
    pValues[ 3 ] <<= ( GetFieldDivisionY() ? ( (double) GetFieldDrawY() / GetFieldDivisionY() - 1.0 ) : (double) 0 );
    pValues[ 4 ] <<= (sal_Int32) GetFieldSnapX();
    pValues[ 5 ] <<= (sal_Int32) GetFieldSnapY();
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

SdOptionsGridItem::SdOptionsGridItem( sal_uInt16 _nWhich, SdOptions* pOpts, ::sd::FrameView* pView ) :
    SvxGridItem( _nWhich )
{
    SetSynchronize( pOpts->IsSynchronize() );
    SetEqualGrid( pOpts->IsEqualGrid() );

    if( pView )
    {
        SetFieldDrawX( pView->GetGridCoarse().Width() );
        SetFieldDrawY( pView->GetGridCoarse().Height() );
        SetFieldDivisionX( pView->GetGridFine().Width() ? ( GetFieldDrawX() / pView->GetGridFine().Width() - 1 ) : 0 );
        SetFieldDivisionY( pView->GetGridFine().Height() ? ( GetFieldDrawY() / pView->GetGridFine().Height() - 1 ) : 0 );
        SetFieldSnapX( long(pView->GetSnapGridWidthX()) );
        SetFieldSnapY( long(pView->GetSnapGridWidthY()) );
        SetUseGridSnap( pView->IsGridSnap() );
        SetGridVisible( pView->IsGridVisible() );
    }
    else
    {
        SetFieldDrawX( pOpts->GetFieldDrawX() );
        SetFieldDrawY( pOpts->GetFieldDrawY() );
        SetFieldDivisionX( pOpts->GetFieldDivisionX() ? ( pOpts->GetFieldDrawX() / pOpts->GetFieldDivisionX() - 1 ) : 0 );
        SetFieldDivisionY( pOpts->GetFieldDivisionY() ? ( pOpts->GetFieldDrawY() / pOpts->GetFieldDivisionY() - 1 ) : 0 );
        SetFieldSnapX( pOpts->GetFieldSnapX() );
        SetFieldSnapY( pOpts->GetFieldSnapY() );
        SetUseGridSnap( pOpts->IsUseGridSnap() );
        SetGridVisible( pOpts->IsGridVisible() );
    }
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

SdOptionsPrint::SdOptionsPrint( sal_uInt16 nConfigId, bool bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        OUString( "Office.Draw/Print" ) :
                        OUString( "Office.Impress/Print" ) ) :
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
    if( GetConfigId() == SDCFG_IMPRESS )
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
    if( pValues[0].hasValue() ) SetDate( *static_cast<sal_Bool const *>(pValues[ 0 ].getValue()) );
    if( pValues[1].hasValue() ) SetTime( *static_cast<sal_Bool const *>(pValues[ 1 ].getValue()) );
    if( pValues[2].hasValue() ) SetPagename( *static_cast<sal_Bool const *>(pValues[ 2 ].getValue()) );
    if( pValues[3].hasValue() ) SetHiddenPages( *static_cast<sal_Bool const *>(pValues[ 3 ].getValue()) );
    if( pValues[4].hasValue() ) SetPagesize( *static_cast<sal_Bool const *>(pValues[ 4 ].getValue()) );
    if( pValues[5].hasValue() ) SetPagetile( *static_cast<sal_Bool const *>(pValues[ 5 ].getValue()) );
    if( pValues[6].hasValue() ) SetBooklet( *static_cast<sal_Bool const *>(pValues[ 6 ].getValue()) );
    if( pValues[7].hasValue() ) SetFrontPage( *static_cast<sal_Bool const *>(pValues[ 7 ].getValue()) );
    if( pValues[8].hasValue() ) SetBackPage( *static_cast<sal_Bool const *>(pValues[ 8 ].getValue()) );
    if( pValues[9].hasValue() ) SetPaperbin( *static_cast<sal_Bool const *>(pValues[ 9 ].getValue()) );
    if( pValues[10].hasValue() ) SetOutputQuality( (sal_uInt16) *static_cast<sal_Int32 const *>(pValues[ 10 ].getValue()) );
    if( pValues[11].hasValue() ) SetDraw( *static_cast<sal_Bool const *>(pValues[ 11 ].getValue()) );

    // just for impress
    if( GetConfigId() == SDCFG_IMPRESS )
    {
        if( pValues[12].hasValue() ) SetNotes( *static_cast<sal_Bool const *>(pValues[ 12 ].getValue()) );
        if( pValues[13].hasValue() ) SetHandout( *static_cast<sal_Bool const *>(pValues[ 13 ].getValue()) );
        if( pValues[14].hasValue() ) SetOutline( *static_cast<sal_Bool const *>(pValues[ 14 ].getValue()) );
        if( pValues[15].hasValue() ) SetHandoutHorizontal( *static_cast<sal_Bool const *>(pValues[15].getValue()) );
        if( pValues[16].hasValue() ) SetHandoutPages( (sal_uInt16)*static_cast<sal_Int32 const *>(pValues[16].getValue()) );
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
    pValues[ 10 ] <<= (sal_Int32) GetOutputQuality();
    pValues[ 11 ] <<= IsDraw();

    // just for impress
    if( GetConfigId() == SDCFG_IMPRESS )
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

SdOptionsPrintItem::SdOptionsPrintItem( sal_uInt16 _nWhich )
:   SfxPoolItem     ( _nWhich )
,   maOptionsPrint  ( 0, false )
{
}

SdOptionsPrintItem::SdOptionsPrintItem( sal_uInt16 _nWhich, SdOptions* pOpts, ::sd::FrameView* )
:   SfxPoolItem     ( _nWhich )
,   maOptionsPrint  ( 0, false )
{
    if( pOpts )
    {
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
}

SfxPoolItem* SdOptionsPrintItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsPrintItem( *this );
}

bool SdOptionsPrintItem::operator==( const SfxPoolItem& rAttr ) const
{
    const bool bSameType = SfxPoolItem::operator==(rAttr);
    DBG_ASSERT( bSameType, "SdOptionsPrintItem::operator==(), different pool item type!" );
    return bSameType && ( maOptionsPrint == static_cast< const SdOptionsPrintItem& >( rAttr ).maOptionsPrint );
}

void SdOptionsPrintItem::SetOptions( SdOptions* pOpts ) const
{
    if( pOpts )
    {
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
}

/*************************************************************************
|*
|* SdOptions
|*
\************************************************************************/

SdOptions::SdOptions( sal_uInt16 nConfigId ) :
    SdOptionsLayout( nConfigId, true ),
    SdOptionsContents( nConfigId, true ),
    SdOptionsMisc( nConfigId, true ),
    SdOptionsSnap( nConfigId, true ),
    SdOptionsZoom( nConfigId, true ),
    SdOptionsGrid( nConfigId, true ),
    SdOptionsPrint( nConfigId, true )
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
