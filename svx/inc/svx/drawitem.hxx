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
#include <boost/shared_ptr.hpp>

//==================================================================
//  SvxColorTableItem
//==================================================================

class XColorList;
typedef ::boost::shared_ptr< XColorList > XColorListSharedPtr;

class SVX_DLLPUBLIC SvxColorTableItem : public SfxPoolItem
{
    XColorListSharedPtr     maColorTable;

public:
                            TYPEINFO();
                            SvxColorTableItem();
                            SvxColorTableItem( XColorListSharedPtr aTable, sal_uInt16 nWhich );
                            SvxColorTableItem( const SvxColorTableItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XColorListSharedPtr GetColorTable() const { return maColorTable; }
    void SetColorTable( XColorListSharedPtr aTable ) { maColorTable = aTable; }
};


//==================================================================
//  SvxGradientListItem
//==================================================================


class XGradientList;
typedef ::boost::shared_ptr< XGradientList > XGradientListSharedPtr;

class SVX_DLLPUBLIC SvxGradientListItem : public SfxPoolItem
{
    XGradientListSharedPtr  maGradientList;

public:
                            TYPEINFO();
                            SvxGradientListItem();
                            SvxGradientListItem( XGradientListSharedPtr aList, sal_uInt16 nWhich  );
                            SvxGradientListItem( const SvxGradientListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XGradientListSharedPtr GetGradientList() const { return maGradientList; }
    void SetGradientList(XGradientListSharedPtr aList ) { maGradientList = aList; }
};



//==================================================================
//  SvxHatchListItem
//==================================================================


class XHatchList;
typedef ::boost::shared_ptr< XHatchList > XHatchListSharedPtr;

class SVX_DLLPUBLIC SvxHatchListItem : public SfxPoolItem
{
    XHatchListSharedPtr     maHatchList;

public:
                            TYPEINFO();
                            SvxHatchListItem();
                            SvxHatchListItem( XHatchListSharedPtr aList, sal_uInt16 nWhich  );
                            SvxHatchListItem( const SvxHatchListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XHatchListSharedPtr GetHatchList() const { return maHatchList; }
    void SetHatchList(XHatchListSharedPtr aList ) { maHatchList = aList; }
};



//==================================================================
//  SvxBitmapListItem
//==================================================================


class XBitmapList;
typedef ::boost::shared_ptr< XBitmapList > XBitmapListSharedPtr;

class SVX_DLLPUBLIC SvxBitmapListItem : public SfxPoolItem
{
    XBitmapListSharedPtr    maBitmapList;

public:
                            TYPEINFO();
                            SvxBitmapListItem();
                            SvxBitmapListItem( XBitmapListSharedPtr aBL, sal_uInt16 nWhich  );
                            SvxBitmapListItem( const SvxBitmapListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XBitmapListSharedPtr GetBitmapList() const { return maBitmapList; }
    void SetBitmapList(XBitmapListSharedPtr aList ) { maBitmapList = aList; }
};



//==================================================================
//  SvxDashListItem
//==================================================================


class XDashList;
typedef ::boost::shared_ptr< XDashList > XDashListSharedPtr;

class SVX_DLLPUBLIC SvxDashListItem : public SfxPoolItem
{
    XDashListSharedPtr      maDashList;

public:
                            TYPEINFO();
                            SvxDashListItem();
                            SvxDashListItem( XDashListSharedPtr aList, sal_uInt16 nWhich  );
                            SvxDashListItem( const SvxDashListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XDashListSharedPtr GetDashList() const { return maDashList; }
    void SetDashList(XDashListSharedPtr aList) { maDashList = aList; }
};



//==================================================================
//  SvxLineEndListItem
//==================================================================


class XLineEndList;
typedef ::boost::shared_ptr< XLineEndList > XLineEndListSharedPtr;

class SVX_DLLPUBLIC SvxLineEndListItem : public SfxPoolItem
{
    XLineEndListSharedPtr   maLineEndList;

public:
                            TYPEINFO();
                            SvxLineEndListItem();
                            SvxLineEndListItem( XLineEndListSharedPtr aList, sal_uInt16 nWhich  );
                            SvxLineEndListItem( const SvxLineEndListItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId );

    XLineEndListSharedPtr GetLineEndList() const { return maLineEndList; }
    void SetLineEndList(XLineEndListSharedPtr aList ) { maLineEndList = aList; }
};




#endif

