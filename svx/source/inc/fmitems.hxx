/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_FMITEMS_HXX
#define _SVX_FMITEMS_HXX


#include <svl/poolitem.hxx>
#include <comphelper/uno3.hxx>
//  FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
FORWARD_DECLARE_INTERFACE(uno,Any)
//  class ::com::sun::star::uno::Any;

//==================================================================
class FmInterfaceItem : public SfxPoolItem
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface;

public:
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

