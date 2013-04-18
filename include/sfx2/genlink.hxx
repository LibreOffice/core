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
#ifndef _SFXGENLINK_HXX
#define _SFXGENLINK_HXX


#include <tools/link.hxx>
typedef long (*CFuncPtr)(void*);

class GenLink
{
    Link        aLink;
    CFuncPtr    pFunc;

public:
    GenLink(): pFunc(0) {}
    GenLink( CFuncPtr pCFunc ): pFunc(pCFunc) {}
    GenLink( const Link& rLink ): aLink(rLink), pFunc(0) {}
    GenLink( const GenLink& rOrig ):
        aLink(rOrig.aLink), pFunc(rOrig.pFunc) {}

    GenLink& operator = ( const GenLink& rOrig )
        { pFunc = rOrig.pFunc; aLink = rOrig.aLink; return *this; }

    sal_Bool operator!() const { return !aLink && !pFunc; }
    sal_Bool IsSet() const { return aLink.IsSet() || pFunc; }

    long Call( void* pCaller )
         { return pFunc ? (*pFunc)(pCaller) : aLink.Call(pCaller); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
