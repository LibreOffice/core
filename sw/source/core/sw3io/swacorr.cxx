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

#include <swacorr.hxx>
#include <swblocks.hxx>
#include "SwXMLTextBlocks.hxx"
#include <swerror.h>
#include <docsh.hxx>
#include <editsh.hxx>
#include <sot/storage.hxx>

using namespace ::com::sun::star;


TYPEINIT1( SwAutoCorrect, SvxAutoCorrect );


    //  - return den Ersetzungstext (nur fuer SWG-Format, alle anderen
    //      koennen aus der Wortliste herausgeholt werden!)
    //      rShort ist der Stream-Name - gecryptet!

sal_Bool SwAutoCorrect::GetLongText( const uno::Reference < embed::XStorage >& rStg,
                                     const OUString& rFileName, const OUString& rShort, OUString& rLong )
{
    sal_uLong nRet = 0;
    if (rStg.is())
    {
        // mba: relative URLs don't make sense here
        SwXMLTextBlocks aBlk( rStg, rFileName );
        nRet = aBlk.GetText( rShort, rLong );
    }
    else {
       OSL_ENSURE( rStg.is(), "Someone passed SwAutoCorrect::GetLongText a dud storage!");
    }
    return !IsError( nRet ) && !rLong.isEmpty();
}

    //  - Text mit Attributierung (kann nur der SWG - SWG-Format!)
    //      rShort ist der Stream-Name - gecryptet!
sal_Bool SwAutoCorrect::PutText( const uno::Reference < embed::XStorage >&  rStg,
                                 const OUString& rFileName, const OUString& rShort,
                                 SfxObjectShell& rObjSh, OUString& rLong )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
