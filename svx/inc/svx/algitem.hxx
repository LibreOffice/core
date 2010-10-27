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

// include ---------------------------------------------------------------

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
        const USHORT nId );

    SvxOrientationItem(
        INT32 nRotation, BOOL bStacked,
        const USHORT nId );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual USHORT          GetValueCount() const;
    virtual String          GetValueText( USHORT nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, USHORT nVer ) const;

    inline  SvxOrientationItem& operator=(const SvxOrientationItem& rOrientation)
            {
                SetValue( rOrientation.GetValue() );
                return *this;
            }

    /** Returns TRUE, if the item represents STACKED state. */
    BOOL                    IsStacked() const;
    /** Returns the rotation this item represents (returns nStdAngle for STANDARD and STACKED state). */
    INT32                   GetRotation( INT32 nStdAngle = 0 ) const;
    /** Fills this item according to passed item values. */
    void                    SetFromRotation( INT32 nRotation, BOOL bStacked );
};

//------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxMarginItem: public SfxPoolItem
{
    INT16       nLeftMargin;
    INT16       nTopMargin;
    INT16       nRightMargin;
    INT16       nBottomMargin;
public:
    TYPEINFO();
    SvxMarginItem( const USHORT nId  );
    SvxMarginItem( INT16 nLeft, INT16 nTop /*= 0*/,
                   INT16 nRight /*= 0*/, INT16 nBottom /*= 0*/,
                   const USHORT nId  );
    SvxMarginItem( const SvxMarginItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStream, USHORT nVer ) const;
    virtual SvStream&        Store( SvStream&, USHORT nItemVersion ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

            INT16           GetLeftMargin() const {return nLeftMargin; }
            BOOL            SetLeftMargin(INT16 nLeft);
            INT16           GetTopMargin() const {return nTopMargin; }
            BOOL            SetTopMargin(INT16 nTop);
            INT16           GetRightMargin() const {return nRightMargin; }
            BOOL            SetRightMargin(INT16 nRight);
            INT16           GetBottomMargin() const {return nBottomMargin; }
            BOOL            SetBottomMargin(INT16 nBottom);

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
