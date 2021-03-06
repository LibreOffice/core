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

#pragma once

#include <vector>
#include <memory>
#include <rtl/ustring.hxx>

struct RootData;
class XclImpRoot;
class XclImpStream;

class ExcScenarioCell
{
private:
    OUString                        aValue;
public:
    const sal_uInt16                nCol;
    const sal_uInt16                nRow;

    ExcScenarioCell( const sal_uInt16 nC, const sal_uInt16 nR );

    void SetValue( const OUString& rVal ) { aValue = rVal; }

    const OUString& GetValue() const { return aValue; }
};

class ExcScenario final
{
public:
    ExcScenario( XclImpStream& rIn, const RootData& rRoot );

    void Apply( const XclImpRoot& rRoot, const bool bLast );

private:
    OUString         aName;
    OUString         aComment;
    sal_uInt8        nProtected;
    const sal_uInt16 nTab;
    std::vector<ExcScenarioCell> aEntries;
};

struct ExcScenarioList
{
    ExcScenarioList () : nLastScenario(0) {}

    void Apply( const XclImpRoot& rRoot );

    sal_uInt16 nLastScenario;
    std::vector< std::unique_ptr<ExcScenario> > aEntries;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
