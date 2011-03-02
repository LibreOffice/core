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

#ifndef _SDASAITM_HXX
#define _SDASAITM_HXX

#include <vector>

#include <svl/poolitem.hxx>

class SdrCustomShapeAdjustmentValue
{
    sal_uInt32  nValue;

    friend class SdrCustomShapeAdjustmentItem;

    public :

        void        SetValue( sal_Int32 nVal ) { nValue = nVal; };
        sal_Int32   GetValue() const { return nValue; };

};

class SdrCustomShapeAdjustmentItem : public SfxPoolItem
{
            std::vector<SdrCustomShapeAdjustmentValue>  aAdjustmentValueList;

    public :

            TYPEINFO();
            SVX_DLLPUBLIC SdrCustomShapeAdjustmentItem();
            SdrCustomShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion );
            SVX_DLLPUBLIC ~SdrCustomShapeAdjustmentItem();

            virtual int                 operator==( const SfxPoolItem& ) const;
            virtual SfxItemPresentation GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                                String &rText, const IntlWrapper * = 0) const;
            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const;
            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

            virtual bool QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
            virtual bool PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

            sal_uInt32                          GetCount() const { return aAdjustmentValueList.size(); };
            SVX_DLLPUBLIC const SdrCustomShapeAdjustmentValue&  GetValue( sal_uInt32 nIndex ) const;
            SVX_DLLPUBLIC void                              SetValue( sal_uInt32 nIndex,
                                                        const SdrCustomShapeAdjustmentValue& rVal );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
