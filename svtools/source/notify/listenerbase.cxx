/*************************************************************************
 *
 *  $RCSfile: listenerbase.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-14 18:52:13 $
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

#pragma hdrstop

#ifndef DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include "listenerbase.hxx"
#include "listeneriter.hxx"
#include "listener.hxx"
#include "broadcast.hxx"


SvtListenerBase::SvtListenerBase( SvtListener& rLst,
                                  SvtBroadcaster& rBroadcaster )
    : pBroadcaster( &rBroadcaster ), pListener( &rLst ),
    pLeft( 0 ), pRight( 0 )
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


