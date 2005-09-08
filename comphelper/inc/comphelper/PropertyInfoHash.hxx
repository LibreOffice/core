/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyInfoHash.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:23:50 $
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

#ifndef _COMPHELPER_PROPERTYINFOHASH_HXX_
#define _COMPHELPER_PROPERTYINFOHASH_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COMPHELPER_TYPEGENERATION_HXX_
#include <comphelper/TypeGeneration.hxx>
#endif
#include <hash_map>
namespace comphelper
{
    struct PropertyInfo
    {
        const sal_Char* mpName;
        sal_uInt16 mnNameLen;
        sal_Int32 mnHandle;
        CppuTypes meCppuType;
        sal_Int16 mnAttributes;
        sal_uInt8 mnMemberId;
    };
    struct PropertyData
    {
        sal_uInt8 mnMapId;
        PropertyInfo *mpInfo;
        PropertyData ( sal_uInt8 nMapId, PropertyInfo *pInfo )
        : mnMapId ( nMapId )
        , mpInfo ( pInfo ) {}
    };
    struct eqFunc
    {
        sal_Bool operator()( const rtl::OUString &r1,
                             const rtl::OUString &r2) const
        {
            return r1 == r2;
        }
    };
}

typedef std::hash_map < ::rtl::OUString,
                        ::comphelper::PropertyInfo*,
                        ::rtl::OUStringHash,
                        ::comphelper::eqFunc > PropertyInfoHash;
typedef std::hash_map < ::rtl::OUString,
                        ::comphelper::PropertyData*,
                        ::rtl::OUStringHash,
                        ::comphelper::eqFunc > PropertyDataHash;
#endif
