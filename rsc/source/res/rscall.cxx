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


#include <stdlib.h>
#include <stdio.h>

#include <rscall.h>
#include <rsckey.hxx>

Atom    nRsc_XYMAPMODEId = InvalidAtom;
Atom    nRsc_WHMAPMODEId = InvalidAtom;
Atom    nRsc_X = InvalidAtom;
Atom    nRsc_Y = InvalidAtom;
Atom    nRsc_WIDTH = InvalidAtom;
Atom    nRsc_HEIGHT = InvalidAtom;
Atom    nRsc_DELTALANG = InvalidAtom;
Atom    nRsc_DELTASYSTEM = InvalidAtom;
Atom    nRsc_EXTRADATA = InvalidAtom;

void InitRscCompiler()
{
    pStdParType      = new OString(RTL_CONSTASCII_STRINGPARAM(
        "( const ResId & rResId, sal_Bool"));
    pStdPar1         = new OString('(');
    pStdPar2         = new OString('(');

    pWinParType      = new OString(RTL_CONSTASCII_STRINGPARAM(
        "( Window * pParent, const ResId & rResId, sal_Bool"));
    pWinPar1         = new OString(RTL_CONSTASCII_STRINGPARAM(
        "( pParent,"));
    pWinPar2         = new OString(RTL_CONSTASCII_STRINGPARAM(
        "( this,"));
    nRefDeep         = 10;
    nRsc_XYMAPMODEId = InvalidAtom;
    nRsc_WHMAPMODEId = InvalidAtom;
    pHS = new AtomContainer();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
