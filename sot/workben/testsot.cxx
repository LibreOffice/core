/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testsot.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:12:05 $
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
#include "precompiled_sot.hxx"

#include <storage.hxx>

/*
 * main.
 */
int main (int argc, sal_Char **argv)
{
     SotStorageRef xStor = new SotStorage( "c:\\temp\\65254.ppt" );
/*
    SotStorageRef xStor = new SotStorage( "c:\\temp\\video.sdb" );
    SotStorageRef xStor = new SotStorage( "c:\\temp\\video.sdb" );
    SotStorageRef x2Stor = xStor->OpenSotStorage( "1117" );

    SotStorageStreamRef xStm = x2Stor->OpenSotStream( "Genres", STREAM_STD_READWRITE | STREAM_TRUNC);
    //BYTE szData[100];
    //xStm->Write( szData, 100 );
    UINT32 nSize = xStm->GetSize();
    xStm->SetSize( 0 );
    nSize = xStm->GetSize();
    x2Stor->Commit();
    xStor->Commit();
*/
    return 0;
}

