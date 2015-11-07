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
#ifndef INCLUDED_SW_INC_GRFATR_HXX
#define INCLUDED_SW_INC_GRFATR_HXX

#include <hintids.hxx>
#include <tools/gen.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/grfcrop.hxx>
#include "swdllapi.h"
#include <swatrset.hxx>
#include <format.hxx>

enum MirrorGraph
{
RES_MIRROR_GRAPH_BEGIN,
    RES_MIRROR_GRAPH_DONT = RES_MIRROR_GRAPH_BEGIN,
    RES_MIRROR_GRAPH_VERT,
    RES_MIRROR_GRAPH_HOR,
    RES_MIRROR_GRAPH_BOTH,
RES_MIRROR_GRAPH_END
};

class SW_DLLPUBLIC SwMirrorGrf : public SfxEnumItem
{
    bool bGrfToggle; // Flip graphics on even pages.

public:
    SwMirrorGrf( MirrorGraph eMiro = RES_MIRROR_GRAPH_DONT )
        : SfxEnumItem( RES_GRFATR_MIRRORGRF, static_cast< sal_uInt16 >(eMiro) ), bGrfToggle( false )
    {}
    SwMirrorGrf( const SwMirrorGrf &rMirrorGrf )
        : SfxEnumItem( RES_GRFATR_MIRRORGRF, rMirrorGrf.GetValue()),
        bGrfToggle( rMirrorGrf.IsGrfToggle() )
    {}

    // pure virtual methods of SfxPoolItem
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    // pure virtual methods of SfxEnumItem
    virtual sal_uInt16          GetValueCount() const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = nullptr ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) override;

    inline SwMirrorGrf& operator=( const SwMirrorGrf& rMirrorGrf )
        {
            SfxEnumItem::SetValue( rMirrorGrf.GetValue() );
            bGrfToggle = rMirrorGrf.IsGrfToggle();
            return *this;
        }

    inline bool IsGrfToggle() const         { return bGrfToggle; }
    inline void SetGrfToggle( bool bNew )   { bGrfToggle = bNew; }
};

class SW_DLLPUBLIC SwCropGrf : public SvxGrfCrop
{
public:
    SwCropGrf();
    SwCropGrf(  sal_Int32 nLeft,    sal_Int32 nRight,
                sal_Int32 nTop,     sal_Int32 nBottom );

    // "pure virtual methods" of SfxPoolItem
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = nullptr ) const override;
};

class SwRotationGrf : public SfxUInt16Item
{
    Size aUnrotatedSize;
public:
    SwRotationGrf( sal_Int16 nVal = 0 )
        : SfxUInt16Item( RES_GRFATR_ROTATION, nVal )
    {}
    SwRotationGrf( sal_Int16 nVal, const Size& rSz )
        : SfxUInt16Item( RES_GRFATR_ROTATION, nVal ), aUnrotatedSize( rSz )
    {}

    // pure virtual methods from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal,
                                            sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal,
                                            sal_uInt8 nMemberId ) override;

    const Size& GetUnrotatedSize() const            { return aUnrotatedSize; }
};

class SW_DLLPUBLIC SwLuminanceGrf : public SfxInt16Item
{
public:
    SwLuminanceGrf( sal_Int16 nVal = 0 )
        : SfxInt16Item( RES_GRFATR_LUMINANCE, nVal )
    {}

    // pure virtual methods from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;
};

class SW_DLLPUBLIC SwContrastGrf : public SfxInt16Item
{
public:
    SwContrastGrf( sal_Int16 nVal = 0 )
        : SfxInt16Item( RES_GRFATR_CONTRAST, nVal )
    {}

    // pure virtual methods from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;
};

class SwChannelGrf : public SfxInt16Item
{
protected:
    SwChannelGrf( sal_Int16 nVal, sal_uInt16 nWhichL )
        : SfxInt16Item( nWhichL, nVal )
    {}

public:
    // pure virtual methods from SfxInt16Item
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;
};

class SwChannelRGrf : public SwChannelGrf
{
public:
    SwChannelRGrf( sal_Int16 nVal = 0 )
        : SwChannelGrf( nVal, RES_GRFATR_CHANNELR )
    {}
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};
class SwChannelGGrf : public SwChannelGrf
{
public:
    SwChannelGGrf( sal_Int16 nVal = 0 )
        : SwChannelGrf( nVal, RES_GRFATR_CHANNELG )
    {}
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};
class SwChannelBGrf : public SwChannelGrf
{
public:
    SwChannelBGrf( sal_Int16 nVal = 0 )
        : SwChannelGrf( nVal, RES_GRFATR_CHANNELB )
    {}
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
};

