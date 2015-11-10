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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLEXPIT_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLEXPIT_HXX

#include <tools/solar.h>
#include <xmloff/xmlexppr.hxx>
#include "xmlitmap.hxx"
#include <vector>

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
                    SvXmlExportFlags nFlags,
                    std::vector<sal_uInt16> *pIndexArray ) const;

    void exportXML( const SvXMLExport& rExport,
                    SvXMLAttributeList& rAttrList,
                    const SfxPoolItem& rItem,
                    const SvXMLItemMapEntry &rEntry,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap,
                    SvXmlExportFlags nFlags,
                    const SfxItemSet *pSet ) const;

    void exportElementItems(  SvXMLExport& rExport,
                              const SvXMLUnitConverter& rUnitConverter,
                              const SfxItemSet &rSet,
                              SvXmlExportFlags nFlags,
                              const std::vector<sal_uInt16> &rIndexArray ) const;

    static const SfxPoolItem* GetItem( const SfxItemSet &rSet,
                                       sal_uInt16 nWhichId,
                                       SvXmlExportFlags nFlags );

public:
    explicit SvXMLExportItemMapper( SvXMLItemMapEntriesRef rMapEntries );
    virtual ~SvXMLExportItemMapper();

    void exportXML( SvXMLExport& rExport,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    ::xmloff::token::XMLTokenEnum ePropToken,
                    SvXmlExportFlags nFlags = SvXmlExportFlags::NONE ) const;

    /** this method is called for every item that has the
        MID_SW_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem( SvXMLAttributeList& rAttrList,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap,
                                    const SfxItemSet *pSet = nullptr ) const;

    /** this method is called for every item that has the
        MID_SW_FLAG_NO_ITEM_EXPORT flag set */
    static void handleNoItem( SvXMLAttributeList& rAttrList,
                               const SvXMLItemMapEntry& rEntry,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap,
                               const SfxItemSet& rSet );

    /** this method is called for every item that has the
        MID_SW_FLAG_ELEMENT_EXPORT flag set */
    virtual void handleElementItem( SvXMLExport& rExport,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SfxItemSet& rSet,
                                    SvXmlExportFlags nFlags ) const;

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

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLEXPIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
