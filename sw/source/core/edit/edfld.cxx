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

#include <config_features.h>

#include <unotools/charclass.hxx>
#include <editsh.hxx>
#include <fldbas.hxx>
#include <doc.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <docary.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <edimp.hxx>
#include <dbfld.hxx>
#include <expfld.hxx>
#include <flddat.hxx>
#include <swundo.hxx>
#include <dbmgr.hxx>
#include <swddetbl.hxx>
#include <hints.hxx>
#include <switerator.hxx>
#include <fieldhint.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentContentOperations.hxx>

/// count field types with a ResId, if 0 count all
sal_uInt16 SwEditShell::GetFldTypeCount(sal_uInt16 nResId, bool bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->getIDocumentFieldsAccess().GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();

    if(nResId == USHRT_MAX)
    {
        if(!bUsed)
            return nSize;
        else
        {
            sal_uInt16 nUsed = 0;
            for ( sal_uInt16 i = 0; i < nSize; i++ )
            {
                if(IsUsed(*(*pFldTypes)[i]))
                    nUsed++;
            }
            return nUsed;
        }
    }

    // all types with the same ResId
    sal_uInt16 nIdx  = 0;
    for(sal_uInt16 i = 0; i < nSize; ++i)
    {   // same ResId -> increment index
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        if(rFldType.Which() == nResId)
            nIdx++;
    }
    return nIdx;
}

/// get field types with a ResId, if 0 get all
SwFieldType* SwEditShell::GetFldType(sal_uInt16 nFld, sal_uInt16 nResId, bool bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->getIDocumentFieldsAccess().GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();

    if(nResId == USHRT_MAX && nFld < nSize)
    {
        if(!bUsed)
            return (*pFldTypes)[nFld];
        else
        {
            sal_uInt16 i, nUsed = 0;
            for ( i = 0; i < nSize; i++ )
            {
                if(IsUsed(*(*pFldTypes)[i]))
                {
                    if(nUsed == nFld)
                        break;
                    nUsed++;
                }
            }
            return i < nSize ? (*pFldTypes)[i] : 0;
        }
    }

    sal_uInt16 nIdx = 0;
    for(sal_uInt16 i = 0; i < nSize; ++i)
    {   // same ResId -> increment index
        SwFieldType* pFldType = (*pFldTypes)[i];
        if(pFldType->Which() == nResId)
        {
            if (!bUsed || IsUsed(*pFldType))
            {
                if(nIdx == nFld)
                    return pFldType;
                nIdx++;
            }
        }
    }
    return 0;
}

/// get first type with given ResId and name
SwFieldType* SwEditShell::GetFldType(sal_uInt16 nResId, const OUString& rName) const
{
    return GetDoc()->getIDocumentFieldsAccess().GetFldType( nResId, rName, false );
}

/// delete field type
void SwEditShell::RemoveFldType(sal_uInt16 nFld, sal_uInt16 nResId)
{
    if( USHRT_MAX == nResId )
    {
        GetDoc()->getIDocumentFieldsAccess().RemoveFldType(nFld);
        return;
    }

    const SwFldTypes* pFldTypes = GetDoc()->getIDocumentFieldsAccess().GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();
    sal_uInt16 nIdx = 0;
    for( sal_uInt16 i = 0; i < nSize; ++i )
        // Gleiche ResId -> Index erhoehen
        if( (*pFldTypes)[i]->Which() == nResId &&
            nIdx++ == nFld )
        {
            GetDoc()->getIDocumentFieldsAccess().RemoveFldType( i );
            return;
        }
}

/// delete field type based on its name
void SwEditShell::RemoveFldType(sal_uInt16 nResId, const OUString& rStr)
{
    const SwFldTypes* pFldTypes = GetDoc()->getIDocumentFieldsAccess().GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();
    const CharClass& rCC = GetAppCharClass();

    OUString aTmp( rCC.lowercase( rStr ));

    for(sal_uInt16 i = 0; i < nSize; ++i)
    {
        // same ResId -> increment index
        SwFieldType* pFldType = (*pFldTypes)[i];
        if( pFldType->Which() == nResId )
        {
            if( aTmp == rCC.lowercase( pFldType->GetName() ) )
            {
                GetDoc()->getIDocumentFieldsAccess().RemoveFldType(i);
                return;
            }
        }
    }
}

