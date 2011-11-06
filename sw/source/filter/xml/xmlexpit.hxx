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



#ifndef _XMLITMPR_HXX
#define _XMLITMPR_HXX

#include <tools/solar.h>
#include <tools/ref.hxx>
#include "xmlitmap.hxx"

#define XML_EXPORT_FLAG_DEFAULTS    0x0001      // export also default items
#define XML_EXPORT_FLAG_DEEP        0x0002      // export also items from
                                                // parent item sets
#define XML_EXPORT_FLAG_EMPTY       0x0004      // export attribs element
                                                // even if its empty
#define XML_EXPORT_FLAG_IGN_WS      0x0008

namespace rtl { class OUString; }
class SvXMLUnitConverter;
class SfxPoolItem;
class SfxItemSet;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
class SvUShorts;
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
                    SvUShorts* pIndexArray ) const;

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
                              const SvUShorts& rIndexArray ) const;

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

    static sal_Bool QueryXMLValue( const SfxPoolItem& rItem,
                                 ::rtl::OUString& rValue, sal_uInt16 nMemberId,
                                 const SvXMLUnitConverter& rUnitConverter );
};

inline void
SvXMLExportItemMapper::setMapEntries( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = rMapEntries;
}

#endif  //  _XMLITMPR_HXX
