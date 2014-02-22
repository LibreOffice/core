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

#include <unotools/charclass.hxx>
#include <editsh.hxx>
#include <fldbas.hxx>
#include <doc.hxx>
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


sal_uInt16 SwEditShell::GetFldTypeCount(sal_uInt16 nResId, sal_Bool bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
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

    
    sal_uInt16 nIdx  = 0;
    for(sal_uInt16 i = 0; i < nSize; ++i)
    {   
        SwFieldType& rFldType = *((*pFldTypes)[i]);
        if(rFldType.Which() == nResId)
            nIdx++;
    }
    return nIdx;
}


SwFieldType* SwEditShell::GetFldType(sal_uInt16 nFld, sal_uInt16 nResId, sal_Bool bUsed ) const
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
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
    {   
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


SwFieldType* SwEditShell::GetFldType(sal_uInt16 nResId, const OUString& rName) const
{
    return GetDoc()->GetFldType( nResId, rName, false );
}


void SwEditShell::RemoveFldType(sal_uInt16 nFld, sal_uInt16 nResId)
{
    if( USHRT_MAX == nResId )
    {
        GetDoc()->RemoveFldType(nFld);
        return;
    }

    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();
    sal_uInt16 nIdx = 0;
    for( sal_uInt16 i = 0; i < nSize; ++i )
        
        if( (*pFldTypes)[i]->Which() == nResId &&
            nIdx++ == nFld )
        {
            GetDoc()->RemoveFldType( i );
            return;
        }
}


void SwEditShell::RemoveFldType(sal_uInt16 nResId, const OUString& rStr)
{
    const SwFldTypes* pFldTypes = GetDoc()->GetFldTypes();
    const sal_uInt16 nSize = pFldTypes->size();
    const CharClass& rCC = GetAppCharClass();

    OUString aTmp( rCC.lowercase( rStr ));

    for(sal_uInt16 i = 0; i < nSize; ++i)
    {
        
        SwFieldType* pFldType = (*pFldTypes)[i];
        if( pFldType->Which() == nResId )
        {
            if( aTmp == rCC.lowercase( pFldType->GetName() ) )
            {
                GetDoc()->RemoveFldType(i);
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

    Pop( sal_False );
    EndAllAction();
    EndUndo( UNDO_DELETE );
}


void SwEditShell::Insert2(SwField& rFld, const bool bForceExpandHints)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    SwFmtFld aFld( rFld );

    const SetAttrMode nInsertFlags = (bForceExpandHints)
        ? nsSetAttrMode::SETATTR_FORCEHINTEXPAND
        : nsSetAttrMode::SETATTR_DEFAULT;

    FOREACHPAM_START(GetCrsr()) 
        const bool bSuccess(GetDoc()->InsertPoolItem(*PCURCRSR, aFld, nInsertFlags));
        OSL_ENSURE( bSuccess, "Doc->Insert(Field) failed");
        (void) bSuccess;
    FOREACHPAM_END()

    EndAllAction();
}



static SwTxtFld* lcl_FindInputFld( SwDoc* pDoc, SwField& rFld )
{
    
    SwTxtFld* pTFld = 0;
    if( RES_INPUTFLD == rFld.Which() )
    {
        const SfxPoolItem* pItem = NULL;
        const sal_uInt32 nMaxItems =
            pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_INPUTFIELD );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_INPUTFIELD, n ) )
                && ((SwFmtFld*)pItem)->GetField() == &rFld )
            {
                pTFld = ((SwFmtFld*)pItem)->GetTxtFld();
                break;
            }
    }
    else if( RES_SETEXPFLD == rFld.Which()
        && ((SwSetExpField&)rFld).GetInputFlag() )
    {
        const SfxPoolItem* pItem = NULL;
        const sal_uInt32 nMaxItems =
            pDoc->GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
        for( sal_uInt32 n = 0; n < nMaxItems; ++n )
            if( 0 != (pItem = pDoc->GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) )
                && ((SwFmtFld*)pItem)->GetField() == &rFld )
            {
                pTFld = ((SwFmtFld*)pItem)->GetTxtFld();
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

            if (!pTxtFld) 
                pTxtFld = lcl_FindInputFld( GetDoc(), rFld);

            if (pTxtFld != 0)
                GetDoc()->UpdateFld(pTxtFld, rFld, pMsgHnt, true);
        }

        
        
        
        bool bOkay = true;
        sal_Bool bTblSelBreak = sal_False;

        SwMsgPoolItem aFldHint( RES_TXTATR_FIELD );  
        SwMsgPoolItem aAnnotationFldHint( RES_TXTATR_ANNOTATION );
        SwMsgPoolItem aInputFldHint( RES_TXTATR_INPUTFIELD );
        FOREACHPAM_START(GetCrsr())               
            if( PCURCRSR->HasMark() && bOkay )    
            {
                
                SwPaM aCurPam( *PCURCRSR->GetMark(), *PCURCRSR->GetPoint() );
                SwPaM aPam( *PCURCRSR->GetPoint() );

                SwPosition *pCurStt = aCurPam.Start(), *pCurEnd =
                    aCurPam.End();
                /*
                 * In case that there are two contiguous fields in a PaM, the aPam goes step by step
                 * to the end. aCurPam is reduced in each loop. If aCurPam was searched completely,
                 * the loop terminates because Start = End.
                 */

                
                while(  bOkay
                     && pCurStt->nContent != pCurEnd->nContent
                     && ( aPam.Find( aFldHint, false, fnMoveForward, &aCurPam )
                          || aPam.Find( aAnnotationFldHint, false, fnMoveForward, &aCurPam )
                          || aPam.Find( aInputFldHint, false, fnMoveForward, &aCurPam ) ) )
                {
                    
                    if( aPam.Start()->nContent != pCurStt->nContent )
                        bOkay = false;

                    if( 0 != (pTxtFld = GetTxtFldAtPos( pCurStt, true )) )
                    {
                        pFmtFld = (SwFmtFld*)&pTxtFld->GetFmtFld();
                        SwField *pCurFld = pFmtFld->GetField();

                        
                        if( pCurFld->GetTyp()->Which() !=
                            rFld.GetTyp()->Which() )
                            bOkay = false;

                        bTblSelBreak = GetDoc()->UpdateFld(pTxtFld, rFld,
                                                           pMsgHnt, false);
                    }
                    
                    pCurStt->nContent++;
                }
            }

            if( bTblSelBreak ) 
                break;

        FOREACHPAM_END()
    }
    GetDoc()->SetModified();
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


