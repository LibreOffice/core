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


//  SvxColorListItem


class SVX_DLLPUBLIC SvxColorListItem: public SfxPoolItem
{
    XColorListRef     pColorList;

public:
                            TYPEINFO();
                            SvxColorListItem();
                            SvxColorListItem( XColorListRef pTable,
                                    sal_uInt16 nWhich  );
                            SvxColorListItem( const SvxColorListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XColorListRef           GetColorList() const { return pColorList; }
    void                    SetColorList( const XColorListRef &pTable ) { pColorList = pTable; }
};



//  SvxGradientListItem


class SVX_DLLPUBLIC SvxGradientListItem: public SfxPoolItem
{
    XGradientListRef  pGradientList;

public:
                            TYPEINFO();
                            SvxGradientListItem();
                            SvxGradientListItem( XGradientListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxGradientListItem( const SvxGradientListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XGradientListRef        GetGradientList() const { return pGradientList; }
    void                    SetGradientList( XGradientListRef pList ) {
                                    pGradientList = pList; }
};



//  SvxHatchListItem

class SVX_DLLPUBLIC SvxHatchListItem: public SfxPoolItem
{
    XHatchListRef     pHatchList;

public:
                            TYPEINFO();
                            SvxHatchListItem();
                            SvxHatchListItem( XHatchListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxHatchListItem( const SvxHatchListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XHatchListRef           GetHatchList() const { return pHatchList; }
    void                    SetHatchList( XHatchListRef pList ) {
                                    pHatchList = pList; }
};




//  SvxBitmapListItem

class SVX_DLLPUBLIC SvxBitmapListItem: public SfxPoolItem
{
    XBitmapListRef    pBitmapList;

public:
                            TYPEINFO();
                            SvxBitmapListItem();
                            SvxBitmapListItem( XBitmapListRef pBL,
                                    sal_uInt16 nWhich  );
                            SvxBitmapListItem( const SvxBitmapListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XBitmapListRef          GetBitmapList() const { return pBitmapList; }
    void                    SetBitmapList( XBitmapListRef pList ) {
                                    pBitmapList = pList; }
};




//  SvxDashListItem

class SVX_DLLPUBLIC SvxDashListItem: public SfxPoolItem
{
    XDashListRef      pDashList;

public:
                            TYPEINFO();
                            SvxDashListItem();
                            SvxDashListItem( XDashListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxDashListItem( const SvxDashListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XDashListRef            GetDashList() const { return pDashList; }
};



//  SvxLineEndListItem

class SVX_DLLPUBLIC SvxLineEndListItem: public SfxPoolItem
{
    XLineEndListRef   pLineEndList;

public:
                            TYPEINFO();
                            SvxLineEndListItem();
                            SvxLineEndListItem( XLineEndListRef pList,
                                    sal_uInt16 nWhich  );
                            SvxLineEndListItem( const SvxLineEndListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual bool            operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XLineEndListRef         GetLineEndList() const { return pLineEndList; }
    void                    SetLineEndList( XLineEndListRef pList ) {
                                    pLineEndList = pList; }
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
