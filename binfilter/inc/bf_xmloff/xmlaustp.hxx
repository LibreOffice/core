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

#ifndef _XMLOFF_XMLASTPLP_HXX
#define _XMLOFF_XMLASTPLP_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <bf_xmloff/xmlprmap.hxx>

#include <bf_xmloff/uniref.hxx>
namespace binfilter {

class SvXMLExportPropertyMapper;
class SvXMLNamespaceMap;
class SvXMLAttributeList;
class SvXMLAutoStylePoolP_Impl;
class SvXMLExport;

class SvXMLAutoStylePoolP : public UniRefBase
{
    friend class SvXMLAutoStylePoolP_Impl;

    SvXMLAutoStylePoolP_Impl	*pImpl;

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

    /// Register a name that must not be used as a generated name.
    void RegisterName( sal_Int32 nFamily, const ::rtl::OUString& rName );

    /// Add an item set to the pool and return its generated name.
    ::rtl::OUString Add( sal_Int32 nFamily, const ::std::vector< XMLPropertyState >& rProperties );
    ::rtl::OUString Add( sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties );
    sal_Bool Add(::rtl::OUString& rName, sal_Int32 nFamily, const ::rtl::OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties );


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

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLASTPLP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
