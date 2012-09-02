/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _FMTANCHR_HXX
#define _FMTANCHR_HXX

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
    ~SwFmtAnchor();

    SwFmtAnchor &operator=( const SwFmtAnchor& );

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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
