/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

class XColorList;

class SVX_DLLPUBLIC SvxColorTableItem: public SfxPoolItem
{
    XColorList*         pColorTable;

public:
                            TYPEINFO();
                            SvxColorTableItem();
                            SvxColorTableItem( XColorList* pTable,
                                    sal_uInt16 nWhich  );
                            SvxColorTableItem( const SvxColorTableItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XColorList*         GetColorTable() const { return pColorTable; }
    void                    SetColorTable( XColorList* pTable ) {
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
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

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
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

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
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

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
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

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
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XLineEndList*           GetLineEndList() const { return pLineEndList; }
    void                    SetLineEndList( XLineEndList* pList ) {
                                    pLineEndList = pList; }
};




#endif

