/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_XMLOFF_NMSPMAP_HXX
#define INCLUDED_XMLOFF_NMSPMAP_HXX

#include <sal/config.h>

#include <map>
#include <unordered_map>
#include <utility>

#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
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
    OUString     sName;
    // sPrefix is the prefix used to declare a given item to be from a given namespace
    OUString     sPrefix;
    // nKey is the unique identifier of a namespace
    sal_uInt16          nKey;
};

struct uInt32lt
{
    bool operator()( const sal_uInt32 &r1, const sal_uInt32 &r2) const
    {
        return r1 < r2;
    }
};
typedef ::std::pair < sal_uInt16, OUString > QNamePair;

struct QNamePairHash
{
    size_t operator()( const QNamePair &r1 ) const
    {
        return (size_t) r1.second.hashCode() + r1.first;
    }
};

typedef std::unordered_map < QNamePair, OUString, QNamePairHash > QNameCache;
typedef std::unordered_map < OUString, ::rtl::Reference <NameSpaceEntry >, OUStringHash > NameSpaceHash;
typedef std::map < sal_uInt16, ::rtl::Reference < NameSpaceEntry >, uInt32lt > NameSpaceMap;

class XMLOFF_DLLPUBLIC SvXMLNamespaceMap
{
    const OUString       sXMLNS;
    const OUString       sEmpty;

    NameSpaceHash               aNameHash;
    mutable NameSpaceHash       aNameCache;
    NameSpaceMap                aNameMap;
    mutable QNameCache          aQNameCache;
    SAL_DLLPRIVATE sal_uInt16 Add_( const OUString& rPrefix, const OUString &rName, sal_uInt16 nKey );

public:

    SvXMLNamespaceMap();
    ~SvXMLNamespaceMap();

    SvXMLNamespaceMap( const SvXMLNamespaceMap& );

    void operator =( const SvXMLNamespaceMap& rCmp );
    bool operator ==( const SvXMLNamespaceMap& rCmp ) const;

    sal_uInt16 Add( const OUString& rPrefix,
                const OUString& rName,
                sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
    sal_uInt16 AddIfKnown( const OUString& rPrefix,
                const OUString& rName );

    sal_uInt16 GetKeyByName( const OUString& rName ) const;
    const OUString& GetNameByKey( sal_uInt16 nKey ) const;

    sal_uInt16 GetKeyByPrefix( const OUString& rPrefix ) const;
    const OUString& GetPrefixByKey( sal_uInt16 nKey ) const;

    OUString GetQNameByKey( sal_uInt16 nKey,
                           const OUString& rLocalName,
                           bool bCache = true) const;

    OUString GetAttrNameByKey( sal_uInt16 nKey ) const;

    /* This will replace the version with the unused 5th default parameter */
    sal_uInt16 GetKeyByAttrName_( const OUString& rAttrName,
                             OUString *pPrefix,
                             OUString *pLocalName,
                             OUString *pNamespace = nullptr,
                             bool bCache = true) const;

    /* This will replace the version with the unused 3rd default parameter */
    sal_uInt16 GetKeyByAttrName_( const OUString& rAttrName,
                             OUString *pLocalName = nullptr) const;

    sal_uInt16 GetFirstKey() const;
    sal_uInt16 GetNextKey( sal_uInt16 nOldKey ) const;

    /* Give access to all namespace definitions, including multiple entries
       for the same key (needed for saving sheets separately in Calc).
       This might be replaced by a better interface later. */
    const NameSpaceHash& GetAllEntries() const { return aNameHash; }

    static bool NormalizeOasisURN( OUString& rName );
    static bool NormalizeW3URI( OUString& rName );
    static bool NormalizeURI( OUString& rName );

/* deprecated */ void AddAtIndex( sal_uInt16 nIdx, const OUString& rPrefix,
                     const OUString& rName, sal_uInt16 nKey = XML_NAMESPACE_UNKNOWN );
/* deprecated */ static sal_uInt16 GetIndexByKey( sal_uInt16 nKey );
/* deprecated */ sal_uInt16 GetIndexByPrefix( const OUString& rPrefix ) const;
/* deprecated */ sal_uInt16 GetFirstIndex() const;
/* deprecated */ sal_uInt16 GetNextIndex( sal_uInt16 nOldIdx ) const;
/* deprecated */ const OUString& GetPrefixByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ const OUString& GetNameByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ OUString GetAttrNameByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ OUString GetQNameByIndex( sal_uInt16 nIdx,
                            const OUString& rLocalName ) const;
/* deprecated */ sal_uInt16 GetKeyByAttrName( const OUString& rAttrName,
                             OUString *pPrefix,
                             OUString *pLocalName,
                             OUString *pNamespace=nullptr,
                             sal_uInt16 nIdxGuess = USHRT_MAX ) const;
/* deprecated */ sal_uInt16 GetKeyByAttrName( const OUString& rAttrName,
                             OUString *pLocalName = nullptr,
                             sal_uInt16 nIdxGuess = USHRT_MAX ) const;
};

#endif // INCLUDED_XMLOFF_NMSPMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
