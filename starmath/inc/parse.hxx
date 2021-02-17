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

/** Parses the starmath code and creates the nodes.
  *
  */

#ifndef INCLUDED_STARMATH_INC_PARSE_HXX
#define INCLUDED_STARMATH_INC_PARSE_HXX

#include "parse5.hxx"
#include "parse6.hxx"

class SmParser
{
private:
    uint_fast8_t nSmSyntaxVersion;
    SmParser5* smp5;
    SmParser6* smp6;

public:
    SmParser();
    ~SmParser();

    /** Parse rBuffer to formula tree */
    std::unique_ptr<SmTableNode> Parse(const OUString& rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    std::unique_ptr<SmNode> ParseExpression(const OUString& rBuffer);

    const OUString& GetText() const;

    bool IsImportSymbolNames() const;
    void SetImportSymbolNames(bool bVal);
    bool IsExportSymbolNames() const;
    void SetExportSymbolNames(bool bVal);

    const SmErrorDesc* NextError();
    const SmErrorDesc* PrevError();
    const SmErrorDesc* GetError() const;
    const std::set<OUString>& GetUsedSymbols() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
