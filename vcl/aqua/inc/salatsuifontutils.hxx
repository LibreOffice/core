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

#ifndef _SV_SALATSUIFONTUTILS_HXX
#define _SV_SALATSUIFONTUTILS_HXX

class ImplMacFontData;
class ImplDevFontList;

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include <map>

/* This class has the responsibility of assembling a list
   of atsui compatible fonts available on the system and
   enabling access to that list.
 */
class SystemFontList
{
public:
    SystemFontList();
    ~SystemFontList();

    void AnnounceFonts( ImplDevFontList& ) const;
    ImplMacFontData* GetFontDataFromId( ATSUFontID ) const;

    ATSUFontFallbacks maFontFallbacks;

private:
    typedef std::hash_map<ATSUFontID,ImplMacFontData*> MacFontContainer;
    MacFontContainer maFontContainer;

    void InitGlyphFallbacks();
};

#endif  // _SV_SALATSUIFONTUTILS_HXX

