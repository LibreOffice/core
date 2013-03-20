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

sal_Bool SvpSalGraphics::CreateFontSubset( const OUString&    /* rToFile */,
                                           const PhysicalFontFace* /* pFontData */,
                                           sal_Int32*              /* pGlyphIDs */,
                                           sal_uInt8*              /* pEncoding */,
                                           sal_Int32*              /* pGlyphWidths */,
                                           int                     /* nGlyphCount */,
                                           FontSubsetInfo&         /* rInfo */ )
{
    return sal_False;
}

bool SvpSalGraphics::AddTempDevFont( ImplDevFontList*     /* pFontList */,
                                     const OUString& /* rFileURL */,
                                     const OUString& /* rFontName */ )
{
    return false;
}

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

SalPrinter* SalGenericInstance::CreatePrinter( SalInfoPrinter* /* pInfoPrinter */ )
{
    return NULL;
}

OUString SalGenericInstance::GetDefaultPrinter()
{
    return OUString();
}

void SalGenericInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

void SalGenericInstance::PostPrintersChanged()
{
}

SalInfoPrinter* SalGenericInstance::CreateInfoPrinter( SalPrinterQueueInfo* /* pQueueInfo */,
                                                       ImplJobSetup*        /* pJobSetup */ )
{
    return NULL;
}

void SalGenericInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

void SalGenericInstance::GetPrinterQueueInfo( ImplPrnQueueList* /* pList */ )
{
}

void SalGenericInstance::GetPrinterQueueState( SalPrinterQueueInfo* /* pInfo */ )
{
}

void SalGenericInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

void SalGenericInstance::updatePrinterUpdate()
{
}

void SalGenericInstance::jobStartedPrinterUpdate()
{
}

void SalGenericInstance::jobEndedPrinterUpdate()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
