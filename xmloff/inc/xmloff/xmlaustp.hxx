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

#ifndef _XMLOFF_XMLASTPLP_HXX
#define _XMLOFF_XMLASTPLP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/uniref.hxx>

class SvXMLExportPropertyMapper;
class SvXMLAutoStylePoolP;
class SvXMLNamespaceMap;
class SvXMLAutoStylePoolP_Impl;
class SvXMLAttributeList;
class SvXMLExport;
namespace com { namespace sun { namespace star { namespace uno
    { template<typename A> class Sequence; }
} } }

class XMLFamilyData_Impl;

/**
 * Encapsulates the process of filtering properties and
 * creating automatic styles for various families.
 */
class XMLOFF_DLLPUBLIC SvXMLAutoFilteredSet
{
    UniReference< SvXMLAutoStylePoolP > mxPool;
    XMLFamilyData_Impl                 *mpFamily;
 protected:
    ::std::vector< XMLPropertyState >   maProperties;
 public:
    SvXMLAutoFilteredSet( const UniReference< SvXMLAutoStylePoolP > &xPool, sal_Int32 nFamily );
    explicit SvXMLAutoFilteredSet( const SvXMLAutoFilteredSet &xRef );
    ~SvXMLAutoFilteredSet();

    /// return the relevant family's property mapper
    const UniReference < SvXMLExportPropertyMapper > & getMapper();
    /// filter properties
    SvXMLAutoFilteredSet &filter( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::beans::XPropertySet > &xPropSet );
    /// discard all content
    void      clear();
    /// do we have any valid properties ie. some were not filtered ?
    bool      hasValidContent();
    /// how many valid properties do we have ?
    sal_Int32 countValidProperties();
    /// Insert into the auto style pool and return the name of the new auto-style
    OUString  add( const OUString &rParent,  bool bCache = false, bool bDontSeek = false );
    /// Find the auto name of this style in the pool
    OUString  findInPool( const OUString &rParent );

    bool      empty()
        { return maProperties.empty(); }
    void      push_back( const XMLPropertyState &rState )
        { maProperties.push_back( rState ); }
    size_t    size()
        { return maProperties.size(); }
};

class XMLOFF_DLLPUBLIC SvXMLAutoStylePoolP : public UniRefBase
{
    friend class Test;
    friend class SvXMLAutoFilteredSet;
    friend class SvXMLAutoStylePoolP_Impl;

    SvXMLAutoStylePoolP_Impl    *pImpl;

protected:

    virtual void exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;

// this methode is executed after Properties Export, so you can e.g. export a map or so on
    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;


public:

    SvXMLAutoStylePoolP( SvXMLExport& rExport);

    virtual ~SvXMLAutoStylePoolP();

    SvXMLExport& GetExport() const;

    /** register a new family with its appropriate instance of a derivation of XMLPropertySetMapper
        for family numbers see families.hxx
        if bAsFamily is sal_False, the family name is used as element name
     */
    // TODO: Remove this ugly method as soon as possible
    void AddFamily( sal_Int32 nFamily, const ::rtl::OUString& rStrName, SvXMLExportPropertyMapper* pMapper, ::rtl::OUString aStrPrefix, sal_Bool bAsFamily = sal_True );
    void AddFamily( sal_Int32 nFamily, const ::rtl::OUString& rStrName,
                    const UniReference< SvXMLExportPropertyMapper >& rMapper,
                    const ::rtl::OUString& rStrPrefix, sal_Bool bAsFamily = sal_True );
    void SetFamilyPropSetMapper( sal_Int32 nFamily,
                    const UniReference< SvXMLExportPropertyMapper >& rMapper );

    /// Register a name that must not be used as a generated name.
    void RegisterName( sal_Int32 nFamily, const ::rtl::OUString& rName );

    /// retrieve the registered names (names + families)
    void GetRegisteredNames(
        com::sun::star::uno::Sequence<sal_Int32>& aFamilies,
        com::sun::star::uno::Sequence<rtl::OUString>& aNames );

    /// register (families + names)
    void RegisterNames(
        com::sun::star::uno::Sequence<sal_Int32>& aFamilies,
        com::sun::star::uno::Sequence<rtl::OUString>& aNames );

    /// Add an item set to the pool and return its generated name.
    ::rtl::OUString Add( sal_Int32 nFamily, const ::std::vector< XMLPropertyState >& rProperties );
    ::rtl::OUString Add( sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties, bool bDontSeek = false );
    sal_Bool        Add( ::rtl::OUString& rName, sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties );

    /// Add an item set with a pre-defined name (needed for saving sheets separately in Calc).
    sal_Bool        AddNamed( const ::rtl::OUString& rName, sal_Int32 nFamily, const ::rtl::OUString& rParent,
                              const ::std::vector< XMLPropertyState >& rProperties );

    /// Find an item set's name.
    ::rtl::OUString Find( sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties ) const;

    /** Export all item sets ofs a certain class in the order in that they have been added. */
    void exportXML( sal_Int32 nFamily
        , const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap
        ) const;

    void ClearEntries();
};

#endif  //  _XMLOFF_XMLASTPLP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
