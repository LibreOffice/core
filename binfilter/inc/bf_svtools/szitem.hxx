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
#ifndef _SFXSZITEM_HXX
#define _SFXSZITEM_HXX

#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#include <bf_svtools/poolitem.hxx>

class SvStream;

namespace binfilter
{

class SfxArguments;

DBG_NAMEEX(SfxSizeItem)

// -----------------------------------------------------------------------

class SfxSizeItem : public SfxPoolItem
{
private:
    Size					 aVal;

public:
                             TYPEINFO();
                             SfxSizeItem();
                             SfxSizeItem( USHORT nWhich, const Size& rVal );
                             SfxSizeItem( const SfxSizeItem& );
                             ~SfxSizeItem() { DBG_DTOR(SfxSizeItem, 0); }

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const ::IntlWrapper * = 0 ) const;

    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual	BOOL 			 QueryValue( com::sun::star::uno::Any& rVal,
                                          BYTE nMemberId = 0 ) const;
    virtual	BOOL 			 PutValue( const com::sun::star::uno::Any& rVal,
                                          BYTE nMemberId = 0 );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT nItemVersion) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion) const;

    const Size&				GetValue() const { return aVal; }
    void					SetValue( const Size& rNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 aVal = rNewVal; }
};

}

#endif

