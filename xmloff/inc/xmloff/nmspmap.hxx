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



#ifndef _XMLOFF_NMSPMAP_HXX
#define _XMLOFF_NMSPMAP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"
#include <rtl/ustring.hxx>
#include <hash_map>
#ifndef __SGI_STL_MAP
#include <map>
#endif
#include <vos/ref.hxx>
#include <cppuhelper/weak.hxx>

#include <limits.h>

const sal_uInt16 XML_NAMESPACE_XMLNS  = (USHRT_MAX-2);
const sal_uInt16 XML_NAMESPACE_NONE  = (USHRT_MAX-1);
const sal_uInt16 XML_NAMESPACE_UNKNOWN  = (USHRT_MAX);
const sal_uInt16 XML_NAMESPACE_UNKNOWN_FLAG  = 0x8000;

class NameSpaceEntry : public cppu::OWeakObject
{
public:
    // sName refers to the full namespace name
    ::rtl::OUString     sName;
    // sPrefix is the prefix used to declare a given item to be from a given namespace
    ::rtl::OUString     sPrefix;
    // nKey is the unique identifier of a namespace
    sal_uInt16          nKey;
};

struct OUStringEqFunc
{
    sal_Bool operator()( const rtl::OUString &r1,
                         const rtl::OUString &r2) const
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
typedef ::std::hash_map < ::rtl::OUString, ::vos::ORef <NameSpaceEntry >, rtl::OUStringHash, OUStringEqFunc > NameSpaceHash;
typedef ::std::map < sal_uInt16, ::vos::ORef < NameSpaceEntry >, uInt32lt > NameSpaceMap;

class XMLOFF_DLLPUBLIC SvXMLNamespaceMap
{
    const ::rtl::OUString       sXMLNS;
    const ::rtl::OUString       sEmpty;

    NameSpaceHash               aNameHash, aNameCache;
    NameSpaceMap                aNameMap;
    QNameCache                  aQNameCache;
    SAL_DLLPRIVATE sal_uInt16 _Add( const rtl::OUString& rPrefix, const rtl::OUString &rName, sal_uInt16 nKey );

public:

    SvXMLNamespaceMap();
    ~SvXMLNamespaceMap();

    SvXMLNamespaceMap( const SvXMLNamespaceMap& );

    void operator =( const SvXMLNamespaceMap& rCmp );
    int operator ==( const SvXMLNamespaceMap& rCmp ) const;

    sal_uInt16 Add( const ::rtl::OUString& rPrefix,
                const ::rtl::OUString& rName,
                sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
    sal_uInt16 AddIfKnown( const ::rtl::OUString& rPrefix,
                const ::rtl::OUString& rName );

    sal_uInt16 GetKeyByName( const ::rtl::OUString& rName ) const;
    const ::rtl::OUString& GetNameByKey( sal_uInt16 nKey ) const;

    sal_uInt16 GetKeyByPrefix( const ::rtl::OUString& rPrefix ) const;
    const ::rtl::OUString& GetPrefixByKey( sal_uInt16 nKey ) const;

    ::rtl::OUString GetQNameByKey( sal_uInt16 nKey,
                           const ::rtl::OUString& rLocalName,
                           sal_Bool bCache = sal_True) const;

    ::rtl::OUString GetAttrNameByKey( sal_uInt16 nKey ) const;

    /* This will replace the version with the unused 5th default parameter */
    sal_uInt16 _GetKeyByAttrName( const ::rtl::OUString& rAttrName,
                             ::rtl::OUString *pPrefix,
                             ::rtl::OUString *pLocalName,
                             ::rtl::OUString *pNamespace = 0,
                             sal_Bool bCache = sal_True) const;

    /* This will replace the version with the unused 3rd default parameter */
    sal_uInt16 _GetKeyByAttrName( const ::rtl::OUString& rAttrName,
                             ::rtl::OUString *pLocalName = 0,
                             sal_Bool bCache = sal_True) const;

    sal_uInt16 GetFirstKey() const;
    sal_uInt16 GetNextKey( sal_uInt16 nOldKey ) const;

    /* Give access to all namespace definitions, including multiple entries
       for the same key (needed for saving sheets separately in Calc).
       This might be replaced by a better interface later. */
    const NameSpaceHash& GetAllEntries() const { return aNameHash; }

    static sal_Bool NormalizeOasisURN( ::rtl::OUString& rName );
    static sal_Bool NormalizeW3URI( ::rtl::OUString& rName );
    static sal_Bool NormalizeURI( ::rtl::OUString& rName );

/* deprecated */ sal_Bool AddAtIndex( sal_uInt16 nIdx, const ::rtl::OUString& rPrefix,
                     const ::rtl::OUString& rName, sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
/* deprecated */ sal_Bool AddAtIndex( sal_uInt16 nIdx, const sal_Char *pPrefix,
                     const sal_Char *pName, sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
/* deprecated */ sal_uInt16 GetIndexByKey( sal_uInt16 nKey ) const;
/* deprecated */ sal_uInt16 GetKeyByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ sal_uInt16 GetIndexByPrefix( const ::rtl::OUString& rPrefix ) const;
/* deprecated */ sal_uInt16 GetFirstIndex() const;
/* deprecated */ sal_uInt16 GetNextIndex( sal_uInt16 nOldIdx ) const;
/* deprecated */ const ::rtl::OUString& GetPrefixByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ const ::rtl::OUString& GetNameByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ ::rtl::OUString GetAttrNameByIndex( sal_uInt16 nIdx ) const;
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

#endif  //  _XMLOFF_NMSPMAP_HXX
