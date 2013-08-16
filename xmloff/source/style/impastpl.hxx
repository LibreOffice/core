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

#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#define _XMLOFF_XMLASTPL_IMPL_HXX

#include <boost/ptr_container/ptr_set.hpp>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <set>
#include <vector>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <o3tl/sorted_vector.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlexppr.hxx>

class SvXMLAutoStylePoolP;
class SvXMLAutoStylePoolParentsP_Impl;
typedef std::set<OUString> SvXMLAutoStylePoolNamesP_Impl;
class SvXMLExportPropertyMapper;
class SvXMLExport;

#define MAX_CACHE_SIZE 65536

// Implementationclass for stylefamily-information

class XMLFamilyData_Impl
{
public:
    sal_uInt32                          mnFamily;
    OUString                     maStrFamilyName;
    UniReference < SvXMLExportPropertyMapper >  mxMapper;

    SvXMLAutoStylePoolParentsP_Impl*    mpParentList;
    SvXMLAutoStylePoolNamesP_Impl*      mpNameList;
    sal_uInt32                          mnCount;
    sal_uInt32                          mnName;
    OUString                     maStrPrefix;
    sal_Bool                            bAsFamily;

public:
    XMLFamilyData_Impl( sal_Int32 nFamily, const OUString& rStrName,
            const UniReference < SvXMLExportPropertyMapper > &  rMapper,
            const OUString& rStrPrefix, sal_Bool bAsFamily = sal_True );

    XMLFamilyData_Impl( sal_Int32 nFamily ) :
        mnFamily( nFamily ), mpParentList( NULL ),
        mpNameList( NULL ), mnCount( 0 ), mnName( 0 ),
        bAsFamily( false )

    {}
    ~XMLFamilyData_Impl();

    friend bool operator<(const XMLFamilyData_Impl& r1, const XMLFamilyData_Impl& r2);

    void ClearEntries();
};

// Properties of a pool

class SvXMLAutoStylePoolPropertiesP_Impl
{
    OUString                     msName;
    ::std::vector< XMLPropertyState >   maProperties;
    sal_uInt32                          mnPos;

public:

    SvXMLAutoStylePoolPropertiesP_Impl( XMLFamilyData_Impl& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties );

    ~SvXMLAutoStylePoolPropertiesP_Impl()
    {
    }

    const OUString& GetName() const { return msName; }
    const ::std::vector< XMLPropertyState >& GetProperties() const { return maProperties; }
    sal_uInt32 GetPos() const { return mnPos; }

    void SetName( const OUString& rNew ) { msName = rNew; }
};

typedef SvXMLAutoStylePoolPropertiesP_Impl* SvXMLAutoStylePoolPropertiesPPtr;
typedef ::std::vector< SvXMLAutoStylePoolPropertiesPPtr > SvXMLAutoStylePoolPropertiesPList_Impl;

// Parents of AutoStylePool's
class SvXMLAutoStylePoolParentP_Impl
{
    OUString                         msParent;
    SvXMLAutoStylePoolPropertiesPList_Impl  maPropertiesList;

public:

    SvXMLAutoStylePoolParentP_Impl( const OUString & rParent ) :
        msParent( rParent )
    {
    }

    ~SvXMLAutoStylePoolParentP_Impl();

    sal_Bool Add( XMLFamilyData_Impl& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, OUString& rName, bool bDontSeek = false );

    sal_Bool AddNamed( XMLFamilyData_Impl& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties, const OUString& rName );

    OUString Find( const XMLFamilyData_Impl& rFamilyData, const ::std::vector< XMLPropertyState >& rProperties ) const;

    const OUString& GetParent() const { return msParent; }

    const SvXMLAutoStylePoolPropertiesPList_Impl& GetPropertiesList() const
    {
        return maPropertiesList;
    }
};

struct SvXMLAutoStylePoolParentPCmp_Impl
{
    bool operator()( SvXMLAutoStylePoolParentP_Impl* const& lhs, SvXMLAutoStylePoolParentP_Impl* const& rhs) const
    {
        return lhs->GetParent().compareTo( rhs->GetParent() ) < 0;
    }
};
class SvXMLAutoStylePoolParentsP_Impl : public o3tl::sorted_vector<SvXMLAutoStylePoolParentP_Impl*, SvXMLAutoStylePoolParentPCmp_Impl>
{
public:
    ~SvXMLAutoStylePoolParentsP_Impl() { DeleteAndDestroyAll(); }
};

// Implementationclass of SvXMLAutoStylePool

class SvXMLAutoStylePoolP_Impl
{
    // A set that finds and sorts based only on mnFamily
    typedef boost::ptr_set<XMLFamilyData_Impl> FamilyListType;

    SvXMLExport& rExport;
    FamilyListType maFamilyList;

public:

    SvXMLAutoStylePoolP_Impl( SvXMLExport& rExport );
    ~SvXMLAutoStylePoolP_Impl();

    SvXMLExport& GetExport() const { return rExport; }

    void AddFamily( sal_Int32 nFamily, const OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix, sal_Bool bAsFamily = sal_True );
    void SetFamilyPropSetMapper( sal_Int32 nFamily,
        const UniReference < SvXMLExportPropertyMapper > & rMapper );
    void RegisterName( sal_Int32 nFamily, const OUString& rName );
    void GetRegisteredNames(
        com::sun::star::uno::Sequence<sal_Int32>& aFamilies,
        com::sun::star::uno::Sequence<OUString>& aNames );

    bool Add(
        OUString& rName, sal_Int32 nFamily,
        const OUString& rParent,
        const ::std::vector< XMLPropertyState >& rProperties,
        bool bDontSeek = false );

    sal_Bool AddNamed( const OUString& rName, sal_Int32 nFamily,
                const OUString& rParent,
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

struct SvXMLAutoStylePoolPExport_Impl
{
    const OUString                   *mpParent;
    const SvXMLAutoStylePoolPropertiesP_Impl    *mpProperties;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
