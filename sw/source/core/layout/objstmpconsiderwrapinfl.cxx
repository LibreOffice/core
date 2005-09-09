/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objstmpconsiderwrapinfl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:23:28 $
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
#ifndef _OBJSTMPCONSIDERWRAPINFL_HXX
#include <objstmpconsiderwrapinfl.hxx>
#endif

#ifndef _ANCHOREDOBJECT_HXX
#include <anchoredobject.hxx>
#endif

SwObjsMarkedAsTmpConsiderWrapInfluence::SwObjsMarkedAsTmpConsiderWrapInfluence()
{
}

SwObjsMarkedAsTmpConsiderWrapInfluence::~SwObjsMarkedAsTmpConsiderWrapInfluence()
{
    Clear();
}

void SwObjsMarkedAsTmpConsiderWrapInfluence::Insert( SwAnchoredObject& _rAnchoredObj )
{

    bool bAlreadyInserted( false );
    std::vector< SwAnchoredObject* >::const_iterator aIter = maObjsTmpConsiderWrapInfl.begin();
    for ( ; aIter != maObjsTmpConsiderWrapInfl.end(); ++aIter )
    {
        const SwAnchoredObject* pAnchoredObj = *(aIter);
        if ( pAnchoredObj == &_rAnchoredObj )
        {
            bAlreadyInserted = true;
            break;
        }
    }

    if ( !bAlreadyInserted )
    {
        maObjsTmpConsiderWrapInfl.push_back( &_rAnchoredObj );
    }
}

void SwObjsMarkedAsTmpConsiderWrapInfluence::Clear()
{
    while ( maObjsTmpConsiderWrapInfl.size() )
    {
        SwAnchoredObject* pAnchoredObj = maObjsTmpConsiderWrapInfl.back();
        pAnchoredObj->SetTmpConsiderWrapInfluence( false );
        pAnchoredObj->SetClearedEnvironment( false );

        maObjsTmpConsiderWrapInfl.pop_back();
    }
}
