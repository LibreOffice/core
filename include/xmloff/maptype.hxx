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

#ifndef INCLUDED_XMLOFF_MAPTYPE_HXX
#define INCLUDED_XMLOFF_MAPTYPE_HXX

#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmltoken.hxx>
#include <unotools/saveopt.hxx>


/** Represents a property with its API-name, its XML-name and the type of
    its value.
*/
struct XMLPropertyMapEntry
{
    static constexpr OUStringLiteral EMPTY = u"";

    rtl::OUStringConstExpr msApiName;      /// Property-Name
    enum ::xmloff::token::XMLTokenEnum meXMLName;       /// XML-Name
    sal_uInt16      mnNameSpace;    /** declares the Namespace in which this
                                        property exists */

    /**
     * The lowest 14 bits specify the basic XML type of the property value, of
     * which the 11th bit specifies the application type.  The basic type has
     * the 11th bit off and the 14th bit on.  For the most part, the lower 14
     * bits are used as a single value.
     *
     * <p>The next 4 bits specify the family type of the property value. This
     * can be one of the following:</p>
     *
     * <ul>
     *     <li>XML_TYPE_PROP_GRAPHIC</li>
     *     <li>XML_TYPE_PROP_DRAWING_PAGE</li>
     *     <li>XML_TYPE_PROP_PAGE_LAYOUT</li>
     *     <li>XML_TYPE_PROP_HEADER_FOOTER</li>
     *     <li>XML_TYPE_PROP_TEXT</li>
     *     <li>XML_TYPE_PROP_PARAGRAPH</li>
     *     <li>XML_TYPE_PROP_RUBY</li>
     *     <li>XML_TYPE_PROP_SECTION</li>
     *     <li>XML_TYPE_PROP_TABLE</li>
     *     <li>XML_TYPE_PROP_TABLE_COLUMN</li>
     *     <li>XML_TYPE_PROP_TABLE_ROW</li>
     *     <li>XML_TYPE_PROP_TABLE_CELL</li>
     *     <li>XML_TYPE_PROP_LIST_LEVEL</li>
     *     <li>XML_TYPE_PROP_CHART</li>
     * </ul>
     *
     * <p>The next 2 bits are not used.</p>
     *
     * <p>The last 12 bits specify additional rules on how to special-case the
     * value during import and export.  This value may be a combination of the
     * following flags:</p>
     *
     * <ul>
     *     <li>MID_FLAG_PROPERTY_MAY_THROW</li>
     *     <li>MID_FLAG_DEFAULT_ITEM_EXPORT</li>
     *     <li>MID_FLAG_MUST_EXIST</li>
     *     <li>MID_FLAG_MERGE_ATTRIBUTE</li>
     *     <li>MID_FLAG_MERGE_PROPERTY</li>
     *     <li>MID_FLAG_MULTI_PROPERTY</li>
     *     <li>MID_FLAG_ELEMENT_ITEM_IMPORT</li>
     *     <li>MID_FLAG_ELEMENT_ITEM_EXPORT</li>
     *     <li>MID_FLAG_SPECIAL_ITEM_IMPORT</li>
     *     <li>MID_FLAG_SPECIAL_ITEM_EXPORT</li>
     *     <li>MID_FLAG_NO_PROPERTY_IMPORT</li>
     *     <li>MID_FLAG_NO_PROPERTY_EXPORT</li>
     * </ul>
     */
    sal_uInt32 mnType;

    sal_Int16       mnContextId;    /// User defined id for context filtering
    /** no export to standard namespace when the used ODF version is lower than this;
        no export to extension namespace when the used ODF version is at least this
      */
    SvtSaveOptions::ODFSaneDefaultVersion mnEarliestODFVersionForExport;

    /** Flag to specify whether entry is only used during import.

        Allows to handle more than one Namespace/XML-Name to Property-Name
        mapping, i.e. for extensions. If several entries for the same
        Property-Name exist, all except one must have this flag set.
     */
    bool            mbImportOnly;

    template< std::size_t N >
    constexpr XMLPropertyMapEntry(
            const OUStringLiteral<N> & sApiName,
            sal_uInt16      nNameSpace,
            enum ::xmloff::token::XMLTokenEnum eXMLName,
            sal_uInt32 nType,
            sal_Int16       nContextId,
            SvtSaveOptions::ODFSaneDefaultVersion nEarliestODFVersionForExport,
            bool            bImportOnly)
        :
        msApiName(sApiName),
        meXMLName(eXMLName), mnNameSpace(nNameSpace), mnType(nType),
        mnContextId(nContextId), mnEarliestODFVersionForExport(nEarliestODFVersionForExport),
        mbImportOnly(bImportOnly)
    {}
    XMLPropertyMapEntry(
            std::nullptr_t ,
            sal_uInt16      nNameSpace,
            enum ::xmloff::token::XMLTokenEnum eXMLName,
            sal_uInt32 nType,
            sal_Int16       nContextId,
            SvtSaveOptions::ODFSaneDefaultVersion nEarliestODFVersionForExport,
            bool            bImportOnly)
        :
        msApiName(EMPTY), meXMLName(eXMLName), mnNameSpace(nNameSpace), mnType(nType),
        mnContextId(nContextId), mnEarliestODFVersionForExport(nEarliestODFVersionForExport),
        mbImportOnly(bImportOnly)
    {}
};


/** Smart struct to transport an Any with an index to the appropriate
    property-name
*/
struct XMLPropertyState
{
    sal_Int32                  mnIndex;
    css::uno::Any              maValue;

    XMLPropertyState( sal_Int32 nIndex )
        : mnIndex( nIndex ) {}
    XMLPropertyState( sal_Int32 nIndex, const css::uno::Any& rValue )
        : mnIndex( nIndex ), maValue( rValue ) {}
};

#endif // INCLUDED_XMLOFF_MAPTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
