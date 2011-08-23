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

#ifndef _SVTOOLS_XMLCNIMP_HXX
#define _SVTOOLS_XMLCNIMP_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _SVTOOLS_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

namespace rtl { class OUString; }

namespace binfilter
{

typedef ::rtl::OUString *OUStringPtr;

SV_DECL_PTRARR_DEL( OUStringsDtor2_Impl, OUStringPtr, 5, 5 )

class SvXMLAttrContainerItem_Impl
{
    SvXMLNamespaceMap	aNamespaceMap;
    SvUShorts			aPrefixPoss;
    OUStringsDtor2_Impl	aLNames;
    OUStringsDtor2_Impl	aValues;

    inline sal_uInt16 GetPrefixPos( sal_uInt16 i ) const;
public:

    SvXMLAttrContainerItem_Impl( const SvXMLAttrContainerItem_Impl& rImpl );
    SvXMLAttrContainerItem_Impl();

    int  operator ==( const SvXMLAttrContainerItem_Impl& rCmp ) const;

    BOOL AddAttr( const ::rtl::OUString& rLName, const ::rtl::OUString& rValue );
    BOOL AddAttr( const ::rtl::OUString& rPrefix, const ::rtl::OUString& rNamespace,
                    const ::rtl::OUString& rLName, const ::rtl::OUString& rValue );
    BOOL AddAttr( const ::rtl::OUString& rPrefix,
                  const ::rtl::OUString& rLName,
                  const ::rtl::OUString& rValue );

    sal_uInt16 GetAttrCount() const { return aLNames.Count(); }
    inline ::rtl::OUString GetAttrNamespace( sal_uInt16 i ) const;
    inline ::rtl::OUString GetAttrPrefix( sal_uInt16 i ) const;
    inline const ::rtl::OUString& GetAttrLName( sal_uInt16 i ) const;
    inline const ::rtl::OUString& GetAttrValue( sal_uInt16 i ) const;

    sal_uInt16 GetFirstNamespaceIndex() const { return aNamespaceMap.GetFirstIndex(); }
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const { return aNamespaceMap.GetNextIndex( nIdx ); }
    inline const ::rtl::OUString& GetNamespace( sal_uInt16 i ) const;
    inline const ::rtl::OUString& GetPrefix( sal_uInt16 i ) const;

    BOOL SetAt( sal_uInt16 i,
                const ::rtl::OUString& rLName, const ::rtl::OUString& rValue );
    BOOL SetAt( sal_uInt16 i,
                const ::rtl::OUString& rPrefix, const ::rtl::OUString& rNamespace,
                const ::rtl::OUString& rLName, const ::rtl::OUString& rValue );
    BOOL SetAt( sal_uInt16 i,
                const ::rtl::OUString& rPrefix,
                const ::rtl::OUString& rLName,
                const ::rtl::OUString& rValue );

    void Remove( sal_uInt16 i );
};

inline sal_uInt16 SvXMLAttrContainerItem_Impl::GetPrefixPos( sal_uInt16 i ) const
{
//	DBG_ASSERT( i >= 0 && i < aPrefixPoss.Count(),
//				"SvXMLAttrContainerItem_Impl::GetPrefixPos: illegal index" );
    return aPrefixPoss[i];
}

inline ::rtl::OUString SvXMLAttrContainerItem_Impl::GetAttrNamespace( sal_uInt16 i ) const
{
    ::rtl::OUString sRet;
    sal_uInt16 nPos = GetPrefixPos( i );
    if( USHRT_MAX != nPos )
        sRet = aNamespaceMap.GetNameByIndex( nPos );
    return sRet;
}

inline ::rtl::OUString SvXMLAttrContainerItem_Impl::GetAttrPrefix( sal_uInt16 i ) const
{
    ::rtl::OUString sRet;
    sal_uInt16 nPos = GetPrefixPos( i );
    if( USHRT_MAX != nPos )
        sRet = aNamespaceMap.GetPrefixByIndex( nPos );
    return sRet;
}

inline const ::rtl::OUString& SvXMLAttrContainerItem_Impl::GetAttrLName(sal_uInt16 i) const
{
    DBG_ASSERT( i >= 0 && i < aLNames.Count(),
                "SvXMLAttrContainerItem_Impl::GetLName: illegal index" );
    return *aLNames[i];
}

inline const ::rtl::OUString& SvXMLAttrContainerItem_Impl::GetAttrValue(sal_uInt16 i) const
{
    DBG_ASSERT( i >= 0 && i < aValues.Count(),
                "SvXMLAttrContainerItem_Impl::GetValue: illegal index" );
    return *aValues[i];
}

inline const ::rtl::OUString& SvXMLAttrContainerItem_Impl::GetNamespace(
                                                            sal_uInt16 i ) const
{
    return aNamespaceMap.GetNameByIndex( i );
}

inline const ::rtl::OUString& SvXMLAttrContainerItem_Impl::GetPrefix( sal_uInt16 i ) const
{
    return aNamespaceMap.GetPrefixByIndex( i );
}

}

#endif

