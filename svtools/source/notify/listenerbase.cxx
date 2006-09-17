/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: listenerbase.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:19:16 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#ifndef GCC
#endif

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "listenerbase.hxx"
#include "listeneriter.hxx"
#include "listener.hxx"
#include "broadcast.hxx"


SvtListenerBase::SvtListenerBase( SvtListener& rLst,
                                  SvtBroadcaster& rBroadcaster )
    : pLeft( 0 ), pRight( 0 ),
    pBroadcaster( &rBroadcaster ), pListener( &rLst )
{
    pNext = rLst.pBrdCastLst;
    rLst.pBrdCastLst = this;

    if( pBroadcaster->pRoot )
    {
        // set ever behind the root
        pRight = pBroadcaster->pRoot->pRight;
        pBroadcaster->pRoot->pRight = this;
        this->pLeft = pBroadcaster->pRoot;
        if( pRight )
            pRight->pLeft = this;
    }
    else
        pBroadcaster->pRoot = this;
}

SvtListenerBase::~SvtListenerBase()
{
    SvtListenerBase *pR = pRight, *pL = pLeft;
    if( pBroadcaster->pRoot )
        pBroadcaster->pRoot = pL ? pL : pR;

    if( pL )
        pL->pRight = pR;
    if( pR )
        pR->pLeft = pL;

    SvtListenerIter::RemoveListener( *this, pR );

    if( !pBroadcaster->pRoot )
        pBroadcaster->ListenersGone();
}


