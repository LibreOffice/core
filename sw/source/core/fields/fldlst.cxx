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

#include <calbck.hxx>
#include "editsh.hxx"
#include "doc.hxx"
#include <IDocumentFieldsAccess.hxx>
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
    pSrtLst = new _SetGetExpFields();

    const SwFieldTypes& rFieldTypes = *pSh->GetDoc()->getIDocumentFieldsAccess().GetFieldTypes();
    const size_t nSize = rFieldTypes.size();

    // iterate over all types
    for(size_t i=0; i < nSize; ++i)
    {
        SwFieldType* pFieldType = rFieldTypes[ i ];
        const sal_uInt16 nType = pFieldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType || RES_DROPDOWN == nType )
        {
            SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
            for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
            {
                const SwTextField* pTextField = pFormatField->GetTextField();

                // only process InputFields, interactive SetExpFields and DropDown fields
                if( !pTextField || ( RES_SETEXPFLD == nType &&
                    !static_cast<SwSetExpField*>(pFormatField->GetField())->GetInputFlag()))
                    continue;

                const SwTextNode& rTextNode = pTextField->GetTextNode();
                if( rTextNode.GetNodes().IsDocNodes() )
                {
                    if( bBuildTmpLst )
                    {
                        aTmpLst.insert( pTextField );
                    }
                    else
                    {
                        SwNodeIndex aIdx( rTextNode );
                        _SetGetExpField* pNew = new _SetGetExpField(aIdx, pTextField );
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
    const SwTextField* pTextField = (*pSrtLst)[ nId ]->GetTextField();
    OSL_ENSURE( pTextField, "no TextField" );
    return const_cast<SwField*>(pTextField->GetFormatField().GetField());
}

/// save cursor
void SwInputFieldList::PushCursor()
{
    pSh->Push();
    pSh->ClearMark();
}

/// get cursor
void SwInputFieldList::PopCursor()
{
    pSh->Pop(false);
}

/// go to position of a field
void SwInputFieldList::GotoFieldPos(size_t nId)
{
    pSh->StartAllAction();
    (*pSrtLst)[ nId ]->GetPosOfContent( *pSh->GetCursor()->GetPoint() );
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
    const SwFieldTypes& rFieldTypes = *pSh->GetDoc()->getIDocumentFieldsAccess().GetFieldTypes();
    const size_t nSize = rFieldTypes.size();

    // iterate over all types
    for( size_t i = 0; i < nSize; ++i )
    {
        SwFieldType* pFieldType = rFieldTypes[ i ];
        const sal_uInt16 nType = pFieldType->Which();

        if( RES_SETEXPFLD == nType || RES_INPUTFLD == nType )
        {
            SwIterator<SwFormatField,SwFieldType> aIter( *pFieldType );
            for( SwFormatField* pFormatField = aIter.First(); pFormatField; pFormatField = aIter.Next() )
            {
                const SwTextField* pTextField = pFormatField->GetTextField();

                //  process only InputFields and interactive SetExpFields
                if( !pTextField || ( RES_SETEXPFLD == nType &&
                    !static_cast<SwSetExpField*>(pFormatField->GetField())->GetInputFlag()))
                    continue;

                const SwTextNode& rTextNode = pTextField->GetTextNode();
                if( rTextNode.GetNodes().IsDocNodes() )
                {
                    // not in TempList, thus add to SortList
                    std::set<const SwTextField*>::iterator it = aTmpLst.find( pTextField );
                    if( aTmpLst.end() == it )
                    {
                        SwNodeIndex aIdx( rTextNode );
                        _SetGetExpField* pNew = new _SetGetExpField(aIdx, pTextField );
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
