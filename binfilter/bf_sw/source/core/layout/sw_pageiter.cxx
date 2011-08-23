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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <horiornt.hxx>

#include "doc.hxx"
#include "pagefrm.hxx"
#include "cntfrm.hxx"
#include "pam.hxx"

#include <node.hxx>
#include "pageiter.hxx"
#include "txtfrm.hxx"
namespace binfilter {




SwPageIter::SwPageIter( const SwDoc &rDoc, const SwPosition &rStartPos )
    : rPDoc( rDoc ), pPage(0)
{
    Seek( rStartPos );
}



BOOL SwPageIter::NextPage()
{
    if( IsEnd() )
        return FALSE;
    pPage = (SwPageFrm*)pPage->GetNext();
    return TRUE;
}



const SwPageDesc* SwPageIter::GetPageDesc() const
{
    return  ( IsEnd() )? 0 : pPage->GetPageDesc();
}



BOOL SwPageIter::Seek( const SwPosition &rPos )
{
    const SwTxtFrm *pTxt = (SwTxtFrm*)rPDoc.GetNodes()[rPos.nNode.GetIndex()]->
                                                GetCntntNode()->GetFrm();
    if ( !pTxt )
        return FALSE;

    pTxt = pTxt->GetFrmAtPos( rPos );
    pPage = pTxt->FindPageFrm();
    return TRUE;
}



BOOL SwPageIter::GetPosition( SwPosition &rPos ) const
{
    if( IsEnd() )
        return FALSE;

    const SwCntntFrm *pCnt = pPage->FindFirstBodyCntnt();
    if ( !pCnt )
        return FALSE;

    pCnt = ((SwTxtFrm*)pCnt)->GetFrmAtPos( rPos );
    if ( !pCnt )
        return FALSE;

    rPos.nNode = *pCnt->GetNode();
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                            ((SwTxtFrm*)pCnt)->GetOfst() );

    return TRUE;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
