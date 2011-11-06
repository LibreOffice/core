/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "errhdl.hxx"

#include "txtcache.hxx"
#include "txtfrm.hxx"
#include "porlay.hxx"

/*************************************************************************
|*
|*  SwTxtLine::SwTxtLine(), ~SwTxtLine()
|*
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 16. Mar. 94
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 16. Mar. 94
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 16. Mar. 94
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 16. Mar. 94
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
|*  Ersterstellung      MA 23. Mar. 94
|*  Letzte Aenderung    MA 23. Mar. 94
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 22. Aug. 94
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 22. Aug. 94
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 22. Aug. 94
|*
|*************************************************************************/

void SwTxtFrm::ClearPara()
{
    ASSERT( !IsLocked(), "+SwTxtFrm::ClearPara: this is locked." );
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
|*  Ersterstellung      MA 16. Mar. 94
|*  Letzte Aenderung    MA 22. Aug. 94
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
            ASSERT( !pNew, "+SetPara: Losing SwParaPortion" );
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
            ASSERT( sal_False, "+SetPara: InsertCache failed." );
        }
    }
}


