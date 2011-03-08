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




#include "txtcache.hxx"
#include "txtfrm.hxx"
#include "porlay.hxx"

/*************************************************************************
|*
|*  SwTxtLine::SwTxtLine(), ~SwTxtLine()
|*
|*************************************************************************/

SwTxtLine::SwTxtLine( SwTxtFrm *pFrm, SwParaPortion *pNew ) :
    SwCacheObj( (void*)pFrm ),
    pLine( pNew )
{
}

SwTxtLine::~SwTxtLine()
{
    delete pLine;
}

/*************************************************************************
|*
|*  SwTxtLineAccess::NewObj()
|*
|*************************************************************************/

SwCacheObj *SwTxtLineAccess::NewObj()
{
    return new SwTxtLine( (SwTxtFrm*)pOwner );
}

/*************************************************************************
|*
|*  SwTxtLineAccess::GetPara()
|*
|*************************************************************************/

SwParaPortion *SwTxtLineAccess::GetPara()
{
    SwTxtLine *pRet;
    if ( pObj )
        pRet = (SwTxtLine*)pObj;
    else
    {
        pRet = (SwTxtLine*)Get();
        ((SwTxtFrm*)pOwner)->SetCacheIdx( pRet->GetCachePos() );
    }
    if ( !pRet->GetPara() )
        pRet->SetPara( new SwParaPortion );
    return pRet->GetPara();
}


/*************************************************************************
|*
|*  SwTxtLineAccess::SwTxtLineAccess()
|*
|*************************************************************************/

SwTxtLineAccess::SwTxtLineAccess( const SwTxtFrm *pOwn ) :
    SwCacheAccess( *SwTxtFrm::GetTxtCache(), pOwn, pOwn->GetCacheIdx() )
{
}

/*************************************************************************
|*
|*  SwTxtLineAccess::IsAvailable
|*
|*************************************************************************/

sal_Bool SwTxtLineAccess::IsAvailable() const
{
    if ( pObj )
        return ((SwTxtLine*)pObj)->GetPara() != 0;
    return sal_False;
}

/*************************************************************************
|*
|*  SwTxtFrm::HasPara()
|*
|*************************************************************************/

sal_Bool SwTxtFrm::_HasPara() const
{
    SwTxtLine *pTxtLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
                                            Get( this, GetCacheIdx(), sal_False );
    if ( pTxtLine )
    {
        if ( pTxtLine->GetPara() )
            return sal_True;
    }
    else
        ((SwTxtFrm*)this)->nCacheIdx = MSHRT_MAX;

    return sal_False;
}

/*************************************************************************
|*
|*  SwTxtFrm::GetPara()
|*
|*************************************************************************/

SwParaPortion *SwTxtFrm::GetPara()
{
    if ( GetCacheIdx() != MSHRT_MAX )
    {   SwTxtLine *pLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
                                        Get( this, GetCacheIdx(), sal_False );
        if ( pLine )
            return pLine->GetPara();
        else
            nCacheIdx = MSHRT_MAX;
    }
    return 0;
}


/*************************************************************************
|*
|*  SwTxtFrm::ClearPara()
|*
|*************************************************************************/

void SwTxtFrm::ClearPara()
{
    OSL_ENSURE( !IsLocked(), "+SwTxtFrm::ClearPara: this is locked." );
    if ( !IsLocked() && GetCacheIdx() != MSHRT_MAX )
    {
        SwTxtLine *pTxtLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
                                        Get( this, GetCacheIdx(), sal_False );
        if ( pTxtLine )
        {
            delete pTxtLine->GetPara();
            pTxtLine->SetPara( 0 );
        }
        else
            nCacheIdx = MSHRT_MAX;
    }
}

/*************************************************************************
|*
|*  SwTxtFrm::SetPara()
|*
|*************************************************************************/

void SwTxtFrm::SetPara( SwParaPortion *pNew, sal_Bool bDelete )
{
    if ( GetCacheIdx() != MSHRT_MAX )
    {
        //Nur die Information Auswechseln, das CacheObj bleibt stehen.
        SwTxtLine *pTxtLine = (SwTxtLine*)SwTxtFrm::GetTxtCache()->
                                        Get( this, GetCacheIdx(), sal_False );
        if ( pTxtLine )
        {
            if( bDelete )
                delete pTxtLine->GetPara();
            pTxtLine->SetPara( pNew );
        }
        else
        {
            OSL_ENSURE( !pNew, "+SetPara: Losing SwParaPortion" );
            nCacheIdx = MSHRT_MAX;
        }
    }
    else if ( pNew )
    {   //Einen neuen einfuegen.
        SwTxtLine *pTxtLine = new SwTxtLine( this, pNew );
        if ( SwTxtFrm::GetTxtCache()->Insert( pTxtLine ) )
            nCacheIdx = pTxtLine->GetCachePos();
        else
        {
            OSL_ENSURE( sal_False, "+SetPara: InsertCache failed." );
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
