/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdiocmpt.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:21:50 $
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
#include "precompiled_sd.hxx"


#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#include "sdiocmpt.hxx"

//////////////////////////////////////////////////////////////////////////////

old_SdrDownCompat::old_SdrDownCompat(SvStream& rNewStream, UINT16 nNewMode)
:   rStream(rNewStream),
    nSubRecSiz(0),
    nSubRecPos(0),
    nMode(nNewMode),
    bOpen(FALSE)
{
    OpenSubRecord();
}

old_SdrDownCompat::~old_SdrDownCompat()
{
    if(bOpen)
        CloseSubRecord();
}

void old_SdrDownCompat::Read()
{
    rStream >> nSubRecSiz;
}

void old_SdrDownCompat::Write()
{
    rStream << nSubRecSiz;
}

void old_SdrDownCompat::OpenSubRecord()
{
    if(rStream.GetError())
        return;

    nSubRecPos = rStream.Tell();

    if(nMode == STREAM_READ)
    {
        Read();
    }
    else if(nMode == STREAM_WRITE)
    {
        Write();
    }

    bOpen = TRUE;
}

void old_SdrDownCompat::CloseSubRecord()
{
    if(rStream.GetError())
        return;

    UINT32 nAktPos(rStream.Tell());

    if(nMode == STREAM_READ)
    {
        UINT32 nReadAnz(nAktPos - nSubRecPos);
        if(nReadAnz != nSubRecSiz)
        {
            rStream.Seek(nSubRecPos + nSubRecSiz);
        }
    }
    else if(nMode == STREAM_WRITE)
    {
        nSubRecSiz = nAktPos - nSubRecPos;
        rStream.Seek(nSubRecPos);
        Write();
        rStream.Seek(nAktPos);
    }

    bOpen = FALSE;
}

/*************************************************************************
|*
|* Konstruktor, schreibt bzw. liest Versionsnummer
|*
\************************************************************************/

SdIOCompat::SdIOCompat(SvStream& rNewStream, USHORT nNewMode, UINT16 nVer)
:   old_SdrDownCompat(rNewStream, nNewMode), nVersion(nVer)
{
    if (nNewMode == STREAM_WRITE)
    {
        DBG_ASSERT(nVer != SDIOCOMPAT_VERSIONDONTKNOW,
                   "kann unbekannte Version nicht schreiben");
        rNewStream << nVersion;
    }
    else if (nNewMode == STREAM_READ)
    {
        DBG_ASSERT(nVer == SDIOCOMPAT_VERSIONDONTKNOW,
                   "Lesen mit Angabe der Version ist Quatsch!");
        rNewStream >> nVersion;
    }
}

SdIOCompat::~SdIOCompat()
{
}

// eof