void SwEditShell::UpdateExpFlds(sal_Bool bCloseDB)
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UpdateExpFlds(NULL, true);
    if (bCloseDB)
        GetDoc()->GetNewDBMgr()->CloseAll(); 
    EndAllAction();
}

SwNewDBMgr* SwEditShell::GetNewDBMgr() const
{
    return GetDoc()->GetNewDBMgr();
}


SwFieldType* SwEditShell::InsertFldType(const SwFieldType& rFldType)
{
    return GetDoc()->InsertFldType(rFldType);
}

void SwEditShell::LockExpFlds()
{
    GetDoc()->LockExpFlds();
}

void SwEditShell::UnlockExpFlds()
{
    GetDoc()->UnlockExpFlds();
}

void SwEditShell::SetFldUpdateFlags( SwFldUpdateFlags eFlags )
{
    getIDocumentSettingAccess()->setFieldUpdateFlags( eFlags );
}

SwFldUpdateFlags SwEditShell::GetFldUpdateFlags(sal_Bool bDocSettings) const
{
    return getIDocumentSettingAccess()->getFieldUpdateFlags( !bDocSettings );
}

void SwEditShell::SetLabelDoc( sal_Bool bFlag )
{
    GetDoc()->set(IDocumentSettingAccess::LABEL_DOCUMENT, bFlag );
}

sal_Bool SwEditShell::IsLabelDoc() const
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::LABEL_DOCUMENT);
}

void SwEditShell::ChangeAuthorityData(const SwAuthEntry* pNewData)
{
    GetDoc()->ChangeAuthorityData(pNewData);
}

sal_Bool SwEditShell::IsAnyDatabaseFieldInDoc()const
{
    const SwFldTypes * pFldTypes = GetDoc()->GetFldTypes();
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
                            return sal_True;
                        pFld = aIter.Next();
                    }
                }
                break;
            }
        }
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