class SW_DLLPUBLIC SwGammaGrf : public SfxPoolItem
{
    double nValue;
public:
    SwGammaGrf() : SfxPoolItem( RES_GRFATR_GAMMA ), nValue( 1.0 )
    {}

    SwGammaGrf( const double& rVal )
        : SfxPoolItem( RES_GRFATR_GAMMA ), nValue( rVal )
    {}

    inline SwGammaGrf& operator=( const SwGammaGrf& rCopy )
        {
            SetValue( rCopy.GetValue() );
            return *this;
        }

    // pure virtual methods from SfxEnumItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal,
                                            sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal,
                                            sal_uInt8 nMemberId ) override;

    const double& GetValue() const              { return nValue; }
    void SetValue( const double& rVal )         { nValue = rVal; }
};

class SwInvertGrf: public SfxBoolItem
{
public:
    SwInvertGrf( bool bVal = false )
        : SfxBoolItem( RES_GRFATR_INVERT, bVal )
    {}

    // pure virtual methods from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;
};

class SwTransparencyGrf : public SfxByteItem
{
public:
    SwTransparencyGrf( sal_Int8 nVal = 0 )
        : SfxByteItem( RES_GRFATR_TRANSPARENCY, nVal )
    {}

    // pure virtual methods from SfxInt16Item
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) override;
};

class SW_DLLPUBLIC SwDrawModeGrf : public SfxEnumItem
{
public:
    SwDrawModeGrf( sal_uInt16 nMode = 0 )
        : SfxEnumItem( RES_GRFATR_DRAWMODE, nMode )
    {}

    // pure virtual methods of SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;

    // pure virtual methods of SfxEnumItem
    virtual sal_uInt16          GetValueCount() const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = nullptr ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal,
                                        sal_uInt8 nMemberId ) override;
};

// Implementation of graphics attributes methods of SwAttr
inline const SwMirrorGrf &SwAttrSet::GetMirrorGrf(bool bInP) const
    { return static_cast<const SwMirrorGrf&>(Get( RES_GRFATR_MIRRORGRF,bInP)); }
inline const SwCropGrf   &SwAttrSet::GetCropGrf(bool bInP) const
    { return static_cast<const SwCropGrf&>(Get( RES_GRFATR_CROPGRF,bInP)); }
inline const SwRotationGrf &SwAttrSet::GetRotationGrf(bool bInP) const
    { return static_cast<const SwRotationGrf&>(Get( RES_GRFATR_ROTATION,bInP)); }
inline const SwLuminanceGrf &SwAttrSet::GetLuminanceGrf(bool bInP) const
    { return static_cast<const SwLuminanceGrf&>(Get( RES_GRFATR_LUMINANCE,bInP)); }
inline const SwContrastGrf &SwAttrSet::GetContrastGrf(bool bInP) const
    { return static_cast<const SwContrastGrf&>(Get( RES_GRFATR_CONTRAST,bInP)); }
inline const SwChannelRGrf &SwAttrSet::GetChannelRGrf(bool bInP) const
    { return static_cast<const SwChannelRGrf&>(Get( RES_GRFATR_CHANNELR,bInP)); }
inline const SwChannelGGrf &SwAttrSet::GetChannelGGrf(bool bInP) const
    { return static_cast<const SwChannelGGrf&>(Get( RES_GRFATR_CHANNELG,bInP)); }
inline const SwChannelBGrf &SwAttrSet::GetChannelBGrf(bool bInP) const
    { return static_cast<const SwChannelBGrf&>(Get( RES_GRFATR_CHANNELB,bInP)); }
inline const SwGammaGrf &SwAttrSet::GetGammaGrf(bool bInP) const
    { return static_cast<const SwGammaGrf&>(Get( RES_GRFATR_GAMMA,bInP)); }
inline const SwInvertGrf &SwAttrSet::GetInvertGrf(bool bInP) const
    { return static_cast<const SwInvertGrf&>(Get( RES_GRFATR_INVERT,bInP)); }
inline const SwTransparencyGrf &SwAttrSet::GetTransparencyGrf(bool bInP) const
    { return static_cast<const SwTransparencyGrf&>(Get( RES_GRFATR_TRANSPARENCY,bInP)); }
inline const SwDrawModeGrf      &SwAttrSet::GetDrawModeGrf(bool bInP) const
    { return static_cast<const SwDrawModeGrf&>(Get( RES_GRFATR_DRAWMODE,bInP)); }

#endif // INCLUDED_SW_INC_GRFATR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
