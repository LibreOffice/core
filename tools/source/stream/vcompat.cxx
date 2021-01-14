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

#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

VersionCompatRead::VersionCompatRead(SvStream& rStm)
    : mrRStm(rStm)
    , mnCompatPos(0)
    , mnTotalSize(0)
    , mnVersion(1)
{
    if (mrRStm.GetError())
        return;

    mrRStm.ReadUInt16(mnVersion);
    mrRStm.ReadUInt32(mnTotalSize);
    mnCompatPos = mrRStm.Tell();
}

VersionCompatWrite::VersionCompatWrite(SvStream& rStm, sal_uInt16 nVersion)
    : mrWStm(rStm)
    , mnCompatPos(0)
    , mnTotalSize(0)
{
    if (mrWStm.GetError())
        return;

    mrWStm.WriteUInt16(nVersion);
    mnCompatPos = mrWStm.Tell();
    mnTotalSize = mnCompatPos + 4;
    mrWStm.SeekRel(4);
}

VersionCompatRead::~VersionCompatRead()
{
    const sal_uInt32 nReadSize = mrRStm.Tell() - mnCompatPos;

    if (mnTotalSize > nReadSize)
        mrRStm.SeekRel(mnTotalSize - nReadSize);
}

VersionCompatWrite::~VersionCompatWrite()
{
    const sal_uInt32 nEndPos = mrWStm.Tell();

    mrWStm.Seek(mnCompatPos);
    mrWStm.WriteUInt32(nEndPos - mnTotalSize);
    mrWStm.Seek(nEndPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
