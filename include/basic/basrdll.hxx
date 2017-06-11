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

#ifndef INCLUDED_BASIC_BASRDLL_HXX
#define INCLUDED_BASIC_BASRDLL_HXX

#include <basic/basicdllapi.h>
#include <memory>

class BASIC_DLLPUBLIC BasicDLL
{
public:
    struct Impl;
    std::unique_ptr<Impl> m_xImpl;

public:
    BasicDLL();
    ~BasicDLL();

    const std::locale& GetBasResLocale() const;

    static void BasicBreak();

    static void EnableBreak( bool bEnable );
    static void SetDebugMode( bool bDebugMode );
};

#endif // INCLUDED_BASIC_BASRDLL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
