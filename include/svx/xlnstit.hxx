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

#ifndef _SVX_XLNSTIT_HXX
#define _SVX_XLNSTIT_HXX

#include <svx/xit.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/polygon/b2dpolypolygon.hxx>

class SdrModel;

//------------------------
// class XLineStartItem
//------------------------
class SVX_DLLPUBLIC XLineStartItem : public NameOrIndex
{
    basegfx::B2DPolyPolygon     maPolyPolygon;

public:
            TYPEINFO();
            XLineStartItem(sal_Int32 nIndex = -1);
            XLineStartItem(const OUString& rName, const basegfx::B2DPolyPolygon& rPolyPolygon);
            XLineStartItem(SfxItemPool* pPool, const basegfx::B2DPolyPolygon& rPolyPolygon);
            XLineStartItem(const XLineStartItem& rItem);
            XLineStartItem(SvStream& rIn);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    basegfx::B2DPolyPolygon GetLineStartValue() const;
    void SetLineStartValue(const basegfx::B2DPolyPolygon& rPolyPolygon) { maPolyPolygon = rPolyPolygon; Detach(); }

    XLineStartItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
