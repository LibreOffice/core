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
#include "precompiled_sd.hxx"


#include <tools/debug.hxx>

#include "sdiocmpt.hxx"

//////////////////////////////////////////////////////////////////////////////

old_SdrDownCompat::old_SdrDownCompat(SvStream& rNewStream, sal_uInt16 nNewMode)
:   rStream(rNewStream),
    nSubRecSiz(0),
    nSubRecPos(0),
    nMode(nNewMode),
    bOpen(false)
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

    bOpen = true;
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

    bOpen = false;
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

// eof
