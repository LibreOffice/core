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


TYPEINIT1( SwAutoCorrect, SvxAutoCorrect );


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
    if( !rObjSh.IsA( TYPE(SwDocShell) ) )
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
