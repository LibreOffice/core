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

#include <parse.hxx>
#include <smmod.hxx>
#include <cfgitem.hxx>

SmParser::SmParser()
{
    SmModule* pMod = SM_MOD();
    if (pMod)
        nSmSyntaxVersion = pMod->GetConfig()->GetSmSyntaxVersion();
    else
        // Defaulted as 5 so I have time to code the parser 6
        nSmSyntaxVersion = 5;
    switch (nSmSyntaxVersion)
    {
        case 5:
            smp5 = new SmParser5();
            break;
        case 6:
            smp6 = new SmParser6();
            break;
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

SmParser::~SmParser()
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            delete smp5;
            break;
        case 6:
            delete smp6;
            break;
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

std::unique_ptr<SmTableNode> SmParser::Parse(const OUString& rBuffer)
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return std::move(smp5->Parse(rBuffer));
        case 6:
            return std::move(smp6->Parse(rBuffer));
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

std::unique_ptr<SmNode> SmParser::ParseExpression(const OUString& rBuffer)
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return std::move(smp5->ParseExpression(rBuffer));
        case 6:
            return std::move(smp6->ParseExpression(rBuffer));
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

const OUString& SmParser::GetText() const
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->GetText();
        case 6:
            return smp6->GetText();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

bool SmParser::IsImportSymbolNames() const
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->IsImportSymbolNames();
        case 6:
            return smp6->IsImportSymbolNames();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

void SmParser::SetImportSymbolNames(bool bVal)
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            smp5->SetImportSymbolNames(bVal);
            return;
        case 6:
            smp6->SetImportSymbolNames(bVal);
            return;
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

bool SmParser::IsExportSymbolNames() const
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->IsExportSymbolNames();
        case 6:
            return smp6->IsExportSymbolNames();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

void SmParser::SetExportSymbolNames(bool bVal)
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->SetExportSymbolNames(bVal);
        case 6:
            return smp6->SetExportSymbolNames(bVal);
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

const SmErrorDesc* SmParser::NextError()
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->NextError();
        case 6:
            return smp6->NextError();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

const SmErrorDesc* SmParser::PrevError()
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->PrevError();
        case 6:
            return smp6->PrevError();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

const SmErrorDesc* SmParser::GetError() const
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->GetError();
        case 6:
            return smp6->GetError();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

const std::set<OUString>& SmParser::GetUsedSymbols() const
{
    switch (nSmSyntaxVersion)
    {
        case 5:
            return smp5->GetUsedSymbols();
        case 6:
            return smp6->GetUsedSymbols();
        default:
            throw std::range_error("invalid smsyntax version");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
