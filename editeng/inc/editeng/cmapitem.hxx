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
#ifndef _SVX_CMAPITEM_HXX
#define _SVX_CMAPITEM_HXX

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxCaseMapItem --------------------------------------------------

/*  [Description]

    This item describe the font type (uppercase, small caps ,...).
*/

class EDITENG_DLLPUBLIC SvxCaseMapItem : public SfxEnumItem
{
public:
    TYPEINFO();

    SvxCaseMapItem( const SvxCaseMap eMap /*= SVX_CASEMAP_NOT_MAPPED*/,
                    const sal_uInt16 nId );

    // "pure virtual Methods" from SfxPoolItem + SfxEnumItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual OUString   GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16      GetValueCount() const;

    // MS VC4.0 kommt durcheinander
    void                    SetValue( sal_uInt16 nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }

    inline SvxCaseMapItem& operator=(const SvxCaseMapItem& rMap)
        {
            SetValue( rMap.GetValue() );
            return *this;
        }

    // enum cast
    SvxCaseMap              GetCaseMap() const
                                { return (SvxCaseMap)GetValue(); }
    void                    SetCaseMap( SvxCaseMap eNew )
                                { SetValue( (sal_uInt16)eNew ); }
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
