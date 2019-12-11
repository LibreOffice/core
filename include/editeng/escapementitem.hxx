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
#ifndef INCLUDED_EDITENG_ESCAPEMENTITEM_HXX
#define INCLUDED_EDITENG_ESCAPEMENTITEM_HXX

#include <svl/cenumitm.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

// class SvxEscapementItem -----------------------------------------------

#define DFLT_ESC_SUPER   33     // 1/3
#define DFLT_ESC_SUB    -33     // also 1/3 previously 8/100
#define DFLT_ESC_PROP    58
#define MAX_ESC_POS      13999
#define DFLT_ESC_AUTO_SUPER  (MAX_ESC_POS+1)
#define DFLT_ESC_AUTO_SUB    -DFLT_ESC_AUTO_SUPER

/*  [Description]

    This item describes the writing position.
*/

class EDITENG_DLLPUBLIC SvxEscapementItem final : public SfxEnumItemInterface
{
    short nEsc;
    sal_uInt8  nProp;
public:
    static SfxPoolItem* CreateDefault();

    explicit SvxEscapementItem( const sal_uInt16 nId  );
    SvxEscapementItem( const SvxEscapement eEscape,
                   const sal_uInt16 nId  );
    SvxEscapementItem( const short nEsc, const sal_uInt8 nProp,
                   const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    MapUnit eCoreMetric,
                                    MapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper& ) const override;

    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual SvxEscapementItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    void SetEscapement( const SvxEscapement eNew )
    {
        if( SvxEscapement::Off == eNew )
        {
            nEsc = 0;
            nProp = 100;
        }
        else
        {
            nProp = DFLT_ESC_PROP;
            if( SvxEscapement::Superscript == eNew )
                nEsc = DFLT_ESC_SUPER;
            else
                nEsc = DFLT_ESC_SUB;
        }
    }
    SvxEscapement GetEscapement() const { return static_cast< SvxEscapement >( GetEnumValue() ); }

    short &GetEsc() { return nEsc; }
    short GetEsc() const { return nEsc; }

    sal_uInt8 &GetProportionalHeight() { return nProp; }
    sal_uInt8  GetProportionalHeight() const { return nProp; }

    virtual sal_uInt16      GetValueCount() const override;
    static OUString         GetValueTextByPos( sal_uInt16 nPos );
    virtual sal_uInt16      GetEnumValue() const override;
    virtual void            SetEnumValue( sal_uInt16 nNewVal ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
