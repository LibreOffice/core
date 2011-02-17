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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <sot/object.hxx>
#include <tools/stream.hxx>
#include <vcl/splitwin.hxx>
#include <svl/itemset.hxx>
#ifndef GCC
#endif

#include <sfx2/frmdescr.hxx>
#include <sfx2/app.hxx>

DBG_NAME(SfxFrameDescriptor);

#define VERSION (sal_uInt16) 3

struct SfxFrameDescriptor_Impl
{
    Wallpaper*  pWallpaper;
    SfxItemSet* pArgs;
    sal_Bool        bEditable;

    SfxFrameDescriptor_Impl() : pWallpaper( NULL ), pArgs( NULL ), bEditable( sal_True ) {}
    ~SfxFrameDescriptor_Impl()
    {
        delete pWallpaper;
        delete pArgs;
    }
};

SfxFrameDescriptor::SfxFrameDescriptor() :
    aMargin( -1, -1 ),
    nWidth( 0L ),
    eScroll( ScrollingAuto ),
    eSizeSelector( SIZE_ABS ),
    nHasBorder( BORDER_YES ),
    nItemId( 0 ),
    bResizeHorizontal( sal_True ),
    bResizeVertical( sal_True ),
    bHasUI( sal_True ),
    bReadOnly( sal_False )
{
    DBG_CTOR(SfxFrameDescriptor, 0);

    pImp = new SfxFrameDescriptor_Impl;
}

SfxFrameDescriptor::~SfxFrameDescriptor()
{
    DBG_DTOR(SfxFrameDescriptor, 0);
    delete pImp;
}

SfxItemSet* SfxFrameDescriptor::GetArgs()
{
    if( !pImp->pArgs )
        pImp->pArgs = new SfxAllItemSet( SFX_APP()->GetPool() );
    return pImp->pArgs;
}

void SfxFrameDescriptor::SetURL( const String& rURL )
{
    aURL = INetURLObject(rURL);
    SetActualURL( aURL );
}

void SfxFrameDescriptor::SetURL( const INetURLObject& rURL )
{
    aURL = rURL.GetMainURL( INetURLObject::DECODE_TO_IURI );
    SetActualURL( aURL );
}

void SfxFrameDescriptor::SetActualURL( const String& rURL )
{
    aActualURL = INetURLObject(rURL);
    if ( pImp->pArgs )
        pImp->pArgs->ClearItem();
}

void SfxFrameDescriptor::SetActualURL( const INetURLObject& rURL )
{
    SetActualURL(String(rURL.GetMainURL( INetURLObject::DECODE_TO_IURI )));
}

void SfxFrameDescriptor::SetEditable( sal_Bool bSet )
{
    pImp->bEditable = bSet;
}

sal_Bool SfxFrameDescriptor::IsEditable() const
{
    return pImp->bEditable;
}

sal_Bool SfxFrameDescriptor::CompareOriginal( SfxFrameDescriptor& rDescr ) const
{
    if( aURL != rDescr.aURL )
        return sal_False;
    else
        return sal_True;
}

sal_Bool SfxFrameDescriptor::CheckContent() const
{
    sal_Bool bRet = !( aURL == aActualURL );
    return bRet;
}

void SfxFrameDescriptor::UnifyContent( sal_Bool bTakeActual )
{
    if ( bTakeActual )
        aURL = aActualURL;
    else
        aActualURL = aURL;
}

SfxFrameDescriptor* SfxFrameDescriptor::Clone( sal_Bool bWithIds ) const
{
    SfxFrameDescriptor *pFrame = new SfxFrameDescriptor;

    pFrame->aURL = aURL;
    pFrame->aActualURL = aActualURL;
    pFrame->aName = aName;
    pFrame->aMargin = aMargin;
    pFrame->nWidth = nWidth;
    pFrame->eSizeSelector = eSizeSelector;
    pFrame->eScroll = eScroll;
    pFrame->bResizeHorizontal = bResizeHorizontal;
    pFrame->bResizeVertical = bResizeVertical;
    pFrame->nHasBorder = nHasBorder;
    pFrame->bHasUI = bHasUI;
    pFrame->SetReadOnly( IsReadOnly() );
    pFrame->SetEditable( IsEditable() );
    if ( pImp->pWallpaper )
        pFrame->pImp->pWallpaper = new Wallpaper( *pImp->pWallpaper );
    if( pImp->pArgs )
    {
        // Aktuell ist im Clone von SfxAllItemSets noch ein Bug...
        pFrame->pImp->pArgs = new SfxAllItemSet( SFX_APP()->GetPool() );
        pFrame->pImp->pArgs->Put(*pImp->pArgs);
    }

    if ( bWithIds )
        pFrame->nItemId = nItemId;
    else
        pFrame->nItemId = 0;

    return pFrame;
}

sal_uInt16 SfxFrameDescriptor::GetWinBits() const
{
    sal_uInt16 nBits = 0;
    if ( eSizeSelector == SIZE_REL )
        nBits |= SWIB_RELATIVESIZE;
    if ( eSizeSelector == SIZE_PERCENT )
        nBits |= SWIB_PERCENTSIZE;
    if ( !IsResizable() )
        nBits |= SWIB_FIXED;
    if ( !nWidth )
        nBits |= SWIB_INVISIBLE;
    return nBits;
}

