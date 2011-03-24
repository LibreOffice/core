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
#ifndef _XMLIMPIT_HXX
#define _XMLIMPIT_HXX

#include <limits.h>
#include <tools/solar.h>
#include <tools/ref.hxx>


#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include "xmlitmap.hxx"

namespace rtl { class OUString; }

class SvXMLUnitConverter;
class SfxPoolItem;
class SfxItemSet;
class SvXMLNamespaceMap;
struct SvXMLItemMapEntry;

class SvXMLImportItemMapper
{
protected:
    SvXMLItemMapEntriesRef mrMapEntries;
    sal_uInt16 nUnknownWhich;

public:
    SvXMLImportItemMapper( SvXMLItemMapEntriesRef rMapEntries ,
                           sal_uInt16 nUnknWhich=USHRT_MAX );
    virtual ~SvXMLImportItemMapper();

    /** fills the given itemset with the attributes in the given list */
    void importXML( SfxItemSet& rSet,
                    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap );

    /** this method is called for every item that has the
        MID_SW_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual sal_Bool handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                    SfxPoolItem& rItem,
                                    SfxItemSet& rSet,
                                    const ::rtl::OUString& rValue,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap );

    /** this method is called for every item that has the
        MID_SW_FLAG_NO_ITEM_IMPORT flag set */
    virtual sal_Bool handleNoItem( const SvXMLItemMapEntry& rEntry,
                               SfxItemSet& rSet,
                               const ::rtl::OUString& rValue,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap );

    /** This method is called when all attributes have benn processed. It
      * may be used to remove items that are incomplete */
    virtual void finished(SfxItemSet & rSet,
                          SvXMLUnitConverter const& rUnitConverter) const;

    virtual void setMapEntries( SvXMLItemMapEntriesRef rMapEntries );
    inline SvXMLItemMapEntriesRef getMapEntries() const;


    /** This method is called for every item that should be set based
        upon an XML attribute value. */
    static sal_Bool PutXMLValue(
        SfxPoolItem& rItem,
        const ::rtl::OUString& rValue,
        sal_uInt16 nMemberId,
        const SvXMLUnitConverter& rUnitConverter );
};

inline SvXMLItemMapEntriesRef
SvXMLImportItemMapper::getMapEntries() const
{
    return mrMapEntries;
}


#endif  //  _XMLIMPIT_HXX
