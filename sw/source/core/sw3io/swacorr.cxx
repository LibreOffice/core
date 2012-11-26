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


#include <tools/urlobj.hxx>
#include <swacorr.hxx>
#include <swblocks.hxx>
#include "SwXMLTextBlocks.hxx"
#ifndef _SWSERROR_H
#include <swerror.h>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <editsh.hxx>
#include <sot/storage.hxx>

using namespace ::com::sun::star;


    //  - return den Ersetzungstext (nur fuer SWG-Format, alle anderen
    //      koennen aus der Wortliste herausgeholt werden!)
    //      rShort ist der Stream-Name - gecryptet!

sal_Bool SwAutoCorrect::GetLongText( const uno::Reference < embed::XStorage >& rStg, const String& rFileName, const String& rShort, String& rLong )
{
    sal_uLong nRet = 0;
    if (rStg.is())
    {
        // mba: relative URLs don't make sense here
        SwXMLTextBlocks aBlk( rStg, rFileName );
        nRet = aBlk.GetText( rShort, rLong );
    }
    else {
        ASSERT ( rStg.is(), "Someone passed SwAutoCorrect::GetLongText a dud storage!");
    }
    return !IsError( nRet ) && rLong.Len();
}

    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    //      rShort ist der Stream-Name - gecryptet!
sal_Bool SwAutoCorrect::PutText( const uno::Reference < embed::XStorage >&  rStg, const String& rFileName, const String& rShort,
                            SfxObjectShell& rObjSh, String& rLong )
{
    if( !dynamic_cast< SwDocShell* >(&rObjSh) )
        return sal_False;

    SwDocShell& rDShell = (SwDocShell&)rObjSh;
    sal_uLong nRet = 0;

    // mba: relative URLs don't make sense here
    SwXMLTextBlocks aBlk( rStg, rFileName );
    SwDoc* pDoc = aBlk.GetDoc();

    nRet = aBlk.BeginPutDoc( rShort, rShort );
    if( !IsError( nRet ) )
    {
        ((SwEditShell*)rDShell.GetWrtShell())->_CopySelToDoc( pDoc );
        nRet = aBlk.PutDoc();
        aBlk.AddName ( rShort, rShort, sal_False );
        if( !IsError( nRet ) )
            nRet = aBlk.GetText( rShort, rLong );
    }
    return !IsError( nRet );
}


SwAutoCorrect::SwAutoCorrect( const SvxAutoCorrect& rACorr )
    : SvxAutoCorrect( rACorr )
{
    SwEditShell::SetAutoFmtFlags(&GetSwFlags());
}

SwAutoCorrect::~SwAutoCorrect()
{
}
