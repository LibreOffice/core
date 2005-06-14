/*************************************************************************
 *
 *  $RCSfile: accfrmobjslist.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-06-14 16:27:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _ACCFRMOBJSLIST_HXX
#define _ACCFRMOBJSLIST_HXX

#ifndef _ACCFRMOBJ_HXX
#include <accfrmobj.hxx>
#endif

class SwFrmOrObjSList;

class SwFrmOrObjSList_const_iterator
{
    friend class SwFrmOrObjSList;
    const SwFrmOrObjSList& rList;   // The frame we are iterating over
    SwFrmOrObj aCurr;               // The current object
    sal_uInt16 nNextObj;            // The index of the current sdr object

    inline SwFrmOrObjSList_const_iterator( const SwFrmOrObjSList& rLst );
    SwFrmOrObjSList_const_iterator( const SwFrmOrObjSList& rLst, sal_Bool );

//  SwFrmOrObjSList_const_iterator& begin();
    SwFrmOrObjSList_const_iterator& next();
    SwFrmOrObjSList_const_iterator& next_visible();

public:

    inline SwFrmOrObjSList_const_iterator(
            const SwFrmOrObjSList_const_iterator& rIter );
    inline sal_Bool operator==(
            const SwFrmOrObjSList_const_iterator& r ) const;
    inline sal_Bool operator!=(
            const SwFrmOrObjSList_const_iterator& r ) const;
    inline SwFrmOrObjSList_const_iterator& operator++();
    inline const SwFrmOrObj& operator*() const;
};

// An iterator to iterate over a frame's child in any order
class SwFrmOrObjSList
{
    friend class SwFrmOrObjSList_const_iterator;

    SwRect aVisArea;
    const SwFrm *pFrm;  // The frame we are iterating over
    sal_Bool bVisibleOnly;

public:

    typedef SwFrmOrObjSList_const_iterator const_iterator;

    inline SwFrmOrObjSList( const SwFrm *pF );
    inline SwFrmOrObjSList( const SwRect& rVisArea, const SwFrm *pF );

    inline const_iterator begin() const;
    inline const_iterator end() const;
};

inline SwFrmOrObjSList_const_iterator::SwFrmOrObjSList_const_iterator(
        const SwFrmOrObjSList& rLst ) :
    rList( rLst ), nNextObj( 0 )
{
}

inline SwFrmOrObjSList_const_iterator::SwFrmOrObjSList_const_iterator(
        const SwFrmOrObjSList_const_iterator& rIter ) :
    rList( rIter.rList ),
    aCurr( rIter.aCurr ),
    nNextObj( rIter.nNextObj )
{
}

inline sal_Bool SwFrmOrObjSList_const_iterator::operator==(
    const SwFrmOrObjSList_const_iterator& r ) const
{
    return aCurr == r.aCurr;
}

inline sal_Bool SwFrmOrObjSList_const_iterator::operator!=(
    const SwFrmOrObjSList_const_iterator& r ) const
{
    return !(aCurr == r.aCurr);
}

inline SwFrmOrObjSList_const_iterator& SwFrmOrObjSList_const_iterator::operator++()
{
    return rList.bVisibleOnly ? next_visible() : next();
}

inline const SwFrmOrObj& SwFrmOrObjSList_const_iterator::operator*() const
{
    return aCurr;
}

inline SwFrmOrObjSList::SwFrmOrObjSList( const SwFrm *pF ) :
    pFrm( pF ),
    bVisibleOnly( sal_False )
{
}

inline SwFrmOrObjSList::SwFrmOrObjSList( const SwRect& rVisArea,
                                             const SwFrm *pF ) :
    aVisArea( rVisArea ),
    pFrm( pF )
{
    SwFrmOrObj aFrm( pFrm );
    bVisibleOnly = aFrm.IsVisibleChildrenOnly();
}

inline SwFrmOrObjSList_const_iterator SwFrmOrObjSList::begin() const
{
//  SwFrmOrObjSList_const_iterator aIter2( *this );
//  aIter2.begin();
//  return aIter2;
    return SwFrmOrObjSList_const_iterator( *this, sal_True );
}

inline SwFrmOrObjSList_const_iterator SwFrmOrObjSList::end() const
{
    return SwFrmOrObjSList_const_iterator( *this );
}

#endif
