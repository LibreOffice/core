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

#ifndef _XMLOFF_XMLPAGEEXPORT_HXX
#define _XMLOFF_XMLPAGEEXPORT_HXX

#include <rtl/ustring.hxx>
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include <bf_xmloff/attrlist.hxx>
#include <bf_xmloff/uniref.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
    namespace container { class XIndexReplace; }
    namespace beans { class XPropertySet; }
} } }
namespace binfilter {

class SvXMLExport;
class XMLPropertyHandlerFactory;
class XMLPropertySetMapper;
class SvXMLExportPropertyMapper;

//______________________________________________________________________________

struct XMLPageExportNameEntry
{
    ::rtl::OUString			sPageMasterName;
    ::rtl::OUString			sStyleName;
};

//______________________________________________________________________________

class XMLPageExport : public UniRefBase
{
    SvXMLExport& rExport;

    const ::rtl::OUString sIsPhysical;
    const ::rtl::OUString sFollowStyle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XIndexAccess > xPageStyles;

    ::std::vector< XMLPageExportNameEntry > aNameVector;
    sal_Bool findPageMasterName( const ::rtl::OUString& rStyleName, ::rtl::OUString& rPMName ) const;

    UniReference < XMLPropertyHandlerFactory > xPageMasterPropHdlFactory;
    UniReference < XMLPropertySetMapper > xPageMasterPropSetMapper;
    UniReference < SvXMLExportPropertyMapper > xPageMasterExportPropMapper;

protected:

    SvXMLExport& GetExport() { return rExport; }

    virtual void collectPageMasterAutoStyle(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                ::rtl::OUString& rPageMasterName );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 sal_Bool bAutoStyles );

    sal_Bool exportStyle(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::style::XStyle >& rStyle,
                sal_Bool bAutoStyles );

    void exportStyles( sal_Bool bUsed, sal_Bool bAutoStyles );

public:
    XMLPageExport( SvXMLExport& rExp );
    ~XMLPageExport();

    void	collectAutoStyles( sal_Bool bUsed )		{ exportStyles( bUsed, sal_True ); }
    void	exportAutoStyles();
    void	exportMasterStyles( sal_Bool bUsed )	{ exportStyles( bUsed, sal_False ); }
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
