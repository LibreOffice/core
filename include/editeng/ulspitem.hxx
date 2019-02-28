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
#ifndef INCLUDED_EDITENG_ULSPITEM_HXX
#define INCLUDED_EDITENG_ULSPITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>


// class SvxULSpaceItem --------------------------------------------------

/*  [Description]

    This item describes the Upper- and Lower space of a page or paragraph.
*/

#define ULSPACE_16_VERSION  (sal_uInt16(0x0001))

class EDITENG_DLLPUBLIC SvxULSpaceItem : public SfxPoolItem
{
    sal_uInt16 nUpper;  // Upper space
    sal_uInt16 nLower;  // Lower space
    bool       bContext; // Contextual spacing?
    sal_uInt16 nPropUpper, nPropLower;      // relative or absolute (=100%)
public:
    static SfxPoolItem* CreateDefault();

    explicit SvxULSpaceItem( const sal_uInt16 nId  );
    SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
                    const sal_uInt16 nId  );
    inline SvxULSpaceItem& operator=( const SvxULSpaceItem &rCpy );
    SvxULSpaceItem(SvxULSpaceItem const &) = default; // SfxPoolItem copy function dichotomy

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual void                 ScaleMetrics( long nMult, long nDiv ) override;
    virtual bool                 HasMetrics() const override;

    inline void SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp = 100 );
    inline void SetLower( const sal_uInt16 nL, const sal_uInt16 nProp = 100 );

    void SetUpperValue( const sal_uInt16 nU ) { nUpper = nU; }
    void SetLowerValue( const sal_uInt16 nL ) { nLower = nL; }
    void SetContextValue( const bool bC )     { bContext = bC; }
    void SetPropUpper( const sal_uInt16 nU )  { nPropUpper = nU; }
    void SetPropLower( const sal_uInt16 nL )  { nPropLower = nL; }

    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    bool GetContext() const { return bContext; }
    sal_uInt16 GetPropUpper() const { return nPropUpper; }
    sal_uInt16 GetPropLower() const { return nPropLower; }
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline SvxULSpaceItem &SvxULSpaceItem::operator=( const SvxULSpaceItem &rCpy )
{
    nUpper = rCpy.GetUpper();
    nLower = rCpy.GetLower();
    bContext = rCpy.GetContext();
    nPropUpper = rCpy.GetPropUpper();
    nPropLower = rCpy.GetPropLower();
    return *this;
}

inline void SvxULSpaceItem::SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp )
{
    nUpper = sal_uInt16((sal_uInt32(nU) * nProp ) / 100); nPropUpper = nProp;
}
inline void SvxULSpaceItem::SetLower( const sal_uInt16 nL, const sal_uInt16 nProp )
{
    nLower = sal_uInt16((sal_uInt32(nL) * nProp ) / 100); nPropLower = nProp;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
