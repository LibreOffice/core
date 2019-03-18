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
#ifndef INCLUDED_SW_INC_FMTCNCT_HXX
#define INCLUDED_SW_INC_FMTCNCT_HXX

#include "hintids.hxx"
#include <svl/poolitem.hxx>
#include "format.hxx"
#include "calbck.hxx"
#include "frmfmt.hxx"

class IntlWrapper;

/// Connection (text flow) between two FlyFrames.
class SW_DLLPUBLIC SwFormatChain: public SfxPoolItem
{
    SwClient m_aPrev, ///< Previous SwFlyFrameFormat (if existent).
             m_aNext; ///< Next SwFlyFrameFormat (if existent).

public:
    SwFormatChain() : SfxPoolItem( RES_CHAIN ) {}
    SwFormatChain( const SwFormatChain &rCpy );

    inline SwFormatChain &operator=( const SwFormatChain& );

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;

    SwFlyFrameFormat* GetPrev() const { return const_cast<SwFlyFrameFormat*>(static_cast<const SwFlyFrameFormat*>(m_aPrev.GetRegisteredIn())); }
    SwFlyFrameFormat* GetNext() const { return const_cast<SwFlyFrameFormat*>(static_cast<const SwFlyFrameFormat*>(m_aNext.GetRegisteredIn())); }

    void SetPrev( SwFlyFrameFormat *pFormat );
    void SetNext( SwFlyFrameFormat *pFormat );
};

SwFormatChain &SwFormatChain::operator=( const SwFormatChain &rCpy )
{
    SetPrev( rCpy.GetPrev() );
    SetNext( rCpy.GetNext() );
    return *this;
}

inline const SwFormatChain &SwAttrSet::GetChain(bool bInP) const
    { return Get( RES_CHAIN,bInP); }

inline const SwFormatChain &SwFormat::GetChain(bool bInP) const
    { return m_aSet.GetChain(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
