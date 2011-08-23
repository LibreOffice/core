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
#ifndef _SVX_BRKITEM_HXX
#define _SVX_BRKITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/eitem.hxx>
#include <bf_svx/svxenum.hxx>
#include <bf_svx/svxids.hrc>

namespace rtl
{
    class OUString;
}
namespace binfilter {

// class SvxFmtBreakItem -------------------------------------------------

/*
    [Beschreibung]
    Dieses Item beschreibt ein Umbruch-Attribut
    Automatisch?, Seiten- oder Spaltenumbruch, davor oder danach?
*/
#define	FMTBREAK_NOAUTO	((USHORT)0x0001)

class SvxFmtBreakItem : public SfxEnumItem
{
public:
    TYPEINFO();

    inline SvxFmtBreakItem( const SvxBreak eBrk = SVX_BREAK_NONE,
                            const USHORT nWhich = ITEMID_FMTBREAK );
    inline SvxFmtBreakItem( const SvxFmtBreakItem& rBreak );
    inline SvxFmtBreakItem& operator=( const SvxFmtBreakItem& rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SvStream&		 Store( SvStream& , USHORT nItemVersion ) const;
    virtual USHORT			 GetVersion( USHORT nFileVersion ) const;
    virtual SfxPoolItem*	 Create( SvStream&, USHORT ) const;
    virtual USHORT			 GetValueCount() const;

    // MS VC4.0 kommt durcheinander
    void			 SetValue( USHORT nNewVal )
                                {SfxEnumItem::SetValue(nNewVal); }
    SvxBreak			 GetBreak() const { return SvxBreak( GetValue() ); }
    void					 SetBreak( const SvxBreak eNew )
                                { SetValue( (USHORT)eNew ); }
};

inline SvxFmtBreakItem::SvxFmtBreakItem( const SvxBreak eBreak,
                                         const USHORT nWhich ) :
    SfxEnumItem( nWhich, (USHORT)eBreak )
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


}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
