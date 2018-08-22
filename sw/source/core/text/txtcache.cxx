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
#include <txtfrm.hxx>
#include "porlay.hxx"

SwTextLine::SwTextLine( SwTextFrame const *pFrame, std::unique_ptr<SwParaPortion> pNew ) :
    SwCacheObj( static_cast<void const *>(pFrame) ),
    pLine( std::move(pNew) )
{
}

SwTextLine::~SwTextLine()
{
}

SwCacheObj *SwTextLineAccess::NewObj()
{
    return new SwTextLine( static_cast<SwTextFrame const *>(m_pOwner) );
}

SwParaPortion *SwTextLineAccess::GetPara()
{
    SwTextLine *pRet;
    if ( m_pObj )
        pRet = static_cast<SwTextLine*>(m_pObj);
    else
    {
        pRet = static_cast<SwTextLine*>(Get());
        const_cast<SwTextFrame *>(static_cast<SwTextFrame const *>(m_pOwner))->SetCacheIdx( pRet->GetCachePos() );
    }
    if ( !pRet->GetPara() )
        pRet->SetPara( new SwParaPortion, true/*bDelete*/ );
    return pRet->GetPara();
}

SwTextLineAccess::SwTextLineAccess( const SwTextFrame *pOwn ) :
    SwCacheAccess( *SwTextFrame::GetTextCache(), pOwn, pOwn->GetCacheIdx() )
{
}

bool SwTextLineAccess::IsAvailable() const
{
    return m_pObj && static_cast<SwTextLine*>(m_pObj)->GetPara();
}

bool SwTextFrame::HasPara_() const
{
    SwTextLine *pTextLine = static_cast<SwTextLine*>(SwTextFrame::GetTextCache()->
                                            Get( this, GetCacheIdx(), false ));
    if ( pTextLine )
    {
        if ( pTextLine->GetPara() )
            return true;
    }
    else
        const_cast<SwTextFrame*>(this)->mnCacheIndex = USHRT_MAX;

    return false;
}

SwParaPortion *SwTextFrame::GetPara()
{
    if ( GetCacheIdx() != USHRT_MAX )
    {
        SwTextLine *pLine = static_cast<SwTextLine*>(SwTextFrame::GetTextCache()->
                                        Get( this, GetCacheIdx(), false ));
        if ( pLine )
            return pLine->GetPara();
        else
            mnCacheIndex = USHRT_MAX;
    }
    return nullptr;
}

void SwTextFrame::ClearPara()
{
    OSL_ENSURE( !IsLocked(), "+SwTextFrame::ClearPara: this is locked." );
    if ( !IsLocked() && GetCacheIdx() != USHRT_MAX )
    {
        SwTextLine *pTextLine = static_cast<SwTextLine*>(SwTextFrame::GetTextCache()->
                                        Get( this, GetCacheIdx(), false ));
        if ( pTextLine )
        {
            pTextLine->SetPara( nullptr, true/*bDelete*/ );
        }
        else
            mnCacheIndex = USHRT_MAX;
    }
}

void SwTextFrame::SetPara( SwParaPortion *pNew, bool bDelete )
{
    if ( GetCacheIdx() != USHRT_MAX )
    {
        // Only change the information, the CacheObj stays there
        SwTextLine *pTextLine = static_cast<SwTextLine*>(SwTextFrame::GetTextCache()->
                                        Get( this, GetCacheIdx(), false ));
        if ( pTextLine )
        {
            pTextLine->SetPara( pNew, bDelete );
        }
        else
        {
            OSL_ENSURE( !pNew, "+SetPara: Losing SwParaPortion" );
            mnCacheIndex = USHRT_MAX;
        }
    }
    else if ( pNew )
    {   // Insert a new one
        SwTextLine *pTextLine = new SwTextLine( this, std::unique_ptr<SwParaPortion>(pNew) );
        if ( SwTextFrame::GetTextCache()->Insert( pTextLine ) )
            mnCacheIndex = pTextLine->GetCachePos();
        else
        {
            OSL_FAIL( "+SetPara: InsertCache failed." );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
