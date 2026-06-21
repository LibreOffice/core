/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <map>
#include <vector>

#include <formula/opcode.hxx>
#include <formula/token.hxx>
#include <formula/types.hxx>
#include <sal/log.hxx>

#include <formula/callable.hxx>

namespace formula
{
/// a static hash of all the built-ins that have been accessed
static std::map<OpCode, FormulaCallableRef> aAllBuiltIns;

// We're ensuring that a FormulaBuiltInFunction corresponding to eOpCode is
// in our map, and returning a reference to it. This allows us to ensure that
// at most one FormulaBuiltInFunction exists for each OpCode.
FormulaCallableRef FormulaBuiltInFunction::Get(OpCode eOpCode)
{
    auto iter = aAllBuiltIns.find(eOpCode);
    if (iter == aAllBuiltIns.end())
        return (aAllBuiltIns[eOpCode] = new FormulaBuiltInFunction(eOpCode));
    else
        return iter->second;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
