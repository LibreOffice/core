/*************************************************************************
 *
 *  $RCSfile: file.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-05-14 08:08:46 $
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

#include <precomp.h>
#include <cosv/file.hxx>

// NOT FULLY DECLARED SERVICES


namespace csv
{


File::File( uintt  i_nMode )
    :   // aPath,
        pStream(0),
        nMode(i_nMode),
        eLastIO(io_none)
{
}

File::File( const ploc::Path &  i_rLocation,
            uintt               i_nMode )
    :   aPath(i_rLocation),
        pStream(0),
        nMode(i_nMode),
        eLastIO(io_none)
{
}

File::File( const char *    i_sLocation,
            uintt           i_nMode )
    :   aPath(i_sLocation),
        pStream(0),
        nMode(i_nMode),
        eLastIO(io_none)
{
}

File::File( const String &  i_sLocation,
            uintt           i_nMode )
    :   aPath(i_sLocation),
        pStream(0),
        nMode(i_nMode),
        eLastIO(io_none)
{
}

File::~File()
{
    if ( inq_is_open() )
        close();
}

bool
File::Assign( ploc::Path i_rLocation )
{
    if (is_open() )
        return false;

    InvalidatePath();
    aPath = i_rLocation;
    return true;
}

bool
File::Assign( const char * i_sLocation )
{
    if (is_open() )
        return false;

    InvalidatePath();
    aPath.Set( i_sLocation );
    return true;
}

bool
File::Assign( const String & i_sLocation )
{
    if (is_open() )
        return false;

    InvalidatePath();
    aPath.Set( i_sLocation );
    return true;
}

uintt
File::do_read( void *          out_pDest,
               uintt           i_nNrofBytes )
{
    if ( NOT inq_is_open() )
        return 0;

    if ( eLastIO == io_write )
        ::fseek( pStream, 0, SEEK_CUR );
    uintt ret = position();
    ::fread( out_pDest, 1, i_nNrofBytes, pStream );
    ret = position() - ret;

    eLastIO = io_read;
    return ret;
}

bool
File::inq_eod() const
{
    if ( NOT inq_is_open() )
        return true;
    return feof(pStream) != 0;
}

uintt
File::do_write( const void *    i_pSrc,
                uintt           i_nNrofBytes )
{
    if ( NOT inq_is_open() )
        return 0;

    if ( eLastIO == io_write )
        ::fseek( pStream, 0, SEEK_CUR );
    uintt ret = position();
    ::fwrite( i_pSrc, 1, i_nNrofBytes, pStream );
    ret = position() - ret;

    eLastIO = io_write;
    return ret;
}

uintt
File::do_seek( intt     i_nDistance,
               seek_dir i_eStartPoint )
{
    if ( NOT inq_is_open() )
        return uintt(-1);

    static int eSearchDir[3] = { SEEK_SET, SEEK_CUR, SEEK_END };

    ::fseek( pStream,
             intt(i_nDistance),
             eSearchDir[i_eStartPoint] );
    return position();
}

uintt
File::inq_position() const
{
    if ( inq_is_open() )
        return uintt( ::ftell(pStream) );
    else
        return uintt(-1);
}

bool
File::do_open( uintt i_nOpenMode )
{
    if ( inq_is_open() )
    {
        if ( i_nOpenMode == 0 OR i_nOpenMode == nMode )
            return true;
        close();
    }

    if ( i_nOpenMode != 0 )
        nMode = i_nOpenMode;

    const char * sFacadeMode = "";
    switch ( nMode )
    {
         case CFM_RW:        sFacadeMode = "r+b";
                            break;
         case CFM_CREATE:    sFacadeMode = "w+b";
                            break;
         case CFM_READ:      sFacadeMode = "rb";
                            break;
        default:
                            sFacadeMode = "rb";
    }

    pStream = ::fopen( StrPath(), sFacadeMode );
    if ( pStream == 0 AND nMode == CFM_RW )
    {
        sFacadeMode = "w+b";
        pStream = ::fopen( StrPath(), sFacadeMode );
    }

    return pStream != 0;
}

void
File::do_close()
{
    if ( inq_is_open() )
    {
        ::fclose(pStream);
        pStream = 0;
    }
    eLastIO = io_none;
}

bool
File::inq_is_open() const
{
    return pStream != 0;
}

const ploc::Path &
File::inq_MyPath() const
{
     return aPath;
}


}   // namespace csv

