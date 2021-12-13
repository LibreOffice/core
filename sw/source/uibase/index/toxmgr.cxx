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
#include <swwait.hxx>
#include <view.hxx>
#include <toxmgr.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <swundo.hxx>
#include <osl/diagnose.h>

// handle indexes with TOXMgr
SwTOXMgr::SwTOXMgr(SwWrtShell* pShell):
    m_pSh(pShell)
{
    m_pSh->GetCurTOXMarks(m_aCurMarks);
    SetCurTOXMark(0);
}

SwTOXMark* SwTOXMgr::GetTOXMark(sal_uInt16 nId)
{
    if(!m_aCurMarks.empty())
        return m_aCurMarks[nId];
    return nullptr;
}

void SwTOXMgr::DeleteTOXMark()
{
    SwTOXMark* pNext = nullptr;
    if( m_pCurTOXMark )
    {
        pNext = const_cast<SwTOXMark*>(&m_pSh->GotoTOXMark( *m_pCurTOXMark, TOX_NXT ));
        if( pNext == m_pCurTOXMark )
            pNext = nullptr;

        m_pSh->DeleteTOXMark( m_pCurTOXMark );
        m_pSh->SetModified();
    }
    // go to next one
    m_pCurTOXMark = pNext;
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
            pMark = new SwTOXMark(m_pSh->GetTOXType(TOX_CONTENT, 0));
            pMark->SetLevel( static_cast< sal_uInt16 >(rDesc.GetLevel()) );

            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
        }
        break;
        case  TOX_INDEX:
        {
            pMark = new SwTOXMark(m_pSh->GetTOXType(TOX_INDEX, 0));

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
            pMark = new SwTOXMark(m_pSh->GetTOXType(TOX_USER, nId));
            pMark->SetLevel( static_cast< sal_uInt16 >(rDesc.GetLevel()) );

            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
        }
        break;
        case  TOX_BIBLIOGRAPHY:
        {
            pMark = new SwTOXMark(m_pSh->GetTOXType(TOX_BIBLIOGRAPHY, 0));

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

    m_pSh->StartAllAction();
    m_pSh->SwEditShell::Insert(*pMark);
    m_pSh->EndAllAction();
}

// Update of TOXMarks
void SwTOXMgr::UpdateTOXMark(const SwTOXMarkDescription& rDesc)
{
    assert(m_pCurTOXMark && "no current TOXMark");
    m_pSh->StartAllAction();
    if(m_pCurTOXMark->GetTOXType()->GetType() == TOX_INDEX)
    {
        if(rDesc.GetPrimKey() && !rDesc.GetPrimKey()->isEmpty() )
        {
            m_pCurTOXMark->SetPrimaryKey( *rDesc.GetPrimKey() );
            if(rDesc.GetPhoneticReadingOfPrimKey())
                m_pCurTOXMark->SetPrimaryKeyReading( *rDesc.GetPhoneticReadingOfPrimKey() );
            else
                m_pCurTOXMark->SetPrimaryKeyReading(OUString());

            if( rDesc.GetSecKey() && !rDesc.GetSecKey()->isEmpty() )
            {
                m_pCurTOXMark->SetSecondaryKey( *rDesc.GetSecKey() );
                if(rDesc.GetPhoneticReadingOfSecKey())
                    m_pCurTOXMark->SetSecondaryKeyReading( *rDesc.GetPhoneticReadingOfSecKey() );
                else
                    m_pCurTOXMark->SetSecondaryKeyReading(OUString());
            }
            else
            {
                m_pCurTOXMark->SetSecondaryKey(OUString());
                m_pCurTOXMark->SetSecondaryKeyReading(OUString());
            }
        }
        else
        {
            m_pCurTOXMark->SetPrimaryKey(OUString());
            m_pCurTOXMark->SetPrimaryKeyReading(OUString());
            m_pCurTOXMark->SetSecondaryKey(OUString());
            m_pCurTOXMark->SetSecondaryKeyReading(OUString());
        }
        if(rDesc.GetPhoneticReadingOfAltStr())
            m_pCurTOXMark->SetTextReading( *rDesc.GetPhoneticReadingOfAltStr() );
        else
            m_pCurTOXMark->SetTextReading(OUString());
        m_pCurTOXMark->SetMainEntry(rDesc.IsMainEntry());
    }
    else
        m_pCurTOXMark->SetLevel( static_cast< sal_uInt16 >(rDesc.GetLevel()) );

    if(rDesc.GetAltStr())
    {
        // JP 26.08.96: Bug 30344 - either the text of a Doc or an alternative test,
        //                          not both!
        bool bReplace = m_pCurTOXMark->IsAlternativeText();
        if( bReplace )
            m_pCurTOXMark->SetAlternativeText( *rDesc.GetAltStr() );
        else
        {
            SwTOXMark aCpy( *m_pCurTOXMark );
            m_aCurMarks.clear();
            m_pSh->DeleteTOXMark(m_pCurTOXMark);
            aCpy.SetAlternativeText( *rDesc.GetAltStr() );
            m_pSh->SwEditShell::Insert( aCpy );
            m_pCurTOXMark = nullptr;
        }
    }
    m_pSh->SetModified();
    m_pSh->EndAllAction();
    // Bug 36207 pCurTOXMark points nowhere here!
    if(!m_pCurTOXMark)
    {
        m_pSh->Left(CRSR_SKIP_CHARS, false, 1, false );
        m_pSh->GetCurTOXMarks(m_aCurMarks);
        SetCurTOXMark(0);
    }
}

