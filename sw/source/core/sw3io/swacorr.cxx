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
#include <SwXMLTextBlocks.hxx>
#include <swerror.h>
#include <docsh.hxx>
#include <editsh.hxx>
#include <sot/storage.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

/**
 * Returns the replacement text
 *
 * Only for the SWG format, all others can be extracted from the word list
 *
 * @param rShort - the stream name (encrypted)
 */
bool SwAutoCorrect::GetLongText( const OUString& rShort, OUString& rLong )
{
    ErrCode nRet = ERRCODE_NONE;
    assert( m_pTextBlocks );
    nRet = m_pTextBlocks->GetText( rShort, rLong );
    return !nRet.IsError() && !rLong.isEmpty();
}

void SwAutoCorrect::refreshBlockList( const uno::Reference< embed::XStorage >& rStg )
{
    if (rStg.is())
    {
        // mba: relative URLs don't make sense here
        m_pTextBlocks.reset( new SwXMLTextBlocks( rStg, OUString() ) );
    }
    else {
       OSL_ENSURE( rStg.is(), "Someone passed SwAutoCorrect::refreshBlockList a dud storage!");
    }
}

/**
 * Text with attributes
 *
 * Only for SWG format
 *
 * @param rShort - the stream name (encrypted)
 */
bool SwAutoCorrect::PutText( const uno::Reference < embed::XStorage >&  rStg,
                                 const OUString& rFileName, const OUString& rShort,
                                 SfxObjectShell& rObjSh, OUString& rLong )
{
    if( nullptr == dynamic_cast<const SwDocShell*>( &rObjSh) )
        return false;

    SwDocShell& rDShell = static_cast<SwDocShell&>(rObjSh);
    ErrCode nRet = ERRCODE_NONE;

    // mba: relative URLs don't make sense here
    SwXMLTextBlocks aBlk( rStg, rFileName );
    SwDoc* pDoc = aBlk.GetDoc();

    nRet = aBlk.BeginPutDoc( rShort, rShort );
    if( ! nRet.IsError() )
    {
        rDShell.GetEditShell()->CopySelToDoc( pDoc );
        nRet = aBlk.PutDoc();
        aBlk.AddName ( rShort, rShort );
        if( ! nRet.IsError() )
            nRet = aBlk.GetText( rShort, rLong );
    }
    return ! nRet.IsError();
}

SwAutoCorrect::SwAutoCorrect( const SvxAutoCorrect& rACorr )
    : SvxAutoCorrect( rACorr )
{
    SwEditShell::SetAutoFormatFlags(&GetSwFlags());
}

SwAutoCorrect::~SwAutoCorrect()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
