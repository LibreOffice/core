/*************************************************************************
 *
 *  $RCSfile: rscall.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscall.cxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.12  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.11  2000/07/26 17:13:22  willem.vandorp
    Headers/footers replaced

    Revision 1.10  2000/07/12 11:37:45  th
    Unicode

    Revision 1.9  1997/08/27 18:17:36  MM
    neue Headerstruktur

**************************************************************************/
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
#ifndef _RSCHASH_HXX
#include <rschash.hxx>
#endif

USHORT  nRsc_XYMAPMODEId;
USHORT  nRsc_WHMAPMODEId;
USHORT  nRsc_X;
USHORT  nRsc_Y;
USHORT  nRsc_WIDTH;
USHORT  nRsc_HEIGHT;
USHORT  nRsc_DELTALANG;
USHORT  nRsc_DELTASYSTEM;
USHORT  nRsc_EXTRADATA;

void InitRscCompiler()
{
    pHS              = new HashString( 3271 );

    pStdParType      = new ByteString( "( const ResId & rResId, BOOL" );
    pStdPar1         = new ByteString( '(' );
    pStdPar2         = new ByteString( '(' );

    pWinParType      = new ByteString( "( Window * pParent, const ResId & rResId, BOOL" );
    pWinPar1         = new ByteString( "( pParent," );
    pWinPar2         = new ByteString( "( this," );
    nRefDeep         = 10;
    nRsc_XYMAPMODEId = HASH_NONAME;
    nRsc_WHMAPMODEId = HASH_NONAME;
};

