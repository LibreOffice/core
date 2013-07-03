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

#ifndef _SOT_STORINFO_HXX
#define _SOT_STORINFO_HXX

#include <tools/globname.hxx>
#include <tools/solar.h>
#include <vector>
#include "sot/sotdllapi.h"

class StgDirEntry;
class SvStorageInfo
{
friend class SvStorage;
    OUString        aName;
    SvGlobalName    aClassName;
    sal_uLong       nSize;
    bool            bStream;
    bool            bStorage;

                            SvStorageInfo(){}; // Fuer SvStorage
public:
                            SvStorageInfo( const StgDirEntry& );
                            SvStorageInfo( const OUString& rName, sal_uLong nSz, bool bIsStorage )
                                : aName( rName )
                                , nSize( nSz )
                                , bStream( !bIsStorage )
                                , bStorage( bIsStorage )
                            {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const OUString &        GetName() const { return aName; }
    bool                    IsStream() const { return bStream; }
    bool                    IsStorage() const { return bStorage; }
    sal_uLong               GetSize() const { return nSize;      }
};

typedef ::std::vector< SvStorageInfo > SvStorageInfoList;

SOT_DLLPUBLIC sal_uLong ReadClipboardFormat( SvStream & rStm );
SOT_DLLPUBLIC void WriteClipboardFormat( SvStream & rStm, sal_uLong nFormat );

#endif // _STORINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
