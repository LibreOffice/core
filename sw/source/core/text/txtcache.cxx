/*************************************************************************
 *
 *  $RCSfile: txtcache.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "errhdl.hxx"
#include "segmentc.hxx"

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

SwTxtLineAccess::SwTxtLineAccess( const SwTxtFrm *pOwner ) :
    SwCacheAccess( *SwTxtFrm::GetTxtCache(), pOwner, pOwner->GetCacheIdx() )
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


