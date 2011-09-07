/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _WLDCRD_HXX
#define _WLDCRD_HXX

#include "tools/toolsdllapi.h"
#include <tools/string.hxx>
#include <osl/thread.h>

// ------------
// - WildCard -
// ------------

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
