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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include <sfx2/objsh.hxx>

#include "adiasync.hxx"
#include "brdcst.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"       // FID_DATACHANGED
#include <osl/thread.h>


//------------------------------------------------------------------------

ScAddInAsyncs theAddInAsyncTbl;
static ScAddInAsync aSeekObj;


SV_IMPL_OP_PTRARR_SORT( ScAddInAsyncs, ScAddInAsyncPtr );

SV_IMPL_PTRARR_SORT( ScAddInDocs, ScAddInDocPtr );

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData )
{
    ScAddInAsync::CallBack( sal_uLong( nHandle ), pData );
}
}



ScAddInAsync::ScAddInAsync() :
    SvtBroadcaster(),
    nHandle( 0 )
{   // nur fuer aSeekObj !
}



ScAddInAsync::ScAddInAsync( sal_uLong nHandleP, sal_uInt16 nIndex, ScDocument* pDoc ) :
    SvtBroadcaster(),
    pStr( NULL ),
    nHandle( nHandleP ),
    bValid( sal_False )
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



ScAddInAsync* ScAddInAsync::Get( sal_uLong nHandleP )
{
    sal_uInt16 nPos;
    ScAddInAsync* pRet = 0;
    aSeekObj.nHandle = nHandleP;
    if ( theAddInAsyncTbl.Seek_Entry( &aSeekObj, &nPos ) )
        pRet = theAddInAsyncTbl[ nPos ];
    aSeekObj.nHandle = 0;
    return pRet;
}



void ScAddInAsync::CallBack( sal_uLong nHandleP, void* pData )
{
    ScAddInAsync* p;
    if ( (p = Get( nHandleP )) == NULL )
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
    p->bValid = sal_True;
    p->Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress(), NULL ) );

    const ScDocument** ppDoc = (const ScDocument**) p->pDocs->GetData();
    sal_uInt16 nCount = p->pDocs->Count();
    for ( sal_uInt16 j=0; j<nCount; j++, ppDoc++ )
    {
        ScDocument* pDoc = (ScDocument*)*ppDoc;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
    }
}



void ScAddInAsync::RemoveDocument( ScDocument* pDocumentP )
{
    sal_uInt16 nPos = theAddInAsyncTbl.Count();
    if ( nPos )
    {
        const ScAddInAsync** ppAsync =
            (const ScAddInAsync**) theAddInAsyncTbl.GetData() + nPos - 1;
        for ( ; nPos-- >0; ppAsync-- )
        {   // rueckwaerts wg. Pointer-Aufrueckerei im Array
            ScAddInDocs* p = ((ScAddInAsync*)*ppAsync)->pDocs;
            sal_uInt16 nFoundPos;
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



