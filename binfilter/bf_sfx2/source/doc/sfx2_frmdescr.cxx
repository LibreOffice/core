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


#ifndef _SV_SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "frmdescr.hxx"
#include "app.hxx"
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

/*N*/ DBG_NAME(SfxFrameSetDescriptor)
/*N*/ DBG_NAME(SfxFrameDescriptor)

#define VERSION (USHORT) 3

/*N*/ struct SfxFrameSetDescriptor_Impl
/*N*/ {
/*N*/ 	Wallpaper*	pWallpaper;
/*N*/ 	Bitmap*		pBitmap;
/*N*/ 	BOOL		bNetscapeCompat;
/*N*/ };

/*N*/ struct SfxFrameDescriptor_Impl
/*N*/ {
/*N*/ 	Wallpaper*	pWallpaper;
/*N*/ 	SfxItemSet*	pArgs;
/*N*/ 	BOOL		bEditable;
/*N*/ 
/*N*/ 	SfxFrameDescriptor_Impl() : pWallpaper( NULL ), pArgs( NULL ), bEditable( TRUE ) {}
/*N*/ 	~SfxFrameDescriptor_Impl()
/*N*/ 	{
/*N*/ 		delete pWallpaper;
/*N*/ 		delete pArgs;
/*N*/ 	}
/*N*/ };

/*N*/ SfxFrameDescriptor::SfxFrameDescriptor( SfxFrameSetDescriptor *pParSet ) :
/*N*/ 	aMargin( -1, -1 ),
/*N*/ 	nWidth( 0L ),
/*N*/ 	nItemId( 0 ),
/*N*/ 	pFrameSet( 0L ),
/*N*/ 	pParentFrameSet( pParSet ),
/*N*/ 	eScroll( ScrollingAuto ),
/*N*/ 	eSizeSelector( SIZE_ABS ),
/*N*/ 	nHasBorder( BORDER_YES ),
/*N*/ 	bResizeHorizontal( TRUE ),
/*N*/ 	bResizeVertical( TRUE ),
/*N*/ 	bHasUI( TRUE ),
/*N*/ 	bReadOnly( FALSE )
/*N*/ {
/*N*/ 	DBG_CTOR(SfxFrameDescriptor, 0);
/*N*/ 
/*N*/ 	pImp = new SfxFrameDescriptor_Impl;
/*N*/ 	if ( pParentFrameSet )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 	pParentFrameSet->InsertFrame( this );
/*N*/ }

/*N*/ SfxFrameDescriptor::~SfxFrameDescriptor()
/*N*/ {
/*N*/ 	DBG_DTOR(SfxFrameDescriptor, 0);
/*N*/ 
/*N*/ 	if ( pFrameSet )
/*N*/ 		delete pFrameSet;
/*N*/ 	if ( pParentFrameSet )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pParentFrameSet->RemoveFrame( this );
/*N*/ 	delete pImp;
/*N*/ }

/*?*/ void SfxFrameDescriptor::SetURL( const String& rURL )
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*?*/ }

/*N*/ SfxFrameDescriptor* SfxFrameDescriptor::Clone(
/*N*/ 	SfxFrameSetDescriptor *pSet, BOOL bWithIds ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*N*/ }

/*N*/ SfxFrameProperties&	SfxFrameProperties::operator =(
/*N*/ 	const SfxFrameProperties &rProp )
/*N*/ {
/*N*/ 	aURL = rProp.aURL;
/*N*/ 	aName = rProp.aName;
/*N*/ 	lMarginWidth = rProp.lMarginWidth;
/*N*/ 	lMarginHeight = rProp.lMarginHeight;
/*N*/ 	lSize = rProp.lSize;
/*N*/ 	lSetSize = rProp.lSetSize;
/*N*/ 	lFrameSpacing = rProp.lFrameSpacing;
/*N*/ 	lInheritedFrameSpacing = rProp.lInheritedFrameSpacing;
/*N*/ 	eScroll = rProp.eScroll;
/*N*/ 	eSizeSelector = rProp.eSizeSelector;
/*N*/ 	eSetSizeSelector = rProp.eSetSizeSelector;
/*N*/ 	bHasBorder = rProp.bHasBorder;
/*N*/ 	bBorderSet = rProp.bBorderSet;
/*N*/ 	bResizable = rProp.bResizable;
/*N*/ 	bSetResizable = rProp.bSetResizable;
/*N*/ 	bIsRootSet = rProp.bIsRootSet;
/*N*/ 	bIsInColSet = rProp.bIsInColSet;
/*N*/ 	bHasBorderInherited = rProp.bHasBorderInherited;
/*N*/ 	pFrame = rProp.pFrame->Clone();
/*N*/ 	return *this;
/*N*/ }

/*N*/ int SfxFrameProperties::operator ==( const SfxFrameProperties& rProp ) const
/*N*/ {
/*N*/ 	return aURL == rProp.aURL && aName == rProp.aName && lMarginWidth == rProp.lMarginWidth && lMarginHeight == rProp.lMarginHeight &&
/*N*/ 			lSize == rProp.lSize && eScroll == rProp.eScroll && eSizeSelector == rProp.eSizeSelector &&
/*N*/ 			lSetSize == rProp.lSetSize && lFrameSpacing == rProp.lFrameSpacing && eSetSizeSelector == rProp.eSetSizeSelector &&
/*N*/ 			bHasBorder == rProp.bHasBorder && bBorderSet == rProp.bBorderSet &&
/*N*/ 			bResizable == rProp.bResizable && bSetResizable == rProp.bSetResizable;
/*N*/ }

/*?*/ TYPEINIT1(SfxFrameDescriptorItem, SfxPoolItem);

/*?*/ SfxFrameDescriptorItem::~SfxFrameDescriptorItem()
/*?*/ {}

/*?*/ int SfxFrameDescriptorItem::operator==( const SfxPoolItem& rAttr ) const
/*?*/ {
/*?*/ 	DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );
/*?*/ 
/*?*/ 	return aProperties == ((SfxFrameDescriptorItem&)rAttr).aProperties;
/*?*/ }

// -----------------------------------------------------------------------

/*?*/ SfxPoolItem* SfxFrameDescriptorItem::Clone( SfxItemPool* ) const
/*?*/ {
/*?*/ 	return new SfxFrameDescriptorItem( *this );
/*?*/ }

//------------------------------------------------------------------------

/*?*/ SfxItemPresentation SfxFrameDescriptorItem::GetPresentation
/*?*/ (
/*?*/ 	SfxItemPresentation ePres,
/*?*/ 	SfxMapUnit			eCoreUnit,
/*?*/ 	SfxMapUnit			ePresUnit,
/*?*/ 	XubString& 			rText,
/*?*/     const ::IntlWrapper *
/*?*/ )	const
/*?*/ {
/*?*/ 	rText.Erase();
/*?*/ 	return SFX_ITEM_PRESENTATION_NONE;
/*?*/ }


}
