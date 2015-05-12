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

#include <swselectionlist.hxx>
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
        - the (master) SwFootnoteFrm if the frame is part of footnote
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
            if( pFrm->IsFootnoteFrm() )
            {
                const SwFootnoteFrm* pFootnote = static_cast<const SwFootnoteFrm*>( pFrm );
                while( pFootnote->GetMaster() )
                    pFootnote = pFootnote->GetMaster();
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
