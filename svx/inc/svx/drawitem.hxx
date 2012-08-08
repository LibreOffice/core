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
#ifndef _SVX_DRAWITEM_HXX
#define _SVX_DRAWITEM_HXX

#include <svl/poolitem.hxx>
#include <svx/xtable.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/LineDash.hpp>
#include "svx/svxdllapi.h"

//==================================================================
//  SvxColorListItem
//==================================================================

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
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XColorListRef           GetColorList() const { return pColorList; }
    void                    SetColorList( const XColorListRef &pTable ) { pColorList = pTable; }
};


//==================================================================
//  SvxGradientListItem
//==================================================================

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
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XGradientListRef        GetGradientList() const { return pGradientList; }
    void                    SetGradientList( XGradientListRef pList ) {
                                    pGradientList = pList; }
};


//==================================================================
//  SvxHatchListItem
//==================================================================
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
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XHatchListRef           GetHatchList() const { return pHatchList; }
    void                    SetHatchList( XHatchListRef pList ) {
                                    pHatchList = pList; }
};



//==================================================================
//  SvxBitmapListItem
//==================================================================
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
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XBitmapListRef          GetBitmapList() const { return pBitmapList; }
    void                    SetBitmapList( XBitmapListRef pList ) {
                                    pBitmapList = pList; }
};



//==================================================================
//  SvxDashListItem
//==================================================================
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
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XDashListRef            GetDashList() const { return pDashList; }
};


//==================================================================
//  SvxLineEndListItem
//==================================================================
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
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XLineEndListRef         GetLineEndList() const { return pLineEndList; }
    void                    SetLineEndList( XLineEndListRef pList ) {
                                    pLineEndList = pList; }
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
