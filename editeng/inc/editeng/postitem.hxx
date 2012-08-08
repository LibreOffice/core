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
#ifndef _SVX_POSTITEM_HXX
#define _SVX_POSTITEM_HXX

#include <vcl/vclenum.hxx>
#include <svl/eitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxPostureItem --------------------------------------------------

/*  [Description]

    This item describes the font setting (Italic)
*/

class EDITENG_DLLPUBLIC SvxPostureItem : public SfxEnumItem
{
public:
    TYPEINFO();

    SvxPostureItem( const FontItalic ePost /*= ITALIC_NONE*/,
                    const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem + SwEnumItem
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual rtl::OUString   GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16      GetValueCount() const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual int             HasBoolValue() const;
    virtual sal_Bool        GetBoolValue() const;
    virtual void            SetBoolValue( sal_Bool bVal );

    inline SvxPostureItem& operator=(const SvxPostureItem& rPost) {
        SetValue( rPost.GetValue() );
        return *this;
    }

    // enum cast
    FontItalic              GetPosture() const
                                { return (FontItalic)GetValue(); }
    void                    SetPosture( FontItalic eNew )
                                { SetValue( (sal_uInt16)eNew ); }
};

#endif // #ifndef _SVX_POSTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
