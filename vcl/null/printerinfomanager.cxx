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


#include "vcl/printerinfomanager.hxx"

#include "generic/gendata.hxx"

using namespace psp;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringHash;

PrinterInfoManager& PrinterInfoManager::get()
{
    SalData* pSalData = GetSalData();
    if( ! pSalData->m_pPIManager )
        pSalData->m_pPIManager = new PrinterInfoManager();
    return *pSalData->m_pPIManager;
}

void PrinterInfoManager::release()
{
    SalData* pSalData = GetSalData();
    delete pSalData->m_pPIManager;
    pSalData->m_pPIManager = NULL;
}

PrinterInfoManager::PrinterInfoManager( Type eType ) :
    m_pQueueInfo( NULL ),
    m_eType( eType ),
    m_bUseIncludeFeature( false ),
    m_bUseJobPatch( true ),
    m_aSystemDefaultPaper( RTL_CONSTASCII_USTRINGPARAM( "A4" ) ),
    m_bDisableCUPS( false )
{
    // initSystemDefaultPaper();
}

void PrinterInfoManager::listPrinters( ::std::list< OUString >& rList ) const
{
    rList.clear();
}

const PrinterInfo& PrinterInfoManager::getPrinterInfo( const OUString& /* rPrinter */ ) const
{
    static PrinterInfo aEmptyInfo;

    return aEmptyInfo;
}

bool PrinterInfoManager::checkFeatureToken( const rtl::OUString& /* rPrinterName */, const char* /* pToken */ ) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
