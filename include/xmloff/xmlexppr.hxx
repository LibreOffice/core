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

#ifndef INCLUDED_XMLOFF_XMLEXPPR_HXX
#define INCLUDED_XMLOFF_XMLEXPPR_HXX

#include <rtl/ref.hxx>
#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/xmlprmap.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

enum class SvXmlExportFlags {
    NONE        = 0x0000,
    DEFAULTS    = 0x0001,  // export also default items
    DEEP        = 0x0002,  // export also items from
                           // parent item sets
    EMPTY       = 0x0004,  // export attribs element
                           // even if its empty
    IGN_WS      = 0x0008
};
namespace o3tl
{
    template<> struct typed_flags<SvXmlExportFlags> : is_typed_flags<SvXmlExportFlags, 0xf> {};
}

class SvXMLUnitConverter;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
class FilterPropertiesInfos_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC SvXMLExportPropertyMapper : public salhelper::SimpleReferenceObject
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
    std::vector<XMLPropertyState> _Filter(
            const css::uno::Reference<css::beans::XPropertySet>& rPropSet,
            bool bDefault, bool bDisableFoFontFamily ) const;

    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            bool bEnableFoFontFamily,
            ::std::vector< XMLPropertyState >& rProperties,
            css::uno::Reference<css::beans::XPropertySet > rPropSet ) const;

    /** fills the given attribute list with the items in the given set */
    void _exportXML( sal_uInt16 nPropType, sal_uInt16& rPropTypeFlags,
                     SvXMLAttributeList& rAttrList,
                     const ::std::vector< XMLPropertyState >& rProperties,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     SvXmlExportFlags nFlags,
                     std::vector<sal_uInt16>* pIndexArray,
                       sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx ) const;

    void _exportXML( SvXMLAttributeList& rAttrList,
                     const XMLPropertyState& rProperty,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     SvXmlExportFlags nFlags,
                     const ::std::vector< XMLPropertyState > *pProperties = 0,
                     sal_uInt32 nIdx = 0 ) const;

    void exportElementItems(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            SvXmlExportFlags nFlags,
            const std::vector<sal_uInt16>& rIndexArray ) const;

public:

    SvXMLExportPropertyMapper(
            const rtl::Reference< XMLPropertySetMapper >& rMapper );
    virtual ~SvXMLExportPropertyMapper();

    // Add a ExportPropertyMapper at the end of the import mapper chain.
    // The added mapper MUST not be used outside the Mapper chain any longer,
    // because its PropertyMapper will be replaced.
    void ChainExportMapper(
        const rtl::Reference< SvXMLExportPropertyMapper>& rMapper );

    /** Filter all properties we don't want to export:
        Take all properties of the XPropertySet which are also found in the
        XMLPropertyMapEntry-array and which are not set directly (so, the value isn't
        default and isn't inherited)
        After this process It'll called 'Contextfilter' for application-specific
        filter-processes. */
    std::vector<XMLPropertyState> Filter(
        const css::uno::Reference<css::beans::XPropertySet>& rPropSet, bool bEnableFoFontFamily = false ) const;

    /** Like Filter(), except that:
      * - only properties that have the map flag MID_FLAG_DEFAULT_ITEM_EXPORT
      *   set are exported,
      * - instead of the property's value, its default value is exported.
      */
    std::vector<XMLPropertyState> FilterDefaults(
        const css::uno::Reference<css::beans::XPropertySet>& rPropSet, bool bEnableFoFontFamily = false ) const;

    /** Compare to arrays of XMLPropertyState */
    bool Equals( const ::std::vector< XMLPropertyState >& aProperties1,
                     const ::std::vector< XMLPropertyState >& aProperties2 ) const;

    void exportXML(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            SvXmlExportFlags nFlags = SvXmlExportFlags::NONE,
            bool bUseExtensionNamespaceForGraphicProperties = false ) const;

    /** like above but only properties whose property map index is within the
     *  specified range are exported
     *
     * @param bExtensionNamespace use the extension namespace for graphic-properties
     */
    void exportXML(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
            SvXmlExportFlags nFlags = SvXmlExportFlags::NONE, bool bExtensionNamespace = false ) const;

    /** this method is called for every item that has the
        MID_FLAG_ELEMENT_EXPORT flag set */
    virtual void handleElementItem(
            SvXMLExport& rExport,
            const XMLPropertyState& rProperty,
            SvXmlExportFlags nFlags,
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

    const rtl::Reference<XMLPropertySetMapper>& getPropertySetMapper() const;

    void SetStyleName( const OUString& rStyleName );
    const OUString& GetStyleName() const;
};

#endif // INCLUDED_XMLOFF_XMLEXPPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
