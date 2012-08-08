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
#ifndef _SVX_LANGITEM_HXX
#define _SVX_LANGITEM_HXX

#include <svl/eitem.hxx>
#include <i18npool/lang.h>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxLanguageItem -------------------------------------------------

/*  [Description]

    This item describes a Language.
*/

class EDITENG_DLLPUBLIC SvxLanguageItem : public SfxEnumItem
{
public:
    TYPEINFO();

    SvxLanguageItem( const LanguageType eLang /*= LANGUAGE_GERMAN*/,
                     const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16          GetValueCount() const;

    inline SvxLanguageItem& operator=(const SvxLanguageItem& rLang)
        {
            SetValue( rLang.GetValue() );
            return *this;
        }

    // enum cast
    LanguageType            GetLanguage() const
                                { return (LanguageType)GetValue(); }
    void                    SetLanguage( const LanguageType eLang )
                                { SetValue( (sal_uInt16)eLang ); }
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
