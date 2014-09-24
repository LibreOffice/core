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
#ifndef INCLUDED_EDITENG_FHGTITEM_HXX
#define INCLUDED_EDITENG_FHGTITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

// class SvxFontHeightItem -----------------------------------------------

// Warning: twips values

/*  [Description]

    This item describes the font height
*/

class EDITENG_DLLPUBLIC SvxFontHeightItem : public SfxPoolItem
{
    sal_uInt32  nHeight;
    sal_uInt16  nProp;              // default 100%
    SfxMapUnit ePropUnit;       // Percent, Twip, ...
public:
    TYPEINFO_OVERRIDE();

    SvxFontHeightItem( const sal_uLong nSz /*= 240*/, const sal_uInt16 nPropHeight /*= 100*/,
                       const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    virtual bool            GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const SAL_OVERRIDE;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Create(SvStream &) const SAL_OVERRIDE;
    virtual SvStream&       Store(SvStream &) const SAL_OVERRIDE;
    virtual bool            ScaleMetrics( long nMult, long nDiv ) SAL_OVERRIDE;
    virtual bool            HasMetrics() const SAL_OVERRIDE;

    inline SvxFontHeightItem& operator=(const SvxFontHeightItem& rSize)
        {
            SetHeightValue( rSize.GetHeight() );
            SetProp( rSize.GetProp(), ePropUnit );
            return *this;
        }

    void SetHeight( sal_uInt32 nNewHeight, const sal_uInt16 nNewProp = 100,
                     SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE );

    void SetHeight( sal_uInt32 nNewHeight, sal_uInt16 nNewProp,
                     SfxMapUnit eUnit, SfxMapUnit eCoreUnit );

    sal_uInt32 GetHeight() const { return nHeight; }

    void SetHeightValue( sal_uInt32 nNewHeight )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nHeight = nNewHeight;
        }

    void SetProp( const sal_uInt16 nNewProp,
                    SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
            ePropUnit = eUnit;
        }

    sal_uInt16 GetProp() const { return nProp; }

    SfxMapUnit GetPropUnit() const { return ePropUnit;  }   // Percent, Twip, ...
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
