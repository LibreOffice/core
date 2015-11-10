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

#include <wrtsh.hxx>
#include <shellres.hxx>
#include <swwait.hxx>
#include <view.hxx>
#include <toxmgr.hxx>
#include <crsskip.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <globals.hrc>

// handle indexes with TOXMgr
SwTOXMgr::SwTOXMgr(SwWrtShell* pShell):
    pSh(pShell)
{
    GetTOXMarks();
    SetCurTOXMark(0);
}

// handle current TOXMarks
sal_uInt16 SwTOXMgr::GetTOXMarks()
{
    return pSh->GetCurTOXMarks(aCurMarks);
}

SwTOXMark* SwTOXMgr::GetTOXMark(sal_uInt16 nId)
{
    if(!aCurMarks.empty())
        return aCurMarks[nId];
    return nullptr;
}

void SwTOXMgr::DeleteTOXMark()
{
    SwTOXMark* pNext = nullptr;
    if( pCurTOXMark )
    {
        pNext = const_cast<SwTOXMark*>(&pSh->GotoTOXMark( *pCurTOXMark, TOX_NXT ));
        if( pNext == pCurTOXMark )
            pNext = nullptr;

        pSh->DeleteTOXMark( pCurTOXMark );
        pSh->SetModified();
    }
    // go to next one
    pCurTOXMark = pNext;
}

void    SwTOXMgr::InsertTOXMark(const SwTOXMarkDescription& rDesc)
{
    SwTOXMark* pMark = nullptr;
    switch(rDesc.GetTOXType())
    {
        case  TOX_CONTENT:
        {
            OSL_ENSURE(rDesc.GetLevel() > 0 && rDesc.GetLevel() <= MAXLEVEL,
                       "invalid InsertTOCMark level");
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_CONTENT, 0));
            pMark->SetLevel( static_cast< sal_uInt16 >(rDesc.GetLevel()) );

            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
        }
        break;
        case  TOX_INDEX:
        {
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_INDEX, 0));

            if( rDesc.GetPrimKey() && !rDesc.GetPrimKey()->isEmpty() )
            {
                pMark->SetPrimaryKey( *rDesc.GetPrimKey() );
                if(rDesc.GetPhoneticReadingOfPrimKey())
                    pMark->SetPrimaryKeyReading( *rDesc.GetPhoneticReadingOfPrimKey() );

                if( rDesc.GetSecKey() && !rDesc.GetSecKey()->isEmpty() )
                {
                    pMark->SetSecondaryKey( *rDesc.GetSecKey() );
                    if(rDesc.GetPhoneticReadingOfSecKey())
                        pMark->SetSecondaryKeyReading( *rDesc.GetPhoneticReadingOfSecKey() );
                }
            }
            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
            if(rDesc.GetPhoneticReadingOfAltStr())
                pMark->SetTextReading( *rDesc.GetPhoneticReadingOfAltStr() );
            pMark->SetMainEntry(rDesc.IsMainEntry());
        }
        break;
        case  TOX_USER:
        {
            OSL_ENSURE(rDesc.GetLevel() > 0 && rDesc.GetLevel() <= MAXLEVEL,
                       "invalid InsertTOCMark level");
            sal_uInt16 nId = rDesc.GetTOUName() ?
                GetUserTypeID(*rDesc.GetTOUName()) : 0;
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_USER, nId));
            pMark->SetLevel( static_cast< sal_uInt16 >(rDesc.GetLevel()) );

            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
        }
        break;
        case  TOX_BIBLIOGRAPHY:
        {
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_BIBLIOGRAPHY, 0));

            if( rDesc.GetPrimKey() && !rDesc.GetPrimKey()->isEmpty() )
            {
                pMark->SetPrimaryKey( *rDesc.GetPrimKey() );
                if(rDesc.GetPhoneticReadingOfPrimKey())
                    pMark->SetPrimaryKeyReading( *rDesc.GetPhoneticReadingOfPrimKey() );

                if( rDesc.GetSecKey() && !rDesc.GetSecKey()->isEmpty() )
                {
                    pMark->SetSecondaryKey( *rDesc.GetSecKey() );
                    if(rDesc.GetPhoneticReadingOfSecKey())
                        pMark->SetSecondaryKeyReading( *rDesc.GetPhoneticReadingOfSecKey() );
                }
            }
            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
            if(rDesc.GetPhoneticReadingOfAltStr())
                pMark->SetTextReading( *rDesc.GetPhoneticReadingOfAltStr() );
            pMark->SetMainEntry(rDesc.IsMainEntry());
        }
        break;
        default:; //prevent warning
    }

    if (!pMark)
        return;

    pSh->StartAllAction();
    pSh->SwEditShell::Insert(*pMark);
    pSh->EndAllAction();
}

