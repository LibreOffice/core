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

#ifndef _XMLOFF_XMLCNIMP_HXX
#define _XMLOFF_XMLCNIMP_HXX

#include <com/sun/star/container/XNameAccess.hpp>

#include <bf_svtools/svarray.hxx>

#include <bf_xmloff/nmspmap.hxx>

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>

namespace rtl { class OUString; }
namespace binfilter {

class SvXMLAttrContainerData_Impl;

class SvXMLAttrContainerData
{
    SvXMLNamespaceMap	aNamespaceMap;
    SvUShorts			aPrefixPoss;
    SvXMLAttrContainerData_Impl	*pLNames;
    SvXMLAttrContainerData_Impl	*pValues;

    inline sal_uInt16 GetPrefixPos( sal_uInt16 i ) const;
public:

    SvXMLAttrContainerData( const SvXMLAttrContainerData& rImpl );
    SvXMLAttrContainerData();
    ~SvXMLAttrContainerData();

    int  operator ==( const SvXMLAttrContainerData& rCmp ) const;

    BOOL AddAttr( const ::rtl::OUString& rLName, const ::rtl::OUString& rValue );
    BOOL AddAttr( const ::rtl::OUString& rPrefix, const ::rtl::OUString& rNamespace,
                    const ::rtl::OUString& rLName, const ::rtl::OUString& rValue );
    BOOL AddAttr( const ::rtl::OUString& rPrefix,
                  const ::rtl::OUString& rLName,
                  const ::rtl::OUString& rValue );

    sal_uInt16 GetAttrCount() const;
    inline ::rtl::OUString GetAttrNamespace( sal_uInt16 i ) const;
    inline ::rtl::OUString GetAttrPrefix( sal_uInt16 i ) const;
    const ::rtl::OUString& GetAttrLName( sal_uInt16 i ) const;
    const ::rtl::OUString& GetAttrValue( sal_uInt16 i ) const;

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

inline sal_uInt16 SvXMLAttrContainerData::GetPrefixPos( sal_uInt16 i ) const
{
//	DBG_ASSERT( i >= 0 && i < aPrefixPoss.Count(),
//				"SvXMLAttrContainerData::GetPrefixPos: illegal index" );
    return aPrefixPoss[i];
}

inline ::rtl::OUString SvXMLAttrContainerData::GetAttrNamespace( sal_uInt16 i ) const
{
    ::rtl::OUString sRet;
    sal_uInt16 nPos = GetPrefixPos( i );
    if( USHRT_MAX != nPos )
        sRet = aNamespaceMap.GetNameByIndex( nPos );
    return sRet;
}

inline ::rtl::OUString SvXMLAttrContainerData::GetAttrPrefix( sal_uInt16 i ) const
{
    ::rtl::OUString sRet;
    sal_uInt16 nPos = GetPrefixPos( i );
    if( USHRT_MAX != nPos )
        sRet = aNamespaceMap.GetPrefixByIndex( nPos );
    return sRet;
}

inline const ::rtl::OUString& SvXMLAttrContainerData::GetNamespace(
                                                            sal_uInt16 i ) const
{
    return aNamespaceMap.GetNameByIndex( i );
}

inline const ::rtl::OUString& SvXMLAttrContainerData::GetPrefix( sal_uInt16 i ) const
{
    return aNamespaceMap.GetPrefixByIndex( i );
}

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