void SwEditShell::FieldToText( SwFieldType* pType )
{
    if( !pType->GetDepends() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();
    StartUndo( UNDO_DELETE );
    Push();
    SwPaM* pPaM = GetCrsr();

    SwFieldHint aHint( pPaM );
    SwClientIter aIter( *pType );
    for ( SwClient* pClient = aIter.GoStart(); pClient; pClient = ++aIter )
    {
        pPaM->DeleteMark();
        pClient->SwClientNotifyCall( *pType, aHint );
     }

    Pop( false );
    EndAllAction();
    EndUndo( UNDO_DELETE );
}

/// add a field at the cursor position
void SwEditShell::Insert2(SwField& rFld, const bool bForceExpandHints)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwFmtFld aFld( rFld );

    const SetAttrMode nInsertFlags = (bForceExpandHints)
        ? nsSetAttrMode::SETATTR_FORCEHINTEXPAND
        : nsSetAttrMode::SETATTR_DEFAULT;

    FOREACHPAM_START(GetCrsr()) // for each PaM
        const bool bSuccess(GetDoc()->getIDocumentContentOperations().InsertPoolItem(*PCURCRSR, aFld, nInsertFlags));
        OSL_ENSURE( bSuccess, "Doc->Insert(Field) failed");
        (void) bSuccess;
    FOREACHPAM_END()

    EndAllAction();
}

/// Are the PaMs positioned on fields?
static SwTxtFld* lcl_FindInputFld( SwDoc* pDoc, SwField& rFld )
{
    // Search field via its address. For input fields this needs to be done in protected fields.
    SwTxtFld* pTFld = 0;
    if( RES_INPUTFLD == rFld.Which() )
    {
        const SfxPoolItem* pItem = NULL;
        const sal_uInt32 nMaxItems =
            pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INPUTFIELD );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_INPUTFIELD, n ) )
                && static_cast<const SwFmtFld*>(pItem)->GetField() == &rFld )
            {
                pTFld = const_cast<SwFmtFld*>(static_cast<const SwFmtFld*>(pItem))->GetTxtFld();
                break;
            }
    }
    else if( RES_SETEXPFLD == rFld.Which()
        && static_cast<SwSetExpField&>(rFld).GetInputFlag() )
    {
        const SfxPoolItem* pItem = NULL;
        const sal_uInt32 nMaxItems =
            pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) )
                && static_cast<const SwFmtFld*>(pItem)->GetField() == &rFld )
            {
                pTFld = const_cast<SwFmtFld*>(static_cast<const SwFmtFld*>(pItem))->GetTxtFld();
                break;
            }
    }
    return pTFld;
}

void SwEditShell::UpdateFlds( SwField &rFld )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    {
        // // If there are no selections so take the value of the current cursor position.
        SwMsgPoolItem* pMsgHnt = 0;
        SwRefMarkFldUpdate aRefMkHt( GetOut() );
        sal_uInt16 nFldWhich = rFld.GetTyp()->Which();
        if( RES_GETREFFLD == nFldWhich )
            pMsgHnt = &aRefMkHt;

        SwPaM* pCrsr = GetCrsr();
        SwTxtFld *pTxtFld;
        SwFmtFld *pFmtFld;

        if ( pCrsr->GetNext() == pCrsr && !pCrsr->HasMark())
        {
            pTxtFld = GetTxtFldAtPos( pCrsr->Start(), true );

            if (!pTxtFld) // #i30221#
                pTxtFld = lcl_FindInputFld( GetDoc(), rFld);

            if (pTxtFld != 0)
                GetDoc()->getIDocumentFieldsAccess().UpdateFld(pTxtFld, rFld, pMsgHnt, true);
        }

        // bOkay (instead of return because of EndAllAction) becomes false,
        // 1) if only one PaM has more than one field or
        // 2) if there are mixed field types
        bool bOkay = true;
        bool bTblSelBreak = false;

        SwMsgPoolItem aFldHint( RES_TXTATR_FIELD );  // Search-Hint
        SwMsgPoolItem aAnnotationFldHint( RES_TXTATR_ANNOTATION );
        SwMsgPoolItem aInputFldHint( RES_TXTATR_INPUTFIELD );
        FOREACHPAM_START(GetCrsr())               // for each PaM
            if( PCURCRSR->HasMark() && bOkay )    // ... with selection
            {
                // copy of the PaM
                SwPaM aCurPam( *PCURCRSR->GetMark(), *PCURCRSR->GetPoint() );
                SwPaM aPam( *PCURCRSR->GetPoint() );

                SwPosition *pCurStt = aCurPam.Start(), *pCurEnd =
                    aCurPam.End();
                /*
                 * In case that there are two contiguous fields in a PaM, the aPam goes step by step
                 * to the end. aCurPam is reduced in each loop. If aCurPam was searched completely,
                 * the loop terminates because Start = End.
                 */

                // Search for SwTxtFld ...
                while(  bOkay
                     && pCurStt->nContent != pCurEnd->nContent
                     && ( aPam.Find( aFldHint, false, fnMoveForward, &aCurPam, true )
                          || aPam.Find( aAnnotationFldHint, false, fnMoveForward, &aCurPam )
                          || aPam.Find( aInputFldHint, false, fnMoveForward, &aCurPam ) ) )
                {
                    // if only one PaM has more than one field  ...
                    if( aPam.Start()->nContent != pCurStt->nContent )
                        bOkay = false;

                    if( 0 != (pTxtFld = GetTxtFldAtPos( pCurStt, true )) )
                    {
                        pFmtFld = (SwFmtFld*)&pTxtFld->GetFmtFld();
                        SwField *pCurFld = pFmtFld->GetField();

                        // if there are mixed field types
                        if( pCurFld->GetTyp()->Which() !=
                            rFld.GetTyp()->Which() )
                            bOkay = false;

                        bTblSelBreak = GetDoc()->getIDocumentFieldsAccess().UpdateFld(pTxtFld, rFld,
                                                           pMsgHnt, false);
                    }
                    // The search area is reduced by the found area:
                    pCurStt->nContent++;
                }
            }

            if( bTblSelBreak ) // If table section and table formula are updated -> finish
                break;

        FOREACHPAM_END()
    }
    GetDoc()->getIDocumentState().SetModified();
    EndAllAction();
}

