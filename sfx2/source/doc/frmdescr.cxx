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


#include <sot/object.hxx>
#include <tools/stream.hxx>
#include <vcl/splitwin.hxx>
#include <svl/itemset.hxx>

#include <sfx2/frmdescr.hxx>
#include <sfx2/app.hxx>

DBG_NAME(SfxFrameDescriptor);

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

void SfxFrameDescriptor::SetURL( const OUString& rURL )
{
    aURL = INetURLObject(rURL);
    SetActualURL( aURL );
}

void SfxFrameDescriptor::SetActualURL( const OUString& rURL )
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
        // Currently in the clone of SfxAllItemSets there is still a bug ...
        pFrame->pImp->pArgs = new SfxAllItemSet( SFX_APP()->GetPool() );
        pFrame->pImp->pArgs->Put(*pImp->pArgs);
    }

    if ( bWithIds )
        pFrame->nItemId = nItemId;
    else
        pFrame->nItemId = 0;

    return pFrame;
}

sal_Bool SfxFrameDescriptor::HasFrameBorder() const
{
    return (nHasBorder & BORDER_YES) != 0;
}

void SfxFrameDescriptor::SetWallpaper( const Wallpaper& rWallpaper )
{
    DELETEZ( pImp->pWallpaper );

    if ( rWallpaper.GetStyle() != WALLPAPER_NULL )
        pImp->pWallpaper = new Wallpaper( rWallpaper );
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
    OUString&           rText,
    const IntlWrapper *
)   const
{
    rText = OUString();
    return SFX_ITEM_PRESENTATION_NONE;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
