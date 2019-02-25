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
#include <vcl/svapp.hxx>
#include "salprn.hxx"
#include "quartz/salgdi.h"
#include "headless/svpinst.hxx"
#include "unx/fontmanager.hxx"
#include "unx/gendata.hxx"

class GlyphCache
{
};

std::unique_ptr<SalPrinter> SvpSalInstance::CreatePrinter( SalInfoPrinter* /* pInfoPrinter */ )
{
    return nullptr;
}

OUString SvpSalInstance::GetDefaultPrinter()
{
    return OUString();
}

GenPspGraphics *SvpSalInstance::CreatePrintGraphics()
{
    return NULL;
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

std::unique_ptr<SalPrinter> SalGenericInstance::CreatePrinter( SalInfoPrinter* /* pInfoPrinter */ )
{
    return nullptr;
}

OUString SalGenericInstance::GetDefaultPrinter()
{
    return OUString();
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

void SalGenericInstance::updatePrinterUpdate()
{
}

void SalGenericInstance::jobStartedPrinterUpdate()
{
}

void SalGenericInstance::jobEndedPrinterUpdate()
{
}

bool AquaSalGraphics::drawEPS( long, long, long, long, void*, sal_uInt32 )
{
    return false;
}

using namespace psp;

GenericUnixSalData::GenericUnixSalData(GenericUnixSalDataType const t, SalInstance *const pInstance)
    : m_eType(t)
    , m_pDisplay(nullptr)
    , m_pGlyphCache(new GlyphCache)
    , m_pPrintFontManager(nullptr)
{
    m_pInstance = pInstance;
    SetSalData(this);
}

GenericUnixSalData::~GenericUnixSalData()
{
}

PrintFontManager::~PrintFontManager()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
