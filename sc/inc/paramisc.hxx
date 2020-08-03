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

#include "address.hxx"
#include <optional>

struct ScSolveParam
{
    ScAddress                   aRefFormulaCell;
    ScAddress                   aRefVariableCell;
    std::optional<OUString>   pStrTargetVal;

    ScSolveParam();
    ScSolveParam( const ScSolveParam& r );
    ScSolveParam( const ScAddress&  rFormulaCell,
                  const ScAddress&  rVariableCell,
                  const OUString& rTargetValStr );
    ~ScSolveParam();

    ScSolveParam&  operator=   ( const ScSolveParam& r );
    bool           operator==  ( const ScSolveParam& r ) const;
};

/**
 * Parameter for data table aka multiple operations.
 */
struct SC_DLLPUBLIC ScTabOpParam
{
    enum Mode { Column = 0, Row = 1, Both = 2 };

    ScRefAddress    aRefFormulaCell;
    ScRefAddress    aRefFormulaEnd;
    ScRefAddress    aRefRowCell;
    ScRefAddress    aRefColCell;
    Mode meMode;

    ScTabOpParam();
    ScTabOpParam( const ScTabOpParam& r );
    ScTabOpParam(
        const ScRefAddress& rFormulaCell, const ScRefAddress& rFormulaEnd,
        const ScRefAddress& rRowCell, const ScRefAddress& rColCell, Mode eMode );

    ScTabOpParam&   operator=       ( const ScTabOpParam& r );
    bool operator== ( const ScTabOpParam& r ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
