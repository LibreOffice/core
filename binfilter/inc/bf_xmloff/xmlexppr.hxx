/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_XMLEXPPR_HXX
#define _XMLOFF_XMLEXPPR_HXX

#include <bf_xmloff/xmlprmap.hxx>
#include <bf_xmloff/uniref.hxx>

namespace rtl { class OUString; }
namespace binfilter {
class SvUShorts;

class SvXMLUnitConverter;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
class FilterPropertiesInfos_Impl;
class SvXMLExport;

#define XML_EXPORT_FLAG_DEFAULTS	0x0001		// export also default items
#define XML_EXPORT_FLAG_DEEP		0x0002		// export also items from
                                                // parent item sets
#define XML_EXPORT_FLAG_EMPTY		0x0004		// export attribs element
                                                // even if its empty
#define XML_EXPORT_FLAG_IGN_WS		0x0008

class SvXMLExportPropertyMapper : public UniRefBase
{
    UniReference< SvXMLExportPropertyMapper> mxNextMapper;

    FilterPropertiesInfos_Impl *pCache;

protected:
    UniReference< XMLPropertySetMapper > maPropMapper;

    /** Filter all properties we don't want to export:
        Take all properties of the XPropertySet which are also found in the
        XMLPropertyMapEntry-array and which are not set directly (so, the value isn't
        default and isn't inherited, apart from bDefault is true)
        After this process It'll called 'Contextfilter' for application-specific
        filter-processes. */
    ::std::vector< XMLPropertyState > _Filter(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet,
            const sal_Bool bDefault ) const;

    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const;

    /** fills the given attribute list with the items in the given set */
    void _exportXML( SvXMLAttributeList& rAttrList,
                     const ::std::vector< XMLPropertyState >& rProperties,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     sal_uInt16 nFlags,
                     SvUShorts* pIndexArray,
                       sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx ) const;

    void _exportXML( SvXMLAttributeList& rAttrList,
                     const XMLPropertyState& rProperty,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     sal_uInt16 nFlags,
                     const ::std::vector< XMLPropertyState > *pProperties = 0,
                     sal_uInt32 nIdx = 0 ) const;

    void exportElementItems(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_uInt16 nFlags,
            const SvUShorts& rIndexArray ) const;

public:

    SvXMLExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~SvXMLExportPropertyMapper();

    // Add a ExportPropertyMapper at the end of the import mapper chain.
    // The added mapper MUST not be used outside the Mapper chain any longer,
    // because its PropertyMapper will be replaced.
    void ChainExportMapper(
        const UniReference< SvXMLExportPropertyMapper>& rMapper );

    /** Filter all properties we don't want to export:
        Take all properties of the XPropertySet which are also found in the
        XMLPropertyMapEntry-array and which are not set directly (so, the value isn't
        default and isn't inherited)
        After this process It'll called 'Contextfilter' for application-specific
        filter-processes. */
    ::std::vector< XMLPropertyState > Filter(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const
                    { return _Filter(rPropSet, sal_False); }

    /** Like Filter(), excepti that:
      * - only properties that have the map flag MID_FLAG_DEFAULT_ITEM_EXPORT
      *   set are exported,
      * - instead of the property's value, its defualt value is exported.
      */
    ::std::vector< XMLPropertyState > FilterDefaults(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const
                    { return _Filter(rPropSet, sal_True); }

    /** Compare to arrays of XMLPropertyState */
    sal_Bool Equals( const ::std::vector< XMLPropertyState >& aProperties1,
                     const ::std::vector< XMLPropertyState >& aProperties2 ) const;

    /** fills the given attribute list with the representation of one item */
    void exportXML(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_uInt16 nFlags = 0 ) const;

    /** like above but only properties whose property map index is within the
        specified range are exported */
    void exportXML(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
            sal_uInt16 nFlags = 0 ) const;

    /** this method is called for every item that has the
        MID_FLAG_ELEMENT_EXPORT flag set */
    virtual void handleElementItem(
            SvXMLExport& rExport,
            const XMLPropertyState& rProperty,
            sal_uInt16 nFlags,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;

    /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;

    inline const UniReference< XMLPropertySetMapper >&
        getPropertySetMapper() const { return maPropMapper; }

};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLEXPPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
