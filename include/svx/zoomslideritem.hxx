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
#ifndef INCLUDED_SVX_ZOOMSLIDERITEM_HXX
#define INCLUDED_SVX_ZOOMSLIDERITEM_HXX

#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <com/sun/star/uno/Sequence.hxx>
#include <svx/svxdllapi.h>



class SVX_DLLPUBLIC SvxZoomSliderItem: public SfxUInt16Item
{
    com::sun::star::uno::Sequence < sal_Int32 > maValues;
    sal_uInt16 mnMinZoom;
    sal_uInt16 mnMaxZoom;

public:
    TYPEINFO_OVERRIDE();

    SvxZoomSliderItem( sal_uInt16 nCurrentZoom = 100, sal_uInt16 nMinZoom = 20, sal_uInt16 nMaxZoom = 600, sal_uInt16 nWhich = SID_ATTR_ZOOMSLIDER );
    SvxZoomSliderItem( const SvxZoomSliderItem& );
    virtual ~SvxZoomSliderItem();

    void AddSnappingPoint( sal_Int32 nNew );
    const com::sun::star::uno::Sequence < sal_Int32 >& GetSnappingPoints() const { return maValues;}
    sal_uInt16 GetMinZoom() const {return mnMinZoom;}
    sal_uInt16 GetMaxZoom() const {return mnMaxZoom;}

    // "pure virtual Methoden" vom SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*    Create( SvStream& rStrm, sal_uInt16 nVersion ) const override;                       // leer
    virtual SvStream&       Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const override;                   // leer
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
