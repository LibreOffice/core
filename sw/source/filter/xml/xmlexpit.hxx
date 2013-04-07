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

#ifndef _XMLITMPR_HXX
#define _XMLITMPR_HXX

#include <tools/solar.h>
#include "xmlitmap.hxx"
#include <vector>

#define XML_EXPORT_FLAG_DEFAULTS    0x0001      // export also default items
#define XML_EXPORT_FLAG_DEEP        0x0002      // export also items from
                                                // parent item sets
#define XML_EXPORT_FLAG_EMPTY       0x0004      // export attribs element
                                                // even if its empty
#define XML_EXPORT_FLAG_IGN_WS      0x0008

class SvXMLUnitConverter;
class SfxPoolItem;
class SfxItemSet;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
class SvXMLExport;


class SvXMLExportItemMapper
{
protected:
    SvXMLItemMapEntriesRef mrMapEntries;

    /** fills the given attribute list with the items in the given set */
    void exportXML( const SvXMLExport& rExport,
                    SvXMLAttributeList& rAttrList,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap,
                    sal_uInt16 nFlags,
                    std::vector<sal_uInt16> *pIndexArray ) const;

    void exportXML( const SvXMLExport& rExport,
                    SvXMLAttributeList& rAttrList,
                    const SfxPoolItem& rItem,
                    const SvXMLItemMapEntry &rEntry,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap,
                    sal_uInt16 nFlags,
                    const SfxItemSet *pSet ) const;


    void exportElementItems(  SvXMLExport& rExport,
                              const SvXMLUnitConverter& rUnitConverter,
                              const SfxItemSet &rSet,
                              sal_uInt16 nFlags,
                              const std::vector<sal_uInt16> &rIndexArray ) const;

    static const SfxPoolItem* GetItem( const SfxItemSet &rSet,
                                       sal_uInt16 nWhichId,
                                       sal_uInt16 nFlags );

public:
    SvXMLExportItemMapper( SvXMLItemMapEntriesRef rMapEntries );
    virtual ~SvXMLExportItemMapper();

    void exportXML( SvXMLExport& rExport,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    ::xmloff::token::XMLTokenEnum ePropToken,
                    sal_uInt16 nFlags = 0 ) const;

    /** this method is called for every item that has the
        MID_SW_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem( SvXMLAttributeList& rAttrList,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap,
                                    const SfxItemSet *pSet = NULL ) const;


    /** this method is called for every item that has the
        MID_SW_FLAG_NO_ITEM_EXPORT flag set */
    virtual void handleNoItem( SvXMLAttributeList& rAttrList,
                               const SvXMLItemMapEntry& rEntry,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap,
                               const SfxItemSet& rSet ) const;

    /** this method is called for every item that has the
        MID_SW_FLAG_ELEMENT_EXPORT flag set */
    virtual void handleElementItem( SvXMLExport& rExport,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SfxItemSet& rSet,
                                    sal_uInt16 nFlags ) const;

    inline void setMapEntries( SvXMLItemMapEntriesRef rMapEntries );

    static bool QueryXMLValue( const SfxPoolItem& rItem,
                                 OUString& rValue, sal_uInt16 nMemberId,
                                 const SvXMLUnitConverter& rUnitConverter );
};

inline void
SvXMLExportItemMapper::setMapEntries( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = rMapEntries;
}

#endif  //  _XMLITMPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
