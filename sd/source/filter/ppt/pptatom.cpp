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

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _PPTATOM_HXX_
#include "pptatom.hxx"
#endif

using namespace ppt;

Atom::Atom( const DffRecordHeader& rRecordHeader, SvStream& rStream )
: mrStream( rStream )
, maRecordHeader( rRecordHeader )
, mpFirstChild( 0 )
, mpNextAtom( 0 )
{
    if( isContainer() )
    {
        if( seekToContent() )
        {
            DffRecordHeader aChildHeader;

            Atom* pLastAtom = NULL;

            // retrieve file size (to allow sanity checks)
            const sal_Size nStreamPos = mrStream.Tell();
            mrStream.Seek( STREAM_SEEK_TO_END );
            const sal_Size nStreamSize = mrStream.Tell();
            mrStream.Seek( nStreamPos );

            while( (mrStream.GetError() == 0 )
                && ( mrStream.Tell() < nStreamSize )
                && ( mrStream.Tell() < maRecordHeader.GetRecEndFilePos() ) )
            {
                mrStream >> aChildHeader;

                if( mrStream.GetError() == 0 )
                {
                    Atom* pAtom = new Atom( aChildHeader, mrStream );

                    if( pLastAtom )
                        pLastAtom->mpNextAtom = pAtom;
                    if( mpFirstChild == NULL )
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
        return NULL;
    }
}

/** returns the next child atom after pLast with nRecType or NULL */
const Atom* Atom::findNextChildAtom( sal_uInt16 nRecType, const Atom* pLast ) const
{
    Atom* pChild = pLast != NULL ? pLast->mpNextAtom : mpFirstChild;
    while( pChild && pChild->maRecordHeader.nRecType != nRecType )
    {
        pChild = pChild->mpNextAtom;
    }

    return pChild;
}
