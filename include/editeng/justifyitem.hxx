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

#ifndef INCLUDED_EDITENG_JUSTIFYITEM_HXX
#define INCLUDED_EDITENG_JUSTIFYITEM_HXX

#include <editeng/editengdllapi.h>
#include <editeng/svxenum.hxx>
#include <svl/eitem.hxx>
#include <sal/types.h>

class EDITENG_DLLPUBLIC SvxHorJustifyItem: public SfxEnumItem<SvxCellHorJustify>
{
public:
    static SfxPoolItem* CreateDefault();

    explicit SvxHorJustifyItem( const sal_uInt16 nId );

    SvxHorJustifyItem(
        const SvxCellHorJustify eJustify /*= SvxCellHorJustify::Standard*/,
        const sal_uInt16 nId );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual sal_uInt16       GetValueCount() const override;
    static OUString          GetValueText( sal_uInt16 nVal );
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const override;

    SvxHorJustifyItem& operator=(const SvxHorJustifyItem& rHorJustify)
            {
                SetValue( rHorJustify.GetValue() );
                return *this;
            }
    /*TODO: SfxPoolItem copy function dichotomy*/SvxHorJustifyItem(SvxHorJustifyItem const &) = default;
};


class EDITENG_DLLPUBLIC SvxVerJustifyItem: public SfxEnumItem<SvxCellVerJustify>
{
public:
    static SfxPoolItem* CreateDefault();

    explicit SvxVerJustifyItem( const sal_uInt16 nId  );

    SvxVerJustifyItem(
        const SvxCellVerJustify eJustify,
        const sal_uInt16 nId  );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual sal_uInt16       GetValueCount() const override;
    static OUString          GetValueText( SvxCellVerJustify nVal );
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const override;

    SvxVerJustifyItem& operator=(const SvxVerJustifyItem& rVerJustify)
            {
                SetValue( rVerJustify.GetValue() );
                return *this;
            }
    /*TODO: SfxPoolItem copy function dichotomy*/SvxVerJustifyItem(SvxVerJustifyItem const &) = default;
};


class EDITENG_DLLPUBLIC SvxJustifyMethodItem: public SfxEnumItem<SvxCellJustifyMethod>
{
public:
    SvxJustifyMethodItem(
        const SvxCellJustifyMethod eMethod,
        const sal_uInt16 nId );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual sal_uInt16       GetValueCount() const override;
    static OUString          GetValueText( SvxCellJustifyMethod nVal );
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
