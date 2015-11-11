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
#ifndef INCLUDED_SFX2_FRMDESCR_HXX
#define INCLUDED_SFX2_FRMDESCR_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/sfxsids.hrc>
#include <svl/poolitem.hxx>
#include <tools/gen.hxx>
#include <tools/urlobj.hxx>

class SvStream;
struct SfxFrameDescriptor_Impl;
class SfxFrameDescriptor;
class Wallpaper;


// The SfxFrame descriptors build a recursive structure, that covers all the
// required data in-order to display the frame document.
// Through a SfxFrameSetDescriptor access is given to the whole underlying
// structure. Due to the nature of the SfxFrames document, the
// SfxFramesSetDescriptor is not only the content of this document, but also
// describes the view on it. The FrameSet is made up of lines, which in turn,
// contains the actual window . A line can be horizontally or vertically
// aligned, from which also the alignment of the FrameSet is given.


enum ScrollingMode
{
    ScrollingYes,
    ScrollingNo,
    ScrollingAuto
};

enum SizeSelector
{
    SIZE_ABS,
    SIZE_PERCENT,
    SIZE_REL
};

#define SPACING_NOT_SET     -1L
#define SIZE_NOT_SET        -1L

class SfxItemSet;
struct SfxFrameProperties;

class SFX2_DLLPUBLIC SfxFrameDescriptor
{
    INetURLObject           aURL;
    INetURLObject           aActualURL;
    OUString                aName;
    Size                    aMargin;
    long                    nWidth;
    ScrollingMode           eScroll;
    SizeSelector            eSizeSelector;
    bool                    bHasBorder;
    bool                    bHasBorderSet;
    sal_uInt16              nItemId;
    bool                    bResizeHorizontal;
    bool                    bResizeVertical;
    bool                    bHasUI;
    bool                    bReadOnly;
    SfxFrameDescriptor_Impl* pImp;

public:
                            SfxFrameDescriptor();
                            ~SfxFrameDescriptor();

                            // FileName/URL
    SfxItemSet*             GetArgs();
    const INetURLObject&    GetURL() const
                            { return aURL; }
    void                    SetURL( const OUString& rURL );
    void                    SetActualURL( const INetURLObject& rURL );
    void                    SetActualURL( const OUString& rURL );
    void                    SetReadOnly( bool bSet ) { bReadOnly = bSet;}
    bool                    IsReadOnly(  ) const { return bReadOnly;}
    void                    SetEditable( bool bSet );
    bool                    IsEditable() const;

                            // Size
    void                    SetResizable( bool bRes )
                            { bResizeHorizontal = bResizeVertical = bRes; }

                            // FrameName
    const OUString&         GetName() const
                            { return aName; }
    void                    SetName( const OUString& rName )
                            { aName = rName; }

                            // Margin, Scrolling
    const Size&             GetMargin() const
                            { return aMargin; }
    void                    SetMargin( const Size& rMargin )
                            { aMargin = rMargin; }
    ScrollingMode           GetScrollingMode() const
                            { return eScroll; }
    void                    SetScrollingMode( ScrollingMode eMode )
                            { eScroll = eMode; }

    void                    SetWallpaper( const Wallpaper& rWallpaper );

                            // FrameBorder
    bool                    HasFrameBorder() const
                            { return bHasBorder; }
    bool                    IsFrameBorderOn() const
                            { return bHasBorder; }
    void                    SetFrameBorder( bool bBorder )
                            { bHasBorder = bBorder; bHasBorderSet = true; }
    bool                    IsFrameBorderSet() const
                            { return bHasBorderSet; }
    void                    ResetBorder()
                            { bHasBorder = false; bHasBorderSet = false; }

                            // Copy for example for Views
    SfxFrameDescriptor*     Clone( bool bWithIds = true ) const;
};

// No block to implement a =operator
struct SfxFrameProperties
{
    OUString                            aURL;
    OUString                            aName;
    long                                lMarginWidth;
    long                                lMarginHeight;
    long                                lSize;
    long                                lSetSize;
    long                                lFrameSpacing;
    long                                lInheritedFrameSpacing;
    ScrollingMode                       eScroll;
    SizeSelector                        eSizeSelector;
    SizeSelector                        eSetSizeSelector;
    bool                                bHasBorder;
    bool                                bBorderSet;
    bool                                bResizable;
    bool                                bSetResizable;
    bool                                bIsRootSet;
    bool                                bIsInColSet;
    bool                                bHasBorderInherited;
    SfxFrameDescriptor*                 pFrame;

private:
    SfxFrameProperties( SfxFrameProperties& ) {}
public:
                                        SfxFrameProperties()
                                            : lMarginWidth( SIZE_NOT_SET ),
                                              lMarginHeight( SIZE_NOT_SET ),
                                              lSize( 1L ),
                                              lSetSize( 1L ),
                                              lFrameSpacing( SPACING_NOT_SET ),
                                              lInheritedFrameSpacing( SPACING_NOT_SET ),
                                              eScroll( ScrollingAuto ),
                                              eSizeSelector( SIZE_REL ),
                                              eSetSizeSelector( SIZE_REL ),
                                              bHasBorder( true ),
                                              bBorderSet( true ),
                                              bResizable( true ),
                                              bSetResizable( true ),
                                              bIsRootSet( false ),
                                              bIsInColSet( false ),
                                              bHasBorderInherited( true ),
                                              pFrame( nullptr ) {}

                                        ~SfxFrameProperties() { delete pFrame; }

    bool                                operator ==( const SfxFrameProperties& ) const;
    SfxFrameProperties&                 operator =( const SfxFrameProperties &rProp );
};

class SfxFrameDescriptorItem : public SfxPoolItem
{
    SfxFrameProperties                  aProperties;
public:

                                        SfxFrameDescriptorItem ( const sal_uInt16 nId = SID_FRAMEDESCRIPTOR )
                                            : SfxPoolItem( nId )
                                        {}

                                        SfxFrameDescriptorItem( const SfxFrameDescriptorItem& rCpy )
                                            : SfxPoolItem( rCpy )
                                        {
                                            aProperties = rCpy.aProperties;
                                        }

    virtual                             ~SfxFrameDescriptorItem();

    virtual bool                        operator ==( const SfxPoolItem& ) const override;
    SfxFrameDescriptorItem&             operator =( const SfxFrameDescriptorItem & );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                            SfxMapUnit eCoreMetric,
                                            SfxMapUnit ePresMetric,
                                            OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*                Clone( SfxItemPool *pPool = nullptr ) const override;
};

#endif // INCLUDED_SFX2_FRMDESCR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