SwDBData SwEditShell::GetDBData() const
{
    return GetDoc()->GetDBData();
}

const SwDBData& SwEditShell::GetDBDesc() const
{
    return GetDoc()->GetDBDesc();
}

void SwEditShell::ChgDBData(const SwDBData& rNewData)
{
    GetDoc()->ChgDBData(rNewData);
}

void SwEditShell::GetAllUsedDB( std::vector<OUString>& rDBNameList,
                                std::vector<OUString>* pAllDBNames )
{
    GetDoc()->GetAllUsedDB( rDBNameList, pAllDBNames );
}

void SwEditShell::ChangeDBFields( const std::vector<OUString>& rOldNames,
                                  const OUString& rNewName )
{
    GetDoc()->ChangeDBFields( rOldNames, rNewName );
}

/// Update all expression fields
void SwEditShell::UpdateExpFlds(bool bCloseDB)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->getIDocumentFieldsAccess().UpdateExpFlds(NULL, true);
    if (bCloseDB)
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        GetDoc()->GetDBManager()->CloseAll(); // close all database connections
#endif
    }
    EndAllAction();
}

SwDBManager* SwEditShell::GetDBManager() const
{
#if HAVE_FEATURE_DBCONNECTIVITY
    return GetDoc()->GetDBManager();
#else
    return NULL;
#endif
}

/// insert field type
SwFieldType* SwEditShell::InsertFldType(const SwFieldType& rFldType)
{
    return GetDoc()->getIDocumentFieldsAccess().InsertFldType(rFldType);
}

void SwEditShell::LockExpFlds()
{
    GetDoc()->getIDocumentFieldsAccess().LockExpFlds();
}

void SwEditShell::UnlockExpFlds()
{
    GetDoc()->getIDocumentFieldsAccess().UnlockExpFlds();
}

void SwEditShell::SetFldUpdateFlags( SwFldUpdateFlags eFlags )
{
    getIDocumentSettingAccess()->setFieldUpdateFlags( eFlags );
}

SwFldUpdateFlags SwEditShell::GetFldUpdateFlags(bool bDocSettings) const
{
    return getIDocumentSettingAccess()->getFieldUpdateFlags( !bDocSettings );
}

void SwEditShell::SetLabelDoc( bool bFlag )
{
    GetDoc()->GetDocumentSettingManager().set(IDocumentSettingAccess::LABEL_DOCUMENT, bFlag );
}

bool SwEditShell::IsLabelDoc() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::LABEL_DOCUMENT);
}

void SwEditShell::ChangeAuthorityData(const SwAuthEntry* pNewData)
{
    GetDoc()->ChangeAuthorityData(pNewData);
}

bool SwEditShell::IsAnyDatabaseFieldInDoc()const
{
    const SwFldTypes * pFldTypes = GetDoc()->getIDocumentFieldsAccess().GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();
    for(sal_uInt16 i = 0; i < nSize; ++i)
    {
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        sal_uInt16 nWhich = rFldType.Which();
        if(IsUsed(rFldType))
        {
            switch(nWhich)
            {
                case RES_DBFLD:
                case RES_DBNEXTSETFLD:
                case RES_DBNUMSETFLD:
                case RES_DBSETNUMBERFLD:
                {
                    SwIterator<SwFmtFld,SwFieldType> aIter( rFldType );
                    SwFmtFld* pFld = aIter.First();
                    while(pFld)
                    {
                        if(pFld->IsFldInDoc())
                            return true;
                        pFld = aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
