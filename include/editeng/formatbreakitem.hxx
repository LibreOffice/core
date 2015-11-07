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

    This item Describes a wrap-attribute
    Automatic?, Page or column break, before or after?
*/
#define FMTBREAK_NOAUTO ((sal_uInt16)0x0001)

class EDITENG_DLLPUBLIC SvxFormatBreakItem : public SfxEnumItem
{
public:
    TYPEINFO_OVERRIDE();
    static SfxPoolItem* CreateDefault();

    inline SvxFormatBreakItem( const SvxBreak eBrk /*= SVX_BREAK_NONE*/,
                            const sal_uInt16 nWhich );
    inline SvxFormatBreakItem( const SvxFormatBreakItem& rBreak );
    inline SvxFormatBreakItem& operator=( const SvxFormatBreakItem& rCpy );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;
    virtual OUString    GetValueTextByPos( sal_uInt16 nPos ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16       GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const override;
    virtual sal_uInt16       GetValueCount() const override;

    // MS VC4.0 messes things up
    void                     SetValue( sal_uInt16 nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }
    SvxBreak                 GetBreak() const { return SvxBreak( GetValue() ); }
};


inline SvxFormatBreakItem::SvxFormatBreakItem( const SvxBreak eBreak,
                                         const sal_uInt16 _nWhich ) :
    SfxEnumItem( _nWhich, (sal_uInt16)eBreak )
{}

inline SvxFormatBreakItem::SvxFormatBreakItem( const SvxFormatBreakItem& rBreak ) :
    SfxEnumItem( rBreak )
{}

inline SvxFormatBreakItem& SvxFormatBreakItem::operator=(
    const SvxFormatBreakItem& rBreak )
{
    SetValue( rBreak.GetValue() );
    return *this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