// Update of TOXMarks
void SwTOXMgr::UpdateTOXMark(const SwTOXMarkDescription& rDesc)
{
    assert(pCurTOXMark && "no current TOXMark");
    pSh->StartAllAction();
    if(pCurTOXMark->GetTOXType()->GetType() == TOX_INDEX)
    {
        if(rDesc.GetPrimKey() && !rDesc.GetPrimKey()->isEmpty() )
        {
            pCurTOXMark->SetPrimaryKey( *rDesc.GetPrimKey() );
            if(rDesc.GetPhoneticReadingOfPrimKey())
                pCurTOXMark->SetPrimaryKeyReading( *rDesc.GetPhoneticReadingOfPrimKey() );
            else
                pCurTOXMark->SetPrimaryKeyReading( aEmptyOUStr );

            if( rDesc.GetSecKey() && !rDesc.GetSecKey()->isEmpty() )
            {
                pCurTOXMark->SetSecondaryKey( *rDesc.GetSecKey() );
                if(rDesc.GetPhoneticReadingOfSecKey())
                    pCurTOXMark->SetSecondaryKeyReading( *rDesc.GetPhoneticReadingOfSecKey() );
                else
                    pCurTOXMark->SetSecondaryKeyReading( aEmptyOUStr );
            }
            else
            {
                pCurTOXMark->SetSecondaryKey( aEmptyOUStr );
                pCurTOXMark->SetSecondaryKeyReading( aEmptyOUStr );
            }
        }
        else
        {
            pCurTOXMark->SetPrimaryKey( aEmptyOUStr );
            pCurTOXMark->SetPrimaryKeyReading( aEmptyOUStr );
            pCurTOXMark->SetSecondaryKey( aEmptyOUStr );
            pCurTOXMark->SetSecondaryKeyReading( aEmptyOUStr );
        }
        if(rDesc.GetPhoneticReadingOfAltStr())
            pCurTOXMark->SetTextReading( *rDesc.GetPhoneticReadingOfAltStr() );
        else
            pCurTOXMark->SetTextReading( aEmptyOUStr );
        pCurTOXMark->SetMainEntry(rDesc.IsMainEntry());
    }
    else
        pCurTOXMark->SetLevel( static_cast< sal_uInt16 >(rDesc.GetLevel()) );

    if(rDesc.GetAltStr())
    {
        // JP 26.08.96: Bug 30344 - either the text of a Doc or an alternative test,
        //                          not both!
        bool bReplace = pCurTOXMark->IsAlternativeText();
        if( bReplace )
            pCurTOXMark->SetAlternativeText( *rDesc.GetAltStr() );
        else
        {
            SwTOXMark aCpy( *pCurTOXMark );
            aCurMarks.clear();
            pSh->DeleteTOXMark(pCurTOXMark);
            aCpy.SetAlternativeText( *rDesc.GetAltStr() );
            pSh->SwEditShell::Insert( aCpy );
            pCurTOXMark = nullptr;
        }
    }
    pSh->SetModified();
    pSh->EndAllAction();
    // Bug 36207 pCurTOXMark points nowhere here!
    if(!pCurTOXMark)
    {
        pSh->Left(CRSR_SKIP_CHARS, false, 1, false );
        pSh->GetCurTOXMarks(aCurMarks);
        SetCurTOXMark(0);
    }
}

// determine UserTypeID
sal_uInt16 SwTOXMgr::GetUserTypeID(const OUString& rStr)
{
    sal_uInt16 nSize = pSh->GetTOXTypeCount(TOX_USER);
    for(sal_uInt16 i=0; i < nSize; ++i)
    {
        const SwTOXType* pTmp = pSh->GetTOXType(TOX_USER, i);
        if(pTmp && pTmp->GetTypeName() == rStr)
            return i;
    }
    SwTOXType aUserType(TOX_USER, rStr);
    pSh->InsertTOXType(aUserType);
    return nSize;
}

// traveling between TOXMarks
void SwTOXMgr::NextTOXMark(bool bSame)
{
    OSL_ENSURE(pCurTOXMark, "no current TOXMark");
    if( pCurTOXMark )
    {
        SwTOXSearch eDir = bSame ? TOX_SAME_NXT : TOX_NXT;
        pCurTOXMark = const_cast<SwTOXMark*>(&pSh->GotoTOXMark( *pCurTOXMark, eDir ));
    }
}

