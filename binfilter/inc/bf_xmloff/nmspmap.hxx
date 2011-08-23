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

#ifndef _XMLOFF_NMSPMAP_HXX
#define _XMLOFF_NMSPMAP_HXX

#include <limits.h>

#include <hash_map>
#ifndef __SGI_STL_MAP
#include <map>
#endif
#include <rtl/ref.hxx>
#include <cppuhelper/weak.hxx>
namespace binfilter {

const sal_uInt16 XML_NAMESPACE_XMLNS  = (USHRT_MAX-2);
const sal_uInt16 XML_NAMESPACE_NONE  = (USHRT_MAX-1);
const sal_uInt16 XML_NAMESPACE_UNKNOWN  = (USHRT_MAX);
const sal_uInt16 XML_NAMESPACE_UNKNOWN_FLAG  = 0x8000;

class NameSpaceEntry : public ::cppu::OWeakObject
{
public:
    // sName refers to the full namespace name
    ::rtl::OUString 	sName;
    // sPrefix is the prefix used to declare a given item to be from a given namespace
    ::rtl::OUString 	sPrefix;
    // nKey is the unique identifier of a namespace
    sal_uInt16			nKey;
};

struct OUStringEqFunc
{
    sal_Bool operator()( const ::rtl::OUString &r1,
                         const ::rtl::OUString &r2) const
    {
        return r1 == r2;
    }
};

struct uInt32lt
{
    sal_Bool operator()( const sal_uInt32 &r1,
                         const sal_uInt32 &r2) const
    {
        return r1 < r2;
    }
};
typedef ::std::pair < sal_uInt16, const ::rtl::OUString* > QNamePair;

struct QNamePairHash
{
    size_t operator()( const QNamePair &r1 ) const
    {
        return (size_t) r1.second->hashCode() + r1.first;
    }
};
struct QNamePairEq
{
    bool operator()( const QNamePair &r1,
                     const QNamePair &r2 ) const
    {
        return r1.first == r2.first && *(r1.second) == *(r2.second);
    }
};

typedef ::std::hash_map < QNamePair, ::rtl::OUString, QNamePairHash, QNamePairEq > QNameCache;
typedef ::std::hash_map < ::rtl::OUString, ::rtl::Reference <NameSpaceEntry >, ::rtl::OUStringHash, OUStringEqFunc > NameSpaceHash;
typedef ::std::map < sal_uInt16, ::rtl::Reference < NameSpaceEntry >, uInt32lt > NameSpaceMap;

class SvXMLNamespaceMap
{
    const ::rtl::OUString		sXMLNS;
    const ::rtl::OUString		sEmpty;

    NameSpaceHash 				aNameHash, aNameCache;
    NameSpaceMap				aNameMap;
    QNameCache					aQNameCache;
    sal_uInt16 _Add( const ::rtl::OUString& rPrefix, const ::rtl::OUString &rName, sal_uInt16 nKey );

public:

    SvXMLNamespaceMap();
    ~SvXMLNamespaceMap();

    SvXMLNamespaceMap( const SvXMLNamespaceMap& );

    int operator ==( const SvXMLNamespaceMap& rCmp ) const;

    sal_uInt16 Add( const ::rtl::OUString& rPrefix,
                const ::rtl::OUString& rName,
                sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );

    sal_uInt16 GetKeyByName( const ::rtl::OUString& rName ) const;
    const ::rtl::OUString& GetNameByKey( sal_uInt16 nKey ) const;

    sal_uInt16 GetKeyByPrefix( const ::rtl::OUString& rPrefix ) const;
    const ::rtl::OUString& GetPrefixByKey( sal_uInt16 nKey ) const;

    ::rtl::OUString GetQNameByKey( sal_uInt16 nKey, 
                           const ::rtl::OUString& rLocalName ) const;

    ::rtl::OUString GetAttrNameByKey( sal_uInt16 nKey ) const;

    /* This will replace the version with the unused 5th default parameter */
    sal_uInt16 _GetKeyByAttrName( const ::rtl::OUString& rAttrName,
                             ::rtl::OUString *pPrefix,
                             ::rtl::OUString *pLocalName,
                             ::rtl::OUString *pNamespace = 0 ) const;

    sal_uInt16 GetFirstKey() const;
    sal_uInt16 GetNextKey( sal_uInt16 nOldKey ) const;

/* deprecated */ sal_Bool AddAtIndex( sal_uInt16 nIdx, const ::rtl::OUString& rPrefix,
                     const ::rtl::OUString& rName, sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
/* deprecated */ sal_Bool AddAtIndex( sal_uInt16 nIdx, const sal_Char *pPrefix,
                     const sal_Char *pName, sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
/* deprecated */ sal_uInt16 GetIndexByPrefix( const ::rtl::OUString& rPrefix ) const;
/* deprecated */ const ::rtl::OUString& GetPrefixByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ const ::rtl::OUString& GetNameByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ ::rtl::OUString GetQNameByIndex( sal_uInt16 nIdx,
                            const ::rtl::OUString& rLocalName ) const;
/* deprecated */ sal_uInt16 GetKeyByAttrName( const ::rtl::OUString& rAttrName,
                             ::rtl::OUString *pPrefix,
                             ::rtl::OUString *pLocalName,
                             ::rtl::OUString *pNamespace=0,
                             sal_uInt16 nIdxGuess = USHRT_MAX ) const;
/* deprecated */ sal_uInt16 GetKeyByAttrName( const ::rtl::OUString& rAttrName,
                             ::rtl::OUString *pLocalName = 0,
                             sal_uInt16 nIdxGuess = USHRT_MAX ) const;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_NMSPMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
