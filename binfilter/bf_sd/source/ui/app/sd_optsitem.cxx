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

#include <bf_sfx2/sfx.hrc>
#include <vcl/salbtype.hxx>
#include <bf_svtools/syslocale.hxx>

#include "optsitem.hxx"
#include "cfgids.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::utl;
using namespace ::com::sun::star::uno;

#define B2U(_def_aStr) (OUString::createFromAscii(_def_aStr))


// -----------------
// - SdOptionsItem -
// -----------------

/*N*/ SdOptionsItem::SdOptionsItem( const SdOptionsGeneric& rParent, const OUString rSubTree ) :
/*N*/ 	ConfigItem	( rSubTree ),
/*N*/ 	mrParent	( rParent )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdOptionsItem::~SdOptionsItem()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

    void SdOptionsItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}
    void SdOptionsItem::Commit() {}


// -----------------------------------------------------------------------------

/*N*/ Sequence< Any >	SdOptionsItem::GetProperties( const Sequence< OUString >& rNames )
/*N*/ {
/*N*/ 	return ConfigItem::GetProperties( rNames );
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdOptionsItem::SetModified()
/*N*/ {
/*N*/ 	ConfigItem::SetModified();
/*N*/ }

// --------------------
// - SdOptionsGeneric -
// --------------------

/*N*/ SdOptionsGeneric::SdOptionsGeneric( USHORT nConfigId, const OUString& rSubTree ) :
/*N*/ 	maSubTree		( rSubTree ),
/*N*/ 	mpCfgItem		( NULL ),
/*N*/ 	mnConfigId		( nConfigId ),
/*N*/ 	mbInit			( rSubTree.getLength() == 0 ),
/*N*/	mbEnableModify	( TRUE )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsGeneric::Init() const
/*N*/ {
/*N*/ 	if( !mbInit )
/*N*/ 	{
/*N*/ 		SdOptionsGeneric* pThis	= (SdOptionsGeneric*) this;
/*N*/ 
/*N*/ 		if( !mpCfgItem )
/*N*/ 			pThis->mpCfgItem = new SdOptionsItem( *this, maSubTree );
/*N*/ 
/*N*/ 		const Sequence< OUString >	aNames( GetPropertyNames() );
/*N*/ 		const Sequence< Any >		aValues = mpCfgItem->GetProperties( aNames );
/*N*/ 
/*N*/ 		if( aNames.getLength() && ( aValues.getLength() == aNames.getLength() ) )
/*N*/ 		{
/*N*/ 			const Any* pValues = aValues.getConstArray();
/*N*/ 
/*N*/ 			pThis->EnableModify( FALSE );
/*N*/ 			pThis->mbInit = pThis->ReadData( pValues );
/*N*/ 			pThis->EnableModify( TRUE );
/*N*/ 		}
/*N*/  		else
/*?*/ 			pThis->mbInit = TRUE;
/*N*/  	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdOptionsGeneric::~SdOptionsGeneric()
/*N*/ {
/*N*/ 	if( mpCfgItem )
/*N*/ 		delete mpCfgItem;
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ Sequence< OUString > SdOptionsGeneric::GetPropertyNames() const
/*N*/ {
/*N*/ 	ULONG			nCount;
/*N*/ 	const char**	ppPropNames;
/*N*/ 
/*N*/ 	GetPropNameArray( ppPropNames, nCount );
/*N*/ 
/*N*/ 	Sequence< OUString > aNames( nCount );
/*N*/ 	OUString*			 pNames = aNames.getArray();
/*N*/ 
/*N*/ 	for( ULONG i = 0; i < nCount; i++ )
/*N*/ 		pNames[ i ] = OUString::createFromAscii( ppPropNames[ i ] );
/*N*/ 
/*N*/ 	return aNames;
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ bool SdOptionsGeneric::isMetricSystem()
/*N*/ {
/*N*/ 	SvtSysLocale aSysLocale;
/*N*/     MeasurementSystem eSys = aSysLocale.GetLocaleDataPtr()->getMeasurementSystemEnum();
/*N*/ 
/*N*/ 	return ( eSys == MEASURE_METRIC );
/*N*/ }

/*************************************************************************
|*
|* SdOptionsLayout
|*
\************************************************************************/

/*N*/ SdOptionsLayout::SdOptionsLayout(  USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, bUseConfig ?
/*N*/ 					  ( ( SDCFG_DRAW == nConfigId ) ?
/*N*/ 						B2U( "Office.Draw/Layout" ) :
/*N*/ 						B2U( "Office.Impress/Layout" ) ) :
/*N*/ 					  OUString() ),
/*N*/	bRuler( TRUE ),
/*N*/	bMoveOutline( TRUE ),
/*N*/	bDragStripes( FALSE ),
/*N*/	bHandlesBezier( FALSE ),
/*N*/	bHelplines( TRUE ),
/*N*/	nMetric( isMetricSystem() ? (UINT16)FUNIT_CM : (UINT16)FUNIT_INCH ),
/*N*/	nDefTab( 1250 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsLayout::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aPropNamesMetric[] =
/*N*/ 	{
/*N*/ 		"Display/Ruler",
/*N*/ 		"Display/Bezier",
/*N*/ 		"Display/Contour",
/*N*/ 		"Display/Guide",
/*N*/ 		"Display/Helpline",
/*N*/ 		"Other/MeasureUnit/Metric",
/*N*/ 		"Other/TabStop/Metric"
/*N*/ 	};
/*N*/ 
/*N*/ 	static const char* aPropNamesNonMetric[] =
/*N*/ 	{
/*N*/ 		"Display/Ruler",
/*N*/ 		"Display/Bezier",
/*N*/ 		"Display/Contour",
/*N*/ 		"Display/Guide",
/*N*/ 		"Display/Helpline",
/*N*/ 		"Other/MeasureUnit/NonMetric",
/*N*/ 		"Other/TabStop/NonMetric"
/*N*/ 	};
/*N*/ 
/*N*/ 	rCount = 7;
/*N*/ 
/*N*/ 	if( isMetricSystem() )
/*N*/ 		ppNames = aPropNamesMetric;
/*N*/ 	else
/*N*/ 		ppNames = aPropNamesNonMetric;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsLayout::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	if( pValues[0].hasValue() ) SetRulerVisible( *(sal_Bool*) pValues[ 0 ].getValue() );
/*N*/ 	if( pValues[1].hasValue() ) SetHandlesBezier( *(sal_Bool*) pValues[ 1 ].getValue() );
/*N*/ 	if( pValues[2].hasValue() ) SetMoveOutline( *(sal_Bool*) pValues[ 2 ].getValue() );
/*N*/ 	if( pValues[3].hasValue() ) SetDragStripes( *(sal_Bool*) pValues[ 3 ].getValue() );
/*N*/ 	if( pValues[4].hasValue() ) SetHelplines( *(sal_Bool*) pValues[ 4 ].getValue() );
/*N*/ 	if( pValues[5].hasValue() ) SetMetric( (UINT16) *(sal_Int32*) pValues[ 5 ].getValue() );
/*N*/ 	if( pValues[6].hasValue() ) SetDefTab( (UINT16) *(sal_Int32*) pValues[ 6 ].getValue() );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsLayout::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	pValues[ 0 ] <<= IsRulerVisible();
/*N*/ 	pValues[ 1 ] <<= IsHandlesBezier();
/*N*/ 	pValues[ 2 ] <<= IsMoveOutline();
/*N*/ 	pValues[ 3 ] <<= IsDragStripes();
/*N*/ 	pValues[ 4 ] <<= IsHelplines();
/*N*/ 	pValues[ 5 ] <<= (sal_Int32) GetMetric();
/*N*/ 	pValues[ 6 ] <<= (sal_Int32) GetDefTab();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsLayoutItem
|*
\************************************************************************/


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------



// ----------------------------------------------------------------------


// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptionsContents
|*
\************************************************************************/

/*N*/ SdOptionsContents::SdOptionsContents( USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, bUseConfig ?
/*N*/ 					  ( ( SDCFG_DRAW == nConfigId ) ?
/*N*/ 						B2U( "Office.Draw/Content" ) :
/*N*/ 						B2U( "Office.Impress/Content" ) ) :
/*N*/ 					  OUString() ),
/*N*/	bExternGraphic( FALSE ),
/*N*/	bOutlineMode( FALSE ),
/*N*/	bHairlineMode( FALSE ),
/*N*/	bNoText( FALSE )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsContents::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Display/PicturePlaceholder",
/*N*/ 		"Display/ContourMode",
/*N*/ 		"Display/LineContour",
/*N*/ 		"Display/TextPlaceholder"
/*N*/ 	};
/*N*/ 
/*N*/ 	rCount = 4;
/*N*/ 	ppNames = aPropNames;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsContents::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	if( pValues[0].hasValue() ) SetExternGraphic( *(sal_Bool*) pValues[ 0 ].getValue() );
/*N*/ 	if( pValues[1].hasValue() ) SetOutlineMode( *(sal_Bool*)pValues[ 1 ].getValue() );
/*N*/ 	if( pValues[2].hasValue() ) SetHairlineMode( *(sal_Bool*) pValues[ 2 ].getValue() );
/*N*/ 	if( pValues[3].hasValue() ) SetNoText( *(sal_Bool*) pValues[ 3 ].getValue() );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsContents::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	pValues[ 0 ] <<= IsExternGraphic();
/*N*/ 	pValues[ 1 ] <<= IsOutlineMode();
/*N*/ 	pValues[ 2 ] <<= IsHairlineMode();
/*N*/ 	pValues[ 3 ] <<= IsNoText();
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsContentsItem
|*
\************************************************************************/


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------


// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptionsMisc
|*
\************************************************************************/

/*N*/ SdOptionsMisc::SdOptionsMisc( USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, bUseConfig ?
/*N*/ 					  ( ( SDCFG_DRAW == nConfigId ) ?
/*N*/ 						B2U( "Office.Draw/Misc" ) :
/*N*/ 						B2U( "Office.Impress/Misc" ) ) :
/*N*/ 					  OUString() ),
/*N*/	nPreviewQuality( DRAWMODE_DEFAULT ),
/*N*/	nDefaultObjectSizeWidth( 8000 ),
/*N*/	nDefaultObjectSizeHeight( 5000 ),
/*N*/	bStartWithTemplate( TRUE ),
/*N*/	bMarkedHitMovesAlways( TRUE ),
/*N*/	bMoveOnlyDragging( FALSE ),
/*N*/	bCrookNoContortion( FALSE ),
/*N*/	bQuickEdit( GetConfigId() != SDCFG_DRAW ),
/*N*/	bMasterPageCache( TRUE ),
/*N*/	bDragWithCopy( FALSE ),
/*N*/	bPickThrough( TRUE ),
/*N*/	bBigHandles( FALSE ),
/*N*/	bDoubleClickTextEdit( TRUE ),
/*N*/	bClickChangeRotation( FALSE ),
/*N*/	bStartWithActualPage( FALSE ),
/*N*/	bSolidDragging( FALSE ),
/*N*/	bSolidMarkHdl( TRUE ),
/*N*/	bSummationOfParagraphs( FALSE ),
/*N*/	bShowUndoDeleteWarning( TRUE ),
/*N*/	mnPrinterIndependentLayout( 1 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsMisc::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"ObjectMoveable",
/*N*/ 		"NoDistort",
/*N*/ 		"TextObject/QuickEditing",
/*N*/ 		"BackgroundCache",
/*N*/ 		"CopyWhileMoving",
/*N*/ 		"TextObject/Selectable",
/*N*/ 		"BigHandles",
/*N*/ 		"DclickTextedit",
/*N*/ 		"RotateClick",
/*N*/ 		"Preview",
/*N*/ 		"CreateWithAttributes",
/*N*/ 		"SimpleHandles",
/*N*/ 		// #97016#
/*N*/ 		"DefaultObjectSize/Width",
/*N*/ 		"DefaultObjectSize/Height",
/*N*/ 
/*N*/         "Compatibility/PrinterIndependentLayout",
/*N*/ 
/*N*/ 		// just for impress
/*N*/ 		"NewDoc/AutoPilot",
/*N*/ 		"Start/CurrentPage",
/*N*/ 		"Compatibility/AddBetween",
/*N*/ 		// #90356#
/*N*/ 		"ShowUndoDeleteWarning"
/*N*/ 	};
/*N*/ 
/*N*/ 	// #90356# rCount = ( ( GetConfigId() == SDCFG_IMPRESS ) ? 15 : 12 );
/*N*/ 	// #97016# rCount = ( ( GetConfigId() == SDCFG_IMPRESS ) ? 16 : 12 );
/*N*/ 	rCount = ( ( GetConfigId() == SDCFG_IMPRESS ) ? 19 : 15 );
/*N*/ 	ppNames = aPropNames;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsMisc::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	if( pValues[0].hasValue() ) SetMarkedHitMovesAlways( *(sal_Bool*) pValues[ 0 ].getValue() );
/*N*/ 	if( pValues[1].hasValue() ) SetCrookNoContortion( *(sal_Bool*) pValues[ 1 ].getValue() );
/*N*/ 	if( pValues[2].hasValue() ) SetQuickEdit( *(sal_Bool*)pValues[ 2 ].getValue() );
/*N*/ 	if( pValues[3].hasValue() ) SetMasterPagePaintCaching( *(sal_Bool*) pValues[ 3 ].getValue() );
/*N*/ 	if( pValues[4].hasValue() ) SetDragWithCopy( *(sal_Bool*) pValues[ 4 ].getValue() );
/*N*/ 	if( pValues[5].hasValue() ) SetPickThrough( *(sal_Bool*) pValues[ 5 ].getValue() );
/*N*/ 	if( pValues[6].hasValue() ) SetBigHandles( *(sal_Bool*) pValues[ 6 ].getValue() );
/*N*/ 	if( pValues[7].hasValue() ) SetDoubleClickTextEdit( *(sal_Bool*) pValues[ 7 ].getValue() );
/*N*/ 	if( pValues[8].hasValue() ) SetClickChangeRotation( *(sal_Bool*) pValues[ 8 ].getValue() );
/*N*/ 	if( pValues[9].hasValue() ) SetPreviewQuality( FRound( *(double*) pValues[ 9 ].getValue() ) );
/*N*/ 	if( pValues[10].hasValue() ) SetSolidDragging( *(sal_Bool*) pValues[ 10 ].getValue() );
/*N*/ 	if( pValues[11].hasValue() ) SetSolidMarkHdl( *(sal_Bool*) pValues[ 11 ].getValue() );
/*N*/ 	// #97016#
/*N*/ 	if( pValues[12].hasValue() ) SetDefaultObjectSizeWidth( *(sal_uInt32*) pValues[ 12 ].getValue() );
/*N*/ 	if( pValues[13].hasValue() ) SetDefaultObjectSizeHeight( *(sal_uInt32*) pValues[ 13 ].getValue() );
/*N*/ 	if( pValues[14].hasValue() ) SetPrinterIndependentLayout( *(sal_uInt16*) pValues[ 14 ].getValue() );
/*N*/ 
/*N*/ 	// just for Impress
/*N*/ 	if( GetConfigId() == SDCFG_IMPRESS )
/*N*/ 	{
/*N*/ 		if( pValues[15].hasValue() )
/*N*/ 			SetStartWithTemplate( *(sal_Bool*) pValues[ 15 ].getValue() );
/*N*/ 		if( pValues[16].hasValue() )
/*N*/ 			SetStartWithActualPage( *(sal_Bool*) pValues[ 16 ].getValue() );
/*N*/ 		if( pValues[17].hasValue() )
/*N*/ 			SetSummationOfParagraphs( *(sal_Bool*) pValues[ 17 ].getValue() );
/*N*/ 		// #90356#
/*N*/ 		if( pValues[18].hasValue() ) 
/*N*/ 			SetShowUndoDeleteWarning( *(sal_Bool*) pValues[ 18 ].getValue() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsMisc::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	pValues[ 0 ] <<= IsMarkedHitMovesAlways();
/*N*/ 	pValues[ 1 ] <<= IsCrookNoContortion();
/*N*/ 	pValues[ 2 ] <<= IsQuickEdit();
/*N*/ 	pValues[ 3 ] <<= IsMasterPagePaintCaching();
/*N*/ 	pValues[ 4 ] <<= IsDragWithCopy();
/*N*/ 	pValues[ 5 ] <<= IsPickThrough();
/*N*/ 	pValues[ 6 ] <<= IsBigHandles();
/*N*/ 	pValues[ 7 ] <<= IsDoubleClickTextEdit();
/*N*/ 	pValues[ 8 ] <<= IsClickChangeRotation();
/*N*/ 	pValues[ 9 ] <<= (double) GetPreviewQuality();
/*N*/ 	pValues[ 10 ] <<= IsSolidDragging();
/*N*/ 	pValues[ 11 ] <<= IsSolidMarkHdl();
/*N*/ 	// #97016#
/*N*/ 	pValues[ 12 ] <<= GetDefaultObjectSizeWidth();
/*N*/ 	pValues[ 13 ] <<= GetDefaultObjectSizeHeight();
/*N*/ 	pValues[ 14 ] <<= GetPrinterIndependentLayout();
/*N*/ 
/*N*/ 	// just for Impress
/*N*/ 	if( GetConfigId() == SDCFG_IMPRESS )
/*N*/ 	{
/*N*/ 		pValues[ 15 ] <<= IsStartWithTemplate();
/*N*/ 		pValues[ 16 ] <<= IsStartWithActualPage();
/*N*/ 		pValues[ 17 ] <<= IsSummationOfParagraphs();
/*N*/ 		// #90356#
/*N*/ 		pValues[ 18 ] <<= IsShowUndoDeleteWarning();
/*N*/ 	}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsMiscItem
|*
\************************************************************************/


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------



// ----------------------------------------------------------------------


// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptionsSnap
|*
\************************************************************************/

/*N*/ SdOptionsSnap::SdOptionsSnap( USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, bUseConfig ? 
/*N*/ 					  ( ( SDCFG_DRAW == nConfigId ) ? 
/*N*/ 						B2U( "Office.Draw/Snap" ) : 
/*N*/ 						B2U( "Office.Impress/Snap" ) ) :
/*N*/ 					  OUString() ),
/*N*/	bSnapHelplines( TRUE ),
/*N*/	bSnapBorder( TRUE ),
/*N*/	bSnapFrame( FALSE ),
/*N*/	bSnapPoints( FALSE ),
/*N*/	bOrtho( FALSE ),
/*N*/	bBigOrtho( TRUE ),
/*N*/	bRotate( FALSE ),
/*N*/	nSnapArea( 5 ),
/*N*/	nAngle( 1500 ),
/*N*/	nBezAngle( 1500 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsSnap::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"Object/SnapLine",
/*N*/ 		"Object/PageMargin",
/*N*/ 		"Object/ObjectFrame",
/*N*/ 		"Object/ObjectPoint",
/*N*/ 		"Position/CreatingMoving",
/*N*/ 		"Position/ExtendEdges",
/*N*/ 		"Position/Rotating",
/*N*/ 		"Object/Range",
/*N*/ 		"Position/RotatingValue",
/*N*/ 		"Position/PointReduction"
/*N*/ 	};
/*N*/ 	
/*N*/ 	rCount = 10;
/*N*/ 	ppNames = aPropNames;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsSnap::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	if( pValues[0].hasValue() ) SetSnapHelplines( *(sal_Bool*) pValues[ 0 ].getValue() ); 
/*N*/ 	if( pValues[1].hasValue() ) SetSnapBorder( *(sal_Bool*)pValues[ 1 ].getValue() );  
/*N*/ 	if( pValues[2].hasValue() ) SetSnapFrame( *(sal_Bool*) pValues[ 2 ].getValue() ); 
/*N*/ 	if( pValues[3].hasValue() ) SetSnapPoints( *(sal_Bool*) pValues[ 3 ].getValue() ); 
/*N*/ 	if( pValues[4].hasValue() ) SetOrtho( *(sal_Bool*) pValues[ 4 ].getValue() ); 
/*N*/ 	if( pValues[5].hasValue() ) SetBigOrtho( *(sal_Bool*) pValues[ 5 ].getValue() ); 
/*N*/ 	if( pValues[6].hasValue() ) SetRotate( *(sal_Bool*) pValues[ 6 ].getValue() ); 
/*N*/ 	if( pValues[7].hasValue() ) SetSnapArea( (INT16) *(sal_Int32*) pValues[ 7 ].getValue() ); 
/*N*/ 	if( pValues[8].hasValue() ) SetAngle( (INT16) *(sal_Int32*) pValues[ 8 ].getValue() ); 
/*N*/ 	if( pValues[9].hasValue() ) SetEliminatePolyPointLimitAngle( (INT16) *(sal_Int32*) pValues[ 9 ].getValue() ); 
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsSnap::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	pValues[ 0 ] <<= IsSnapHelplines(); 
/*N*/ 	pValues[ 1 ] <<= IsSnapBorder(); 
/*N*/ 	pValues[ 2 ] <<= IsSnapFrame(); 
/*N*/ 	pValues[ 3 ] <<= IsSnapPoints(); 
/*N*/ 	pValues[ 4 ] <<= IsOrtho(); 
/*N*/ 	pValues[ 5 ] <<= IsBigOrtho(); 
/*N*/ 	pValues[ 6 ] <<= IsRotate(); 
/*N*/ 	pValues[ 7 ] <<= (sal_Int32) GetSnapArea(); 
/*N*/ 	pValues[ 8 ] <<= (sal_Int32) GetAngle(); 
/*N*/ 	pValues[ 9 ] <<= (sal_Int32) GetEliminatePolyPointLimitAngle(); 
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsSnapItem
|*
\************************************************************************/


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------



// ----------------------------------------------------------------------


// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptionsZoom
|*
\************************************************************************/

/*N*/ SdOptionsZoom::SdOptionsZoom( USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, ( bUseConfig &&  ( SDCFG_DRAW == nConfigId ) ) ? 
/*N*/ 								 B2U( "Office.Draw/Zoom" ) : 
/*N*/ 							     OUString() ),
/*N*/	nX( 1 ),
/*N*/	nY( 1 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsZoom::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aPropNames[] =
/*N*/ 	{
/*N*/ 		"ScaleX",
/*N*/ 		"ScaleY"
/*N*/ 	};
/*N*/ 	
/*N*/ 	rCount = ( GetConfigId() == SDCFG_DRAW ) ? 2 : 0;
/*N*/ 	ppNames = aPropNames;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsZoom::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	INT32 nX = 1, nY = 1;
/*N*/ 
/*N*/ 	if( pValues[0].hasValue() ) nX = ( *(sal_Int32*) pValues[ 0 ].getValue() ); 
/*N*/ 	if( pValues[1].hasValue() ) nY = ( *(sal_Int32*) pValues[ 1 ].getValue() );  
/*N*/ 	
/*N*/ 	SetScale( nX, nY );
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsZoom::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	INT32 nX, nY;
/*N*/ 
/*N*/ 	GetScale( nX, nY );
/*N*/ 
/*N*/ 	pValues[ 0 ] <<= (sal_Int32) nX; 
/*N*/ 	pValues[ 1 ] <<= (sal_Int32) nY; 
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsZoomItem
|*
\************************************************************************/


// ----------------------------------------------------------------------


// ----------------------------------------------------------------------



// ----------------------------------------------------------------------


// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptionsGrid
|*
\************************************************************************/

/*N*/ SdOptionsGrid::SdOptionsGrid( USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, bUseConfig ? 
/*N*/ 					  ( ( SDCFG_DRAW == nConfigId ) ? 
/*N*/ 						B2U( "Office.Draw/Grid" ) : 
/*N*/ 						B2U( "Office.Impress/Grid" ) ) :
/*N*/ 					  OUString() )
/*N*/ {
/*N*/ 	EnableModify( FALSE );
/*N*/ 	SetDefaults();
/*N*/ 	EnableModify( TRUE );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ SdOptionsGrid::~SdOptionsGrid()
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsGrid::SetDefaults()
/*N*/ {
/*N*/ 	const UINT32 nVal = 1000;
/*N*/ 
/*N*/ 	SetFldDivisionX( nVal );
/*N*/ 	SetFldDivisionY( nVal );
/*N*/ 	SetFldDrawX( nVal );
/*N*/ 	SetFldDrawY( nVal );
/*N*/ 	SetFldSnapX( nVal );
/*N*/ 	SetFldSnapY( nVal );
/*N*/ 	SetUseGridSnap( FALSE );
/*N*/ 	SetSynchronize( TRUE );
/*N*/ 	SetGridVisible( FALSE );
/*N*/ 	SetEqualGrid( TRUE );
/*N*/ }

// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------

/*N*/ void SdOptionsGrid::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aPropNamesMetric[] =
/*N*/ 	{
/*N*/ 		"Resolution/XAxis/Metric",
/*N*/ 		"Resolution/YAxis/Metric",
/*N*/ 		"Subdivision/XAxis",
/*N*/ 		"Subdivision/YAxis",
/*N*/ 		"SnapGrid/XAxis/Metric",
/*N*/ 		"SnapGrid/YAxis/Metric",
/*N*/ 		"Option/SnapToGrid",
/*N*/ 		"Option/Synchronize",
/*N*/ 		"Option/VisibleGrid",
/*N*/ 		"SnapGrid/Size"
/*N*/ 	};
/*N*/ 
/*N*/ 	static const char* aPropNamesNonMetric[] =
/*N*/ 	{
/*N*/ 		"Resolution/XAxis/NonMetric",
/*N*/ 		"Resolution/YAxis/NonMetric",
/*N*/ 		"Subdivision/XAxis",
/*N*/ 		"Subdivision/YAxis",
/*N*/ 		"SnapGrid/XAxis/NonMetric",
/*N*/ 		"SnapGrid/YAxis/NonMetric",
/*N*/ 		"Option/SnapToGrid",
/*N*/ 		"Option/Synchronize",
/*N*/ 		"Option/VisibleGrid",
/*N*/ 		"SnapGrid/Size"
/*N*/ 	};
/*N*/ 
/*N*/ 	rCount = 10;
/*N*/ 
/*N*/ 	if( isMetricSystem() )
/*N*/ 		ppNames = aPropNamesMetric;
/*N*/ 	else
/*N*/ 		ppNames = aPropNamesNonMetric;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsGrid::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	if( pValues[0].hasValue() ) SetFldDrawX( *(sal_Int32*) pValues[ 0 ].getValue() ); 
/*N*/ 	if( pValues[1].hasValue() ) SetFldDrawY( *(sal_Int32*) pValues[ 1 ].getValue() ); 
/*N*/ 
/*N*/ 	if( pValues[2].hasValue() )
/*N*/ 	{
/*N*/ 		const UINT32 nDivX = FRound( *(double*) pValues[ 2 ].getValue() );
/*N*/ 		SetFldDivisionX( SvxOptionsGrid::GetFldDrawX() / ( nDivX + 1 ) ); 
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pValues[3].hasValue() )
/*N*/ 	{
/*N*/ 		const UINT32 nDivY = FRound( *(double*) pValues[ 3 ].getValue() );
/*N*/ 		SetFldDivisionY( SvxOptionsGrid::GetFldDrawY() / ( nDivY + 1 ) ); 
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pValues[4].hasValue() ) SetFldSnapX( *(sal_Int32*) pValues[ 4 ].getValue() ); 
/*N*/ 	if( pValues[5].hasValue() ) SetFldSnapY( *(sal_Int32*) pValues[ 5 ].getValue() ); 
/*N*/ 	if( pValues[6].hasValue() ) SetUseGridSnap( *(sal_Bool*) pValues[ 6 ].getValue() ); 
/*N*/ 	if( pValues[7].hasValue() ) SetSynchronize( *(sal_Bool*) pValues[ 7 ].getValue() ); 
/*N*/ 	if( pValues[8].hasValue() ) SetGridVisible( *(sal_Bool*) pValues[ 8 ].getValue() ); 
/*N*/ 	if( pValues[9].hasValue() ) SetEqualGrid( *(sal_Bool*) pValues[ 9 ].getValue() ); 
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsGrid::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	pValues[ 0 ] <<= (sal_Int32) GetFldDrawX(); 
/*N*/ 	pValues[ 1 ] <<= (sal_Int32) GetFldDrawY(); 
/*N*/ 	pValues[ 2 ] <<= ( GetFldDivisionX() ? ( (double) GetFldDrawX() / GetFldDivisionX() - 1.0 ) : (double) 0 );
/*N*/ 	pValues[ 3 ] <<= ( GetFldDivisionY() ? ( (double) GetFldDrawY() / GetFldDivisionY() - 1.0 ) : (double) 0 );
/*N*/ 	pValues[ 4 ] <<= (sal_Int32) GetFldSnapX(); 
/*N*/ 	pValues[ 5 ] <<= (sal_Int32) GetFldSnapY(); 
/*N*/ 	pValues[ 6 ] <<= IsUseGridSnap(); 
/*N*/ 	pValues[ 7 ] <<= IsSynchronize(); 
/*N*/ 	pValues[ 8 ] <<= IsGridVisible(); 
/*N*/ 	pValues[ 9 ] <<= IsEqualGrid(); 
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsGridItem
|*
\************************************************************************/


// -----------------------------------------------------------------------------	


// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptionsPrint
|*
\************************************************************************/

/*N*/ SdOptionsPrint::SdOptionsPrint( USHORT nConfigId, BOOL bUseConfig ) :
/*N*/ 	SdOptionsGeneric( nConfigId, bUseConfig ? 
/*N*/ 					  ( ( SDCFG_DRAW == nConfigId ) ? 
/*N*/ 						B2U( "Office.Draw/Print" ) : 
/*N*/ 						B2U( "Office.Impress/Print" ) ) :
/*N*/ 					  OUString() ),
/*N*/	bDraw( TRUE ),
/*N*/	bNotes( FALSE ),
/*N*/	bHandout( FALSE ),
/*N*/	bOutline( FALSE ),
/*N*/	bDate( FALSE ),
/*N*/	bTime( FALSE ),
/*N*/	bPagename( FALSE ),
/*N*/	bHiddenPages( TRUE ),
/*N*/	bPagesize( FALSE ),
/*N*/	bPagetile( FALSE ),
/*N*/	bWarningPrinter( TRUE ),
/*N*/	bWarningSize( FALSE ),
/*N*/	bWarningOrientation( FALSE ),
/*N*/	bBooklet( FALSE ),
/*N*/	bFront( TRUE ),
/*N*/	bBack( TRUE ),
/*N*/	bCutPage( FALSE ),
/*N*/	bPaperbin( FALSE ),
/*N*/	nQuality( 0 )
/*N*/ {
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsPrint::operator==( const SdOptionsPrint& rOpt ) const
/*N*/ {
/*N*/ 	return( IsDraw() == rOpt.IsDraw() &&
/*N*/ 			IsNotes() == rOpt.IsNotes() &&
/*N*/ 			IsHandout() == rOpt.IsHandout() &&
/*N*/ 			IsOutline() == rOpt.IsOutline() &&
/*N*/ 			IsDate() == rOpt.IsDate() &&
/*N*/ 			IsTime() == rOpt.IsTime() &&
/*N*/ 			IsPagename() == rOpt.IsPagename() &&
/*N*/ 			IsHiddenPages() == rOpt.IsHiddenPages() &&
/*N*/ 			IsPagesize() == rOpt.IsPagesize() &&
/*N*/ 			IsPagetile() == rOpt.IsPagetile() &&
/*N*/ 			IsWarningPrinter() == rOpt.IsWarningPrinter() &&
/*N*/ 			IsWarningSize() == rOpt.IsWarningSize() &&
/*N*/ 			IsWarningOrientation() == rOpt.IsWarningOrientation() &&
/*N*/ 			IsBooklet() == rOpt.IsBooklet() &&
/*N*/ 			IsFrontPage() == rOpt.IsFrontPage() &&
/*N*/ 			IsBackPage() == rOpt.IsBackPage() &&
/*N*/ 			IsCutPage() == rOpt.IsCutPage() &&
/*N*/ 			IsPaperbin() == rOpt.IsPaperbin() &&
/*N*/ 			GetOutputQuality() == rOpt.GetOutputQuality() );
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ void SdOptionsPrint::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
/*N*/ {
/*N*/ 	static const char* aDrawPropNames[] =
/*N*/ 	{
/*N*/ 		"Other/Date",
/*N*/ 		"Other/Time",
/*N*/ 		"Other/PageName",
/*N*/ 		"Other/HiddenPage",
/*N*/ 		"Page/PageSize",
/*N*/ 		"Page/PageTile",
/*N*/ 		// bWarningPrinter
/*N*/ 		// bWarningSize
/*N*/ 		// bWarningOrientation
/*N*/ 		"Page/Booklet",
/*N*/ 		"Page/BookletFront",
/*N*/ 		"Page/BookletBack",
/*N*/ 		// bCutPage
/*N*/ 		"Other/FromPrinterSetup",
/*N*/ 		"Other/Quality",
/*N*/ 		"Content/Drawing",
/*N*/ 	};
/*N*/ 	static const char* aImpressPropNames[] =
/*N*/ 	{
/*N*/ 		"Other/Date",
/*N*/ 		"Other/Time",
/*N*/ 		"Other/PageName",
/*N*/ 		"Other/HiddenPage",
/*N*/ 		"Page/PageSize",
/*N*/ 		"Page/PageTile",
/*N*/ 		// bWarningPrinter
/*N*/ 		// bWarningSize
/*N*/ 		// bWarningOrientation
/*N*/ 		"Page/Booklet",
/*N*/ 		"Page/BookletFront",
/*N*/ 		"Page/BookletBack",
/*N*/ 		// bCutPage
/*N*/ 		"Other/FromPrinterSetup",
/*N*/ 		"Other/Quality",
/*N*/ 		"Content/Presentation",
/*N*/ 		"Content/Note",
/*N*/ 		"Content/Handout",
/*N*/ 		"Content/Outline"
/*N*/ 	};
/*N*/ 
/*N*/ 	if( GetConfigId() == SDCFG_IMPRESS )
/*N*/ 	{
/*N*/ 		rCount = 15;
/*N*/ 		ppNames = aImpressPropNames;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rCount = 12;
/*N*/ 		ppNames = aDrawPropNames;
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsPrint::ReadData( const Any* pValues )
/*N*/ {
/*N*/ 	if( pValues[0].hasValue() ) SetDate( *(sal_Bool*) pValues[ 0 ].getValue() ); 
/*N*/ 	if( pValues[1].hasValue() ) SetTime( *(sal_Bool*) pValues[ 1 ].getValue() ); 
/*N*/ 	if( pValues[2].hasValue() ) SetPagename( *(sal_Bool*) pValues[ 2 ].getValue() ); 
/*N*/ 	if( pValues[3].hasValue() ) SetHiddenPages( *(sal_Bool*) pValues[ 3 ].getValue() ); 
/*N*/ 	if( pValues[4].hasValue() ) SetPagesize( *(sal_Bool*) pValues[ 4 ].getValue() ); 
/*N*/ 	if( pValues[5].hasValue() ) SetPagetile( *(sal_Bool*) pValues[ 5 ].getValue() ); 
/*N*/ 	if( pValues[6].hasValue() ) SetBooklet( *(sal_Bool*) pValues[ 6 ].getValue() ); 
/*N*/ 	if( pValues[7].hasValue() ) SetFrontPage( *(sal_Bool*) pValues[ 7 ].getValue() ); 
/*N*/ 	if( pValues[8].hasValue() ) SetBackPage( *(sal_Bool*) pValues[ 8 ].getValue() ); 
/*N*/ 	if( pValues[9].hasValue() ) SetPaperbin( *(sal_Bool*) pValues[ 9 ].getValue() ); 
/*N*/ 	if( pValues[10].hasValue() ) SetOutputQuality( (UINT16) *(sal_Int32*) pValues[ 10 ].getValue() ); 
/*N*/ 	if( pValues[11].hasValue() ) SetDraw( *(sal_Bool*) pValues[ 11 ].getValue() ); 
/*N*/ 
/*N*/ 	// just for impress
/*N*/ 	if( GetConfigId() == SDCFG_IMPRESS )
/*N*/ 	{
/*N*/ 		if( pValues[12].hasValue() ) SetNotes( *(sal_Bool*) pValues[ 12 ].getValue() ); 
/*N*/ 		if( pValues[13].hasValue() ) SetHandout( *(sal_Bool*) pValues[ 13 ].getValue() ); 
/*N*/ 		if( pValues[14].hasValue() ) SetOutline( *(sal_Bool*) pValues[ 14 ].getValue() ); 
/*N*/ 	}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

// -----------------------------------------------------------------------------

/*N*/ BOOL SdOptionsPrint::WriteData( Any* pValues ) const
/*N*/ {
/*N*/ 	pValues[ 0 ] <<= IsDate(); 
/*N*/ 	pValues[ 1 ] <<= IsTime(); 
/*N*/ 	pValues[ 2 ] <<= IsPagename();
/*N*/ 	pValues[ 3 ] <<= IsHiddenPages();
/*N*/ 	pValues[ 4 ] <<= IsPagesize(); 
/*N*/ 	pValues[ 5 ] <<= IsPagetile();
/*N*/ 	pValues[ 6 ] <<= IsBooklet();
/*N*/ 	pValues[ 7 ] <<= IsFrontPage(); 
/*N*/ 	pValues[ 8 ] <<= IsBackPage(); 
/*N*/ 	pValues[ 9 ] <<= IsPaperbin();
/*N*/ 	pValues[ 10 ] <<= (sal_Int32) GetOutputQuality(); 
/*N*/ 	pValues[ 11 ] <<= IsDraw();
/*N*/ 	
/*N*/ 	// just for impress
/*N*/ 	if( GetConfigId() == SDCFG_IMPRESS )
/*N*/ 	{
/*N*/ 		pValues[ 12 ] <<= IsNotes(); 
/*N*/ 		pValues[ 13 ] <<= IsHandout(); 
/*N*/ 		pValues[ 14 ] <<= IsOutline(); 
/*N*/ 	}
/*N*/ 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void SdOptionsPrint::SetPrinterOptions( const SdOptionsPrint* pOptions )
/*N*/ {
/*N*/ 	bDraw = pOptions->bDraw;
/*N*/ 	bNotes = pOptions->bNotes;
/*N*/ 	bHandout = pOptions->bHandout;
/*N*/ 	bOutline = pOptions->bOutline;
/*N*/ 	bDate = pOptions->bDate;
/*N*/ 	bTime = pOptions->bTime;
/*N*/ 	bPagename = pOptions->bPagename;
/*N*/ 	bHiddenPages = pOptions->bHiddenPages;
/*N*/ 	bPagesize = pOptions->bPagesize;
/*N*/ 	bPagetile = pOptions->bPagetile;
/*N*/ 	bWarningPrinter = pOptions->bWarningPrinter;
/*N*/ 	bWarningSize = pOptions->bWarningSize;
/*N*/ 	bWarningOrientation = pOptions->bWarningOrientation;
/*N*/ 	bBooklet = pOptions->bBooklet;
/*N*/ 	bFront = pOptions->bFront;
/*N*/ 	bBack = pOptions->bBack;
/*N*/ 	bCutPage = pOptions->bCutPage;
/*N*/ 	bPaperbin = pOptions->bPaperbin;
/*N*/ 	nQuality = pOptions->nQuality;
/*N*/ }

/*************************************************************************
|*
|* SdOptionsPrintItem
|*
\************************************************************************/

// ----------------------------------------------------------------------

/*N*/ SdOptionsPrintItem::SdOptionsPrintItem( USHORT nWhich, SdOptions* pOpts, FrameView* ) : 
/*N*/ 	SfxPoolItem		( nWhich ),
/*N*/ 	SdOptionsPrint	( 0, FALSE )
/*N*/ {
/*N*/ 	SetDraw( pOpts->IsDraw() );
/*N*/ 	SetNotes( pOpts->IsNotes() );
/*N*/ 	SetHandout( pOpts->IsHandout() );
/*N*/ 	SetOutline( pOpts->IsOutline() );
/*N*/ 	SetDate( pOpts->IsDate() );
/*N*/ 	SetTime( pOpts->IsTime() );
/*N*/ 	SetPagename( pOpts->IsPagename() );
/*N*/ 	SetHiddenPages( pOpts->IsHiddenPages() );
/*N*/ 	SetPagesize( pOpts->IsPagesize() );
/*N*/ 	SetPagetile( pOpts->IsPagetile() );
/*N*/ 	SetWarningPrinter( pOpts->IsWarningPrinter() );
/*N*/ 	SetWarningSize( pOpts->IsWarningSize() );
/*N*/ 	SetWarningOrientation( pOpts->IsWarningOrientation() );
/*N*/ 	SetBooklet( pOpts->IsBooklet() );
/*N*/ 	SetFrontPage( pOpts->IsFrontPage() );
/*N*/ 	SetBackPage( pOpts->IsBackPage() );
/*N*/ 	SetCutPage( pOpts->IsCutPage() );
/*N*/ 	SetPaperbin( pOpts->IsPaperbin() );
/*N*/ 	SetOutputQuality( pOpts->GetOutputQuality() );
/*N*/ }

// ----------------------------------------------------------------------

/*N*/ SfxPoolItem* SdOptionsPrintItem::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SdOptionsPrintItem( *this );
/*N*/ }

// ----------------------------------------------------------------------

int SdOptionsPrintItem::operator==( const SfxPoolItem& rAttr ) const
{
    const SdOptionsPrintItem* pRHS = dynamic_cast<const SdOptionsPrintItem *>(&rAttr);
    DBG_ASSERT( pRHS, "SdOptionsPrintItem::operator==(), compare not possible for non SdOptionsPrintItem" );

    if( pRHS )
        return SdOptionsPrint::operator==(*pRHS);
    else
        return 0;
}

// -----------------------------------------------------------------------


/*************************************************************************
|*
|* SdOptions
|*
\************************************************************************/

/*N*/ SdOptions::SdOptions( USHORT nConfigId ) :
/*N*/ 	SdOptionsLayout( nConfigId, TRUE ),
/*N*/ 	SdOptionsContents( nConfigId, TRUE ),
/*N*/ 	SdOptionsMisc( nConfigId, TRUE ),
/*N*/ 	SdOptionsSnap( nConfigId, TRUE ),
/*N*/ 	SdOptionsZoom( nConfigId, TRUE ),
/*N*/ 	SdOptionsGrid( nConfigId, TRUE ),
/*N*/ 	SdOptionsPrint( nConfigId, TRUE )
/*N*/ {
/*N*/ }

// ----------------------------------------------------------------------

/*N*/ SdOptions::~SdOptions()
/*N*/ {
/*N*/ }

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
