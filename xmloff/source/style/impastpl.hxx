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

#ifndef INCLUDED_XMLOFF_SOURCE_STYLE_IMPASTPL_HXX
#define INCLUDED_XMLOFF_SOURCE_STYLE_IMPASTPL_HXX

#include <boost/ptr_container/ptr_set.hpp>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <set>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlexppr.hxx>

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class SvXMLAutoStylePoolP;
class XMLAutoStylePoolParent;
class SvXMLExportPropertyMapper;
class SvXMLExport;

// Implementationclass for stylefamily-information

struct XMLAutoStyleFamily : boost::noncopyable
{
    typedef boost::ptr_set<XMLAutoStylePoolParent> ParentSetType;
    typedef std::set<OUString> NameSetType;

    sal_uInt32                   mnFamily;
    OUString                     maStrFamilyName;
    rtl::Reference < SvXMLExportPropertyMapper >  mxMapper;

    ParentSetType maParentSet;
    NameSetType maNameSet;
    sal_uInt32                          mnCount;
    sal_uInt32                          mnName;
    OUString                     maStrPrefix;
    bool                         mbAsFamily;

    XMLAutoStyleFamily( sal_Int32 nFamily, const OUString& rStrName,
            const rtl::Reference<SvXMLExportPropertyMapper>& rMapper,
            const OUString& rStrPrefix, bool bAsFamily = true );

    XMLAutoStyleFamily( sal_Int32 nFamily );
    ~XMLAutoStyleFamily();

    friend bool operator<(const XMLAutoStyleFamily& r1, const XMLAutoStyleFamily& r2);

    void ClearEntries();
};

// Properties of a pool

class XMLAutoStylePoolProperties
{
    OUString                     msName;
    ::std::vector< XMLPropertyState >   maProperties;
    sal_uInt32                          mnPos;

public:

    XMLAutoStylePoolProperties( XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, OUString& rParentname );

    ~XMLAutoStylePoolProperties()
    {
    }

    const OUString& GetName() const { return msName; }
    const ::std::vector< XMLPropertyState >& GetProperties() const { return maProperties; }
    sal_uInt32 GetPos() const { return mnPos; }

    void SetName( const OUString& rNew ) { msName = rNew; }
};

// Parents of AutoStylePool's
class XMLAutoStylePoolParent
{
public:
    typedef boost::ptr_vector<XMLAutoStylePoolProperties> PropertiesListType;

private:
    OUString msParent;
    PropertiesListType maPropertiesList;

public:

    XMLAutoStylePoolParent( const OUString & rParent ) :
        msParent( rParent )
    {
    }

    ~XMLAutoStylePoolParent();

    bool Add( XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, OUString& rName, bool bDontSeek = false );

    bool AddNamed( XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, const OUString& rName );

    OUString Find( const XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties ) const;

    const OUString& GetParent() const { return msParent; }

    PropertiesListType& GetPropertiesList()
    {
        return maPropertiesList;
    }

    bool operator< (const XMLAutoStylePoolParent& rOther) const;
};

// Implementationclass of SvXMLAutoStylePool

class SvXMLAutoStylePoolP_Impl
{
    // A set that finds and sorts based only on mnFamily
    typedef boost::ptr_set<XMLAutoStyleFamily> FamilySetType;

    SvXMLExport& rExport;
    FamilySetType maFamilySet;

public:

    SvXMLAutoStylePoolP_Impl( SvXMLExport& rExport );
    ~SvXMLAutoStylePoolP_Impl();

    SvXMLExport& GetExport() const { return rExport; }

    void AddFamily( sal_Int32 nFamily, const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix, bool bAsFamily = true );
    void SetFamilyPropSetMapper( sal_Int32 nFamily,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper );
    void RegisterName( sal_Int32 nFamily, const OUString& rName );
    void GetRegisteredNames(
        com::sun::star::uno::Sequence<sal_Int32>& aFamilies,
        com::sun::star::uno::Sequence<OUString>& aNames );

    bool Add(
        OUString& rName, sal_Int32 nFamily,
        const OUString& rParentName,
        const ::std::vector< XMLPropertyState >& rProperties,
        bool bDontSeek = false );

    bool AddNamed(
        const OUString& rName, sal_Int32 nFamily,
        const OUString& rParentName,
        const ::std::vector< XMLPropertyState >& rProperties );

    OUString Find( sal_Int32 nFamily, const OUString& rParent,
                          const ::std::vector< XMLPropertyState >& rProperties ) const;

    void exportXML( sal_Int32 nFamily,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const SvXMLAutoStylePoolP *pAntiImpl) const;

    void ClearEntries();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
