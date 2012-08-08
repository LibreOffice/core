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
#ifndef _SVX_SPLTITEM_HXX
#define _SVX_SPLTITEM_HXX

#include <svl/eitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxFmtSplitItem -------------------------------------------------

/*  [Description]

    This item describes a logical variable "split paragraph yes or no".
*/

class EDITENG_DLLPUBLIC SvxFmtSplitItem : public SfxBoolItem
{
public:
    TYPEINFO();
    ~SvxFmtSplitItem();

    inline SvxFmtSplitItem( const sal_Bool bSplit /*= sal_True*/,
                            const sal_uInt16 nWh  );
    inline SvxFmtSplitItem& operator=( const SvxFmtSplitItem& rSplit );

    // "pure virtual Methods" from SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&       Store( SvStream& , sal_uInt16 nItemVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};

inline SvxFmtSplitItem::SvxFmtSplitItem( const sal_Bool bSplit, const sal_uInt16 nWh ) :
    SfxBoolItem( nWh, bSplit )
{}

inline SvxFmtSplitItem& SvxFmtSplitItem::operator=(
    const SvxFmtSplitItem& rSplit )
{
    SetValue( rSplit.GetValue() );
    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
