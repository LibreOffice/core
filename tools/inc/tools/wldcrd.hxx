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
#ifndef _WLDCRD_HXX
#define _WLDCRD_HXX

#include "tools/toolsdllapi.h"
#include <tools/string.hxx>
#include <osl/thread.h>

class TOOLS_DLLPUBLIC WildCard
{
private:
    rtl::OString aWildString;
    char cSepSymbol;

    sal_uInt16          ImpMatch( const char *pWild, const char *pStr ) const;

public:
    WildCard()
        : aWildString('*')
        , cSepSymbol('\0')
    {
    }

    WildCard(const rtl::OUString& rWildCard, const char cSeparator = '\0')
        : aWildString(rtl::OUStringToOString(rWildCard, osl_getThreadTextEncoding()))
        , cSepSymbol(cSeparator)
    {
    }

    const rtl::OUString getGlob() const
    {
        return rtl::OStringToOUString(aWildString, osl_getThreadTextEncoding());
    }

    void setGlob(const rtl::OUString& rString)
    {
        aWildString = rtl::OUStringToOString(rString, osl_getThreadTextEncoding());
    }

    sal_Bool            Matches( const String& rStr ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
