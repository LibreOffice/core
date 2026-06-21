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

#pragma once

#include <sal/config.h>

#include <formula/callable.hxx>
#include <formula/opcode.hxx>
#include <interpre.hxx>

class ScDocument;
class ScAddress;
class ScFormulaCell;
struct ScInterpreterContext;
class ScDocShell;
class SbMethod;
class SbModule;

/// a callable macro
class SAL_DLLPUBLIC_RTTI ScMacroFunction : public formula::FormulaCallable
{
private:
    FormulaError mnError;
    ScDocShell* mpDocShell;
    SbMethod* mpMethod;
    SbModule* mpModule;
    OUString maMacroStr;
    OUString maBasicStr;
    bool mbInvalid;

public:
    ScMacroFunction(const ScMacroFunction&) = default;
    ScMacroFunction(const ScInterpreter& rInterpreter, const OUString& aMacro);
    virtual OpCode GetOpCode() const override { return ocMacro; }
    bool IsValid() const { return !mbInvalid; }
    FormulaError GetError() const { return mnError; }
    ScDocShell* GetDocumentShell() const { return mpDocShell; }
    SbMethod* GetMethod() const { return mpMethod; }
    SbModule* GetModule() const { return mpModule; }
    const OUString& GetMacroStr() const { return maMacroStr; }
    const OUString& GetBasicStr() const { return maBasicStr; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
