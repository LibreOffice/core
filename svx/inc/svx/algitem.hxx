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
#ifndef _SVX_ALGITEM_HXX
#define _SVX_ALGITEM_HXX

#include <svx/svxids.hrc>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include "svx/svxdllapi.h"

class SvStream;

//------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxOrientationItem: public SfxEnumItem
{
public:
    TYPEINFO();

    SvxOrientationItem(
        const SvxCellOrientation eOrientation /*= SVX_ORIENTATION_STANDARD*/,
        const sal_uInt16 nId );

    SvxOrientationItem(
        sal_Int32 nRotation, sal_Bool bStacked,
        const sal_uInt16 nId );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline  SvxOrientationItem& operator=(const SvxOrientationItem& rOrientation)
            {
                SetValue( rOrientation.GetValue() );
                return *this;
            }

    /** Returns sal_True, if the item represents STACKED state. */
    sal_Bool                    IsStacked() const;
    /** Returns the rotation this item represents (returns nStdAngle for STANDARD and STACKED state). */
    sal_Int32                   GetRotation( sal_Int32 nStdAngle = 0 ) const;
    /** Fills this item according to passed item values. */
    void                    SetFromRotation( sal_Int32 nRotation, sal_Bool bStacked );
};

//------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxMarginItem: public SfxPoolItem
{
    sal_Int16       nLeftMargin;
    sal_Int16       nTopMargin;
    sal_Int16       nRightMargin;
    sal_Int16       nBottomMargin;
public:
    TYPEINFO();
    SvxMarginItem( const sal_uInt16 nId  );
    SvxMarginItem( sal_Int16 nLeft, sal_Int16 nTop /*= 0*/,
                   sal_Int16 nRight /*= 0*/, sal_Int16 nBottom /*= 0*/,
                   const sal_uInt16 nId  );
    SvxMarginItem( const SvxMarginItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const;
    virtual SvStream&        Store( SvStream&, sal_uInt16 nItemVersion ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

            sal_Int16           GetLeftMargin() const {return nLeftMargin; }
            sal_Bool            SetLeftMargin(sal_Int16 nLeft);
            sal_Int16           GetTopMargin() const {return nTopMargin; }
            sal_Bool            SetTopMargin(sal_Int16 nTop);
            sal_Int16           GetRightMargin() const {return nRightMargin; }
            sal_Bool            SetRightMargin(sal_Int16 nRight);
            sal_Int16           GetBottomMargin() const {return nBottomMargin; }
            sal_Bool            SetBottomMargin(sal_Int16 nBottom);

    inline  SvxMarginItem& operator=(const SvxMarginItem& rMargin)
            {
                nLeftMargin = rMargin.nLeftMargin;
                nTopMargin = rMargin.nTopMargin;
                nRightMargin = rMargin.nRightMargin;
                nBottomMargin = rMargin.nBottomMargin;
                return *this;
            }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
