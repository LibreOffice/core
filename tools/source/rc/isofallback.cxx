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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <tools/isofallback.hxx>

// -----------------------------------------------------------------------

// Return true if valid fallback found
bool GetIsoFallback(rtl::OString& rLanguage)
{
    rLanguage = rLanguage.trim();
    if (!rLanguage.isEmpty())
    {
        sal_Int32 nSepPos = rLanguage.indexOf('-');
        if (nSepPos == -1)
        {
            if (rLanguage == rtl::OString(RTL_CONSTASCII_STRINGPARAM("en")))
            {
                // en -> ""
                rLanguage = rtl::OString();
                return false;
            }
            else
            {
                // de -> en-US ;
                rLanguage = rtl::OString(RTL_CONSTASCII_STRINGPARAM("en-US"));
                return true;
            }
        }
        else if( !(nSepPos == 1 && (rLanguage.toChar() == 'x' || rLanguage.toChar() == 'X')) )
        {
            // de-CH -> de ;
            // try erase from -
            sal_Int32 nIndex = 0;
            rLanguage = rLanguage.getToken(0, '-', nIndex);
            return true;
        }
    }
    // "" -> ""; x-no-translate -> ""
    rLanguage = rtl::OString();
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
