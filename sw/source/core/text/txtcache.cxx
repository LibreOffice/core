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

#include "txtcache.hxx"
#include "txtfrm.hxx"
#include "porlay.hxx"

SwTextLine::SwTextLine( SwTextFrm *pFrm, SwParaPortion *pNew ) :
    SwCacheObj( (void*)pFrm ),
    pLine( pNew )
{
}

SwTextLine::~SwTextLine()
{
    delete pLine;
}

SwCacheObj *SwTextLineAccess::NewObj()
{
    return new SwTextLine( const_cast<SwTextFrm *>(static_cast<SwTextFrm const *>(pOwner)) );
}

SwParaPortion *SwTextLineAccess::GetPara()
{
    SwTextLine *pRet;
    if ( pObj )
        pRet = static_cast<SwTextLine*>(pObj);
    else
    {
        pRet = static_cast<SwTextLine*>(Get());
        const_cast<SwTextFrm *>(static_cast<SwTextFrm const *>(pOwner))->SetCacheIdx( pRet->GetCachePos() );
    }
    if ( !pRet->GetPara() )
        pRet->SetPara( new SwParaPortion );
    return pRet->GetPara();
}

SwTextLineAccess::SwTextLineAccess( const SwTextFrm *pOwn ) :
    SwCacheAccess( *SwTextFrm::GetTextCache(), pOwn, pOwn->GetCacheIdx() )
{
}

bool SwTextLineAccess::IsAvailable() const
{
    return pObj && static_cast<SwTextLine*>(pObj)->GetPara();
}

bool SwTextFrm::_HasPara() const
{
    SwTextLine *pTextLine = static_cast<SwTextLine*>(SwTextFrm::GetTextCache()->
                                            Get( this, GetCacheIdx(), false ));
    if ( pTextLine )
    {
        if ( pTextLine->GetPara() )
            return true;
    }
    else
        const_cast<SwTextFrm*>(this)->nCacheIdx = USHRT_MAX;

    return false;
}

SwParaPortion *SwTextFrm::GetPara()
{
    if ( GetCacheIdx() != USHRT_MAX )
    {
        SwTextLine *pLine = static_cast<SwTextLine*>(SwTextFrm::GetTextCache()->
                                        Get( this, GetCacheIdx(), false ));
        if ( pLine )
            return pLine->GetPara();
        else
            nCacheIdx = USHRT_MAX;
    }
    return 0;
}

void SwTextFrm::ClearPara()
{
    OSL_ENSURE( !IsLocked(), "+SwTextFrm::ClearPara: this is locked." );
    if ( !IsLocked() && GetCacheIdx() != USHRT_MAX )
    {
        SwTextLine *pTextLine = static_cast<SwTextLine*>(SwTextFrm::GetTextCache()->
                                        Get( this, GetCacheIdx(), false ));
        if ( pTextLine )
        {
            delete pTextLine->GetPara();
            pTextLine->SetPara( 0 );
        }
        else
            nCacheIdx = USHRT_MAX;
    }
}

void SwTextFrm::SetPara( SwParaPortion *pNew, bool bDelete )
{
    if ( GetCacheIdx() != USHRT_MAX )
    {
        // Only change the information, the CacheObj stays there
        SwTextLine *pTextLine = static_cast<SwTextLine*>(SwTextFrm::GetTextCache()->
                                        Get( this, GetCacheIdx(), false ));
        if ( pTextLine )
        {
            if( bDelete )
                delete pTextLine->GetPara();
            pTextLine->SetPara( pNew );
        }
        else
        {
            OSL_ENSURE( !pNew, "+SetPara: Losing SwParaPortion" );
            nCacheIdx = USHRT_MAX;
        }
    }
    else if ( pNew )
    {   // Insert a new one
        SwTextLine *pTextLine = new SwTextLine( this, pNew );
        if ( SwTextFrm::GetTextCache()->Insert( pTextLine ) )
            nCacheIdx = pTextLine->GetCachePos();
        else
        {
            OSL_FAIL( "+SetPara: InsertCache failed." );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
