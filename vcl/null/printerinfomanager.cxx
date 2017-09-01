/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include "printerinfomanager.hxx"

#include "unx/gendata.hxx"

// needed since we declare a std::unique_ptr<SystemQueueInfo>
namespace psp
{
    class SystemQueueInfo
    {
    };
}

using namespace psp;
using namespace osl;


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
    pSalData->m_pPIManager = nullptr;
}

PrinterInfoManager::PrinterInfoManager( Type eType ) :
    m_pQueueInfo( nullptr ),
    m_eType( eType ),
    m_bUseIncludeFeature( false ),
    m_bUseJobPatch( true ),
    m_aSystemDefaultPaper( "A4" )
{
    // initSystemDefaultPaper();
}

PrinterInfoManager::~PrinterInfoManager()
{

}

bool PrinterInfoManager::checkPrintersChanged( bool /* bWait */ )
{
    return false;
}

void PrinterInfoManager::initialize()
{
    // ???
}

void PrinterInfoManager::listPrinters( ::std::vector< OUString >& rVector ) const
{
    (void) this;

    rVector.clear();
}

const PrinterInfo& PrinterInfoManager::getPrinterInfo( const OUString& /* rPrinter */ ) const
{
    static PrinterInfo aEmptyInfo;

    (void) this;

    return aEmptyInfo;
}

bool PrinterInfoManager::writePrinterConfig()
{
    return false;
}

bool PrinterInfoManager::addPrinter( const OUString& /* rPrinterName */, const OUString& /* rDriverName */ )
{
    return false;
}

bool PrinterInfoManager::removePrinter( const OUString& /* rPrinterName */, bool /* bCheckOnly */ )
{
    return false;
}

bool PrinterInfoManager::setDefaultPrinter( const OUString& /* rPrinterName */ )
{
    return false;
}

bool PrinterInfoManager::checkFeatureToken( const OUString& /* rPrinterName */, const char* /* pToken */ ) const
{
    (void) this;

    return false;
}

FILE* PrinterInfoManager::startSpool( const OUString& /* rPrintername */, bool /* bQuickCommand */ )
{
    return nullptr;
}

bool PrinterInfoManager::endSpool( const OUString& /*rPrintername*/, const OUString& /*rJobTitle*/, FILE* /* pFile */, const JobData& /*rDocumentJobData*/, bool /*bBanner*/, const OUString& /*rFaxNumber*/ )
{
    return true;
}

void PrinterInfoManager::setupJobContextData( JobData& /* rData */ )
{

}

void PrinterInfoManager::setDefaultPaper( PPDContext& /* rContext */ ) const
{
    (void) this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
