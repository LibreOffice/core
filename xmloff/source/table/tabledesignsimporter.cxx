/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabledesignsimporter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:00:13 $
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

    SvXMLImportContext* CreateContext(USHORT nPrefix, const OUString& rLocalName,   const Reference<XAttributeList>& xAttrList);
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

SvXMLImportContext* TabelDesignsImporter::CreateContext(USHORT nPrefix, const OUString& rLocalName, const Reference<XAttributeList>& xAttrList)
{
}

// --------------------------------------------------------------------
