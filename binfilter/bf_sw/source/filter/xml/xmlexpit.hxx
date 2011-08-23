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

#ifndef _XMLITMPR_HXX
#define _XMLITMPR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif

#define XML_EXPORT_FLAG_DEFAULTS	0x0001		// export also default items
#define XML_EXPORT_FLAG_DEEP		0x0002		// export also items from
                                                // parent item sets
#define XML_EXPORT_FLAG_EMPTY		0x0004		// export attribs element
                                                // even if its empty
#define XML_EXPORT_FLAG_IGN_WS		0x0008

namespace rtl { class OUString; }

namespace binfilter {
class SvUShorts; 
class SfxPoolItem;
class SfxItemSet; 
class SvXMLUnitConverter; 
class SvXMLAttributeList; 
class SvXMLNamespaceMap; 
class SvXMLExport; 

class SvXMLExportItemMapper
{
protected:
    SvXMLItemMapEntriesRef mrMapEntries;

    /** fills the given attribute list with the items in the given set */
    void exportXML( SvXMLAttributeList& rAttrList,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap,
                    sal_uInt16 nFlags,
                    SvUShorts* pIndexArray ) const;

    void exportXML( SvXMLAttributeList& rAttrList,
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
                                       sal_uInt32 nWhichId,
                                       sal_uInt16 nFlags );

public:
    SvXMLExportItemMapper( SvXMLItemMapEntriesRef rMapEntries );
    virtual ~SvXMLExportItemMapper();

    /** fills the given attribute list with the representation of one
        item */
    void exportXML( SvXMLExport& rExport,
                    const SfxItemSet& rSet,
                    const SvXMLUnitConverter& rUnitConverter,
                    sal_uInt16 nFlags = 0 ) const;

    /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem( SvXMLAttributeList& rAttrList,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap,
                                    const SfxItemSet *pSet = NULL ) const;


    /** this method is called for every item that has the
        MID_FLAG_NO_ITEM_EXPORT flag set */
    virtual void handleNoItem( SvXMLAttributeList& rAttrList,
                               const SvXMLItemMapEntry& rEntry,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap,
                               const SfxItemSet& rSet ) const;

    /** this method is called for every item that has the
        MID_FLAG_ELEMENT_EXPORT flag set */
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

} //namespace binfilter
#endif	//  _XMLITMPR_HXX
