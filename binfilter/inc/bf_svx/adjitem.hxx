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
#ifndef _SVX_ADJITEM_HXX
#define _SVX_ADJITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/eitem.hxx>
#include <bf_svx/svxenum.hxx>

#include <bf_svx/svxids.hrc>

#include <bf_svx/eeitem.hxx>

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
// class SvxAdjustItem ---------------------------------------------------

/*
[Beschreibung]
Dieses Item beschreibt die Zeilenausrichtung.
*/
#define	ADJUST_LASTBLOCK_VERSION		((USHORT)0x0001)

class SvxAdjustItem : public SfxEnumItemInterface
{
    BOOL    bLeft      : 1;
    BOOL    bRight     : 1;
    BOOL    bCenter    : 1;
    BOOL    bBlock     : 1;

    // nur aktiv, wenn bBlock
    BOOL    bOneBlock : 1;
    BOOL    bLastCenter : 1;
    BOOL    bLastBlock : 1;

    friend SvStream& operator<<( SvStream&, SvxAdjustItem& ); //$ ostream
public:
    TYPEINFO();

    SvxAdjustItem( const SvxAdjust eAdjst = SVX_ADJUST_LEFT,
                   const USHORT nId = ITEMID_ADJUST );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;

    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual USHORT			 GetValueCount() const;
    virtual USHORT			 GetEnumValue() const;
    virtual void			 SetEnumValue( USHORT nNewVal );
    virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT			 GetVersion( USHORT nFileVersion ) const;

    inline void SetOneWord( const SvxAdjust eType )
    {
        bOneBlock  = eType == SVX_ADJUST_BLOCK;
    }

    inline void SetLastBlock( const SvxAdjust eType )
    {
        bLastBlock = eType == SVX_ADJUST_BLOCK;
        bLastCenter = eType == SVX_ADJUST_CENTER;
    }

    inline void SetAdjust( const SvxAdjust eType )
    {
        bLeft = eType == SVX_ADJUST_LEFT;
        bRight = eType == SVX_ADJUST_RIGHT;
        bCenter = eType == SVX_ADJUST_CENTER;
        bBlock = eType == SVX_ADJUST_BLOCK;
    }

    inline SvxAdjust GetLastBlock() const
    {
        SvxAdjust eRet = SVX_ADJUST_LEFT;

        if ( bLastBlock )
            eRet = SVX_ADJUST_BLOCK;
        else if( bLastCenter )
            eRet = SVX_ADJUST_CENTER;
        return eRet;
    }

    inline SvxAdjust GetOneWord() const
    {
        SvxAdjust eRet = SVX_ADJUST_LEFT;

        if ( bBlock && bOneBlock )
            eRet = SVX_ADJUST_BLOCK;
        return eRet;
    }

    inline SvxAdjust GetAdjust() const
    {
        SvxAdjust eRet = SVX_ADJUST_LEFT;

        if ( bRight )
            eRet = SVX_ADJUST_RIGHT;
        else if ( bCenter )
            eRet = SVX_ADJUST_CENTER;
        else if ( bBlock )
            eRet = SVX_ADJUST_BLOCK;
        return eRet;
    }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
