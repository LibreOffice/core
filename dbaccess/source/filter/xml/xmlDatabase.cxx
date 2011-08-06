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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "xmlDatabase.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include "xmlDataSource.hxx"
#include "xmlDocuments.hxx"
#include "xmlEnums.hxx"
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include "xmlstrings.hrc"
#include <tools/debug.hxx>
#include <connectivity/dbtools.hxx>

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
            pContext = new OXMLDataSource( GetOwnImport(), nPrefix, rLocalName, xAttrList, OXMLDataSource::eDataSource );
            break;
        case XML_TOK_FORMS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Any aValue;
                ::rtl::OUString sService;
                dbtools::getDataSourceSetting(GetOwnImport().getDataSource(),"Forms",aValue);
                aValue >>= sService;
                if ( !sService.getLength() )
                {
                    Reference<XFormDocumentsSupplier> xSup(GetOwnImport().GetModel(),UNO_QUERY);
                    if ( xSup.is() )
                        pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getFormDocuments(),SERVICE_NAME_FORM_COLLECTION,SERVICE_SDB_DOCUMENTDEFINITION);
                }
            }
            break;
        case XML_TOK_REPORTS:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Any aValue;
                ::rtl::OUString sService;
                dbtools::getDataSourceSetting(GetOwnImport().getDataSource(),"Reports",aValue);
                aValue >>= sService;
                if ( !sService.getLength() )
                {
                    Reference<XReportDocumentsSupplier> xSup(GetOwnImport().GetModel(),UNO_QUERY);
                    if ( xSup.is() )
                        pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getReportDocuments(),SERVICE_NAME_REPORT_COLLECTION,SERVICE_SDB_DOCUMENTDEFINITION);
                }
            }
            break;
        case XML_TOK_QUERIES:
            {
                GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Any aValue;
                ::rtl::OUString sService;
                dbtools::getDataSourceSetting(GetOwnImport().getDataSource(),"CommandDefinitions",aValue);
                aValue >>= sService;
                if ( !sService.getLength() )
                {
                    Reference<XQueryDefinitionsSupplier> xSup(GetOwnImport().getDataSource(),UNO_QUERY);
                    if ( xSup.is() )
                        pContext = new OXMLDocuments( GetOwnImport(), nPrefix, rLocalName,xSup->getQueryDefinitions(),SERVICE_NAME_QUERY_COLLECTION);
                }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