void SwTOXMgr::PrevTOXMark(bool bSame)
{
    OSL_ENSURE(pCurTOXMark, "no current TOXMark");
    if( pCurTOXMark )
    {
        SwTOXSearch eDir = bSame ? TOX_SAME_PRV : TOX_PRV;
        pCurTOXMark = const_cast<SwTOXMark*>(&pSh->GotoTOXMark(*pCurTOXMark, eDir ));
    }
}

// insert keyword index
const SwTOXBase* SwTOXMgr::GetCurTOX()
{
    return pSh->GetCurTOX();
}
const SwTOXType* SwTOXMgr::GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const
{
    return pSh->GetTOXType(eTyp, nId);
}

void SwTOXMgr::SetCurTOXMark(sal_uInt16 nId)
{
    pCurTOXMark = (nId < aCurMarks.size()) ? aCurMarks[nId] : nullptr;
}

bool SwTOXMgr::UpdateOrInsertTOX(const SwTOXDescription& rDesc,
                                    SwTOXBase** ppBase,
                                    const SfxItemSet* pSet)
{
    SwWait aWait( *pSh->GetView().GetDocShell(), true );
    bool bRet = true;
    const SwTOXBase* pCurTOX = ppBase && *ppBase ? *ppBase : GetCurTOX();
    SwTOXBase* pTOX = const_cast<SwTOXBase*>(pCurTOX);

    SwTOXBase * pNewTOX = nullptr;

    if (pTOX)
        pNewTOX = new SwTOXBase(*pTOX);

    TOXTypes eCurTOXType = rDesc.GetTOXType();
    if(pCurTOX && !ppBase && pSh->HasSelection())
        pSh->EnterStdMode();

    switch(eCurTOXType)
    {
        case TOX_INDEX :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(pType, aForm, nsSwTOXElement::TOX_MARK, pType->GetTypeName());
            }
            pNewTOX->SetOptions(rDesc.GetIndexOptions());
            pNewTOX->SetMainEntryCharStyle(rDesc.GetMainEntryCharStyle());
            pSh->SetTOIAutoMarkURL(rDesc.GetAutoMarkURL());
            pSh->ApplyAutoMark();
        }
        break;
        case TOX_CONTENT :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(pType, aForm, rDesc.GetContentOptions(), pType->GetTypeName());
            }
            pNewTOX->SetCreate(rDesc.GetContentOptions());
            pNewTOX->SetLevel(rDesc.GetLevel());
        }
        break;
        case TOX_USER :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                sal_uInt16 nPos  = 0;
                sal_uInt16 nSize = pSh->GetTOXTypeCount(eCurTOXType);
                for(sal_uInt16 i=0; rDesc.GetTOUName() && i < nSize; ++i)
                {   const SwTOXType* pType = pSh->GetTOXType(TOX_USER, i);
                    if(pType->GetTypeName() == *rDesc.GetTOUName())
                    {   nPos = i;
                        break;
                    }
                }
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, nPos);

                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(pType, aForm, rDesc.GetContentOptions(), pType->GetTypeName());

            }
            else
            {
                const_cast<SwTOXBase*>( pCurTOX )->SetCreate(rDesc.GetContentOptions());
            }
            pNewTOX->SetLevelFromChapter(rDesc.IsLevelFromChapter());
        }
        break;
        case TOX_CITATION: /** TODO */break;
        case TOX_OBJECTS:
        case TOX_TABLES:
        case TOX_AUTHORITIES:
        case TOX_BIBLIOGRAPHY:
        case TOX_ILLUSTRATIONS:
        {
            //Special handling for TOX_AUTHORITY
            if(TOX_AUTHORITIES == eCurTOXType)
            {
                SwAuthorityFieldType* pFType = static_cast<SwAuthorityFieldType*>(
                                                pSh->GetFieldType(RES_AUTHORITY, aEmptyOUStr));
                if (!pFType)
                {
                    SwAuthorityFieldType const type(pSh->GetDoc());
                    pFType = static_cast<SwAuthorityFieldType*>(
                                pSh->InsertFieldType(type));
                }
                pFType->SetPreSuffix(rDesc.GetAuthBrackets()[0],
                    rDesc.GetAuthBrackets()[1]);
                pFType->SetSequence(rDesc.IsAuthSequence());
                SwTOXSortKey rArr[3];
                rArr[0] = rDesc.GetSortKey1();
                rArr[1] = rDesc.GetSortKey2();
                rArr[2] = rDesc.GetSortKey3();
                pFType->SetSortKeys(3, rArr);
                pFType->SetSortByDocument(rDesc.IsSortByDocument());
                pFType->SetLanguage(rDesc.GetLanguage());
                pFType->SetSortAlgorithm(rDesc.GetSortAlgorithm());

                pFType->UpdateFields();
            }
            // TODO: consider properties of the current TOXType
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(
                    pType, aForm,
                    TOX_AUTHORITIES == eCurTOXType ? nsSwTOXElement::TOX_MARK : 0, pType->GetTypeName());
            }
            else
            {
                if((!ppBase || !(*ppBase)) && pSh->HasSelection())
                    pSh->DelRight();
                pNewTOX = const_cast<SwTOXBase*>(pCurTOX);
            }
            pNewTOX->SetFromObjectNames(rDesc.IsCreateFromObjectNames());
            pNewTOX->SetOLEOptions(rDesc.GetOLEOptions());
        }
        break;
    }

    OSL_ENSURE(pNewTOX, "no TOXBase created!" );
    if(!pNewTOX)
        return false;

    pNewTOX->SetFromChapter(rDesc.IsFromChapter());
    pNewTOX->SetSequenceName(rDesc.GetSequenceName());
    pNewTOX->SetCaptionDisplay(rDesc.GetCaptionDisplay());
    pNewTOX->SetProtected(rDesc.IsReadonly());

    for(sal_uInt16 nLevel = 0; nLevel < MAXLEVEL; nLevel++)
        pNewTOX->SetStyleNames(rDesc.GetStyleNames(nLevel), nLevel);

    if(rDesc.GetTitle())
        pNewTOX->SetTitle(*rDesc.GetTitle());
    if(rDesc.GetForm())
        pNewTOX->SetTOXForm(*rDesc.GetForm());
    pNewTOX->SetLanguage(rDesc.GetLanguage());
    pNewTOX->SetSortAlgorithm(rDesc.GetSortAlgorithm());

    if(!pCurTOX || (ppBase && !(*ppBase)) )
    {
        // when ppBase is passed over, TOXBase is only created here
        // and then inserted in a global document by the dialog
        if(ppBase)
            (*ppBase) = pNewTOX;
        else
        {
            pSh->InsertTableOf(*pNewTOX, pSet);
            delete pNewTOX;
        }
    }
    else
    {
        SwDoc * pDoc = pSh->GetDoc();

        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pDoc->GetIDocumentUndoRedo().DelAllUndoObj();
            pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_TOXCHANGE, nullptr);
        }

        pDoc->ChgTOX(*pTOX, *pNewTOX);

        pTOX->DisableKeepExpression();
        bRet = pSh->UpdateTableOf(*pTOX, pSet);
        pTOX->EnableKeepExpression();

        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_TOXCHANGE, nullptr);
        }
    }

    return bRet;
}

