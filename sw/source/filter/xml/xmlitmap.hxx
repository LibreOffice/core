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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLITMAP_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLITMAP_HXX

#include <sal/types.h>
#include <tools/ref.hxx>
#include <xmloff/xmltoken.hxx>
#include <memory>

#define MID_SW_FLAG_MASK                    0x0000ffff

// this flags are used in the item mapper for import and export
#define MID_SW_FLAG_SPECIAL_ITEM_IMPORT 0x80000000
#define MID_SW_FLAG_NO_ITEM_IMPORT          0x40000000
#define MID_SW_FLAG_SPECIAL_ITEM_EXPORT 0x20000000
#define MID_SW_FLAG_NO_ITEM_EXPORT          0x10000000
#define MID_SW_FLAG_ELEMENT_ITEM_IMPORT 0x08000000
#define MID_SW_FLAG_ELEMENT_ITEM_EXPORT 0x04000000
#define MID_SW_FLAG_ELEMENT_ITEM            0x0c000000  // both import and export

struct SvXMLItemMapEntry
{
    sal_uInt16 const nNameSpace;      // declares the Namespace in which this item
                                // exists
    sal_uInt16 nWhichId;        // the WhichId to identify the item
                                // in the pool
    enum ::xmloff::token::XMLTokenEnum const eLocalName;
                                // the local name for the item inside
                                // the Namespace (as an XMLTokenEnum)
    sal_uInt32 const nMemberId;       // the memberid specifies which part
                                // of the item should be imported or
                                // exported with this Namespace
                                // and localName
    SvXMLItemMapEntry(
        sal_uInt16 nameSpace,
        enum ::xmloff::token::XMLTokenEnum localName,
        sal_uInt16 whichId,
        sal_uInt32 memberId)
    : nNameSpace(nameSpace), nWhichId(whichId), eLocalName(localName), nMemberId(memberId) {}
};

class SvXMLItemMapEntries_impl;

/** this class manages an array of SvXMLItemMapEntry. It is
    used for optimizing the static array on startup of import
    or export */
class SvXMLItemMapEntries final : public SvRefBase
{
    std::unique_ptr<SvXMLItemMapEntries_impl> mpImpl;

public:
    explicit SvXMLItemMapEntries(SvXMLItemMapEntry const * pEntrys);
    virtual ~SvXMLItemMapEntries() override;

    SvXMLItemMapEntry const * getByName( sal_uInt16 nNameSpace,
                                  const OUString& rString ) const;
    SvXMLItemMapEntry const & getByIndex( sal_uInt16 nIndex ) const;

    sal_uInt16 getCount() const;
};

typedef tools::SvRef<SvXMLItemMapEntries> SvXMLItemMapEntriesRef;

extern SvXMLItemMapEntry const aXMLTableItemMap[];
extern SvXMLItemMapEntry const aXMLTableColItemMap[];
extern SvXMLItemMapEntry const aXMLTableRowItemMap[];
extern SvXMLItemMapEntry const aXMLTableCellItemMap[];

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLITMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
