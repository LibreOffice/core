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

#ifndef _SVX_XLNSTIT_HXX
#define _SVX_XLNSTIT_HXX

#ifndef _XPOLY_HXX //autogen
#include <bf_svx/xpoly.hxx>
#endif
#ifndef _SVX_XIT_HXX //autogen
#include <bf_svx/xit.hxx>
#endif
namespace binfilter {

class SdrModel;

//------------------------
// class XLineStartItem
//------------------------
class XLineStartItem : public NameOrIndex
{
    XPolygon    aXPolygon;

public:
            TYPEINFO();
            XLineStartItem(long nIndex = -1);
            XLineStartItem(const String& rName, const XPolygon& rXPolygon);
            XLineStartItem(SfxItemPool* pPool, const XPolygon& rXPolygon);
            XLineStartItem(const XLineStartItem& rItem);
            XLineStartItem(SvStream& rIn);

    virtual int             operator==(const SfxPoolItem& rItem) const;
    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, USHORT nVer) const;
    virtual SvStream&       Store(SvStream& rOut, USHORT nItemVersion ) const;

    virtual	sal_Bool        	 QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	sal_Bool			 PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    const XPolygon& GetValue(const XLineEndTable* pTable = 0) const;
    void            SetValue(const XPolygon& rXPolygon) { aXPolygon = rXPolygon; Detach(); }

    XLineStartItem* checkForUniqueItem( SdrModel* pModel ) const;
};

}//end of namespace binfilter
#endif
