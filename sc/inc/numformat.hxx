/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_NUMFORMAT_HXX
#define INCLUDED_SC_NUMFORMAT_HXX

#include <scdllapi.h>

#include <tools/solar.h>

class ScPatternAttr;

namespace sc {

class SC_DLLPUBLIC NumFmtUtil
{
public:

    /**
     * Return whether or not given number format is a 'General' number format.
     */
    static bool isGeneral( sal_uLong nFormat );

    static bool isGeneral( const ScPatternAttr& rPat );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
