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

#include "salprn.hxx"
#include "headless/svpgdi.hxx"
#include "headless/svpinst.hxx"

SalPrinter* SvpSalInstance::CreatePrinter( SalInfoPrinter* /* pInfoPrinter */ )
{
    return NULL;
}

OUString SvpSalInstance::GetDefaultPrinter()
{
    return OUString();
}

GenPspGraphics *SvpSalInstance::CreatePrintGraphics()
{
    return NULL;
}

void SvpSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

void SvpSalInstance::PostPrintersChanged()
{
}

SalInfoPrinter* SvpSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* /* pQueueInfo */,
                                                   ImplJobSetup*        /* pJobSetup */ )
{
    return NULL;
}

void SvpSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

void SvpSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* /* pList */ )
{
}

void SvpSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* /* pInfo */ )
{
}

void SvpSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

SalPrinter* SalInstance::CreatePrinter( SalInfoPrinter* /* pInfoPrinter */ )
{
    return NULL;
}

OUString SalInstance::GetDefaultPrinter()
{
    return OUString();
}

void SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

void SalInstance::PostPrintersChanged()
{
}

SalInfoPrinter* SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* /* pQueueInfo */,
                                                       ImplJobSetup*        /* pJobSetup */ )
{
    return NULL;
}

void SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

void SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* /* pList */ )
{
}

void SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* /* pInfo */ )
{
}

void SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

void SalInstance::updatePrinterUpdate()
{
}

void SalInstance::jobStartedPrinterUpdate()
{
}

void SalInstance::jobEndedPrinterUpdate()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