void SwTOXDescription::SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3)
{
    SwTOXSortKey aArr[3];
    sal_uInt16 nPos = 0;
    if(AUTH_FIELD_END > eKey1.eField)
        aArr[nPos++] = eKey1;
    if(AUTH_FIELD_END > eKey2.eField)
        aArr[nPos++] = eKey2;
    if(AUTH_FIELD_END > eKey3.eField)
        aArr[nPos++] = eKey3;

    eSortKey1 = aArr[0];
    eSortKey2 = aArr[1];
    eSortKey3 = aArr[2];
}

void SwTOXDescription::ApplyTo(SwTOXBase& rTOXBase)
{
    for(sal_uInt16 i = 0; i < MAXLEVEL; i++)
        rTOXBase.SetStyleNames(GetStyleNames(i), i);
    rTOXBase.SetTitle(GetTitle() ? *GetTitle() : OUString());
    rTOXBase.SetCreate(GetContentOptions());

    if(GetTOXType() == TOX_INDEX)
        rTOXBase.SetOptions(GetIndexOptions());
    if(GetTOXType() != TOX_INDEX)
        rTOXBase.SetLevel(GetLevel());
    rTOXBase.SetFromObjectNames(IsCreateFromObjectNames());
    rTOXBase.SetSequenceName(GetSequenceName());
    rTOXBase.SetCaptionDisplay(GetCaptionDisplay());
    rTOXBase.SetFromChapter(IsFromChapter());
    rTOXBase.SetProtected(IsReadonly());
    rTOXBase.SetOLEOptions(GetOLEOptions());
    rTOXBase.SetLevelFromChapter(IsLevelFromChapter());
    rTOXBase.SetLanguage(eLanguage);
    rTOXBase.SetSortAlgorithm(sSortAlgorithm);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
