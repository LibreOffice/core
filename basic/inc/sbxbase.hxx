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

#include <sal/config.h>

#include <basic/sbxvar.hxx>
#include <i18nlangtag/lang.h>
#include <vcl/errcode.hxx>

#include <memory>
#include <vector>

class SbxFactory;
class SbxVariable;
class SbxBasicFormater;

// AppData structure for SBX:
struct SbxAppData
{
    ErrCode             eErrCode;  // Error code
    SbxVariableRef      m_aGlobErr; // Global error object
    std::vector<SbxFactory*> m_Factories; // these are owned by fields in SbiGlobals
    tools::SvRef<SvRefBase>  mrImplRepository;

    // Pointer to Format()-Command helper class
    std::unique_ptr<SbxBasicFormater>   pBasicFormater;

    LanguageType        eBasicFormaterLangType;
    // It might be useful to store this class 'global' because some string resources are saved here

    SbxAppData();
    SbxAppData(const SbxAppData&) = delete;
    const SbxAppData& operator=(const SbxAppData&) = delete;
    ~SbxAppData();
};

SbxAppData& GetSbxData_Impl();
/** returns true if the SbxAppData is still valid, used to check if we are in shutdown. */
bool IsSbxData_Impl();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
