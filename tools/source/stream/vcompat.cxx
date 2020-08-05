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

VersionCompat::VersionCompat( SvStream& rStm, StreamMode nStreamMode, sal_uInt16 nVersion ) :
            mpRWStm     ( &rStm ),
            mnCompatPos ( 0 ),
            mnTotalSize ( 0 ),
            mnStmMode   ( nStreamMode ),
            mnVersion   ( nVersion )
{
    if( mpRWStm->GetError() )
        return;

    if( StreamMode::WRITE == mnStmMode )
    {
        mpRWStm->WriteUInt16( mnVersion );
        mnCompatPos = mpRWStm->Tell();
        mnTotalSize = mnCompatPos + 4;
        mpRWStm->SeekRel( 4 );
    }
    else
    {
        mpRWStm->ReadUInt16( mnVersion );
        mpRWStm->ReadUInt32( mnTotalSize );
        mnCompatPos = mpRWStm->Tell();
    }
}

VersionCompat::~VersionCompat()
{
    if( StreamMode::WRITE == mnStmMode )
    {
        const sal_uInt32 nEndPos = mpRWStm->Tell();

        mpRWStm->Seek( mnCompatPos );
        mpRWStm->WriteUInt32(  nEndPos - mnTotalSize  );
        mpRWStm->Seek( nEndPos );
    }
    else
    {
        const sal_uInt32 nReadSize = mpRWStm->Tell() - mnCompatPos;

        if( mnTotalSize > nReadSize )
            mpRWStm->SeekRel( mnTotalSize - nReadSize );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
