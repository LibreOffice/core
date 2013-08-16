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

#ifndef _XMLOFF_XMLEXPPR_HXX
#define _XMLOFF_XMLEXPPR_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/xmlprmap.hxx>
#include <xmloff/uniref.hxx>


class SvXMLUnitConverter;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
class FilterPropertiesInfos_Impl;
class SvXMLExport;

#define XML_EXPORT_FLAG_DEFAULTS    0x0001      // export also default items
#define XML_EXPORT_FLAG_DEEP        0x0002      // export also items from
                                                // parent item sets
#define XML_EXPORT_FLAG_EMPTY       0x0004      // export attribs element
                                                // even if its empty
#define XML_EXPORT_FLAG_IGN_WS      0x0008

class XMLOFF_DLLPUBLIC SvXMLExportPropertyMapper : public UniRefBase
{
    struct Impl;
    Impl* mpImpl;

protected:

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
    void _exportXML( sal_uInt16 nPropType, sal_uInt16& rPropTypeFlags,
                     SvXMLAttributeList& rAttrList,
                     const ::std::vector< XMLPropertyState >& rProperties,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     sal_uInt16 nFlags,
                     std::vector<sal_uInt16>* pIndexArray,
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
            const std::vector<sal_uInt16>& rIndexArray ) const;

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

    /** Like Filter(), except that:
      * - only properties that have the map flag MID_FLAG_DEFAULT_ITEM_EXPORT
      *   set are exported,
      * - instead of the property's value, its default value is exported.
      */
    ::std::vector< XMLPropertyState > FilterDefaults(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const
                    { return _Filter(rPropSet, sal_True); }

    /** Compare to arrays of XMLPropertyState */
    sal_Bool Equals( const ::std::vector< XMLPropertyState >& aProperties1,
                     const ::std::vector< XMLPropertyState >& aProperties2 ) const;

    /** fills the given attribute list with the items in the given set */
    void exportXML(
           SvXMLAttributeList& rAttrList,
           const ::std::vector< XMLPropertyState >& rProperties,
           const SvXMLUnitConverter& rUnitConverter,
           const SvXMLNamespaceMap& rNamespaceMap,
           sal_uInt16 nFlags = 0 ) const;
    /** like above but only properties whose property map index is within the
        specified range are exported */
    void exportXML(
           SvXMLAttributeList& rAttrList,
           const ::std::vector< XMLPropertyState >& rProperties,
           const SvXMLUnitConverter& rUnitConverter,
           const SvXMLNamespaceMap& rNamespaceMap,
           sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
           sal_uInt16 nFlags = 0 ) const;

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

    const UniReference<XMLPropertySetMapper>& getPropertySetMapper() const;

    void SetStyleName( const OUString& rStyleName );
    const OUString& GetStyleName() const;
};

#endif  //  _XMLOFF_XMLEXPPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
