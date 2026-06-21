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

#include <config_features.h>

#include <sal/config.h>

#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbxvar.hxx>
#include <formula/opcode.hxx>
#include <osl/diagnose.h>
#include <sfx2/app.hxx>
#include <docsh.hxx>
#include <interpre.hxx>

#include <callable.hxx>

ScMacroFunction::ScMacroFunction(const ScInterpreter& rInterpreter, const OUString& aMacro)
    : formula::FormulaCallable()
    , mnError(FormulaError::NONE)
    , mpDocShell(rInterpreter.mrDoc.GetDocumentShell())
    , mpMethod(nullptr)
    , mpModule(nullptr)
    , mbInvalid(false)
{
#if !HAVE_FEATURE_SCRIPTING
    (void)rInterpreter;
    (void)aMacro;
    mbInvalid = true; // without DocShell no CallBasic
#else
    SbxBase::ResetError();

    if (!mpDocShell)
    {
        mbInvalid = true; // without DocShell no CallBasic
        return;
    }

    //  no security queue beforehand (just CheckMacroWarn), moved to  CallBasic

    //  If the  Dok was loaded during a Basic-Call,
    //  is the  Sbx-object created(?)
    //  pDocSh->GetSbxObject();

    //  search function with the name,
    //  then assemble  SfxObjectShell::CallBasic from aBasicStr, aMacroStr

    StarBASIC* pRoot;

    try
    {
        pRoot = mpDocShell->GetBasic();
    }
    catch (...)
    {
        pRoot = nullptr;
    }

    SbxVariable* pVar = pRoot ? pRoot->Find(aMacro, SbxClassType::Method) : nullptr;
    if (!pVar || pVar->GetType() == SbxVOID)
    {
        mbInvalid = true;
        mnError = FormulaError::NoMacro;
        return;
    }
    mpMethod = dynamic_cast<SbMethod*>(pVar);
    if (!mpMethod)
    {
        mbInvalid = true;
        mnError = FormulaError::NoMacro;
        return;
    }
    mpModule = mpMethod->GetModule();
    if (!mpModule)
    {
        mbInvalid = true;
        mnError = FormulaError::NoMacro;
        return;
    }

    bool bUseVBAObjects = mpModule->IsVBASupport();
    SbxObject* pObject = mpModule->GetParent();
    assert(pObject);
    OSL_ENSURE(dynamic_cast<const StarBASIC*>(pObject) != nullptr, "No Basic found!");
    maMacroStr = pObject->GetName() + "." + mpModule->GetName() + "." + mpMethod->GetName();
    if (pRoot && bUseVBAObjects)
    {
        // just here to make sure the VBA objects when we run the macro during ODF import
        pRoot->getVBAGlobals();
    }
    if (pObject->GetParent())
        maBasicStr = pObject->GetParent()->GetName(); // document BASIC
    else
        maBasicStr = SfxGetpApp()->GetName(); // application BASIC
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
