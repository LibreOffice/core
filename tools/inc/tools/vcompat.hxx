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
#ifndef _VCOMPAT_HXX
#define _VCOMPAT_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

#define COMPAT_FORMAT( char1, char2, char3, char4 ) \
    ((sal_uInt32)((((sal_uInt32)(char)(char1)))|                \
    (((sal_uInt32)(char)(char2))<<8UL)|                 \
    (((sal_uInt32)(char)(char3))<<16UL)|                    \
    ((sal_uInt32)(char)(char4))<<24UL))

class SvStream;

class TOOLS_DLLPUBLIC VersionCompat
{
    SvStream*       mpRWStm;
    sal_uInt32          mnCompatPos;
    sal_uInt32          mnTotalSize;
    sal_uInt16          mnStmMode;
    sal_uInt16          mnVersion;

                    VersionCompat() {}
                    VersionCompat( const VersionCompat& ) {}
    VersionCompat&  operator=( const VersionCompat& ) { return *this; }
    sal_Bool            operator==( const VersionCompat& ) { return sal_False; }

public:

                    VersionCompat( SvStream& rStm, sal_uInt16 nStreamMode, sal_uInt16 nVersion = 1 );
                    ~VersionCompat();

    sal_uInt16          GetVersion() const { return mnVersion; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
