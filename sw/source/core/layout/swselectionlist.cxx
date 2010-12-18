/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
