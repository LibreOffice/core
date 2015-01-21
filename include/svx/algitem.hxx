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
#ifndef INCLUDED_SVX_ALGITEM_HXX
#define INCLUDED_SVX_ALGITEM_HXX

#include <svx/svxids.hrc>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <svx/svxdllapi.h>

class SvStream;



class SVX_DLLPUBLIC SvxOrientationItem: public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();

    SvxOrientationItem(
        const SvxCellOrientation eOrientation /*= SVX_ORIENTATION_STANDARD*/,
        const sal_uInt16 nId );

    SvxOrientationItem(
        sal_Int32 nRotation, bool bStacked,
        const sal_uInt16 nId );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual sal_uInt16          GetValueCount() const SAL_OVERRIDE;
    OUString        GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const SAL_OVERRIDE;

    inline  SvxOrientationItem& operator=(const SvxOrientationItem& rOrientation)
            {
                SetValue( rOrientation.GetValue() );
                return *this;
            }

    /** Returns sal_True, if the item represents STACKED state. */
    bool                    IsStacked() const;
    /** Returns the rotation this item represents (returns nStdAngle for STANDARD and STACKED state). */
    sal_Int32               GetRotation( sal_Int32 nStdAngle = 0 ) const;
    /** Fills this item according to passed item values. */
    void                    SetFromRotation( sal_Int32 nRotation, bool bStacked );
};



class SVX_DLLPUBLIC SvxMarginItem: public SfxPoolItem
{
    sal_Int16       nLeftMargin;
    sal_Int16       nTopMargin;
    sal_Int16       nRightMargin;
    sal_Int16       nBottomMargin;
public:
    TYPEINFO_OVERRIDE();
    SvxMarginItem( const sal_uInt16 nId  );
    SvxMarginItem( sal_Int16 nLeft, sal_Int16 nTop /*= 0*/,
                   sal_Int16 nRight /*= 0*/, sal_Int16 nBottom /*= 0*/,
                   const sal_uInt16 nId  );
    SvxMarginItem( const SvxMarginItem& );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    virtual bool             operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const SAL_OVERRIDE;
    virtual SvStream&        Store( SvStream&, sal_uInt16 nItemVersion ) const SAL_OVERRIDE;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

            sal_Int16       GetLeftMargin() const {return nLeftMargin; }
            void            SetLeftMargin(sal_Int16 nLeft);
            sal_Int16       GetTopMargin() const {return nTopMargin; }
            void            SetTopMargin(sal_Int16 nTop);
            sal_Int16       GetRightMargin() const {return nRightMargin; }
            void            SetRightMargin(sal_Int16 nRight);
            sal_Int16       GetBottomMargin() const {return nBottomMargin; }
            void            SetBottomMargin(sal_Int16 nBottom);

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
