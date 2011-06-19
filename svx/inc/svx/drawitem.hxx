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

// include ---------------------------------------------------------------

#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/LineDash.hpp>
#include "svx/svxdllapi.h"

//==================================================================
//  SvxColorTableItem
//==================================================================

class XColorTable;

class SVX_DLLPUBLIC SvxColorTableItem: public SfxPoolItem
{
    XColorTable*            pColorTable;

public:
                            TYPEINFO();
                            SvxColorTableItem();
                            SvxColorTableItem( XColorTable* pTable,
                                    sal_uInt16 nWhich  );
                            SvxColorTableItem( const SvxColorTableItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XColorTable*            GetColorTable() const { return pColorTable; }
    void                    SetColorTable( XColorTable* pTable ) {
                                    pColorTable = pTable; }
};


//==================================================================
//  SvxGradientListItem
//==================================================================


class XGradientList;

class SVX_DLLPUBLIC SvxGradientListItem: public SfxPoolItem
{
    XGradientList*              pGradientList;

public:
                            TYPEINFO();
                            SvxGradientListItem();
                            SvxGradientListItem( XGradientList* pList,
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

    XGradientList*          GetGradientList() const { return pGradientList; }
    void                    SetGradientList( XGradientList* pList ) {
                                    pGradientList = pList; }
};



//==================================================================
//  SvxHatchListItem
//==================================================================


class XHatchList;

class SVX_DLLPUBLIC SvxHatchListItem: public SfxPoolItem
{
    XHatchList*             pHatchList;

public:
                            TYPEINFO();
                            SvxHatchListItem();
                            SvxHatchListItem( XHatchList* pList,
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

    XHatchList*             GetHatchList() const { return pHatchList; }
    void                    SetHatchList( XHatchList* pList ) {
                                    pHatchList = pList; }
};



//==================================================================
//  SvxBitmapListItem
//==================================================================


class XBitmapList;

class SVX_DLLPUBLIC SvxBitmapListItem: public SfxPoolItem
{
    XBitmapList*                pBitmapList;

public:
                            TYPEINFO();
                            SvxBitmapListItem();
                            SvxBitmapListItem( XBitmapList* pBL,
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

    XBitmapList*            GetBitmapList() const { return pBitmapList; }
    void                    SetBitmapList( XBitmapList* pList ) {
                                    pBitmapList = pList; }
};



//==================================================================
//  SvxDashListItem
//==================================================================


class XDashList;

class SVX_DLLPUBLIC SvxDashListItem: public SfxPoolItem
{
    XDashList*              pDashList;

public:
                            TYPEINFO();
                            SvxDashListItem();
                            SvxDashListItem( XDashList* pList,
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

    XDashList*              GetDashList() const { return pDashList; }
    void                    SetDashList( XDashList* pList );
};



//==================================================================
//  SvxLineEndListItem
//==================================================================


class XLineEndList;

class SVX_DLLPUBLIC SvxLineEndListItem: public SfxPoolItem
{
    XLineEndList*           pLineEndList;

public:
                            TYPEINFO();
                            SvxLineEndListItem();
                            SvxLineEndListItem( XLineEndList* pList,
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

    XLineEndList*           GetLineEndList() const { return pLineEndList; }
    void                    SetLineEndList( XLineEndList* pList ) {
                                    pLineEndList = pList; }
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