// determine UserTypeID
sal_uInt16 SwTOXMgr::GetUserTypeID(const OUString& rStr)
{
    sal_uInt16 nSize = m_pSh->GetTOXTypeCount(TOX_USER);
    for(sal_uInt16 i=0; i < nSize; ++i)
    {
        const SwTOXType* pTmp = m_pSh->GetTOXType(TOX_USER, i);
        if(pTmp && pTmp->GetTypeName() == rStr)
            return i;
    }
    SwTOXType aUserType(*m_pSh->GetDoc(), TOX_USER, rStr);
    m_pSh->InsertTOXType(aUserType);
    return nSize;
}

// traveling between TOXMarks
void SwTOXMgr::NextTOXMark(bool bSame)
{
    OSL_ENSURE(m_pCurTOXMark, "no current TOXMark");
    if( m_pCurTOXMark )
    {
        SwTOXSearch eDir = bSame ? TOX_SAME_NXT : TOX_NXT;
        m_pCurTOXMark = const_cast<SwTOXMark*>(&m_pSh->GotoTOXMark( *m_pCurTOXMark, eDir ));
    }
}

void SwTOXMgr::PrevTOXMark(bool bSame)
{
    OSL_ENSURE(m_pCurTOXMark, "no current TOXMark");
    if( m_pCurTOXMark )
    {
        SwTOXSearch eDir = bSame ? TOX_SAME_PRV : TOX_PRV;
        m_pCurTOXMark = const_cast<SwTOXMark*>(&m_pSh->GotoTOXMark(*m_pCurTOXMark, eDir ));
    }
}

const SwTOXType* SwTOXMgr::GetTOXType(TOXTypes eTyp) const
{
    return m_pSh->GetTOXType(eTyp, 0);
}

void SwTOXMgr::SetCurTOXMark(sal_uInt16 nId)
{
    m_pCurTOXMark = (nId < m_aCurMarks.size()) ? m_aCurMarks[nId] : nullptr;
}

bool SwTOXMgr::UpdateOrInsertTOX(const SwTOXDescription& rDesc,
                                    SwTOXBase** ppBase,
                                    const SfxItemSet* pSet)
{
    SwWait aWait( *m_pSh->GetView().GetDocShell(), true );
    bool bRet = true;
    const SwTOXBase *const pCurTOX = ppBase && *ppBase ? *ppBase : m_pSh->GetCurTOX();

    SwTOXBase * pNewTOX = pCurTOX ? new SwTOXBase(*pCurTOX) : nullptr;

    TOXTypes eCurTOXType = rDesc.GetTOXType();
    if(pCurTOX && !ppBase && m_pSh->HasSelection())
        m_pSh->EnterStdMode();

    switch(eCurTOXType)
    {
        case TOX_INDEX :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = m_pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(pType, aForm, SwTOXElement::Mark, pType->GetTypeName());
            }
            pNewTOX->SetOptions(rDesc.GetIndexOptions());
            pNewTOX->SetMainEntryCharStyle(rDesc.GetMainEntryCharStyle());
            m_pSh->SetTOIAutoMarkURL(rDesc.GetAutoMarkURL());
            m_pSh->ApplyAutoMark();
        }
        break;
        case TOX_CONTENT :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = m_pSh->GetTOXType(eCurTOXType, 0);
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
                sal_uInt16 nSize = m_pSh->GetTOXTypeCount(eCurTOXType);
                for(sal_uInt16 i=0; rDesc.GetTOUName() && i < nSize; ++i)
                {   const SwTOXType* pType = m_pSh->GetTOXType(TOX_USER, i);
                    if(pType->GetTypeName() == *rDesc.GetTOUName())
                    {   nPos = i;
                        break;
                    }
                }
                const SwTOXType* pType = m_pSh->GetTOXType(eCurTOXType, nPos);

                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(pType, aForm, rDesc.GetContentOptions(), pType->GetTypeName());

            }
            else
            {
                pNewTOX->SetCreate(rDesc.GetContentOptions());
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
                                                m_pSh->GetFieldType(SwFieldIds::TableOfAuthorities, OUString()));
                if (!pFType)
                {
                    SwAuthorityFieldType const type(m_pSh->GetDoc());
                    pFType = static_cast<SwAuthorityFieldType*>(
                                m_pSh->InsertFieldType(type));
                }
                OUString const& rBrackets(rDesc.GetAuthBrackets());
                if (rBrackets.isEmpty())
                {
                    pFType->SetPreSuffix('\0', '\0');
                }
                else
                {
                    assert(rBrackets.getLength() == 2);
                    pFType->SetPreSuffix(rBrackets[0], rBrackets[1]);
                }
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
                const SwTOXType* pType = m_pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pNewTOX = new SwTOXBase(
                    pType, aForm,
                    TOX_AUTHORITIES == eCurTOXType ? SwTOXElement::Mark : SwTOXElement::NONE,
                    pType->GetTypeName());
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
            m_pSh->InsertTableOf(*pNewTOX, pSet);
            delete pNewTOX;
        }
    }
    else
    {
        SwDoc * pDoc = m_pSh->GetDoc();

        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::TOXCHANGE, nullptr);
        }

        SwTOXBase *const pTOX = const_cast<SwTOXBase*>(pCurTOX);
        pDoc->ChangeTOX(*pTOX, *pNewTOX);

        pTOX->DisableKeepExpression();
        m_pSh->UpdateTableOf(*pTOX, pSet);
        bRet = false;
        pTOX->EnableKeepExpression();

        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::TOXCHANGE, nullptr);
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

    m_eSortKey1 = aArr[0];
    m_eSortKey2 = aArr[1];
    m_eSortKey3 = aArr[2];
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
    rTOXBase.SetLanguage(m_eLanguage);
    rTOXBase.SetSortAlgorithm(m_sSortAlgorithm);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
