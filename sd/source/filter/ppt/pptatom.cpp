/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptatom.cpp,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:45:21 $
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

            while( (mrStream.GetError() == 0 ) && ( mrStream.Tell() < maRecordHeader.GetRecEndFilePos() ) )
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

/** returns the next child atom after pLast with nRecType and nRecInstance or NULL */
const Atom* Atom::findNextChildAtom( sal_uInt16 nRecType, sal_uInt16 nRecInstance, const Atom* pLast ) const
{
    const Atom* pChild = pLast != NULL ? pLast->mpNextAtom : mpFirstChild;
    while( pChild && (pChild->maRecordHeader.nRecType != nRecType) && (pChild->maRecordHeader.nRecInstance != nRecInstance) )
    {
        pChild = findNextChildAtom( pChild );
    }

    return pChild;
}
