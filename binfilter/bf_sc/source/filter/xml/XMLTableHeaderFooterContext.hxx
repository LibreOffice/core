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
#ifndef _SC_XMLTABLEHEADERFOOTERCONTEXT_HXX_
#define _SC_XMLTABLEHEADERFOOTERCONTEXT_HXX_


#ifndef _XMLOFF_XMLICTXT_HXX_
#include <bf_xmloff/xmlictxt.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX_
#include <bf_xmloff/xmlimp.hxx>
#endif
#ifndef _COM_SUN_STAR_SHEET_XHEADERFOOTERCONTENT_HPP_
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>
#endif
namespace binfilter {

namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; }
    namespace beans { class XPropertySet; }
} } }

class XMLTableHeaderFooterContext: public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > xPropSet;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::sheet::XHeaderFooterContent > xHeaderFooterContent;

    const ::rtl::OUString	sOn;
    const ::rtl::OUString	sShareContent;
    const ::rtl::OUString	sContent;
    const ::rtl::OUString	sContentLeft;
    const ::rtl::OUString	sEmpty;
    ::rtl::OUString			sCont;

    sal_Bool	bDisplay : 1;
    sal_Bool	bInsertContent : 1;
    sal_Bool 	bLeft : 1;
    sal_Bool	bContainsLeft : 1;
    sal_Bool	bContainsRight : 1;
    sal_Bool	bContainsCenter : 1;

public:
    TYPEINFO();

    XMLTableHeaderFooterContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const ::com::sun::star::uno::Reference <
                       ::com::sun::star::beans::XPropertySet > & rPageStylePropSet,
               sal_Bool bFooter, sal_Bool bLft );

    virtual ~XMLTableHeaderFooterContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};

class XMLHeaderFooterRegionContext: public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor >& xTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;

public:
    TYPEINFO();

    XMLHeaderFooterRegionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >& xCursor );

    virtual ~XMLHeaderFooterRegionContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();
};


} //namespace binfilter
#endif
