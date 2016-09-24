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
#ifndef INCLUDED_TOOLS_WLDCRD_HXX
#define INCLUDED_TOOLS_WLDCRD_HXX

#include <tools/toolsdllapi.h>
#include <osl/thread.h>
#include <rtl/ustring.hxx>

class TOOLS_DLLPUBLIC WildCard
{
private:
    OString aWildString;
    char cSepSymbol;

    static bool ImpMatch( const char *pWild, const char *pStr );

public:
    WildCard()
        : aWildString('*')
        , cSepSymbol('\0')
    {
    }

    WildCard(const OUString& rWildCard, const char cSeparator = '\0')
        : aWildString(OUStringToOString(rWildCard, osl_getThreadTextEncoding()))
        , cSepSymbol(cSeparator)
    {
    }

    const OUString getGlob() const
    {
        return OStringToOUString(aWildString, osl_getThreadTextEncoding());
    }

    void setGlob(const OUString& rString)
    {
        aWildString = OUStringToOString(rString, osl_getThreadTextEncoding());
    }

    bool Matches( const OUString& rStr ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
