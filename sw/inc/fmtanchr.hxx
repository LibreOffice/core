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
#ifndef INCLUDED_SW_INC_FMTANCHR_HXX
#define INCLUDED_SW_INC_FMTANCHR_HXX

#include "swdllapi.h"
#include "hintids.hxx"
#include "format.hxx"
#include <svl/poolitem.hxx>
#include <svx/swframetypes.hxx>

#include <memory>

struct SwPosition;
class IntlWrapper;

/// FlyAnchors
class SW_DLLPUBLIC SwFormatAnchor: public SfxPoolItem
{
    std::unique_ptr<SwPosition> m_pContentAnchor; /**< 0 for page-bound frames.
                                                     Index for paragraph-bound frames.
                                                     Position for character-bound frames. */
    RndStdIds  m_eAnchorId;
    sal_uInt16     m_nPageNumber;        ///< Page number for page-bound frames.

    /// #i28701# - getting anchor positions ordered
    sal_uInt32 m_nOrder;
    static sal_uInt32 m_nOrderCounter;

public:
    SwFormatAnchor( RndStdIds eRnd = RndStdIds::FLY_AT_PAGE, sal_uInt16 nPageNum = 0 );
    SwFormatAnchor( const SwFormatAnchor &rCpy );
    virtual ~SwFormatAnchor() override;

    SwFormatAnchor &operator=( const SwFormatAnchor& );

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper&    rIntl ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    RndStdIds GetAnchorId() const { return m_eAnchorId; }
    sal_uInt16 GetPageNum() const { return m_nPageNumber; }
    const SwPosition *GetContentAnchor() const { return m_pContentAnchor.get(); }
    // #i28701#
    sal_uInt32 GetOrder() const { return m_nOrder;}

    void SetType( RndStdIds nRndId ) { m_eAnchorId = nRndId; }
    void SetPageNum( sal_uInt16 nNew ) { m_nPageNumber = nNew; }
    void SetAnchor( const SwPosition *pPos );

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatAnchor &SwAttrSet::GetAnchor(bool bInP) const
    { return Get(RES_ANCHOR, bInP); }

 inline const SwFormatAnchor &SwFormat::GetAnchor(bool bInP) const
     { return m_aSet.GetAnchor(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
