/*************************************************************************
 *
 *  $RCSfile: fmitems.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_FMITEMS_HXX
#define _SVX_FMITEMS_HXX


#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#include <unotools/uno3.hxx>
//  FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
FORWARD_DECLARE_INTERFACE(uno,Any)
//  class ::com::sun::star::uno::Any;

struct FmFormInfo
{
    sal_Int32   Pos;
    sal_Int32   Count;
    sal_Bool    ReadOnly;
    FmFormInfo(sal_Int32 _Pos = 0, sal_Int32 _Count = -1, sal_Bool _ReadOnly = sal_False)
        :Pos(_Pos)
        ,Count(_Count)
        ,ReadOnly(_ReadOnly){}
    sal_Bool operator == (const FmFormInfo& rInfo) const
        {return Pos == rInfo.Pos && Count == rInfo.Count && ReadOnly==rInfo.ReadOnly;}
};

//==================================================================
class FmFormInfoItem : public SfxPoolItem
{
    FmFormInfo  aInfo;

public:
    TYPEINFO();

    FmFormInfoItem(const sal_uInt16 nId, const FmFormInfo& rInfo = FmFormInfo())
        :SfxPoolItem(nId)
        ,aInfo(rInfo){}

    inline FmFormInfoItem&   operator=( const FmFormInfoItem &rCpy );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual sal_Bool             QueryValue( ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId = 0 ) const;
    virtual sal_Bool             SetValue( const ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId = 0 );

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;

    const FmFormInfo& GetInfo() const { return aInfo; }
    void              SetInfo(const FmFormInfo& rInfo) { aInfo = rInfo; }
};

inline FmFormInfoItem& FmFormInfoItem::operator=( const FmFormInfoItem &rCpy )
{
    aInfo = rCpy.aInfo;
    return *this;
}


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
    virtual sal_Bool            QueryValue( ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId = 0 ) const;
    virtual sal_Bool            SetValue( const ::com::sun::star::uno::Any& rVal, sal_Int8 nMemberId = 0 );

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

