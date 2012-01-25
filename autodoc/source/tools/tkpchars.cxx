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

#include <precomp.h>
#include <tools/tkpchars.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/bstream.hxx>
#include <cosv/x.hxx>



CharacterSource::CharacterSource()
    :   dpSource(new char[2]),
        nSourceSize(0),
        nCurPos(0),
        nLastCut(0),
        nLastTokenStart(0),
        cCharAtLastCut(0)
{
    dpSource[nSourceSize] = NULCH;
    dpSource[nSourceSize+1] = NULCH;
}

CharacterSource::~CharacterSource()
{
    delete [] dpSource;
}

void
CharacterSource::LoadText(csv::bstream & io_rSource)
{
    if (dpSource != 0)
        delete [] dpSource;

    io_rSource.seek(0, csv::end);
    nSourceSize = intt(io_rSource.position());
    io_rSource.seek(0);

    dpSource = new char[nSourceSize+1];

    intt nCount = (intt) io_rSource.read(dpSource,nSourceSize);
    if (nCount != nSourceSize)
        throw csv::X_Default("IO-Error: Could not load file completely.");

    dpSource[nSourceSize] = NULCH;

    BeginSource();
}

const char *
CharacterSource::CutToken()
{
    dpSource[nLastCut] = cCharAtLastCut;
    nLastTokenStart = nLastCut;
    nLastCut = CurPos();
    cCharAtLastCut = dpSource[nLastCut];
    dpSource[nLastCut] = NULCH;

    return &dpSource[nLastTokenStart];
}

void
CharacterSource::BeginSource()
{
    nCurPos = 0;
    nLastCut = 0;
    nLastTokenStart = 0;
    cCharAtLastCut = dpSource[nLastCut];
    dpSource[nLastCut] = NULCH;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
