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
#include <svx/svxdllapi.h>

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxColorListItem final : public SfxPoolItem
{
    XColorListRef     pColorList;

public:
                            static SfxPoolItem* CreateDefault();

                            DECLARE_ITEM_TYPE_FUNCTION(SvxColorListItem)
                            SvxColorListItem();
                            SvxColorListItem( XColorListRef pTable,
                                    TypedWhichId<SvxColorListItem> nWhich  );
                            SvxColorListItem( const SvxColorListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxColorListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XColorListRef&     GetColorList() const { return pColorList; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxGradientListItem final : public SfxPoolItem
{
    XGradientListRef  pGradientList;

public:
                            static SfxPoolItem* CreateDefault();

                            DECLARE_ITEM_TYPE_FUNCTION(SvxGradientListItem)
                            SvxGradientListItem();
                            SvxGradientListItem( XGradientListRef pList,
                                    TypedWhichId<SvxGradientListItem> nWhich  );
                            SvxGradientListItem( const SvxGradientListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxGradientListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XGradientListRef& GetGradientList() const { return pGradientList; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxHatchListItem final : public SfxPoolItem
{
    XHatchListRef     pHatchList;

public:
                            static SfxPoolItem* CreateDefault();
                            DECLARE_ITEM_TYPE_FUNCTION(SvxHatchListItem)
                            SvxHatchListItem();
                            SvxHatchListItem( XHatchListRef pList,
                                    TypedWhichId<SvxHatchListItem> nWhich  );
                            SvxHatchListItem( const SvxHatchListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxHatchListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XHatchListRef&    GetHatchList() const { return pHatchList; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxBitmapListItem final : public SfxPoolItem
{
    XBitmapListRef    pBitmapList;

public:
                            static SfxPoolItem* CreateDefault();

                            DECLARE_ITEM_TYPE_FUNCTION(SvxBitmapListItem)
                            SvxBitmapListItem();
                            SvxBitmapListItem( XBitmapListRef pBL,
                                    TypedWhichId<SvxBitmapListItem> nWhich  );
                            SvxBitmapListItem( const SvxBitmapListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxBitmapListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XBitmapListRef&   GetBitmapList() const { return pBitmapList; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxPatternListItem final : public SfxPoolItem
{
    XPatternListRef    pPatternList;

public:
                            static SfxPoolItem* CreateDefault();

                            DECLARE_ITEM_TYPE_FUNCTION(SvxPatternListItem)
                            SvxPatternListItem();
                            SvxPatternListItem( XPatternListRef pBL,
                                   TypedWhichId<SvxPatternListItem> nWhich   );
                            SvxPatternListItem( const SvxPatternListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxPatternListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XPatternListRef&  GetPatternList() const { return pPatternList; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxDashListItem final : public SfxPoolItem
{
    XDashListRef      pDashList;

public:
                            static SfxPoolItem* CreateDefault();

                            DECLARE_ITEM_TYPE_FUNCTION(SvxDashListItem)
                            SvxDashListItem();
                            SvxDashListItem( XDashListRef pList,
                                    TypedWhichId<SvxDashListItem> nWhich  );
                            SvxDashListItem( const SvxDashListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxDashListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XDashListRef&     GetDashList() const { return pDashList; }
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC SvxLineEndListItem final : public SfxPoolItem
{
    XLineEndListRef   pLineEndList;

public:
                            static SfxPoolItem* CreateDefault();

                            DECLARE_ITEM_TYPE_FUNCTION(SvxLineEndListItem)
                            SvxLineEndListItem();
                            SvxLineEndListItem( XLineEndListRef pList,
                                    TypedWhichId<SvxLineEndListItem> nWhich  );
                            SvxLineEndListItem( const SvxLineEndListItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SvxLineEndListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const XLineEndListRef&  GetLineEndList() const { return pLineEndList; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
