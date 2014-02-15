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

#include <movedfwdfrmsbyobjpos.hxx>
#include <txtfrm.hxx>
#include <rowfrm.hxx>
#include <pagefrm.hxx>
#include <ndtxt.hxx>
#include <switerator.hxx>

SwMovedFwdFrmsByObjPos::SwMovedFwdFrmsByObjPos()
{
}

SwMovedFwdFrmsByObjPos::~SwMovedFwdFrmsByObjPos()
{
    Clear();
}

void SwMovedFwdFrmsByObjPos::Insert( const SwTxtFrm& _rMovedFwdFrmByObjPos,
                                     const sal_uInt32 _nToPageNum )
{
    if ( maMovedFwdFrms.end() ==
         maMovedFwdFrms.find( _rMovedFwdFrmByObjPos.GetTxtNode() ) )
    {
        const NodeMapEntry aEntry( _rMovedFwdFrmByObjPos.GetTxtNode(), _nToPageNum );
        maMovedFwdFrms.insert( aEntry );
    }
}

void SwMovedFwdFrmsByObjPos::Remove( const SwTxtFrm& _rTxtFrm )
{
    maMovedFwdFrms.erase( _rTxtFrm.GetTxtNode() );
}

bool SwMovedFwdFrmsByObjPos::FrmMovedFwdByObjPos( const SwTxtFrm& _rTxtFrm,
                                                  sal_uInt32& _ornToPageNum ) const
{
    NodeMapIter aIter = maMovedFwdFrms.find( _rTxtFrm.GetTxtNode() );
    if ( maMovedFwdFrms.end() != aIter )
    {
        _ornToPageNum = (*aIter).second;
        return true;
    }

    return false;
}

// #i26945#
bool SwMovedFwdFrmsByObjPos::DoesRowContainMovedFwdFrm( const SwRowFrm& _rRowFrm ) const
{
    bool bDoesRowContainMovedFwdFrm( false );

    const sal_uInt32 nPageNumOfRow = _rRowFrm.FindPageFrm()->GetPhyPageNum();

    NodeMapIter aIter = maMovedFwdFrms.begin();
    for ( ; aIter != maMovedFwdFrms.end(); ++aIter )
    {
        const NodeMapEntry& rEntry = *(aIter);
        if ( rEntry.second >= nPageNumOfRow )
        {
            SwIterator<SwTxtFrm,SwTxtNode> aFrmIter( *rEntry.first );
            for( SwTxtFrm* pTxtFrm = aFrmIter.First(); pTxtFrm; pTxtFrm = (SwTxtFrm*)aFrmIter.Next() )
            {
                // #115759# - assure that found text frame
                // is the first one.
                if ( _rRowFrm.IsAnLower( pTxtFrm ) && !pTxtFrm->GetIndPrev() )
                {
                    bDoesRowContainMovedFwdFrm = true;
                    break;
                }
            }
        }
    }

    return bDoesRowContainMovedFwdFrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
