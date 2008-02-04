/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertySetInfoHash.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:51:17 $
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
#ifndef _XMLOFF_PROPERTYSETINFOHASH_HXX
#define _XMLOFF_PROPERTYSETINFOHASH_HXX

#ifndef _XMLOFF_PROPERTYSETINFOKEY_HXX
#include <xmloff/PropertySetInfoKey.hxx>
#endif

#include <string.h>
#include <memory>

struct PropertySetInfoHash
{
    inline size_t operator()( const PropertySetInfoKey& r ) const;
    inline bool operator()( const PropertySetInfoKey& r1,
                               const PropertySetInfoKey& r2 ) const;
};

inline size_t PropertySetInfoHash::operator()(
        const PropertySetInfoKey& r ) const
{
    const sal_Int32* pBytesAsInt32Array =
        (const sal_Int32*)r.aImplementationId.getConstArray();
    sal_Int32 nId32 =   pBytesAsInt32Array[0] ^
                        pBytesAsInt32Array[1] ^
                        pBytesAsInt32Array[2] ^
                        pBytesAsInt32Array[3];
    return (size_t)nId32 ^ (size_t)r.xPropInfo.get();
}

inline bool PropertySetInfoHash::operator()(
        const PropertySetInfoKey& r1,
        const PropertySetInfoKey& r2 ) const
{
    if( r1.xPropInfo != r2.xPropInfo )
        return sal_False;

    const sal_Int8* pId1 = r1.aImplementationId.getConstArray();
    const sal_Int8* pId2 = r2.aImplementationId.getConstArray();
    return memcmp( pId1, pId2, 16 * sizeof( sal_Int8 ) ) == 0;
}
#endif
