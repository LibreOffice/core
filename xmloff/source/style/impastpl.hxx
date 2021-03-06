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

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <set>
#include <memory>
#include <vector>

#include <comphelper/stl_types.hxx>

#include <xmloff/maptype.hxx>
#include <xmloff/xmlexppr.hxx>
#include <AutoStyleEntry.hxx>

class SvXMLAutoStylePoolP;
class XMLAutoStylePoolParent;
struct XMLAutoStyleFamily;
class SvXMLExportPropertyMapper;
class SvXMLExport;
enum class XmlStyleFamily;

// Properties of a pool

class XMLAutoStylePoolProperties
{
    OUString                     msName;
    ::std::vector< XMLPropertyState >   maProperties;
    sal_uInt32                          mnPos;

public:

    XMLAutoStylePoolProperties( XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, OUString const & rParentname );

    const OUString& GetName() const { return msName; }
    const ::std::vector< XMLPropertyState >& GetProperties() const { return maProperties; }
    sal_uInt32 GetPos() const { return mnPos; }

    void SetName( const OUString& rNew ) { msName = rNew; }
};

// Parents of AutoStylePool's
class XMLAutoStylePoolParent
{
public:
    typedef std::vector<std::unique_ptr<XMLAutoStylePoolProperties>> PropertiesListType;

private:
    OUString msParent;
    PropertiesListType m_PropertiesList;

public:

    explicit XMLAutoStylePoolParent( const OUString & rParent ) :
        msParent( rParent )
    {
    }

    ~XMLAutoStylePoolParent();

    bool Add( XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, OUString& rName, bool bDontSeek );

    bool AddNamed( XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, const OUString& rName );

    OUString Find( const XMLAutoStyleFamily& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties ) const;

    const OUString& GetParent() const { return msParent; }

    PropertiesListType& GetPropertiesList()
    {
        return m_PropertiesList;
    }

    bool operator< (const XMLAutoStylePoolParent& rOther) const;
};

// Implementationclass for stylefamily-information

struct XMLAutoStyleFamily
{
    typedef std::set<std::unique_ptr<XMLAutoStylePoolParent>,
        comphelper::UniquePtrValueLess<XMLAutoStylePoolParent>> ParentSetType;

    XmlStyleFamily mnFamily;
    OUString maStrFamilyName;
    rtl::Reference<SvXMLExportPropertyMapper> mxMapper;

    ParentSetType m_ParentSet;
    std::set<OUString> maNameSet;
    std::set<OUString> maReservedNameSet;
    sal_uInt32 mnCount;
    sal_uInt32 mnName;
    OUString maStrPrefix;
    bool mbAsFamily;

    XMLAutoStyleFamily( XmlStyleFamily nFamily, const OUString& rStrName,
            const rtl::Reference<SvXMLExportPropertyMapper>& rMapper,
            const OUString& rStrPrefix, bool bAsFamily );

    explicit XMLAutoStyleFamily( XmlStyleFamily nFamily );

    XMLAutoStyleFamily(const XMLAutoStyleFamily&) = delete;
    XMLAutoStyleFamily& operator=(const XMLAutoStyleFamily&) = delete;

    friend bool operator<(const XMLAutoStyleFamily& r1, const XMLAutoStyleFamily& r2);

    void ClearEntries();
};

// Implementationclass of SvXMLAutoStylePool

class SvXMLAutoStylePoolP_Impl
{
    // A set that finds and sorts based only on mnFamily
    typedef std::set<std::unique_ptr<XMLAutoStyleFamily>,
            comphelper::UniquePtrValueLess<XMLAutoStyleFamily>> FamilySetType;

    SvXMLExport& rExport;
    FamilySetType m_FamilySet;

public:

    explicit SvXMLAutoStylePoolP_Impl( SvXMLExport& rExport );
    ~SvXMLAutoStylePoolP_Impl();

    SvXMLExport& GetExport() const { return rExport; }

    void AddFamily( XmlStyleFamily nFamily, const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix, bool bAsFamily );
    void SetFamilyPropSetMapper( XmlStyleFamily nFamily,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper );
    void RegisterName( XmlStyleFamily nFamily, const OUString& rName );
    void RegisterDefinedName( XmlStyleFamily nFamily, const OUString& rName );
    void GetRegisteredNames(
        css::uno::Sequence<sal_Int32>& aFamilies,
        css::uno::Sequence<OUString>& aNames );

    bool Add(
        OUString& rName, XmlStyleFamily nFamily,
        const OUString& rParentName,
        const ::std::vector< XMLPropertyState >& rProperties,
        bool bDontSeek = false );

    bool AddNamed(
        const OUString& rName, XmlStyleFamily nFamily,
        const OUString& rParentName,
        const ::std::vector< XMLPropertyState >& rProperties );

    OUString Find( XmlStyleFamily nFamily, const OUString& rParent,
                          const ::std::vector< XMLPropertyState >& rProperties ) const;

    void exportXML( XmlStyleFamily nFamily,
        const SvXMLAutoStylePoolP *pAntiImpl) const;

    void ClearEntries();

    std::vector<xmloff::AutoStyleEntry> GetAutoStyleEntries() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
