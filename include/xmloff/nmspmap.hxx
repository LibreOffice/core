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

#include <unordered_map>
#include <utility>

#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <limits.h>

const sal_uInt16 XML_NAMESPACE_XMLNS         = USHRT_MAX-2;
const sal_uInt16 XML_NAMESPACE_NONE          = USHRT_MAX-1;
const sal_uInt16 XML_NAMESPACE_UNKNOWN       = USHRT_MAX;
const sal_uInt16 XML_NAMESPACE_UNKNOWN_FLAG  = 0x8000;

class NameSpaceEntry final : public salhelper::SimpleReferenceObject
{
public:
    // sName refers to the full namespace name
    OUString     sName;
    // sPrefix is the prefix used to declare a given item to be from a given namespace
    OUString     sPrefix;
    // nKey is the unique identifier of a namespace
    sal_uInt16   nKey;
};

typedef ::std::pair < sal_uInt16, OUString > QNamePair;

struct QNamePairHash
{
    size_t operator()( const QNamePair &r1 ) const
    {
        size_t hash = 17;
        hash = hash * 37 + r1.first;
        hash = hash * 37 + r1.second.hashCode();
        return hash;
    }
};

typedef std::unordered_map < QNamePair, OUString, QNamePairHash > QNameCache;
typedef std::unordered_map < OUString, ::rtl::Reference <NameSpaceEntry > > NameSpaceHash;
typedef std::unordered_map < sal_uInt16, ::rtl::Reference < NameSpaceEntry > > NameSpaceMap;

class XMLOFF_DLLPUBLIC SvXMLNamespaceMap
{
    OUString                    sXMLNS;

    NameSpaceHash               aNameHash;
    mutable NameSpaceHash       aNameCache;
    NameSpaceMap                aNameMap;
    mutable QNameCache          aQNameCache;
    SAL_DLLPRIVATE sal_uInt16 Add_( const OUString& rPrefix, const OUString &rName, sal_uInt16 nKey );

public:

    SvXMLNamespaceMap();
    ~SvXMLNamespaceMap();

    SvXMLNamespaceMap( const SvXMLNamespaceMap& );

    SvXMLNamespaceMap& operator =( const SvXMLNamespaceMap& rCmp );
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

    enum class QNameMode { AttrNameCached, AttrValue };
    sal_uInt16 GetKeyByQName(const OUString& rQName,
                             OUString *pPrefix,
                             OUString *pLocalName,
                             OUString *pNamespace,
                             QNameMode eMode) const;

    sal_uInt16 GetKeyByAttrValueQName(const OUString& rAttrName,
                             OUString *pLocalName) const;

    sal_uInt16 GetFirstKey() const;
    sal_uInt16 GetNextKey( sal_uInt16 nOldKey ) const;

    /* Give access to all namespace definitions, including multiple entries
       for the same key (needed for saving sheets separately in Calc).
       This might be replaced by a better interface later. */
    const NameSpaceHash& GetAllEntries() const { return aNameHash; }

    static bool NormalizeOasisURN( OUString& rName );
    static bool NormalizeW3URI( OUString& rName );
    static bool NormalizeURI( OUString& rName );

/* deprecated */ void AddAtIndex( const OUString& rPrefix,
                     const OUString& rName, sal_uInt16 nKey );
/* deprecated */ static sal_uInt16 GetIndexByKey( sal_uInt16 nKey );
/* deprecated */ sal_uInt16 GetIndexByPrefix( const OUString& rPrefix ) const;
/* deprecated */ sal_uInt16 GetFirstIndex() const;
/* deprecated */ sal_uInt16 GetNextIndex( sal_uInt16 nOldIdx ) const;
/* deprecated */ const OUString& GetPrefixByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ const OUString& GetNameByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ OUString GetAttrNameByIndex( sal_uInt16 nIdx ) const;
/* deprecated */ OUString GetQNameByIndex( sal_uInt16 nIdx,
                            const OUString& rLocalName ) const;
                 sal_uInt16 GetKeyByAttrName( const OUString& rAttrName,
                             OUString *pPrefix,
                             OUString *pLocalName,
                             OUString *pNamespace ) const;
/* deprecated */ sal_uInt16 GetKeyByAttrName( const OUString& rAttrName,
                             OUString *pLocalName = nullptr ) const;
};

#endif // INCLUDED_XMLOFF_NMSPMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
