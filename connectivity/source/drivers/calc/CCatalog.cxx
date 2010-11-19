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
#include "precompiled_connectivity.hxx"


#include "calc/CCatalog.hxx"
#include "calc/CConnection.hxx"
#include "calc/CTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <rtl/logfile.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

// -------------------------------------------------------------------------
using namespace connectivity::calc;
// -------------------------------------------------------------------------
OCalcCatalog::OCalcCatalog(OCalcConnection* _pCon) : file::OFileCatalog(_pCon)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcCatalog::OCalcCatalog" );
}
// -------------------------------------------------------------------------
void OCalcCatalog::refreshTables()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "calc", "Ocke.Janssen@sun.com", "OCalcCatalog::refreshTables" );
    TStringVector aVector;
    Sequence< ::rtl::OUString > aTypes;
    OCalcConnection::ODocHolder aDocHodler(((OCalcConnection*)m_pConnection));
    Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),aTypes);

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(3));
    }
    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OCalcTables(m_xMetaData,*this,m_aMutex,aVector);

    // this avoids that the document will be loaded a 2nd time when one table will be accessed.
    //if ( m_pTables && m_pTables->hasElements() )
    //    m_pTables->getByIndex(0);
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
