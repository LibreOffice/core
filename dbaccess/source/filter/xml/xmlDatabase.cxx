/*************************************************************************
 *
 *  $RCSfile: xmlDatabase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:20:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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

namespace dbaxml
{
    using namespace ::rtl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::xml::sax;

OXMLDatabase::OXMLDatabase( ODBFilter& rImport,
                sal_uInt16 nPrfx, const OUString& rLName,
                const Reference< XAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}
// -----------------------------------------------------------------------------

OXMLDatabase::~OXMLDatabase()
{
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLDatabase::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
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
                Reference<XQueryDefinitionsSupplier> xSup(GetOwnImport().GetModel(),UNO_QUERY);
                if ( xSup.is() )
                    pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getQueryDefinitions(),SERVICE_NAME_QUERY_COLLECTION);
            }
            break;
        case XML_TOK_TABLES:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference<XTablesSupplier> xSup(GetOwnImport().GetModel(),UNO_QUERY);
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

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
