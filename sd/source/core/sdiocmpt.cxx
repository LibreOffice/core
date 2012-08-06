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

#include <tools/debug.hxx>

#include "sdiocmpt.hxx"

//////////////////////////////////////////////////////////////////////////////

old_SdrDownCompat::old_SdrDownCompat(SvStream& rNewStream, sal_uInt16 nNewMode)
:   rStream(rNewStream),
    nSubRecSiz(0),
    nSubRecPos(0),
    nMode(nNewMode),
    bOpen(sal_False)
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

    bOpen = sal_True;
}

void old_SdrDownCompat::CloseSubRecord()
{
    if(rStream.GetError())
        return;

    sal_uInt32 nAktPos(rStream.Tell());

    if(nMode == STREAM_READ)
    {
        sal_uInt32 nReadAnz(nAktPos - nSubRecPos);
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

    bOpen = sal_False;
}

/*************************************************************************
|*
|* Konstruktor, schreibt bzw. liest Versionsnummer
|*
\************************************************************************/

SdIOCompat::SdIOCompat(SvStream& rNewStream, sal_uInt16 nNewMode, sal_uInt16 nVer)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
