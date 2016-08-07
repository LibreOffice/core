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
