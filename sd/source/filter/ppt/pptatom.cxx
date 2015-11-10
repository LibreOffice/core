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
#include "pptatom.hxx"

using namespace ppt;

Atom::Atom( const DffRecordHeader& rRecordHeader, SvStream& rStream )
: mrStream( rStream )
, maRecordHeader( rRecordHeader )
, mpFirstChild( nullptr )
, mpNextAtom( nullptr )
{
    if( isContainer() )
    {
        if( seekToContent() )
        {
            DffRecordHeader aChildHeader;

            Atom* pLastAtom = nullptr;

            // retrieve file size (to allow sanity checks)
            const sal_Size nStreamPos = mrStream.Tell();
            mrStream.Seek( STREAM_SEEK_TO_END );
            const sal_Size nStreamSize = mrStream.Tell();
            mrStream.Seek( nStreamPos );

            while( (mrStream.GetError() == 0 )
                && ( mrStream.Tell() < nStreamSize )
                && ( mrStream.Tell() < maRecordHeader.GetRecEndFilePos() ) )
            {
                ReadDffRecordHeader( mrStream, aChildHeader );

                if( mrStream.GetError() == 0 )
                {
                    Atom* pAtom = new Atom( aChildHeader, mrStream );

                    if( pLastAtom )
                        pLastAtom->mpNextAtom = pAtom;
                    if( mpFirstChild == nullptr )
                        mpFirstChild = pAtom;

                    pLastAtom = pAtom;
                }
            }
        }
    }

    maRecordHeader.SeekToEndOfRecord( mrStream );
}

Atom::~Atom()
{
    Atom* pChild = mpFirstChild;
    while( pChild )
    {
        Atom* pNextChild = pChild->mpNextAtom;
        delete pChild;
        pChild = pNextChild;
    }
}

/** imports this atom and its child atoms */
Atom* Atom::import( const DffRecordHeader& rRootRecordHeader, SvStream& rStCtrl )
{
    Atom* pRootAtom = new Atom( rRootRecordHeader, rStCtrl );

    if( rStCtrl.GetError() == 0 )
    {
        return pRootAtom;
    }
    else
    {
        delete pRootAtom;
        return nullptr;
    }
}

/** returns the next child atom after pLast with nRecType or NULL */
const Atom* Atom::findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const
{
    Atom* pChild = pLast != nullptr ? pLast->mpNextAtom : mpFirstChild;
    while( pChild && pChild->maRecordHeader.nRecType != nRecType )
    {
        pChild = pChild->mpNextAtom;
    }

    return pChild;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
