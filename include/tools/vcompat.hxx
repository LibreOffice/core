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
#ifndef INCLUDED_TOOLS_VCOMPAT_HXX
#define INCLUDED_TOOLS_VCOMPAT_HXX

#include <tools/toolsdllapi.h>

inline sal_uInt32 COMPAT_FORMAT( char char1, char char2, char char3, char char4 )
{
    return
        static_cast<sal_uInt32>(char1) |
        (static_cast<sal_uInt32>(char2) <<  8) |
        (static_cast<sal_uInt32>(char3) << 16) |
        (static_cast<sal_uInt32>(char4) << 24);
};

class SvStream;
enum class StreamMode;

class TOOLS_DLLPUBLIC VersionCompat
{
    SvStream*       mpRWStm;
    sal_uInt32      mnCompatPos;
    sal_uInt32      mnTotalSize;
    StreamMode   mnStmMode;
    sal_uInt16      mnVersion;

                    VersionCompat( const VersionCompat& ) = delete;
    VersionCompat&  operator=( const VersionCompat& ) { return *this; }

public:
                    VersionCompat( SvStream& rStm, StreamMode nStreamMode, sal_uInt16 nVersion = 1 );
                    ~VersionCompat();

    sal_uInt16      GetVersion() const { return mnVersion; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
