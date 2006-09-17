/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rscall.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:01:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>

// Programmabhaengige Includes.
#ifndef _RSCALL_H
#include <rscall.h>
#endif
#ifndef _RSCKEY_HXX
#include <rsckey.hxx>
#endif

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
    pStdParType      = new ByteString( "( const ResId & rResId, BOOL" );
    pStdPar1         = new ByteString( '(' );
    pStdPar2         = new ByteString( '(' );

    pWinParType      = new ByteString( "( Window * pParent, const ResId & rResId, BOOL" );
    pWinPar1         = new ByteString( "( pParent," );
    pWinPar2         = new ByteString( "( this," );
    nRefDeep         = 10;
    nRsc_XYMAPMODEId = InvalidAtom;
    nRsc_WHMAPMODEId = InvalidAtom;
    pHS = new AtomContainer();
};

