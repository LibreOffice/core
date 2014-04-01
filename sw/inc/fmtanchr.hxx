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
#include <hintids.hxx>
#include <swtypes.hxx>
#include <format.hxx>
#include <svl/poolitem.hxx>

#include <boost/scoped_ptr.hpp>

struct SwPosition;
class IntlWrapper;

/// FlyAnchors
class SW_DLLPUBLIC SwFmtAnchor: public SfxPoolItem
{
    ::boost::scoped_ptr<SwPosition> m_pCntntAnchor; /**< 0 for page-bound frames.
                                                     Index for paragraph-bound frames.
                                                     Position for character-bound frames. */
    RndStdIds  nAnchorId;
    sal_uInt16     nPageNum;        ///< Page number for page-bound frames.

    /// #i28701# - getting anchor positions ordered
    sal_uInt32 mnOrder;
    static sal_uInt32 mnOrderCounter;

public:
    SwFmtAnchor( RndStdIds eRnd = FLY_AT_PAGE, sal_uInt16 nPageNum = 0 );
    SwFmtAnchor( const SwFmtAnchor &rCpy );
    virtual ~SwFmtAnchor();

    SwFmtAnchor &operator=( const SwFmtAnchor& );

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    RndStdIds GetAnchorId() const { return nAnchorId; }
    sal_uInt16 GetPageNum() const { return nPageNum; }
    const SwPosition *GetCntntAnchor() const { return m_pCntntAnchor.get(); }
    // #i28701#
    sal_uInt32 GetOrder() const;

    void SetType( RndStdIds nRndId ) { nAnchorId = nRndId; }
    void SetPageNum( sal_uInt16 nNew ) { nPageNum = nNew; }
    void SetAnchor( const SwPosition *pPos );
};

inline const SwFmtAnchor &SwAttrSet::GetAnchor(sal_Bool bInP) const
    { return static_cast<const SwFmtAnchor&>(Get(RES_ANCHOR, bInP)); }

 inline const SwFmtAnchor &SwFmt::GetAnchor(sal_Bool bInP) const
     { return aSet.GetAnchor(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
