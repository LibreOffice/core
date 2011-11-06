/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

