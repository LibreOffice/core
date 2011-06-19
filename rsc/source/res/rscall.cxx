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
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>

// Programmabhaengige Includes.
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
    pStdParType      = new ByteString( "( const ResId & rResId, sal_Bool" );
    pStdPar1         = new ByteString( '(' );
    pStdPar2         = new ByteString( '(' );

    pWinParType      = new ByteString( "( Window * pParent, const ResId & rResId, sal_Bool" );
    pWinPar1         = new ByteString( "( pParent," );
    pWinPar2         = new ByteString( "( this," );
    nRefDeep         = 10;
    nRsc_XYMAPMODEId = InvalidAtom;
    nRsc_WHMAPMODEId = InvalidAtom;
    pHS = new AtomContainer();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
