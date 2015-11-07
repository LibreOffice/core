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
#ifndef INCLUDED_SVX_DRAWITEM_HXX
#define INCLUDED_SVX_DRAWITEM_HXX

#include <svl/poolitem.hxx>
#include <svx/xtable.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/LineDash.hpp>
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxColorListItem: public SfxPoolItem
{
    XColorListRef     pColorList;

public:
                            static SfxPoolItem* CreateDefault();

                            SvxColorListItem();
                            SvxColorListItem( XColorListRef pTable,
                                    sal_uInt16 nWhich  );
                            SvxColorListItem( const SvxColorListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    XColorListRef           GetColorList() const { return pColorList; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxGradientListItem : public SfxPoolItem
{
    XGradientListRef  pGradientList;

public:
                            static SfxPoolItem* CreateDefault();

                            SvxGradientListItem();
                            SvxGradientListItem( XGradientListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxGradientListItem( const SvxGradientListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    XGradientListRef        GetGradientList() const { return pGradientList; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxHatchListItem : public SfxPoolItem
{
    XHatchListRef     pHatchList;

public:
                            static SfxPoolItem* CreateDefault();
                            SvxHatchListItem();
                            SvxHatchListItem( XHatchListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxHatchListItem( const SvxHatchListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    XHatchListRef           GetHatchList() const { return pHatchList; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxBitmapListItem : public SfxPoolItem
{
    XBitmapListRef    pBitmapList;

public:
                            static SfxPoolItem* CreateDefault();

                            SvxBitmapListItem();
                            SvxBitmapListItem( XBitmapListRef pBL,
                                    sal_uInt16 nWhich  );
                            SvxBitmapListItem( const SvxBitmapListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    XBitmapListRef          GetBitmapList() const { return pBitmapList; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxDashListItem : public SfxPoolItem
{
    XDashListRef      pDashList;

public:
                            static SfxPoolItem* CreateDefault();

                            SvxDashListItem();
                            SvxDashListItem( XDashListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxDashListItem( const SvxDashListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    XDashListRef            GetDashList() const { return pDashList; }
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED SvxLineEndListItem : public SfxPoolItem
{
    XLineEndListRef   pLineEndList;

public:
                            static SfxPoolItem* CreateDefault();

                            SvxLineEndListItem();
                            SvxLineEndListItem( XLineEndListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxLineEndListItem( const SvxLineEndListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    XLineEndListRef         GetLineEndList() const { return pLineEndList; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
