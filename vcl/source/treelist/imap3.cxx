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

#include <vcl/imap.hxx>

#include <tools/debug.hxx>

/******************************************************************************
|*
|* Ctor
|*
\******************************************************************************/

IMapCompat::IMapCompat(SvStream& rStm, const StreamMode nStreamMode)
    : pRWStm(&rStm)
    , nCompatPos(0)
    , nTotalSize(0)
    , nStmMode(nStreamMode)
{
    DBG_ASSERT(nStreamMode == StreamMode::READ || nStreamMode == StreamMode::WRITE, "Wrong Mode!");

    if (pRWStm->GetError())
        return;

    if (nStmMode == StreamMode::WRITE)
    {
        nCompatPos = pRWStm->Tell();
        pRWStm->SeekRel(4);
        nTotalSize = nCompatPos + 4;
    }
    else
    {
        sal_uInt32 nTotalSizeTmp;
        pRWStm->ReadUInt32(nTotalSizeTmp);
        nTotalSize = nTotalSizeTmp;
        nCompatPos = pRWStm->Tell();
    }
}

/******************************************************************************
|*
|* Dtor
|*
\******************************************************************************/

IMapCompat::~IMapCompat()
{
    if (pRWStm->GetError())
        return;

    if (nStmMode == StreamMode::WRITE)
    {
        const sal_uInt64 nEndPos = pRWStm->Tell();

        pRWStm->Seek(nCompatPos);
        pRWStm->WriteUInt32(nEndPos - nTotalSize);
        pRWStm->Seek(nEndPos);
    }
    else
    {
        const sal_uInt64 nReadSize = pRWStm->Tell() - nCompatPos;

        if (nTotalSize > nReadSize)
            pRWStm->SeekRel(nTotalSize - nReadSize);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
