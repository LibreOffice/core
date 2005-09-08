/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmitems.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:17:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_FMITEMS_HXX
#define _SVX_FMITEMS_HXX


#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
//  FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
FORWARD_DECLARE_INTERFACE(uno,Any)
//  class ::com::sun::star::uno::Any;

//==================================================================
class FmInterfaceItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface;

public:
    TYPEINFO();

    FmInterfaceItem( const sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxInterface )
        :SfxPoolItem( nId )
        ,xInterface( rxInterface )
    {}

    inline FmInterfaceItem&  operator=( const FmInterfaceItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;

    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >         GetInterface() const { return xInterface; }
};

inline FmInterfaceItem& FmInterfaceItem::operator=( const FmInterfaceItem &rCpy )
{
    xInterface = rCpy.xInterface;

    return *this;
}



#endif // _SVX_FMITEMS_HXX

