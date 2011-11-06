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


#include <viewsh.hxx>
#include "swfntcch.hxx"
#include "fmtcol.hxx"
#include "swfont.hxx"

// aus atrstck.cxx
extern const sal_uInt8 StackPos[];

// globale Variablen, werden in SwFntCch.Hxx bekanntgegeben
// Der FontCache wird in TxtInit.Cxx _TXTINIT erzeugt und in _TXTEXIT geloescht
SwFontCache *pSwFontCache = NULL;

/*************************************************************************
|*
|*  SwFontObj::SwFontObj(), ~SwFontObj()
|*
|*  Ersterstellung      AMA 25. Jun. 95
|*  Letzte Aenderung    AMA 25. Jun. 95
|*
|*************************************************************************/

SwFontObj::SwFontObj( const void *pOwn, ViewShell *pSh ) :
    SwCacheObj( (void*)pOwn ),
    aSwFont( &((SwTxtFmtColl *)pOwn)->GetAttrSet(), pSh ? pSh->getIDocumentSettingAccess() : 0 )
{
    aSwFont.GoMagic( pSh, aSwFont.GetActual() );
    const SwAttrSet& rAttrSet = ((SwTxtFmtColl *)pOwn)->GetAttrSet();
    for (sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i, sal_True );
}

SwFontObj::~SwFontObj()
{
}

/*************************************************************************
|*
|*  SwFontAccess::SwFontAccess()
|*
|*  Ersterstellung      AMA 25. Jun. 95
|*  Letzte Aenderung    AMA 25. Jun. 95
|*
|*************************************************************************/

SwFontAccess::SwFontAccess( const void *pOwn, ViewShell *pSh ) :
    SwCacheAccess( *pSwFontCache, pOwn,
            ((SwTxtFmtColl*)pOwn)->IsInSwFntCache() ),
    pShell( pSh )
{
}

SwFontObj *SwFontAccess::Get( )
{
    return (SwFontObj *) SwCacheAccess::Get( );
}

SwCacheObj *SwFontAccess::NewObj( )
{
    ((SwTxtFmtColl*)pOwner)->SetInSwFntCache( sal_True );
    return new SwFontObj( pOwner, pShell );
}