sal_Bool SfxFrameDescriptor::HasFrameBorder() const
{
    return (nHasBorder & BORDER_YES) != 0;
}

long SfxFrameDescriptor::GetSize() const
{
    return nWidth;
}

void SfxFrameDescriptor::TakeProperties( const SfxFrameProperties& rProp )
{
    aURL = aActualURL = INetURLObject(rProp.aURL);
    aName = rProp.aName;
    aMargin.Width() = rProp.lMarginWidth;
    aMargin.Height() = rProp.lMarginHeight;
    nWidth = rProp.lSize;
    eScroll = rProp.eScroll;
    eSizeSelector = rProp.eSizeSelector;
    nHasBorder = rProp.bHasBorder ? BORDER_YES : BORDER_NO;
    if ( rProp.bBorderSet )
        nHasBorder |= BORDER_SET;
    bResizeHorizontal = bResizeVertical = rProp.bResizable;
}

void SfxFrameDescriptor::SetWallpaper( const Wallpaper& rWallpaper )
{
    DELETEZ( pImp->pWallpaper );

    if ( rWallpaper.GetStyle() != WALLPAPER_NULL )
        pImp->pWallpaper = new Wallpaper( rWallpaper );
}

const Wallpaper* SfxFrameDescriptor::GetWallpaper() const
{
    return pImp->pWallpaper;
}

sal_uInt16 SfxFrameDescriptor::GetItemPos() const
{
    return USHRT_MAX;
}


SfxFrameProperties::SfxFrameProperties( const SfxFrameDescriptor *pD )
    : aURL( pD->GetURL().GetMainURL( INetURLObject::DECODE_TO_IURI ) )
    , aName( pD->GetName() )
    , lMarginWidth( pD->GetMargin().Width() )
    , lMarginHeight( pD->GetMargin().Height() )
    , lSize( pD->GetWidth() )
    , lSetSize( SIZE_NOT_SET )
    , lFrameSpacing( SPACING_NOT_SET )
    , lInheritedFrameSpacing( SPACING_NOT_SET )
    , eScroll( pD->GetScrollingMode() )
    , eSizeSelector( pD->GetSizeSelector() )
    , eSetSizeSelector( SIZE_REL )
    , bHasBorder( pD->HasFrameBorder() )
    , bBorderSet( pD->IsFrameBorderSet() )
    , bResizable( pD->IsResizable() )
    , bSetResizable( sal_False )
    , bIsRootSet( sal_False )
    , bIsInColSet( sal_False )
    , bHasBorderInherited( sal_False )
    , pFrame( pD->Clone() )
{
    bBorderSet = sal_True;
}

SfxFrameProperties& SfxFrameProperties::operator =(
    const SfxFrameProperties &rProp )
{
    aURL = rProp.aURL;
    aName = rProp.aName;
    lMarginWidth = rProp.lMarginWidth;
    lMarginHeight = rProp.lMarginHeight;
    lSize = rProp.lSize;
    lSetSize = rProp.lSetSize;
    lFrameSpacing = rProp.lFrameSpacing;
    lInheritedFrameSpacing = rProp.lInheritedFrameSpacing;
    eScroll = rProp.eScroll;
    eSizeSelector = rProp.eSizeSelector;
    eSetSizeSelector = rProp.eSetSizeSelector;
    bHasBorder = rProp.bHasBorder;
    bBorderSet = rProp.bBorderSet;
    bResizable = rProp.bResizable;
    bSetResizable = rProp.bSetResizable;
    bIsRootSet = rProp.bIsRootSet;
    bIsInColSet = rProp.bIsInColSet;
    bHasBorderInherited = rProp.bHasBorderInherited;
    pFrame = rProp.pFrame->Clone();
    return *this;
}

int SfxFrameProperties::operator ==( const SfxFrameProperties& rProp ) const
{
    return aURL == rProp.aURL && aName == rProp.aName && lMarginWidth == rProp.lMarginWidth && lMarginHeight == rProp.lMarginHeight &&
            lSize == rProp.lSize && eScroll == rProp.eScroll && eSizeSelector == rProp.eSizeSelector &&
            lSetSize == rProp.lSetSize && lFrameSpacing == rProp.lFrameSpacing && eSetSizeSelector == rProp.eSetSizeSelector &&
            bHasBorder == rProp.bHasBorder && bBorderSet == rProp.bBorderSet &&
            bResizable == rProp.bResizable && bSetResizable == rProp.bSetResizable;
}

TYPEINIT1(SfxFrameDescriptorItem, SfxPoolItem);

SfxFrameDescriptorItem::~SfxFrameDescriptorItem()
{}

int SfxFrameDescriptorItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    return aProperties == ((SfxFrameDescriptorItem&)rAttr).aProperties;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxFrameDescriptorItem::Clone( SfxItemPool* ) const
{
    return new SfxFrameDescriptorItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation SfxFrameDescriptorItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText,
    const IntlWrapper *
)   const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}


