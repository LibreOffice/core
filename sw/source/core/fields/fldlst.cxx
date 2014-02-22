/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "switerator.hxx"
#include "editsh.hxx"
#include "doc.hxx"
#include <docary.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include "edimp.hxx"
#include "expfld.hxx"
#include "pam.hxx"
#include "docfld.hxx"
#include "ndtxt.hxx"



SwInputFieldList::SwInputFieldList( SwEditShell* pShell, sal_Bool bBuildTmpLst )
    : pSh(pShell)
{
    
    pSrtLst = new _SetGetExpFlds();

    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = rFldTypes.size();

    
    for(sal_uInt16 i=0; i < nSize; ++i)
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType || RES_DROPDOWN == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();

                
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFmtFld->GetField())->GetInputFlag()))
                    continue;

                const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    if( bBuildTmpLst )
                    {
                        aTmpLst.insert( pTxtFld );
                    }
                    else
                    {
                        SwNodeIndex aIdx( rTxtNode );
                        _SetGetExpFld* pNew = new _SetGetExpFld(aIdx, pTxtFld );
                        pSrtLst->insert( pNew );
                    }
                }
            }
        }
    }
}

SwInputFieldList::~SwInputFieldList()
{
    delete pSrtLst;
}

sal_uInt16 SwInputFieldList::Count() const
{
    return pSrtLst->size();
}


SwField* SwInputFieldList::GetField(sal_uInt16 nId)
{
    const SwTxtFld* pTxtFld = (*pSrtLst)[ nId ]->GetTxtFld();
    OSL_ENSURE( pTxtFld, "no TextFld" );
    return (SwField*)pTxtFld->GetFmtFld().GetField();
}


void SwInputFieldList::PushCrsr()
{
    pSh->Push();
    pSh->ClearMark();
}


void SwInputFieldList::PopCrsr()
{
    pSh->Pop(sal_False);
}


void SwInputFieldList::GotoFieldPos(sal_uInt16 nId)
{
    pSh->StartAllAction();
    (*pSrtLst)[ nId ]->GetPosOfContent( *pSh->GetCrsr()->GetPoint() );
    pSh->EndAllAction();
}

/** Compare TmpLst with current fields.
 *
 * All new ones are added to SortList so that they can be updated.
 * For text blocks: update only input fields.
 *
 * @return count
 */
sal_uInt16 SwInputFieldList::BuildSortLst()
{
    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    sal_uInt16 nSize = rFldTypes.size();

    
    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();

                
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFmtFld->GetField())->GetInputFlag()))
                    continue;

                const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    
                    std::set<const SwTxtFld*>::iterator it = aTmpLst.find( pTxtFld );
                    if( aTmpLst.end() == it )
                    {
                        SwNodeIndex aIdx( rTxtNode );
                        _SetGetExpFld* pNew = new _SetGetExpFld(aIdx, pTxtFld );
                        pSrtLst->insert( pNew );
                    }
                    else
                        aTmpLst.erase( it );
                }
            }
        }
    }

    
    aTmpLst.clear();
    return pSrtLst->size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
