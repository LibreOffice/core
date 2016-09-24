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

#include "sbintern.hxx"
#include "sbunoobj.hxx"
#include "token.hxx"
#include "symtbl.hxx"
#include "parser.hxx"
#include "codegen.hxx"
#include <basic/basmgr.hxx>

SbiGlobals* SbiGlobals::pGlobals = nullptr;

SbiGlobals* GetSbData()
{
    if( !SbiGlobals::pGlobals )
        SbiGlobals::pGlobals = new SbiGlobals;
    return SbiGlobals::pGlobals;
}

SbiGlobals::SbiGlobals()
{
    pInst = nullptr;
    pSbFac= nullptr;
    pUnoFac = nullptr;
    pTypeFac = nullptr;
    pClassFac = nullptr;
    pOLEFac = nullptr;
    pFormFac = nullptr;
    pMod  = nullptr;
    pCompMod = nullptr; // JSM
    nInst = 0;
    nCode = 0;
    nLine = 0;
    nCol1 = nCol2 = 0;
    bCompiler = false;
    bGlobalInitErr = false;
    bRunInit = false;
    pTransliterationWrapper = nullptr;
    bBlockCompilerError = false;
    pAppBasMgr = nullptr;
    pMSOMacroRuntimLib = nullptr;
}

SbiGlobals::~SbiGlobals()
{
    delete pSbFac;
    delete pUnoFac;
    delete pTransliterationWrapper;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
