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
#ifndef _SVX_BRKITEM_HXX
#define _SVX_BRKITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

namespace rtl
{
    class OUString;
}

// class SvxFmtBreakItem -------------------------------------------------

/*
    [Beschreibung]
    Dieses Item beschreibt ein Umbruch-Attribut
    Automatisch?, Seiten- oder Spaltenumbruch, davor oder danach?
*/
#define FMTBREAK_NOAUTO ((sal_uInt16)0x0001)

class EDITENG_DLLPUBLIC SvxFmtBreakItem : public SfxEnumItem
{
public:
    TYPEINFO();

    inline SvxFmtBreakItem( const SvxBreak eBrk /*= SVX_BREAK_NONE*/,
                            const sal_uInt16 nWhich );
    inline SvxFmtBreakItem( const SvxFmtBreakItem& rBreak );
    inline SvxFmtBreakItem& operator=( const SvxFmtBreakItem& rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
    virtual String           GetValueTextByPos( sal_uInt16 nPos ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16           GetVersion( sal_uInt16 nFileVersion ) const;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const;
    virtual sal_uInt16           GetValueCount() const;

    // MS VC4.0 kommt durcheinander
    void             SetValue( sal_uInt16 nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }
    SvxBreak             GetBreak() const { return SvxBreak( GetValue() ); }
    void                     SetBreak( const SvxBreak eNew )
                                { SetValue( (sal_uInt16)eNew ); }
};


inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxBreak eBreak,
                                         const sal_uInt16 _nWhich ) :
    SfxEnumItem( _nWhich, (sal_uInt16)eBreak )
{}

inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxFmtBreakItem& rBreak ) :
    SfxEnumItem( rBreak )
{}

inline SvxFmtBreakItem& SvxFmtBreakItem::operator=(
    const SvxFmtBreakItem& rBreak )
{
    SetValue( rBreak.GetValue() );
    return *this;
}

#endif

