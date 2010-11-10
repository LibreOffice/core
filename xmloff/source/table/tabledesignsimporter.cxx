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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "xmloff/table/XMLTableImport.hxx"
#include "xmloff/xmltkmap.hxx"
#include "xmloff/maptype.hxx"
#include "xmloff/xmlprmap.hxx"
#include "xmloff/txtimp.hxx"
#include "xmloff/xmlimp.hxx"
#include "xmloff/nmspmap.hxx"
#include "xmloff/xmlstyle.hxx"
#include "xmloff/prstylei.hxx"
#include "xmloff/xmlimp.hxx"

#include "xmlnmspe.hxx"
#include "table.hxx"

#include <boost/shared_ptr.hpp>

// --------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

// --------------------------------------------------------------------

class TabelDesignsImporter : public SvXMLImport
{
    TabelDesignsImporter( const Reference< XMultiServiceFactory > & rSMgr );
    ~TabelDesignsImporter() throw ();

    SvXMLImportContext* CreateContext(sal_uInt16 nPrefix, const OUString& rLocalName,   const Reference<XAttributeList>& xAttrList);
}

// --------------------------------------------------------------------

TabelDesignsImporter::TabelDesignsImporter( const Reference< XMultiServiceFactory > & rSMgr )
: SvXMLImport( rSMgr, true )
{
    // add namespaces
    GetNamespaceMap().Add(
        GetXMLToken(XML_NP_PRESENTATION),
        GetXMLToken(XML_N_PRESENTATION),
        XML_NAMESPACE_PRESENTATION);
}

// --------------------------------------------------------------------

TabelDesignsImporter::~TabelDesignsImporter()
{
}

// --------------------------------------------------------------------

SvXMLImportContext* TabelDesignsImporter::CreateContext(sal_uInt16 nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& xAttrList)
{
}

// --------------------------------------------------------------------
