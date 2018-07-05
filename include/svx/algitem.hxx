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
#ifndef INCLUDED_SVX_ALGITEM_HXX
#define INCLUDED_SVX_ALGITEM_HXX

#include <com/sun/star/uno/Any.hxx>
#include <editeng/svxenum.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>

class IntlWrapper;
class SfxItemPool;
class SvStream;

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxOrientationItem: public SfxEnumItem<SvxCellOrientation>
{
public:
    SvxOrientationItem(
        const SvxCellOrientation eOrientation,
        const sal_uInt16 nId );

    SvxOrientationItem(
        sal_Int32 nRotation, bool bStacked,
        const sal_uInt16 nId );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual sal_uInt16      GetValueCount() const override;
    static OUString         GetValueText( SvxCellOrientation nVal );
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const override;

    SvxOrientationItem& operator=(const SvxOrientationItem& rOrientation)
            {
                SetValue( rOrientation.GetValue() );
                return *this;
            }
    /*TODO: SfxPoolItem copy function dichotomy*/SvxOrientationItem(SvxOrientationItem const &) = default;

    /** Returns sal_True, if the item represents STACKED state. */
    bool                    IsStacked() const;
    /** Returns the rotation this item represents (returns nStdAngle for STANDARD and STACKED state). */
    sal_Int32               GetRotation( sal_Int32 nStdAngle ) const;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC SvxMarginItem: public SfxPoolItem
{
    sal_Int16       nLeftMargin;
    sal_Int16       nTopMargin;
    sal_Int16       nRightMargin;
    sal_Int16       nBottomMargin;
public:
    static SfxPoolItem* CreateDefault();
    SvxMarginItem( const sal_uInt16 nId  );
    SvxMarginItem( sal_Int16 nLeft, sal_Int16 nTop /*= 0*/,
                   sal_Int16 nRight /*= 0*/, sal_Int16 nBottom /*= 0*/,
                   const sal_uInt16 nId  );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const override;
    virtual SvStream&        Store( SvStream&, sal_uInt16 nItemVersion ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

            sal_Int16       GetLeftMargin() const {return nLeftMargin; }
            void            SetLeftMargin(sal_Int16 nLeft);
            sal_Int16       GetTopMargin() const {return nTopMargin; }
            void            SetTopMargin(sal_Int16 nTop);
            sal_Int16       GetRightMargin() const {return nRightMargin; }
            void            SetRightMargin(sal_Int16 nRight);
            sal_Int16       GetBottomMargin() const {return nBottomMargin; }
            void            SetBottomMargin(sal_Int16 nBottom);

    SvxMarginItem& operator=(const SvxMarginItem& rMargin)
            {
                nLeftMargin = rMargin.nLeftMargin;
                nTopMargin = rMargin.nTopMargin;
                nRightMargin = rMargin.nRightMargin;
                nBottomMargin = rMargin.nBottomMargin;
                return *this;
            }
    /*TODO: SfxPoolItem copy function dichotomy*/SvxMarginItem(SvxMarginItem const &) = default;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
