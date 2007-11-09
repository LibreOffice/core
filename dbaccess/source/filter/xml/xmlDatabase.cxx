/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlDatabase.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 08:15:57 $
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
#include "precompiled_dbaccess.hxx"
#ifndef DBA_XMLDATABASE_HXX
#include "xmlDatabase.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef DBA_XMLDATASOURCE_HXX
#include "xmlDataSource.hxx"
#endif
#ifndef DBA_XMLDOCUMENTS_HXX
#include "xmlDocuments.hxx"
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace dbaxml
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME(OXMLDatabase)

OXMLDatabase::OXMLDatabase( ODBFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    DBG_CTOR(OXMLDatabase,NULL);

}
// -----------------------------------------------------------------------------

OXMLDatabase::~OXMLDatabase()
{

    DBG_DTOR(OXMLDatabase,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLDatabase::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDatabaseElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_DATASOURCE:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSource( GetOwnImport(), nPrefix, rLocalName,xAttrList );
            break;
        case XML_TOK_FORMS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference<XFormDocumentsSupplier> xSup(GetOwnImport().GetModel(),UNO_QUERY);
                if ( xSup.is() )
                    pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getFormDocuments(),SERVICE_NAME_FORM_COLLECTION,SERVICE_SDB_DOCUMENTDEFINITION);
            }
            break;
        case XML_TOK_REPORTS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference<XReportDocumentsSupplier> xSup(GetOwnImport().GetModel(),UNO_QUERY);
                if ( xSup.is() )
                    pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getReportDocuments(),SERVICE_NAME_REPORT_COLLECTION,SERVICE_SDB_DOCUMENTDEFINITION);
            }
            break;
        case XML_TOK_QUERIES:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference<XQueryDefinitionsSupplier> xSup(GetOwnImport().getDataSource(),UNO_QUERY);
                if ( xSup.is() )
                    pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getQueryDefinitions(),SERVICE_NAME_QUERY_COLLECTION);
            }
            break;
        case XML_TOK_TABLES:
        case XML_TOK_SCHEMA_DEFINITION:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference<XTablesSupplier> xSup(GetOwnImport().getDataSource(),UNO_QUERY);
                if ( xSup.is() )
                    pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getTables());
            }
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );


    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLDatabase::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLDatabase::EndElement()
{
    GetOwnImport().setPropertyInfo();
}

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
