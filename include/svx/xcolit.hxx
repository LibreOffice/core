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

#ifndef _SVX_XCOLORITEM_HXX
#define _SVX_XCOLORITEM_HXX

#include <tools/color.hxx>
#include "svx/svxdllapi.h"

#include <svx/xit.hxx>

class XColorList;

//-----------------
// class XColorItem
//-----------------
class SVX_DLLPUBLIC XColorItem : public NameOrIndex
{
    Color   aColor;

public:
            TYPEINFO();
            XColorItem() {}
            XColorItem(sal_uInt16 nWhich, sal_Int32 nIndex, const Color& rTheColor);

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

            XColorItem(sal_uInt16 nWhich, const Color& rTheColor);
            XColorItem(sal_uInt16 nWhich, const OUString& rName, const Color& rTheColor);
            XColorItem(sal_uInt16 nWhich, SvStream& rIn);
            XColorItem(const XColorItem& rItem);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const;

    const Color&    GetColorValue(const XColorList* pTable = 0) const;
    void            SetColorValue(const Color& rNew) { aColor = rNew; Detach(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
