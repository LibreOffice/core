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

SwInputFieldList::SwInputFieldList( SwEditShell* pShell, sal_Bool bBuildTmpLst )
    : pSh(pShell)
{
    // create sorted list of all  input fields
    pSrtLst = new _SetGetExpFlds();

    const SwFldTypes& rFldTypes = *pSh->GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = rFldTypes.size();

    // iterate over all types
    for(sal_uInt16 i=0; i < nSize; ++i)
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType || RES_DROPDOWN == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFld->GetTxtFld();

                // only process InputFields, interactive SetExpFlds and DropDown fields
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFld->GetFld())->GetInputFlag()))
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

// get field from list in sorted order
SwField* SwInputFieldList::GetField(sal_uInt16 nId)
{
    const SwTxtFld* pTxtFld = (*pSrtLst)[ nId ]->GetFld();
    OSL_ENSURE( pTxtFld, "no TextFld" );
    return (SwField*)pTxtFld->GetFld().GetFld();
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
    pSh->Pop(sal_False);
}

/// go to position of a field
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

    // iterate over all types
    for( sal_uInt16 i = 0; i < nSize; ++i )
    {
        SwFieldType* pFldType = (SwFieldType*)rFldTypes[ i ];
        sal_uInt16 nType = pFldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
            for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
            {
                const SwTxtFld* pTxtFld = pFld->GetTxtFld();

                //  process only InputFields and interactive SetExpFlds
                if( !pTxtFld || ( RES_SETEXPFLD == nType &&
                    !((SwSetExpField*)pFld->GetFld())->GetInputFlag()))
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
    return pSrtLst->size();
}

/// remove all fields outside of any selections from list
void SwInputFieldList::RemoveUnselectedFlds()
{
    _SetGetExpFlds* pNewLst = new _SetGetExpFlds();

    FOREACHPAM_START(pSh->GetCrsr())
    {
        for (sal_uInt16 i = 0; i < Count();)
        {
            _SetGetExpFld* pFld = (*pSrtLst)[i];
            SwPosition aPos(*PCURCRSR->GetPoint());

            pFld->GetPos( aPos );

            if (aPos >= *PCURCRSR->Start() && aPos < *PCURCRSR->End())
            {
                // field in selection
                pNewLst->insert( (*pSrtLst)[i] );
                pSrtLst->erase(i);
            }
            else
                i++;
        }
    }
    FOREACHPAM_END()

    delete pSrtLst;
    pSrtLst = pNewLst;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
