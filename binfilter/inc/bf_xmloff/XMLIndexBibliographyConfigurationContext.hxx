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

#ifndef _XMLOFF_XMLINDEXBIBLIOGRAPHYCONFIGURATIONCONTEXT_HXX_
#define _XMLOFF_XMLINDEXBIBLIOGRAPHYCONFIGURATIONCONTEXT_HXX_

#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_ 
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_ 
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif


#include <vector>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace binfilter {


/**
 * Import bibliography configuration.
 *
 * Little cheat: Cover all child elements in CreateChildContext.
 */
class XMLIndexBibliographyConfigurationContext : public SvXMLStyleContext
{
    const ::rtl::OUString sFieldMaster_Bibliography;
    const ::rtl::OUString sBracketBefore;
    const ::rtl::OUString sBracketAfter;
    const ::rtl::OUString sIsNumberEntries;
    const ::rtl::OUString sIsSortByPosition;
    const ::rtl::OUString sSortKeys;
    const ::rtl::OUString sSortKey;
    const ::rtl::OUString sIsSortAscending;
    const ::rtl::OUString sSortAlgorithm;
    const ::rtl::OUString sLocale;

    ::rtl::OUString sSuffix;
    ::rtl::OUString sPrefix;
    ::rtl::OUString sAlgorithm;
    ::com::sun::star::lang::Locale aLocale;
    sal_Bool bNumberedEntries;
    sal_Bool bSortByPosition;

    ::std::vector< ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue> > aSortKeys;

public:

    TYPEINFO();

    XMLIndexBibliographyConfigurationContext(
        SvXMLImport& rImport, 
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    ~XMLIndexBibliographyConfigurationContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void CreateAndInsert( sal_Bool bOverwrite );

    virtual SvXMLImportContext *CreateChildContext( 
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttribute(
        sal_uInt16 nPrefix,
        const ::rtl::OUString sLocalName,
        const ::rtl::OUString sValue);
};

}//end of namespace binfilter
#endif
