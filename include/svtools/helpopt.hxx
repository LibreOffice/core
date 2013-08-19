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
#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#define INCLUDED_SVTOOLS_HELPOPT_HXX

#include "svtools/svtdllapi.h"

#include <list>
#include <tools/string.hxx>
#include <unotools/options.hxx>

class SvtHelpOptions_Impl;

class SVT_DLLPUBLIC SvtHelpOptions: public utl::detail::Options
{
    SvtHelpOptions_Impl*    pImp;

public:
                    SvtHelpOptions();
                    virtual ~SvtHelpOptions();

    void            SetExtendedHelp( sal_Bool b );
    sal_Bool        IsExtendedHelp() const;
    void            SetHelpTips( sal_Bool b );
    sal_Bool        IsHelpTips() const;

    const OUString& GetHelpStyleSheet()const;
    void            SetHelpStyleSheet(const OUString& rStyleSheet);

    void            SetWelcomeScreen( sal_Bool b );
    sal_Bool        IsWelcomeScreen() const;

    OUString        GetSystem() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
