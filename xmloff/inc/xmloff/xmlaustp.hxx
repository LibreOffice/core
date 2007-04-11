/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlaustp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:31:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLASTPLP_HXX
#define _XMLOFF_XMLASTPLP_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

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
