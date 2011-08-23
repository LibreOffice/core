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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#define READ_OLDVERS

#include "autoform.hxx"

#include <bf_sfx2/docfile.hxx>
#include <bf_svtools/pathoptions.hxx>
#include <vcl/outdev.hxx>
#include <bf_svx/dialmgr.hxx>
#include <bf_svx/dialogs.hrc>
#include <tools/urlobj.hxx>

#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

#include "globstr.hrc"
namespace binfilter {

//------------------------------------------------------------------------

sal_Char *linker_dummy = "";

//	Standard-Name ist jetzt STR_STYLENAME_STANDARD (wie Vorlagen)
//static const sal_Char __FAR_DATA cStandardName[] = "Standard";

static const sal_Char __FAR_DATA sAutoTblFmtName[] = "autotbl.fmt";

// bis SO5PF
const USHORT AUTOFORMAT_ID_X		= 9501;
const USHORT AUTOFORMAT_ID_358		= 9601;
const USHORT AUTOFORMAT_DATA_ID_X	= 9502;

// ab SO5
//! in nachfolgenden Versionen muss der Betrag dieser IDs groesser sein
const USHORT AUTOFORMAT_ID_504		= 9801;
const USHORT AUTOFORMAT_DATA_ID_504	= 9802;

const USHORT AUTOFORMAT_ID_552		= 9901;
const USHORT AUTOFORMAT_DATA_ID_552	= 9902;

// --- from 641 on: CJK and CTL font settings
const USHORT AUTOFORMAT_ID_641      = 10001;
const USHORT AUTOFORMAT_DATA_ID_641 = 10002;

// aktuelle Version
const USHORT AUTOFORMAT_ID          = AUTOFORMAT_ID_641;
const USHORT AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_641;


#ifdef READ_OLDVERS
const USHORT AUTOFORMAT_OLD_ID_OLD	= 4201;
const USHORT AUTOFORMAT_OLD_DATA_ID	= 4202;
const USHORT AUTOFORMAT_OLD_ID_NEW 	= 4203;
#endif


//	Struct mit Versionsnummern der Items

/*N*/ struct ScAfVersions
/*N*/ {
/*N*/ public:
/*N*/ 	USHORT nFontVersion;
/*N*/ 	USHORT nFontHeightVersion;
/*N*/ 	USHORT nWeightVersion;
/*N*/ 	USHORT nPostureVersion;
/*N*/ 	USHORT nUnderlineVersion;
/*N*/ 	USHORT nCrossedOutVersion;
/*N*/ 	USHORT nContourVersion;
/*N*/ 	USHORT nShadowedVersion;
/*N*/ 	USHORT nColorVersion;
/*N*/ 	USHORT nBoxVersion;
/*N*/ 	USHORT nBrushVersion;
/*N*/ 
/*N*/ 	USHORT nAdjustVersion;
/*N*/ 
/*N*/ 	USHORT nHorJustifyVersion;
/*N*/ 	USHORT nVerJustifyVersion;
/*N*/ 	USHORT nOrientationVersion;
/*N*/ 	USHORT nMarginVersion;
/*N*/ 	USHORT nBoolVersion;
/*N*/ 	USHORT nInt32Version;
/*N*/ 	USHORT nRotateModeVersion;
/*N*/ 
/*N*/ 	USHORT nNumFmtVersion;
/*N*/ 
/*N*/ 	ScAfVersions();
/*N*/ 	void Load( SvStream& rStream, USHORT nVer );
/*N*/ 	static void Write(SvStream& rStream);
/*N*/ };

/*N*/ ScAfVersions::ScAfVersions() :
/*N*/ 	nFontVersion(0),
/*N*/ 	nFontHeightVersion(0),
/*N*/ 	nWeightVersion(0),
/*N*/ 	nPostureVersion(0),
/*N*/ 	nUnderlineVersion(0),
/*N*/ 	nCrossedOutVersion(0),
/*N*/ 	nContourVersion(0),
/*N*/ 	nShadowedVersion(0),
/*N*/ 	nColorVersion(0),
/*N*/ 	nBoxVersion(0),
/*N*/ 	nBrushVersion(0),
/*N*/ 	nAdjustVersion(0),
/*N*/ 	nHorJustifyVersion(0),
/*N*/ 	nVerJustifyVersion(0),
/*N*/ 	nOrientationVersion(0),
/*N*/ 	nMarginVersion(0),
/*N*/ 	nBoolVersion(0),
/*N*/ 	nInt32Version(0),
/*N*/ 	nRotateModeVersion(0),
/*N*/ 	nNumFmtVersion(0)
/*N*/ {
/*N*/ }

/*N*/ void ScAfVersions::Load( SvStream& rStream, USHORT nVer )
/*N*/ {
/*N*/ 	rStream >> nFontVersion;
/*N*/ 	rStream >> nFontHeightVersion;
/*N*/ 	rStream >> nWeightVersion;
/*N*/ 	rStream >> nPostureVersion;
/*N*/ 	rStream >> nUnderlineVersion;
/*N*/ 	rStream >> nCrossedOutVersion;
/*N*/ 	rStream >> nContourVersion;
/*N*/ 	rStream >> nShadowedVersion;
/*N*/ 	rStream >> nColorVersion;
/*N*/ 	rStream >> nBoxVersion;
/*N*/ 	rStream >> nBrushVersion;
/*N*/ 	rStream >> nAdjustVersion;
/*N*/ 	rStream >> nHorJustifyVersion;
/*N*/ 	rStream >> nVerJustifyVersion;
/*N*/ 	rStream >> nOrientationVersion;
/*N*/ 	rStream >> nMarginVersion;
/*N*/ 	rStream >> nBoolVersion;
/*N*/ 	if ( nVer >= AUTOFORMAT_ID_504 )
/*N*/ 	{
/*N*/ 		rStream >> nInt32Version;
/*N*/ 		rStream >> nRotateModeVersion;
/*N*/ 	}
/*N*/ 	rStream >> nNumFmtVersion;
/*N*/ }

/*N*/ void ScAfVersions::Write(SvStream& rStream)
/*N*/ {
/*N*/ 	rStream << SvxFontItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxFontHeightItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxWeightItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxPostureItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxUnderlineItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxCrossedOutItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxContourItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxShadowedItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxColorItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxBoxItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxBrushItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 
/*N*/ 	rStream << SvxAdjustItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 
/*N*/ 	rStream << SvxHorJustifyItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxVerJustifyItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxOrientationItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxMarginItem().GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SfxBoolItem(ATTR_LINEBREAK).GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SfxInt32Item(ATTR_ROTATE_VALUE).GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 	rStream << SvxRotateModeItem(SVX_ROTATE_MODE_STANDARD,0).GetVersion(SOFFICE_FILEFORMAT_40);
/*N*/ 
/*N*/ 	rStream << (USHORT)0;		// Num-Format
/*N*/ }

//	---------------------------------------------------------------------------

/*N*/  ScAutoFormatDataField::ScAutoFormatDataField() :
/*N*/      aCJKFont( ATTR_CJK_FONT ),
/*N*/      aCJKHeight( 240, 100, ATTR_CJK_FONT_HEIGHT ),
/*N*/      aCJKWeight( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT ),
/*N*/      aCJKPosture( ITALIC_NONE, ATTR_CJK_FONT_POSTURE ),
/*N*/      aCTLFont( ATTR_CTL_FONT ),
/*N*/      aCTLHeight( 240, 100, ATTR_CTL_FONT_HEIGHT ),
/*N*/      aCTLWeight( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT ),
/*N*/      aCTLPosture( ITALIC_NONE, ATTR_CTL_FONT_POSTURE ),
/*N*/      aLinebreak( ATTR_LINEBREAK ),
/*N*/      aRotateAngle( ATTR_ROTATE_VALUE ),
/*N*/      aRotateMode( SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE )
/*N*/  {
/*N*/  }

/*N*/  ScAutoFormatDataField::ScAutoFormatDataField( const ScAutoFormatDataField& rCopy ) :
/*N*/      aFont( rCopy.aFont ),
/*N*/      aHeight( rCopy.aHeight ),
/*N*/      aWeight( rCopy.aWeight ),
/*N*/      aPosture( rCopy.aPosture ),
/*N*/      aCJKFont( rCopy.aCJKFont ),
/*N*/      aCJKHeight( rCopy.aCJKHeight ),
/*N*/      aCJKWeight( rCopy.aCJKWeight ),
/*N*/      aCJKPosture( rCopy.aCJKPosture ),
/*N*/      aCTLFont( rCopy.aCTLFont ),
/*N*/      aCTLHeight( rCopy.aCTLHeight ),
/*N*/      aCTLWeight( rCopy.aCTLWeight ),
/*N*/      aCTLPosture( rCopy.aCTLPosture ),
/*N*/      aUnderline( rCopy.aUnderline ),
/*N*/      aCrossedOut( rCopy.aCrossedOut ),
/*N*/      aContour( rCopy.aContour ),
/*N*/      aShadowed( rCopy.aShadowed ),
/*N*/      aColor( rCopy.aColor ),
/*N*/      aBox( rCopy.aBox ),
/*N*/      aBackground( rCopy.aBackground ),
/*N*/      aAdjust( rCopy.aAdjust ),
/*N*/      aHorJustify( rCopy.aHorJustify ),
/*N*/      aVerJustify( rCopy.aVerJustify ),
/*N*/      aOrientation( rCopy.aOrientation ),
/*N*/      aMargin( rCopy.aMargin ),
/*N*/      aLinebreak( rCopy.aLinebreak ),
/*N*/      aRotateAngle( rCopy.aRotateAngle ),
/*N*/      aRotateMode( rCopy.aRotateMode ),
/*N*/      aNumFormat( rCopy.aNumFormat )
/*N*/  {
/*N*/  }

/*N*/  ScAutoFormatDataField::~ScAutoFormatDataField()
/*N*/  {
/*N*/  }

/*N*/ void ScAutoFormatDataField::SetAdjust( const SvxAdjustItem& rAdjust )
/*N*/ {
/*?*/     DBG_BF_ASSERT(0, "STRIP"); //STRIP001 aAdjust.SetAdjust( rAdjust.GetAdjust() );
/*N*/ }

/*N*/ #define READ( aItem, ItemType, nVers )      \
/*N*/     pNew = aItem.Create( rStream, nVers );  \
/*N*/     aItem = *(ItemType*)pNew;               \
/*N*/ 	delete pNew;

/*N*/  BOOL ScAutoFormatDataField::Load( SvStream& rStream, const ScAfVersions& rVersions, USHORT nVer )
/*N*/  {
/*N*/      SfxPoolItem* pNew;
/*N*/  
/*N*/      READ( aFont,        SvxFontItem,        rVersions.nFontVersion)
/*N*/      READ( aHeight,      SvxFontHeightItem,  rVersions.nFontHeightVersion)
/*N*/      READ( aWeight,      SvxWeightItem,      rVersions.nWeightVersion)
/*N*/      READ( aPosture,     SvxPostureItem,     rVersions.nPostureVersion)
/*N*/      // --- from 641 on: CJK and CTL font settings
/*N*/      if( AUTOFORMAT_DATA_ID_641 <= nVer )
/*N*/      {
/*N*/          READ( aCJKFont,     SvxFontItem,        rVersions.nFontVersion)
/*N*/          READ( aCJKHeight,   SvxFontHeightItem,  rVersions.nFontHeightVersion)
/*N*/          READ( aCJKWeight,   SvxWeightItem,      rVersions.nWeightVersion)
/*N*/          READ( aCJKPosture,  SvxPostureItem,     rVersions.nPostureVersion)
/*N*/          READ( aCTLFont,     SvxFontItem,        rVersions.nFontVersion)
/*N*/          READ( aCTLHeight,   SvxFontHeightItem,  rVersions.nFontHeightVersion)
/*N*/          READ( aCTLWeight,   SvxWeightItem,      rVersions.nWeightVersion)
/*N*/          READ( aCTLPosture,  SvxPostureItem,     rVersions.nPostureVersion)
/*N*/      }
/*N*/      READ( aUnderline,   SvxUnderlineItem,   rVersions.nUnderlineVersion)
/*N*/      READ( aCrossedOut,  SvxCrossedOutItem,  rVersions.nCrossedOutVersion)
/*N*/      READ( aContour,     SvxContourItem,     rVersions.nContourVersion)
/*N*/      READ( aShadowed,    SvxShadowedItem,    rVersions.nShadowedVersion)
/*N*/      READ( aColor,       SvxColorItem,       rVersions.nColorVersion)
/*N*/      READ( aBox,         SvxBoxItem,         rVersions.nBoxVersion)
/*N*/      READ( aBackground,  SvxBrushItem,       rVersions.nBrushVersion)
/*N*/  
/*N*/      pNew = aAdjust.Create( rStream, rVersions.nAdjustVersion );
/*N*/      SetAdjust( *(SvxAdjustItem*)pNew );
/*N*/      delete pNew;
/*N*/  
/*N*/      READ( aHorJustify,   SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
/*N*/      READ( aVerJustify,   SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
/*N*/      READ( aOrientation,  SvxOrientationItem, rVersions.nOrientationVersion)
/*N*/      READ( aMargin,       SvxMarginItem,      rVersions.nMarginVersion)
/*N*/  
/*N*/      pNew = aLinebreak.Create( rStream, rVersions.nBoolVersion );
/*N*/      SetLinebreak( *(SfxBoolItem*)pNew );
/*N*/      delete pNew;
/*N*/  
/*N*/      if ( nVer >= AUTOFORMAT_DATA_ID_504 )
/*N*/      {
/*N*/          pNew = aRotateAngle.Create( rStream, rVersions.nInt32Version );
/*N*/          SetRotateAngle( *(SfxInt32Item*)pNew );
/*N*/          delete pNew;
/*N*/          pNew = aRotateMode.Create( rStream, rVersions.nRotateModeVersion );
/*N*/          SetRotateMode( *(SvxRotateModeItem*)pNew );
/*N*/          delete pNew;
/*N*/      }
/*N*/  
/*N*/      if( 0 == rVersions.nNumFmtVersion )
/*N*/          aNumFormat.Load( rStream );
/*N*/  
/*N*/      //  adjust charset in font
/*N*/      CharSet eSysSet = gsl_getSystemTextEncoding();
/*N*/      CharSet eSrcSet = rStream.GetStreamCharSet();
/*N*/      if( eSrcSet != eSysSet && aFont.GetCharSet() == eSrcSet )
/*N*/          aFont.GetCharSet() = eSysSet;
/*N*/  
/*N*/      return (rStream.GetError() == 0);
/*N*/  }

/*N*/  #ifdef READ_OLDVERS
/*N*/  BOOL ScAutoFormatDataField::LoadOld( SvStream& rStream, const ScAfVersions& rVersions )
/*N*/  {
/*N*/      SfxPoolItem* pNew;
/*N*/  
/*N*/      aNumFormat.Load(rStream);
/*N*/  
/*N*/      READ( aFont,        SvxFontItem,        rVersions.nFontVersion)
/*N*/      READ( aHeight,      SvxFontHeightItem,  rVersions.nFontHeightVersion)
/*N*/      READ( aWeight,      SvxWeightItem,      rVersions.nWeightVersion)
/*N*/      READ( aPosture,     SvxPostureItem,     rVersions.nPostureVersion)
/*N*/      READ( aUnderline,   SvxUnderlineItem,   rVersions.nUnderlineVersion)
/*N*/      READ( aCrossedOut,  SvxCrossedOutItem,  rVersions.nCrossedOutVersion)
/*N*/      READ( aContour,     SvxContourItem,     rVersions.nContourVersion)
/*N*/      READ( aShadowed,    SvxShadowedItem,    rVersions.nShadowedVersion)
/*N*/      READ( aColor,       SvxColorItem,       rVersions.nColorVersion)
/*N*/      READ( aHorJustify,  SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
/*N*/      READ( aVerJustify,  SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
/*N*/      READ( aOrientation, SvxOrientationItem, rVersions.nOrientationVersion)
/*N*/      pNew = aLinebreak.Create( rStream, rVersions.nBoolVersion );
/*N*/      SetLinebreak( *(SfxBoolItem*)pNew );
/*N*/      delete pNew;
/*N*/      READ( aMargin,      SvxMarginItem,      rVersions.nMarginVersion)
/*N*/      READ( aBox,         SvxBoxItem,         rVersions.nBoxVersion)
/*N*/      READ( aBackground,  SvxBrushItem,       rVersions.nBrushVersion)
/*N*/  
/*N*/      return (rStream.GetError() == 0);
/*N*/  }
/*N*/  #endif
/*N*/  
/*N*/  BOOL ScAutoFormatDataField::Save( SvStream& rStream )
/*N*/  {
/*N*/      aFont.Store         ( rStream, aFont.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aHeight.Store       ( rStream, aHeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aWeight.Store       ( rStream, aWeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aPosture.Store      ( rStream, aPosture.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      // --- from 641 on: CJK and CTL font settings
/*N*/      aCJKFont.Store      ( rStream, aCJKFont.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCJKHeight.Store    ( rStream, aCJKHeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCJKWeight.Store    ( rStream, aCJKWeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCJKPosture.Store   ( rStream, aCJKPosture.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCTLFont.Store      ( rStream, aCTLFont.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCTLHeight.Store    ( rStream, aCTLHeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCTLWeight.Store    ( rStream, aCTLWeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCTLPosture.Store   ( rStream, aCTLPosture.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/  
/*N*/      aUnderline.Store    ( rStream, aUnderline.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aCrossedOut.Store   ( rStream, aCrossedOut.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aContour.Store      ( rStream, aContour.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aShadowed.Store     ( rStream, aShadowed.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aColor.Store        ( rStream, aColor.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aBox.Store          ( rStream, aBox.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aBackground.Store   ( rStream, aBackground.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/  
/*N*/      aAdjust.Store       ( rStream, aAdjust.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/  
/*N*/      aHorJustify.Store   ( rStream, aHorJustify.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aVerJustify.Store   ( rStream, aVerJustify.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aOrientation.Store  ( rStream, aOrientation.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aMargin.Store       ( rStream, aMargin.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aLinebreak.Store    ( rStream, aLinebreak.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      // Rotation ab SO5
/*N*/      aRotateAngle.Store  ( rStream, aRotateAngle.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/      aRotateMode.Store   ( rStream, aRotateMode.GetVersion( SOFFICE_FILEFORMAT_40 ) );
/*N*/  
/*N*/      aNumFormat.Save( rStream );
/*N*/  
/*N*/      return (rStream.GetError() == 0);
/*N*/  }


//	---------------------------------------------------------------------------

/*N*/ ScAutoFormatData::ScAutoFormatData()
/*N*/ {
/*N*/ 	nStrResId = USHRT_MAX;
/*N*/ 
/*N*/ 	bIncludeValueFormat =
/*N*/ 	bIncludeFont =
/*N*/ 	bIncludeJustify =
/*N*/ 	bIncludeFrame =
/*N*/ 	bIncludeBackground =
/*N*/ 	bIncludeWidthHeight = TRUE;
/*N*/ 
/*N*/     ppDataField = new ScAutoFormatDataField*[ 16 ];
/*N*/     for( USHORT nIndex = 0; nIndex < 16; ++nIndex )
/*N*/         ppDataField[ nIndex ] = new ScAutoFormatDataField;
/*N*/ }

/*N*/ ScAutoFormatData::ScAutoFormatData( const ScAutoFormatData& rData ) :
/*N*/ 		aName( rData.aName ),
/*N*/ 		nStrResId( rData.nStrResId ),
/*N*/ 		bIncludeValueFormat( rData.bIncludeValueFormat ),
/*N*/ 		bIncludeFont( rData.bIncludeFont ),
/*N*/ 		bIncludeJustify( rData.bIncludeJustify ),
/*N*/ 		bIncludeFrame( rData.bIncludeFrame ),
/*N*/ 		bIncludeBackground( rData.bIncludeBackground ),
/*N*/ 		bIncludeWidthHeight( rData.bIncludeWidthHeight )
/*N*/ {
/*N*/     ppDataField = new ScAutoFormatDataField*[ 16 ];
/*N*/     for( USHORT nIndex = 0; nIndex < 16; ++nIndex )
/*N*/         ppDataField[ nIndex ] = new ScAutoFormatDataField( rData.GetField( nIndex ) );
/*N*/ }

/*N*/ ScAutoFormatData::~ScAutoFormatData()
/*N*/ {
/*N*/     for( USHORT nIndex = 0; nIndex < 16; ++nIndex )
/*N*/         delete ppDataField[ nIndex ];
/*N*/     delete[] ppDataField;
/*N*/ }

/*N*/  ScAutoFormatDataField& ScAutoFormatData::GetField( USHORT nIndex )
/*N*/  {
/*N*/      DBG_ASSERT( (0 <= nIndex) && (nIndex < 16), "ScAutoFormatData::GetField - illegal index" );
/*N*/      DBG_ASSERT( ppDataField && ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
/*N*/      return *ppDataField[ nIndex ];
/*N*/  }

/*N*/ const ScAutoFormatDataField& ScAutoFormatData::GetField( USHORT nIndex ) const
/*N*/ {
/*N*/     DBG_ASSERT( (0 <= nIndex) && (nIndex < 16), "ScAutoFormatData::GetField - illegal index" );
/*N*/     DBG_ASSERT( ppDataField && ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
/*N*/     return *ppDataField[ nIndex ];
/*N*/ }

/*N*/ const SfxPoolItem* ScAutoFormatData::GetItem( USHORT nIndex, USHORT nWhich ) const
/*N*/ {
/*N*/     const ScAutoFormatDataField& rField = GetField( nIndex );
/*N*/     switch( nWhich )
/*N*/ 	{
/*N*/         case ATTR_FONT:             return &rField.GetFont();
/*N*/         case ATTR_FONT_HEIGHT:      return &rField.GetHeight();
/*N*/         case ATTR_FONT_WEIGHT:      return &rField.GetWeight();
/*N*/         case ATTR_FONT_POSTURE:     return &rField.GetPosture();
/*N*/         case ATTR_CJK_FONT:         return &rField.GetCJKFont();
/*N*/         case ATTR_CJK_FONT_HEIGHT:  return &rField.GetCJKHeight();
/*N*/         case ATTR_CJK_FONT_WEIGHT:  return &rField.GetCJKWeight();
/*N*/         case ATTR_CJK_FONT_POSTURE: return &rField.GetCJKPosture();
/*N*/         case ATTR_CTL_FONT:         return &rField.GetCTLFont();
/*N*/         case ATTR_CTL_FONT_HEIGHT:  return &rField.GetCTLHeight();
/*N*/         case ATTR_CTL_FONT_WEIGHT:  return &rField.GetCTLWeight();
/*N*/         case ATTR_CTL_FONT_POSTURE: return &rField.GetCTLPosture();
/*N*/         case ATTR_FONT_UNDERLINE:   return &rField.GetUnderline();
/*N*/         case ATTR_FONT_CROSSEDOUT:  return &rField.GetCrossedOut();
/*N*/         case ATTR_FONT_CONTOUR:     return &rField.GetContour();
/*N*/         case ATTR_FONT_SHADOWED:    return &rField.GetShadowed();
/*N*/         case ATTR_FONT_COLOR:       return &rField.GetColor();
/*N*/         case ATTR_BORDER:           return &rField.GetBox();
/*N*/         case ATTR_BACKGROUND:       return &rField.GetBackground();
/*N*/         case ATTR_HOR_JUSTIFY:      return &rField.GetHorJustify();
/*N*/         case ATTR_VER_JUSTIFY:      return &rField.GetVerJustify();
/*N*/         case ATTR_ORIENTATION:      return &rField.GetOrientation();
/*N*/         case ATTR_MARGIN:           return &rField.GetMargin();
/*N*/         case ATTR_LINEBREAK:        return &rField.GetLinebreak();
/*N*/         case ATTR_ROTATE_VALUE:     return &rField.GetRotateAngle();
/*N*/         case ATTR_ROTATE_MODE:      return &rField.GetRotateMode();
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*N*/ void ScAutoFormatData::PutItem( USHORT nIndex, const SfxPoolItem& rItem )
/*N*/ {
/*N*/     ScAutoFormatDataField& rField = GetField( nIndex );
/*N*/     switch( rItem.Which() )
/*N*/ 	{
/*N*/         case ATTR_FONT:             rField.SetFont( (const SvxFontItem&)rItem );              break;
/*N*/         case ATTR_FONT_HEIGHT:      rField.SetHeight( (const SvxFontHeightItem&)rItem );      break;
/*N*/         case ATTR_FONT_WEIGHT:      rField.SetWeight( (const SvxWeightItem&)rItem );          break;
/*N*/         case ATTR_FONT_POSTURE:     rField.SetPosture( (const SvxPostureItem&)rItem );        break;
/*N*/         case ATTR_CJK_FONT:         rField.SetCJKFont( (const SvxFontItem&)rItem );           break;
/*N*/         case ATTR_CJK_FONT_HEIGHT:  rField.SetCJKHeight( (const SvxFontHeightItem&)rItem );   break;
/*N*/         case ATTR_CJK_FONT_WEIGHT:  rField.SetCJKWeight( (const SvxWeightItem&)rItem );       break;
/*N*/         case ATTR_CJK_FONT_POSTURE: rField.SetCJKPosture( (const SvxPostureItem&)rItem );     break;
/*N*/         case ATTR_CTL_FONT:         rField.SetCTLFont( (const SvxFontItem&)rItem );           break;
/*N*/         case ATTR_CTL_FONT_HEIGHT:  rField.SetCTLHeight( (const SvxFontHeightItem&)rItem );   break;
/*N*/         case ATTR_CTL_FONT_WEIGHT:  rField.SetCTLWeight( (const SvxWeightItem&)rItem );       break;
/*N*/         case ATTR_CTL_FONT_POSTURE: rField.SetCTLPosture( (const SvxPostureItem&)rItem );     break;
/*N*/         case ATTR_FONT_UNDERLINE:   rField.SetUnderline( (const SvxUnderlineItem&)rItem );    break;
/*N*/         case ATTR_FONT_CROSSEDOUT:  rField.SetCrossedOut( (const SvxCrossedOutItem&)rItem );  break;
/*N*/         case ATTR_FONT_CONTOUR:     rField.SetContour( (const SvxContourItem&)rItem );        break;
/*N*/         case ATTR_FONT_SHADOWED:    rField.SetShadowed( (const SvxShadowedItem&)rItem );      break;
/*N*/         case ATTR_FONT_COLOR:       rField.SetColor( (const SvxColorItem&)rItem );            break;
/*N*/         case ATTR_BORDER:           rField.SetBox( (const SvxBoxItem&)rItem );                break;
/*N*/         case ATTR_BACKGROUND:       rField.SetBackground( (const SvxBrushItem&)rItem );       break;
/*N*/         case ATTR_HOR_JUSTIFY:      rField.SetHorJustify( (const SvxHorJustifyItem&)rItem );  break;
/*N*/         case ATTR_VER_JUSTIFY:      rField.SetVerJustify( (const SvxVerJustifyItem&)rItem );  break;
/*N*/         case ATTR_ORIENTATION:      rField.SetOrientation( (const SvxOrientationItem&)rItem );break;
/*N*/         case ATTR_MARGIN:           rField.SetMargin( (const SvxMarginItem&)rItem );          break;
/*N*/         case ATTR_LINEBREAK:        rField.SetLinebreak( (const SfxBoolItem&)rItem );         break;
/*N*/         case ATTR_ROTATE_VALUE:     rField.SetRotateAngle( (const SfxInt32Item&)rItem );      break;
/*N*/         case ATTR_ROTATE_MODE:      rField.SetRotateMode( (const SvxRotateModeItem&)rItem );  break;
/*N*/ 	}
/*N*/ }







/*N*/  BOOL ScAutoFormatData::Load( SvStream& rStream, const ScAfVersions& rVersions )
/*N*/  {
/*N*/  	BOOL    bRet = TRUE;
/*N*/      USHORT  nVer = 0;
/*N*/      rStream >> nVer;
/*N*/  	bRet = 0 == rStream.GetError();
/*N*/      if( bRet && (nVer == AUTOFORMAT_DATA_ID_X ||
/*N*/              (AUTOFORMAT_DATA_ID_504 <= nVer && nVer <= AUTOFORMAT_DATA_ID)) )
/*N*/  	{
/*N*/  		CharSet eSysSet = gsl_getSystemTextEncoding();
/*N*/  		CharSet eSrcSet = rStream.GetStreamCharSet();
/*N*/  
/*N*/  		BOOL b;
/*N*/  		rStream.ReadByteString( aName, eSrcSet );
/*N*/          if( AUTOFORMAT_DATA_ID_552 <= nVer )
/*N*/  		{
/*N*/  			rStream >> nStrResId;
/*N*/  			USHORT nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
/*N*/  			if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
/*N*/  				nId < RID_SVXSTR_TBLAFMT_END )
/*N*/  			{
/*N*/  				aName = SVX_RESSTR( nId );
/*N*/  			}
/*N*/  			else
/*N*/  				nStrResId = USHRT_MAX;
/*N*/  		}
/*N*/  
/*N*/  		rStream >> b; bIncludeFont = b;
/*N*/  		rStream >> b; bIncludeJustify = b;
/*N*/  		rStream >> b; bIncludeFrame = b;
/*N*/  		rStream >> b; bIncludeBackground = b;
/*N*/  		rStream >> b; bIncludeValueFormat = b;
/*N*/  		rStream >> b; bIncludeWidthHeight = b;
/*N*/  
/*N*/  		bRet = 0 == rStream.GetError();
/*N*/  		for( USHORT i = 0; bRet && i < 16; ++i )
/*N*/              bRet = GetField( i ).Load( rStream, rVersions, nVer );
/*N*/  	}
/*N*/  	else
/*N*/  		bRet = FALSE;
/*N*/  	return bRet;
/*N*/  }

/*N*/  #ifdef READ_OLDVERS
/*N*/  BOOL ScAutoFormatData::LoadOld( SvStream& rStream, const ScAfVersions& rVersions )
/*N*/  {
/*N*/  	BOOL    bRet = TRUE;
/*N*/  	USHORT  nVal = 0;
/*N*/  	rStream >> nVal;
/*N*/  	bRet = (rStream.GetError() == 0);
/*N*/  	if (bRet && (nVal == AUTOFORMAT_OLD_DATA_ID))
/*N*/  	{
/*N*/  		rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
/*N*/  		BOOL b;
/*N*/  		rStream >> b; bIncludeFont = b;
/*N*/  		rStream >> b; bIncludeJustify = b;
/*N*/  		rStream >> b; bIncludeFrame = b;
/*N*/  		rStream >> b; bIncludeBackground = b;
/*N*/  		rStream >> b; bIncludeValueFormat = b;
/*N*/  		rStream >> b; bIncludeWidthHeight = b;
/*N*/  
/*N*/  		bRet = 0 == rStream.GetError();
/*N*/  		for (USHORT i=0; bRet && i < 16; i++)
/*N*/              bRet = GetField( i ).LoadOld( rStream, rVersions );
/*N*/  	}
/*N*/  	else
/*N*/  		bRet = FALSE;
/*N*/  	return bRet;
/*N*/  }
/*N*/  #endif
/*N*/  
/*N*/  BOOL ScAutoFormatData::Save(SvStream& rStream)
/*N*/  {
/*N*/  	USHORT nVal = AUTOFORMAT_DATA_ID;
/*N*/  	BOOL b;
/*N*/  	rStream << nVal;
/*N*/  	rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
/*N*/  
/*N*/  #if 0
/*N*/  	//	This was an internal flag to allow creating AutoFormats with localized names
/*N*/  
/*N*/  	if ( USHRT_MAX == nStrResId )
/*N*/  	{
/*N*/  		String aIniVal( SFX_APP()->GetIniManager()->Get(
/*N*/  			SFX_GROUP_WORKINGSET_IMPL,
/*N*/  			String( RTL_CONSTASCII_USTRINGPARAM( "SaveTableAutoFmtNameId" ))));
/*N*/  		if( 0 != aIniVal.ToInt32() )
/*N*/  		{
/*N*/  			// check Name for ResId
/*N*/  			for( USHORT nId = RID_SVXSTR_TBLAFMT_BEGIN;
/*N*/  						RID_SVXSTR_TBLAFMT_END > nId; ++nId )
/*N*/  			{
/*N*/  				String s( SVX_RES( nId ) );
/*N*/  				if( s == aName )
/*N*/  				{
/*N*/  					nStrResId = nId - RID_SVXSTR_TBLAFMT_BEGIN;
/*N*/  					break;
/*N*/  				}
/*N*/  			}
/*N*/  		}
/*N*/  	}
/*N*/  #endif
/*N*/  
/*N*/  	rStream << nStrResId;
/*N*/  	rStream << ( b = bIncludeFont );
/*N*/  	rStream << ( b = bIncludeJustify );
/*N*/  	rStream << ( b = bIncludeFrame );
/*N*/  	rStream << ( b = bIncludeBackground );
/*N*/  	rStream << ( b = bIncludeValueFormat );
/*N*/  	rStream << ( b = bIncludeWidthHeight );
/*N*/  
/*N*/      BOOL bRet = 0 == rStream.GetError();
/*N*/  	for (USHORT i = 0; bRet && (i < 16); i++)
/*N*/          bRet = GetField( i ).Save( rStream );
/*N*/  
/*N*/  	return bRet;
/*N*/  }

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

/*N*/ ScAutoFormat::ScAutoFormat(USHORT nLim, USHORT nDel, BOOL bDup):
/*N*/ 	SortedCollection        (nLim, nDel, bDup),
/*N*/ 	bSaveLater				(FALSE)
/*N*/ {
/*N*/     //  create default autoformat
/*N*/     ScAutoFormatData* pData = new ScAutoFormatData;
/*N*/ 	String aName(ScGlobal::GetRscString(STR_STYLENAME_STANDARD));
/*N*/ 	pData->SetName(aName);
/*N*/ 
/*N*/     //  default font, default height
/*N*/     Font aStdFont = OutputDevice::GetDefaultFont(
/*N*/         DEFAULTFONT_LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE );
/*N*/     SvxFontItem aFontItem(
/*N*/         aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
/*N*/         aStdFont.GetPitch(), aStdFont.GetCharSet() );
/*N*/ 
/*N*/     aStdFont = OutputDevice::GetDefaultFont(
/*N*/         DEFAULTFONT_CJK_SPREADSHEET, LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE );
/*N*/     SvxFontItem aCJKFontItem(
/*N*/         aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
/*N*/         aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CJK_FONT );
/*N*/ 
/*N*/     aStdFont = OutputDevice::GetDefaultFont(
/*N*/         DEFAULTFONT_CTL_SPREADSHEET, LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE );
/*N*/     SvxFontItem aCTLFontItem(
/*N*/         aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
/*N*/         aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CTL_FONT );
/*N*/ 
/*N*/ 	SvxFontHeightItem aHeight( 200 );		// 10 pt;
/*N*/ 
/*N*/     //  black thin border
/*N*/ 	Color aBlack( COL_BLACK );
/*N*/ 	SvxBorderLine aLine( &aBlack, DEF_LINE_WIDTH_0 );
/*N*/ 	SvxBoxItem aBox;
/*N*/ 	aBox.SetLine(&aLine, BOX_LINE_LEFT);
/*N*/ 	aBox.SetLine(&aLine, BOX_LINE_TOP);
/*N*/ 	aBox.SetLine(&aLine, BOX_LINE_RIGHT);
/*N*/ 	aBox.SetLine(&aLine, BOX_LINE_BOTTOM);
/*N*/ 
/*N*/ 	Color aWhite(COL_WHITE);
/*N*/ 	Color aBlue(COL_BLUE);
/*N*/ 	SvxColorItem aWhiteText( aWhite );
/*N*/ 	SvxColorItem aBlackText( aBlack );
/*N*/ 	SvxBrushItem aBlueBack( aBlue );
/*N*/ 	SvxBrushItem aWhiteBack( aWhite );
/*N*/ 	SvxBrushItem aGray70Back( Color(0x4d, 0x4d, 0x4d) );
/*N*/ 	SvxBrushItem aGray20Back( Color(0xcc, 0xcc, 0xcc) );
/*N*/ 
/*N*/ 	for (USHORT i=0; i<16; i++)
/*N*/ 	{
/*N*/         pData->PutItem( i, aBox );
/*N*/         pData->PutItem( i, aFontItem );
/*N*/         pData->PutItem( i, aCJKFontItem );
/*N*/         pData->PutItem( i, aCTLFontItem );
/*N*/         aHeight.SetWhich( ATTR_FONT_HEIGHT );
/*N*/         pData->PutItem( i, aHeight );
/*N*/         aHeight.SetWhich( ATTR_CJK_FONT_HEIGHT );
/*N*/         pData->PutItem( i, aHeight );
/*N*/         aHeight.SetWhich( ATTR_CTL_FONT_HEIGHT );
/*N*/         pData->PutItem( i, aHeight );
/*N*/         if (i<4)                                    // top: white on blue
/*N*/ 		{
/*N*/             pData->PutItem( i, aWhiteText );
/*N*/             pData->PutItem( i, aBlueBack );
/*N*/ 		}
/*N*/         else if ( i%4 == 0 )                        // left: white on gray70
/*N*/ 		{
/*N*/             pData->PutItem( i, aWhiteText );
/*N*/             pData->PutItem( i, aGray70Back );
/*N*/ 		}
/*N*/         else if ( i%4 == 3 || i >= 12 )             // right and bottom: black on gray20
/*N*/ 		{
/*N*/             pData->PutItem( i, aBlackText );
/*N*/             pData->PutItem( i, aGray20Back );
/*N*/ 		}
/*N*/         else                                        // center: black on white
/*N*/ 		{
/*N*/             pData->PutItem( i, aBlackText );
/*N*/             pData->PutItem( i, aWhiteBack );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	Insert(pData);
/*N*/ }

/*N*/ ScAutoFormat::ScAutoFormat(const ScAutoFormat& rAutoFormat) :
/*N*/ 	SortedCollection (rAutoFormat),
/*N*/ 	bSaveLater		 (FALSE)
/*N*/ {}

/*N*/ ScAutoFormat::~ScAutoFormat()
/*N*/ {
/*N*/ 	//	Bei Aenderungen per StarOne wird nicht sofort gespeichert, sondern zuerst nur
/*N*/ 	//	das SaveLater Flag gesetzt. Wenn das Flag noch gesetzt ist, jetzt speichern.
/*N*/ 
/*N*/ 	if (bSaveLater)
/*N*/ 		Save();
/*N*/ }

/*N*/ void ScAutoFormat::SetSaveLater( BOOL bSet )
/*N*/ {
/*N*/ 	bSaveLater = bSet;
/*N*/ }


/*N*/ BOOL ScAutoFormat::Load()
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 
/*N*/ 	INetURLObject aURL;
/*N*/ 	SvtPathOptions aPathOpt;
/*N*/ 	aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
/*N*/ 	aURL.setFinalSlash();
/*N*/ 	aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( sAutoTblFmtName ) ) );
/*N*/ 
/*N*/ 	SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_READ, TRUE );
/*N*/ 	SvStream* pStream = aMedium.GetInStream();
/*N*/ 	bRet = (pStream && pStream->GetError() == 0);
/*N*/ 	if (bRet)
/*N*/ 	{
/*N*/ 		SvStream& rStream = *pStream;
/*N*/ 		// Achtung hier muss ein allgemeiner Header gelesen werden
/*N*/ 		USHORT nVal = 0;
/*N*/ 		rStream >> nVal;
/*N*/ 		bRet = 0 == rStream.GetError();
/*N*/ 
/*N*/ 		ScAfVersions aVersions;
/*N*/ 
/*N*/ 		if (bRet)
/*N*/ 		{
/*N*/ 			if( nVal == AUTOFORMAT_ID_358 ||
/*N*/ 					(AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
/*N*/ 			{
/*N*/ 				UINT16 nFileVers = SOFFICE_FILEFORMAT_40;
/*N*/ 				BYTE nChrSet, nCnt;
/*N*/ 				long nPos = rStream.Tell();
/*N*/ 				rStream >> nCnt >> nChrSet;
/*N*/ //				if( 4 <= nCnt )
/*N*/ //					rStream >> nFileVers;
/*N*/ 				if( rStream.Tell() != ULONG(nPos + nCnt) )
/*N*/ 				{
/*N*/ 					DBG_ERRORFILE( "Der Header enthaelt mehr/neuere Daten" );
/*N*/ 					rStream.Seek( nPos + nCnt );
/*N*/ 				}
/*N*/                 rStream.SetStreamCharSet( GetSOLoadTextEncoding( nChrSet, nFileVers ) );
/*N*/ 				rStream.SetVersion( nFileVers );
/*N*/ 			}
/*N*/ 
/*N*/ 			if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
/*N*/ 					(AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
/*N*/ 			{
/*N*/ 				aVersions.Load( rStream, nVal );		// Item-Versionen
/*N*/ 
/*N*/ 				ScAutoFormatData* pData;
/*N*/ 				USHORT nAnz = 0;
/*N*/ 				rStream >> nAnz;
/*N*/ 				bRet = (rStream.GetError() == 0);
/*N*/ 				for (USHORT i=0; bRet && (i < nAnz); i++)
/*N*/ 				{
/*N*/ 					pData = new ScAutoFormatData();
/*N*/ 					bRet = pData->Load(rStream, aVersions);
/*N*/ 					Insert(pData);
/*N*/ 				}
/*N*/ 			}
/*N*/ #ifdef READ_OLDVERS
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( AUTOFORMAT_OLD_ID_NEW == nVal )
/*N*/ 				{
/*N*/ 					// alte Version der Versions laden
/*N*/ 					rStream >> aVersions.nFontVersion;
/*N*/ 					rStream >> aVersions.nFontHeightVersion;
/*N*/ 					rStream >> aVersions.nWeightVersion;
/*N*/ 					rStream >> aVersions.nPostureVersion;
/*N*/ 					rStream >> aVersions.nUnderlineVersion;
/*N*/ 					rStream >> aVersions.nCrossedOutVersion;
/*N*/ 					rStream >> aVersions.nContourVersion;
/*N*/ 					rStream >> aVersions.nShadowedVersion;
/*N*/ 					rStream >> aVersions.nColorVersion;
/*N*/ 					rStream >> aVersions.nHorJustifyVersion;
/*N*/ 					rStream >> aVersions.nVerJustifyVersion;
/*N*/ 					rStream >> aVersions.nOrientationVersion;
/*N*/ 					rStream >> aVersions.nBoolVersion;
/*N*/ 					rStream >> aVersions.nMarginVersion;
/*N*/ 					rStream >> aVersions.nBoxVersion;
/*N*/ 					rStream >> aVersions.nBrushVersion;
/*N*/ 				}
/*N*/ 				if( AUTOFORMAT_OLD_ID_OLD == nVal ||
/*N*/ 					AUTOFORMAT_OLD_ID_NEW == nVal )
/*N*/ 				{
/*N*/ 					ScAutoFormatData* pData;
/*N*/ 					USHORT nAnz = 0;
/*N*/ 					rStream >> nAnz;
/*N*/ 					bRet = 0 == rStream.GetError();
/*N*/ 					for( USHORT i=0; bRet && (i < nAnz); ++i )
/*N*/ 					{
/*N*/ 						pData = new ScAutoFormatData();
/*N*/ 						bRet = pData->LoadOld( rStream, aVersions );
/*N*/ 						Insert( pData );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bRet = FALSE;
/*N*/ 			}
/*N*/ #endif
/*N*/ 		}
/*N*/ 	}
/*N*/ 	bSaveLater = FALSE;
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL ScAutoFormat::Save()
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 
/*N*/ 	INetURLObject aURL;
/*N*/ 	SvtPathOptions aPathOpt;
/*N*/ 	aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
/*N*/ 	aURL.setFinalSlash();
/*N*/ 	aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( sAutoTblFmtName ) ) );
/*N*/ 
/*N*/ 	SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_WRITE, TRUE );
/*N*/ 	SvStream* pStream = aMedium.GetOutStream();
/*N*/ 	bRet = (pStream && pStream->GetError() == 0);
/*N*/ 	if (bRet)
/*N*/ 	{
/*N*/ 		SvStream& rStream = *pStream;
/*N*/ 		rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
/*N*/ 
/*N*/ 		// Achtung hier muss ein allgemeiner Header gespeichert werden
/*N*/ 		USHORT nVal = AUTOFORMAT_ID;
/*N*/ 		rStream << nVal
/*N*/ 				<< (BYTE)2 		// Anzahl von Zeichen des Headers incl. diesem
/*N*/                 << (BYTE)::GetSOStoreTextEncoding(
/*N*/                     gsl_getSystemTextEncoding(), rStream.GetVersion() );
/*N*/ //				<< (BYTE)4 		// Anzahl von Zeichen des Headers incl. diesem
/*N*/ //				<< (BYTE)::GetStoreCharSet(::GetSystemCharSet())
/*N*/ //				<< (UNIT16)SOFFICE_FILEFORMAT_NOW;
/*N*/ 		ScAfVersions::Write(rStream);			// Item-Versionen
/*N*/ 
/*N*/ 		bRet = (rStream.GetError() == 0);
/*N*/ 		//-----------------------------------------------------------
/*N*/ 		rStream << (USHORT)(nCount - 1);
/*N*/ 		bRet = (rStream.GetError() == 0);
/*N*/ 		for (USHORT i=1; bRet && (i < nCount); i++)
/*N*/ 			bRet = ((ScAutoFormatData*)pItems[i])->Save(rStream);
/*N*/ 		rStream.Flush();
/*N*/ 
/*N*/ 		aMedium.Commit();
/*N*/ 	}
/*N*/ 	bSaveLater = FALSE;
/*N*/ 	return bRet;
/*N*/ }





}
