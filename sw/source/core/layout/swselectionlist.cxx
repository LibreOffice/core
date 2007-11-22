/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swselectionlist.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:37:25 $
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
#include "precompiled_sw.hxx"

#include <swselectionlist.hxx>
#include <layfrm.hxx>
#include <flyfrm.hxx>
#include <ftnfrm.hxx>

/** This class is used as parameter for functions to create a rectangular text selection
*/

namespace {

    /** Find the context of a given frame

        A context is the environment where text is allowed to flow.
        The context is represented by
        - the SwRootFrm if the frame is part of a page body
        - the SwHeaderFrm if the frame is part of a page header
        - the SwFooterFrm if the frame is part of a page footer
        - the (master) SwFtnFrm if the frame is part of footnote
        - the (first) SwFlyFrm if the frame is part of a (linked) fly frame

        @param pFrm
        the given frame

        @return the context of the frame, represented by a SwFrm*
    */
    const SwFrm* getContext( const SwFrm* pFrm )
    {
        while( pFrm )
        {
            if( pFrm->IsRootFrm() || pFrm->IsHeaderFrm() || pFrm->IsFooterFrm() )
                break;
            if( pFrm->IsFlyFrm() )
            {
                const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>( pFrm );
                while( pFly->GetPrevLink() )
                    pFly = pFly->GetPrevLink();
                break;
            }
            if( pFrm->IsFtnFrm() )
            {
                const SwFtnFrm* pFtn = static_cast<const SwFtnFrm*>( pFrm );
                while( pFtn->GetMaster() )
                    pFtn = pFtn->GetMaster();
                break;
            }
            pFrm = pFrm->GetUpper();
        }
        return pFrm;
    }
}

SwSelectionList::SwSelectionList( const SwFrm* pInitCxt ) :
    pContext( getContext( pInitCxt ) )
{
}

bool SwSelectionList::checkContext( const SwFrm* pCheck )
{
    pCheck = getContext( pCheck );
    if( !pContext )
        pContext = pCheck;
    return pContext == pCheck;
}

