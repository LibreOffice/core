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

#ifndef INCLUDED_SW_SOURCE_CORE_CRSR_CALLNK_HXX
#define INCLUDED_SW_SOURCE_CORE_CRSR_CALLNK_HXX

#include <tools/solar.h>

class SwCursorShell;
class SwTextNode;
class SwRootFrame;

class SwCallLink
{
public:
    SwCursorShell & rShell;
    sal_uLong nNode;
    sal_Int32 nContent;
    sal_uInt8 nNdTyp;
    long nLeftFramePos;
    bool bHasSelection;

    explicit SwCallLink( SwCursorShell & rSh );
    SwCallLink( SwCursorShell & rSh, sal_uLong nAktNode, sal_Int32 nAktContent,
                                    sal_uInt8 nAktNdTyp, long nLRPos,
                                    bool bAktSelection );
    ~SwCallLink();

    static long getLayoutFrame( const SwRootFrame*, SwTextNode& rNd, sal_Int32 nCntPos, bool bCalcFrame );
};

#endif // INCLUDED_SW_SOURCE_CORE_CRSR_CALLNK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
