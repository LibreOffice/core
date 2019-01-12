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

#ifndef INCLUDED_XMLOFF_XMLPRMAP_HXX
#define INCLUDED_XMLOFF_XMLPRMAP_HXX

#include <xmloff/dllapi.h>
#include <unotools/saveopt.hxx>
#include <rtl/ustring.hxx>

#include <salhelper/simplereferenceobject.hxx>
#include <memory>

namespace rtl { template <class reference_type> class Reference; }

class SvXMLUnitConverter;
class XMLPropertyHandler;
class XMLPropertyHandlerFactory;
struct XMLPropertyMapEntry;
struct XMLPropertyState;

class XMLOFF_DLLPUBLIC XMLPropertySetMapper : public salhelper::SimpleReferenceObject
{
    struct Impl;

    std::unique_ptr<Impl> mpImpl;

    XMLPropertySetMapper( const XMLPropertySetMapper& ) = delete;
    XMLPropertySetMapper& operator= ( const XMLPropertySetMapper& ) = delete;

public:
    /** The last element of the XMLPropertyMapEntry-array must contain NULL-values.

        @param  bForExport
                If TRUE, only entries that have the mbImportOnly flag not set
                will be in the mappings.
      */
    XMLPropertySetMapper(
        const XMLPropertyMapEntry* pEntries,
        const rtl::Reference<XMLPropertyHandlerFactory>& rFactory,
        bool bForExport );

    virtual ~XMLPropertySetMapper() override;

    void AddMapperEntry( const rtl::Reference < XMLPropertySetMapper >& rMapper );

    /** Return number of entries in input-array */
    sal_Int32 GetEntryCount() const;

    /** Returns the flags of an entry */
    sal_uInt32 GetEntryFlags( sal_Int32 nIndex ) const;

    /** Returns the type of an entry */
    sal_uInt32 GetEntryType( sal_Int32 nIndex ) const;

    /** Returns the namespace-key of an entry */
    sal_uInt16 GetEntryNameSpace( sal_Int32 nIndex ) const;

    /** Returns the 'local' XML-name of the entry */
    const OUString& GetEntryXMLName( sal_Int32 nIndex ) const;

    /** Returns the entry API name */
    const OUString& GetEntryAPIName( sal_Int32 nIndex ) const;

    /** returns the entry context id. -1 is a valid index here. */
    sal_Int16 GetEntryContextId( sal_Int32 nIndex ) const;

    /** returns the earliest odf version for which this property should be exported. */
    SvtSaveOptions::ODFDefaultVersion GetEarliestODFVersionForExport( sal_Int32 nIndex ) const;

    /** Returns the index of an entry with the given XML-name and namespace
        If there is no matching entry the method returns -1 */
    sal_Int32 GetEntryIndex(
        sal_uInt16 nNamespace, const OUString& rStrName, sal_uInt32 nPropType,
        sal_Int32 nStartAt = -1 ) const;

    /** Retrieves a PropertyHandler for that property which placed at nIndex in the XMLPropertyMapEntry-array */
    const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nIndex ) const;

    /** import/export
        This methods calls the respective im/export-method of the respective PropertyHandler. */
    bool exportXML(
        OUString& rStrExpValue, const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const;

    bool importXML(
        const OUString& rStrImpValue, XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const;

    /** searches for an entry that matches the given api name, namespace and local name or -1 if nothing found */
    sal_Int32 FindEntryIndex(
        const sal_Char* sApiName, sal_uInt16 nNameSpace, const OUString& sXMLName ) const;

    /** searches for an entry that matches the given ContextId or gives -1 if nothing found */
    sal_Int32 FindEntryIndex( const sal_Int16 nContextId ) const;

    /** Remove an entry */
    void RemoveEntry( sal_Int32 nIndex );
};

#endif // INCLUDED_XMLOFF_XMLPRMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
