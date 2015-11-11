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

#ifndef INCLUDED_SVX_XFLGRIT_HXX
#define INCLUDED_SVX_XFLGRIT_HXX

#include <svx/xit.hxx>
#include <svx/xgrad.hxx>
#include <svx/svxdllapi.h>

class SdrModel;


// class XFillGradientItem

class SVX_DLLPUBLIC XFillGradientItem : public NameOrIndex
{
    XGradient   aGradient;

public:
            static SfxPoolItem* CreateDefault();
            XFillGradientItem() : NameOrIndex(XATTR_FILLGRADIENT, -1) {}
            XFillGradientItem(sal_uInt16 nWhich_) : NameOrIndex(nWhich_, -1) {}
            XFillGradientItem(sal_Int32 nIndex, const XGradient& rTheGradient);
            XFillGradientItem(const OUString& rName, const XGradient& rTheGradient, sal_uInt16 nWhich = XATTR_FILLGRADIENT);
            XFillGradientItem(const XGradient& rTheGradient);
            XFillGradientItem(const XFillGradientItem& rItem);
            XFillGradientItem(SvStream& rIn, sal_uInt16 nVer = 0);

    virtual bool            operator==(const SfxPoolItem& rItem) const override;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const override;
    virtual SvStream&       Store(SvStream& rOut, sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16      GetVersion( sal_uInt16 nFileFormatVersion ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;
    const XGradient&        GetGradientValue() const; // GetValue -> GetGradientValue
    void                    SetGradientValue(const XGradient& rNew) { aGradient = rNew; Detach(); } // SetValue -> SetGradientValue

    static bool CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 );
    XFillGradientItem* checkForUniqueItem( SdrModel* pModel ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
