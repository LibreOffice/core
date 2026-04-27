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

#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <comphelper/solarmutex.hxx>

#include <rtl/ustrbuf.hxx>
#include <vcl/QueueInfo.hxx>

#include <osx/saldata.hxx>
#include <osx/MacInstance.hxx>
#include <osx/salprn.h>
#include <osx/salmutex.hxx>
#include <quartz/utils.h>

#include <print.h>
#include <strings.hrc>

#include <Foundation/Foundation.h>

extern "C" {
#include <crt_externs.h>
}

static OUString& getFallbackPrinterName()
{
    static OUString aFallbackPrinter;

    if ( aFallbackPrinter.isEmpty() )
    {
        aFallbackPrinter = VclResId( SV_PRINT_DEFPRT_TXT );
        if ( aFallbackPrinter.isEmpty() )
            aFallbackPrinter = "Printer";
    }

    return aFallbackPrinter;
}

MacInstance::MacInstance()
    : SalInstance(std::make_unique<AquaSalYieldMutex>(), new SalData)
    , mnActivePrintJobs( 0 )
{
}

MacInstance::~MacInstance()
{
}

std::unique_ptr<SalPrinter> MacInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    return std::unique_ptr<SalPrinter>(new AquaSalPrinter( dynamic_cast<AquaSalInfoPrinter*>(pInfoPrinter) ));
}

void MacInstance::GetPrinterQueueInfo(ImplPrnQueueList& rList)
{
    NSArray* pNames = [NSPrinter printerNames];
    NSArray* pTypes = [NSPrinter printerTypes];
    unsigned int nNameCount = pNames ? [pNames count] : 0;
    unsigned int nTypeCount = pTypes ? [pTypes count] : 0;
    SAL_WARN_IF( nTypeCount != nNameCount, "vcl", "type count not equal to printer count" );
    for( unsigned int i = 0; i < nNameCount; i++ )
    {
        NSString* pName = [pNames objectAtIndex: i];
        NSString* pType = i < nTypeCount ? [pTypes objectAtIndex: i] : nil;
        if( pName )
        {
            std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
            pInfo->maPrinterName    = GetOUString( pName );
            if( pType )
                pInfo->maDriver     = GetOUString( pType );
            pInfo->mnStatus         = PrintQueueFlags::NONE;
            pInfo->mnJobs           = 0;

            rList.Add(std::move(pInfo));
        }
    }

    // tdf#151700 Prevent the non-native LibreOffice PrintDialog from
    // displaying by creating a fake printer if there are no printers. This
    // will allow the LibreOffice printing code to proceed with native
    // NSPrintOperation which will display the native print panel.
    if ( !nNameCount )
    {
        std::unique_ptr<SalPrinterQueueInfo> pInfo(new SalPrinterQueueInfo);
        pInfo->maPrinterName    = getFallbackPrinterName();
        pInfo->mnStatus         = PrintQueueFlags::NONE;
        pInfo->mnJobs           = 0;

        rList.Add(std::move(pInfo));
    }
}

OUString MacInstance::GetDefaultPrinter()
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    // WinSalInstance::GetDefaultPrinter() fetches current default printer
    // on every call so do the same here
    OUString aDefaultPrinter;
    {
        NSPrintInfo* pPI = [NSPrintInfo sharedPrintInfo];
        SAL_WARN_IF( !pPI, "vcl", "no print info" );
        if( pPI )
        {
            NSPrinter* pPr = [pPI printer];
            SAL_WARN_IF( !pPr, "vcl", "no printer in default info" );
            if( pPr )
            {
                // Related: tdf#151700 Return the name of the fake printer if
                // there are no printers so that the LibreOffice printing code
                // will be able to find the fake printer returned by
                // AquaSalInstance::GetPrinterQueueInfo()
                NSString* pDefName = [pPr name];
                SAL_WARN_IF( !pDefName, "vcl", "printer has no name" );
                if ( pDefName && [pDefName length])
                    aDefaultPrinter = GetOUString( pDefName );
                else
                    aDefaultPrinter = getFallbackPrinterName();
            }
        }
    }
    return aDefaultPrinter;
}

SalInfoPrinter* MacInstance::CreateInfoPrinter(SalPrinterQueueInfo& rQueueInfo,
                                               ImplJobSetup& rSetupData)
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    SalInfoPrinter* pNewInfoPrinter = new AquaSalInfoPrinter(rQueueInfo);
    pNewInfoPrinter->SetPrinterData(rSetupData);

    return pNewInfoPrinter;
}

void MacInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    // #i113170# may not be the main thread if called from UNO API
    SalData::ensureThreadAutoreleasePool();

    SalInstance::DestroyInfoPrinter(pPrinter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
