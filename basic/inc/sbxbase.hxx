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

#ifndef INCLUDED_BASIC_INC_SBXBASE_HXX
#define INCLUDED_BASIC_INC_SBXBASE_HXX

#include <i18nlangtag/lang.h>
#include <basic/sbxdef.hxx>
#include <basic/basicdllapi.h>
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
    std::vector<std::unique_ptr<SbxFactory>>
                        m_Factories;

    // Pointer to Format()-Command helper class
    std::unique_ptr<SbxBasicFormater>   pBasicFormater;

    LanguageType        eBasicFormaterLangType;
    // It might be useful to store this class 'global' because some string resources are saved here

    SbxAppData();
    SbxAppData(const SbxAppData&) = delete;
    const SbxAppData& operator=(const SbxAppData&) = delete;
    ~SbxAppData();
};

BASIC_DLLPUBLIC SbxAppData& GetSbxData_Impl();

#endif // INCLUDED_BASIC_INC_SBXBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
