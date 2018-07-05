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
#ifndef INCLUDED_EDITENG_FORMATBREAKITEM_HXX
#define INCLUDED_EDITENG_FORMATBREAKITEM_HXX

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>


// class SvxFormatBreakItem -------------------------------------------------

/*  [Description]

    This item describes a wrap-attribute
    Automatic?, Page or column break, before or after?
*/
#define FMTBREAK_NOAUTO (sal_uInt16(0x0001))

class EDITENG_DLLPUBLIC SvxFormatBreakItem : public SfxEnumItem<SvxBreak>
{
public:
    static SfxPoolItem* CreateDefault();

    inline SvxFormatBreakItem( const SvxBreak eBrk /*= SvxBreak::NONE*/,
                            const sal_uInt16 nWhich );
    inline SvxFormatBreakItem& operator=( const SvxFormatBreakItem& rCpy );
    /*TODO: SfxPoolItem copy function dichotomy*/SvxFormatBreakItem(SvxFormatBreakItem const &) = default;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
    OUString                 GetValueTextByPos( sal_uInt16 nPos ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16       GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const override;
    virtual sal_uInt16       GetValueCount() const override;

    SvxBreak                 GetBreak() const { return GetValue(); }
};


inline SvxFormatBreakItem::SvxFormatBreakItem( const SvxBreak eBreak,
                                         const sal_uInt16 _nWhich ) :
    SfxEnumItem( _nWhich, eBreak )
{}


inline SvxFormatBreakItem& SvxFormatBreakItem::operator=(
    const SvxFormatBreakItem& rBreak )
{
    SetValue( rBreak.GetBreak() );
    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
