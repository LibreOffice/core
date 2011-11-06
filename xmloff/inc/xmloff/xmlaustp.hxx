/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _XMLOFF_XMLASTPLP_HXX
#define _XMLOFF_XMLASTPLP_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/uniref.hxx>

class SvXMLExportPropertyMapper;
class SvXMLNamespaceMap;
class SvXMLAutoStylePoolP_Impl;
class SvXMLAttributeList;
class SvXMLExport;
namespace com { namespace sun { namespace star { namespace uno
    { template<typename A> class Sequence; }
} } }

class XMLOFF_DLLPUBLIC SvXMLAutoStylePoolP : public UniRefBase
{
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

    SvXMLAutoStylePoolP();
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
    sal_Bool        Add( ::rtl::OUString& rName, sal_Int32 nFamily, const ::std::vector< XMLPropertyState >& rProperties );
    sal_Bool        Add( ::rtl::OUString& rName, sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties );

    /// Add an item set with a pre-defined name (needed for saving sheets separately in Calc).
    sal_Bool        AddNamed( const ::rtl::OUString& rName, sal_Int32 nFamily, const ::rtl::OUString& rParent,
                              const ::std::vector< XMLPropertyState >& rProperties );

    /// Find an item set's name.
    ::rtl::OUString Find( sal_Int32 nFamily, const ::std::vector< XMLPropertyState >& rProperties ) const;
    ::rtl::OUString Find( sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties ) const;

    // Add a property set to the pool and cache its name.
    ::rtl::OUString AddAndCache( sal_Int32 nFamily, const ::std::vector< XMLPropertyState >& rProperties );
    ::rtl::OUString AddAndCache( sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties );
    ::rtl::OUString AddAndCache( sal_Int32 nFamily, const ::rtl::OUString& rParent );

    ::rtl::OUString FindAndRemoveCached( sal_Int32 nFamily ) const;

    /** Export all item sets ofs a certain class in the order in that they have been added. */
    void exportXML( sal_Int32 nFamily
        , const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap
        ) const;

    void ClearEntries();
};

#endif  //  _XMLOFF_XMLASTPLP_HXX
