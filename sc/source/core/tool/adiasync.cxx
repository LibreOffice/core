/*************************************************************************
 *
 *  $RCSfile: adiasync.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <sfx2/objsh.hxx>

#include "adiasync.hxx"
#include "brdcst.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"       // FID_DATACHANGED

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif


//------------------------------------------------------------------------

#pragma code_seg("SCSTATICS")

ScAddInAsyncs theAddInAsyncTbl;
static ScAddInAsync aSeekObj;

#pragma code_seg()


SV_IMPL_OP_PTRARR_SORT( ScAddInAsyncs, ScAddInAsyncPtr );

SV_IMPL_PTRARR_SORT( ScAddInDocs, ScAddInDocPtr );

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData )
{
    ScAddInAsync::CallBack( ULONG( nHandle ), pData );
}
}



ScAddInAsync::ScAddInAsync() :
    SfxBroadcaster(),
    nHandle( 0 )
{   // nur fuer aSeekObj !
}



ScAddInAsync::ScAddInAsync( ULONG nHandleP, USHORT nIndex, ScDocument* pDoc ) :
    SfxBroadcaster(),
    pStr( NULL ),
    nHandle( nHandleP ),
    bValid( FALSE )
{
    pDocs = new ScAddInDocs( 1, 1 );
    pDocs->Insert( pDoc );
    pFuncData = (FuncData*)ScGlobal::GetFuncCollection()->At(nIndex);
    eType = pFuncData->GetAsyncType();
    theAddInAsyncTbl.Insert( this );
}



ScAddInAsync::~ScAddInAsync()
{
    // aSeekObj hat das alles nicht, Handle 0 gibt es sonst nicht
    if ( nHandle )
    {
        // im dTor wg. theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        pFuncData->Unadvice( (double)nHandle );
        if ( eType == PTR_STRING && pStr )      // mit Typvergleich wg. Union!
            delete pStr;
        delete pDocs;
    }
}



ScAddInAsync* ScAddInAsync::Get( ULONG nHandleP )
{
    USHORT nPos;
    ScAddInAsync* pRet = 0;
    aSeekObj.nHandle = nHandleP;
    if ( theAddInAsyncTbl.Seek_Entry( &aSeekObj, &nPos ) )
        pRet = theAddInAsyncTbl[ nPos ];
    aSeekObj.nHandle = 0;
    return pRet;
}



void ScAddInAsync::CallBack( ULONG nHandleP, void* pData )
{
    ScAddInAsync* p;
    if ( !(p = Get( nHandleP )) )
        return;
    // keiner mehr dran? Unadvice und weg damit
    if ( !p->HasListeners() )
    {
        // nicht im dTor wg. theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        theAddInAsyncTbl.Remove( p );
        delete p;
        return ;
    }
    switch ( p->eType )
    {
        case PTR_DOUBLE :
            p->nVal = *(double*)pData;
            break;
        case PTR_STRING :
            if ( p->pStr )
                *p->pStr = String( (sal_Char*)pData, osl_getThreadTextEncoding() );
            else
                p->pStr = new String( (sal_Char*)pData, osl_getThreadTextEncoding() );
            break;
        default :
            DBG_ERROR( "unbekannter AsyncType" );
            return;
    }
    p->bValid = TRUE;
    p->Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress( 0 ), NULL ) );

    const ScDocument** ppDoc = (const ScDocument**) p->pDocs->GetData();
    USHORT nCount = p->pDocs->Count();
    for ( USHORT j=0; j<nCount; j++, ppDoc++ )
    {
        ScDocument* pDoc = (ScDocument*)*ppDoc;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
    }
}



void ScAddInAsync::RemoveDocument( ScDocument* pDocumentP )
{
    USHORT nPos = theAddInAsyncTbl.Count();
    if ( nPos )
    {
        const ScAddInAsync** ppAsync =
            (const ScAddInAsync**) theAddInAsyncTbl.GetData() + nPos - 1;
        for ( ; nPos-- >0; ppAsync-- )
        {   // rueckwaerts wg. Pointer-Aufrueckerei im Array
            ScAddInDocs* p = ((ScAddInAsync*)*ppAsync)->pDocs;
            USHORT nFoundPos;
            if ( p->Seek_Entry( pDocumentP, &nFoundPos ) )
            {
                p->Remove( nFoundPos );
                if ( p->Count() == 0 )
                {   // dieses AddIn wird nicht mehr benutzt
                    ScAddInAsync* pAsync = (ScAddInAsync*)*ppAsync;
                    theAddInAsyncTbl.Remove( nPos );
                    delete pAsync;
                    ppAsync = (const ScAddInAsync**) theAddInAsyncTbl.GetData()
                        + nPos;
                }
            }
        }
    }
}



