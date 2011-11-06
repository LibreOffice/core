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



#ifndef _XMLOFF_XMLCNIMP_HXX
#define _XMLOFF_XMLCNIMP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/container/XNameAccess.hpp>
#include <svl/svarray.hxx>
#include <xmloff/nmspmap.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>

namespace rtl { class OUString; }

class SvXMLAttrContainerData_Impl;

class XMLOFF_DLLPUBLIC SvXMLAttrContainerData
{
    SvXMLNamespaceMap   aNamespaceMap;
    SvUShorts           aPrefixPoss;
    SvXMLAttrContainerData_Impl *pLNames;
    SvXMLAttrContainerData_Impl *pValues;

    SAL_DLLPRIVATE inline sal_uInt16 GetPrefixPos( sal_uInt16 i ) const;
public:

    SvXMLAttrContainerData( const SvXMLAttrContainerData& rImpl );
    SvXMLAttrContainerData();
    ~SvXMLAttrContainerData();

    int  operator ==( const SvXMLAttrContainerData& rCmp ) const;

    sal_Bool AddAttr( const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool AddAttr( const ::rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
                    const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool AddAttr( const ::rtl::OUString& rPrefix,
                  const ::rtl::OUString& rLName,
                  const ::rtl::OUString& rValue );

    sal_uInt16 GetAttrCount() const;
    inline ::rtl::OUString GetAttrNamespace( sal_uInt16 i ) const;
    inline ::rtl::OUString GetAttrPrefix( sal_uInt16 i ) const;
    const ::rtl::OUString& GetAttrLName( sal_uInt16 i ) const;
    const ::rtl::OUString& GetAttrValue( sal_uInt16 i ) const;

    sal_uInt16 GetFirstNamespaceIndex() const { return aNamespaceMap.GetFirstIndex(); }
    sal_uInt16 GetNextNamespaceIndex( sal_uInt16 nIdx ) const { return aNamespaceMap.GetNextIndex( nIdx ); }
    inline const ::rtl::OUString& GetNamespace( sal_uInt16 i ) const;
    inline const ::rtl::OUString& GetPrefix( sal_uInt16 i ) const;

    sal_Bool SetAt( sal_uInt16 i,
                const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool SetAt( sal_uInt16 i,
                const ::rtl::OUString& rPrefix, const rtl::OUString& rNamespace,
                const ::rtl::OUString& rLName, const rtl::OUString& rValue );
    sal_Bool SetAt( sal_uInt16 i,
                const ::rtl::OUString& rPrefix,
                const ::rtl::OUString& rLName,
                const ::rtl::OUString& rValue );

    void Remove( sal_uInt16 i );
};

inline sal_uInt16 SvXMLAttrContainerData::GetPrefixPos( sal_uInt16 i ) const
{
//  DBG_ASSERT( i >= 0 && i < aPrefixPoss.Count(),
//              "SvXMLAttrContainerData::GetPrefixPos: illegal index" );
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

#endif

