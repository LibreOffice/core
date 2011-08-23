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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif


#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif

namespace rtl { class OUString; }
namespace binfilter {

class SfxPoolItem;
class SfxItemSet; 

class SvXMLUnitConverter; 
class SvXMLAttributeList; 
class SvXMLNamespaceMap; 
struct SvXMLItemMapEntry;

class SvXMLImportItemMapper
{
protected:
    SvXMLItemMapEntriesRef mrMapEntries;
    USHORT nUnknownWhich;

public:
    SvXMLImportItemMapper( SvXMLItemMapEntriesRef rMapEntries ,
                           USHORT nUnknWhich=USHRT_MAX );
    virtual ~SvXMLImportItemMapper();

    /** fills the given itemset with the attributes in the given list */
    void importXML( SfxItemSet& rSet,
                    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList,
                    const SvXMLUnitConverter& rUnitConverter,
                    const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual BOOL handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                    SfxPoolItem& rItem,
                                    SfxItemSet& rSet,
                                    const ::rtl::OUString& rValue,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** this method is called for every item that has the
        MID_FLAG_NO_ITEM_IMPORT flag set */
    virtual BOOL handleNoItem( const SvXMLItemMapEntry& rEntry,
                               SfxItemSet& rSet,
                               const ::rtl::OUString& rValue,
                               const SvXMLUnitConverter& rUnitConverter,
                               const SvXMLNamespaceMap& rNamespaceMap ) const;
    
    /** This method is called when all attributes have benn processed. It
      * may be used to remove items that are incomplete */
    virtual void finished( SfxItemSet& rSet ) const;

    inline void setMapEntries( SvXMLItemMapEntriesRef rMapEntries );
    inline SvXMLItemMapEntriesRef getMapEntries() const;


    /** This method is called for every item that should be set based
        upon an XML attribute value. */
    static sal_Bool PutXMLValue( 
        SfxPoolItem& rItem,
        const ::rtl::OUString& rValue, 
        sal_uInt16 nMemberId,
        const SvXMLUnitConverter& rUnitConverter );
};

inline void
SvXMLImportItemMapper::setMapEntries( SvXMLItemMapEntriesRef rMapEntries )
{
    mrMapEntries = rMapEntries;
}

inline SvXMLItemMapEntriesRef
SvXMLImportItemMapper::getMapEntries() const
{
    return mrMapEntries;
}


} //namespace binfilter
#endif	//  _XMLIMPIT_HXX
