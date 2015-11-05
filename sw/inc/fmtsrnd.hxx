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
#ifndef INCLUDED_SW_INC_FMTSRND_HXX
#define INCLUDED_SW_INC_FMTSRND_HXX

#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <svl/eitem.hxx>

#include <fmtsrndenum.hxx>
class IntlWrapper;

// SwFormatSurround: How document content under the frame shall behave.
class SW_DLLPUBLIC SwFormatSurround: public SfxEnumItem
{
    bool    bAnchorOnly :1;
    bool    bContour    :1;
    bool    bOutside    :1;
public:
    SwFormatSurround( SwSurround eNew = SURROUND_PARALLEL );
    SwFormatSurround( const SwFormatSurround & );
    inline SwFormatSurround &operator=( const SwFormatSurround &rCpy );

    // "Pure virtual Methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;
    virtual sal_uInt16          GetValueCount() const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    SwSurround GetSurround()const { return SwSurround( GetValue() ); }
    bool    IsAnchorOnly()  const { return bAnchorOnly; }
    bool    IsContour()     const { return bContour; }
    bool    IsOutside()     const { return bOutside; }
    void    SetSurround  ( SwSurround eNew ){ SfxEnumItem::SetValue( sal_uInt16( eNew ) ); }
    void    SetAnchorOnly( bool bNew )      { bAnchorOnly = bNew; }
    void    SetContour( bool bNew )         { bContour = bNew; }
    void    SetOutside( bool bNew )         { bOutside = bNew; }

    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

inline SwFormatSurround &SwFormatSurround::operator=( const SwFormatSurround &rCpy )
{
    bAnchorOnly = rCpy.IsAnchorOnly();
    bContour = rCpy.IsContour();
    bOutside = rCpy.IsOutside();
    SfxEnumItem::SetValue( rCpy.GetValue() );
    return *this;
}

inline const SwFormatSurround &SwAttrSet::GetSurround(bool bInP) const
    { return static_cast<const SwFormatSurround&>(Get( RES_SURROUND,bInP)); }

inline const SwFormatSurround &SwFormat::GetSurround(bool bInP) const
    { return m_aSet.GetSurround(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
