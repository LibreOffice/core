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

// sort input values

SwInputFieldList::SwInputFieldList( SwEditShell* pShell, bool bBuildTmpLst )
    : pSh(pShell)
{
    // create sorted list of all  input fields
    pSrtLst = new _SetGetExpFlds();

    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    const size_t nSize = rFldTypes.size();

    // iterate over all types
    for(size_t i=0; i < nSize; ++i)
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        const sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType || RES_DROPDOWN == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();

                // only process InputFields, interactive SetExpFlds and DropDown fields
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

size_t SwInputFieldList::Count() const
{
    return pSrtLst->size();
}

// get field from list in sorted order
SwField* SwInputFieldList::GetField(size_t nId)
{
    const SwTxtFld* pTxtFld = (*pSrtLst)[ nId ]->GetTxtFld();
    OSL_ENSURE( pTxtFld, "no TextFld" );
    return (SwField*)pTxtFld->GetFmtFld().GetField();
}

/// save cursor
void SwInputFieldList::PushCrsr()
{
    pSh->Push();
    pSh->ClearMark();
}

/// get cursor
void SwInputFieldList::PopCrsr()
{
    pSh->Pop(false);
}

/// go to position of a field
void SwInputFieldList::GotoFieldPos(size_t nId)
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
 * @return true if not empty
 */
bool SwInputFieldList::BuildSortLst()
{
    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    const size_t nSize = rFldTypes.size();

    // iterate over all types
    for( size_t i = 0; i < nSize; ++i )
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        const sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();

                //  process only InputFields and interactive SetExpFlds
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFmtFld->GetField())->GetInputFlag()))
                    continue;

                const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
                if( rTxtNode.GetNodes().IsDocNodes() )
                {
                    // not in TempList, thus add to SortList
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

    // the pointers are not needed anymore
    aTmpLst.clear();
    return !pSrtLst->empty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
