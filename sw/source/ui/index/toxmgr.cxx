/*************************************************************************
 *
 *  $RCSfile: toxmgr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:33:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _SWVIEW_HXX
#include <view.hxx>
#endif
#ifndef _TOXMGR_HXX
#include <toxmgr.hxx>
#endif
#ifndef _AUTHFLD_HXX
#include <authfld.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

/*-----------------23.01.98 07:41-------------------
    Standard - Verzeichnisnamen herausreichen
--------------------------------------------------*/
const String&   SwTOXBase::GetTOXName(TOXTypes eType)
{
    const String* pRet;
    ShellResource* pShellRes = ViewShell::GetShellRes();
    switch(eType)
    {
        case  TOX_CONTENT      :    pRet = &pShellRes->aTOXContentName;       break;
        case  TOX_INDEX        :    pRet = &pShellRes->aTOXIndexName;         break;
        case  TOX_USER         :    pRet = &pShellRes->aTOXUserName;          break;
        case  TOX_ILLUSTRATIONS:    pRet = &pShellRes->aTOXIllustrationsName; break;
        case  TOX_OBJECTS      :    pRet = &pShellRes->aTOXObjectsName;       break;
        case  TOX_TABLES       :    pRet = &pShellRes->aTOXTablesName;        break;
        case  TOX_AUTHORITIES :     pRet = &pShellRes->aTOXAuthoritiesName;   break;
    }
    return *pRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Handhabung der Verzeichnisse durch TOXMgr
 --------------------------------------------------------------------*/


SwTOXMgr::SwTOXMgr(SwWrtShell* pShell):
    pSh(pShell)
{
    GetTOXMarks();
    SetCurTOXMark(0);
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuelle TOXMarks behandeln
 --------------------------------------------------------------------*/


USHORT SwTOXMgr::GetTOXMarks()
{
    return pSh->GetCurTOXMarks(aCurMarks);
}


SwTOXMark* SwTOXMgr::GetTOXMark(USHORT nId)
{
    if(aCurMarks.Count() > 0)
        return aCurMarks[nId];
    return 0;
}


void SwTOXMgr::DeleteTOXMark()
{
    SwTOXMark* pNext = 0;
    if( pCurTOXMark )
    {
        pNext = (SwTOXMark*)&pSh->GotoTOXMark( *pCurTOXMark, TOX_NXT );
        if( pNext == pCurTOXMark )
            pNext = 0;

        pSh->DeleteTOXMark( pCurTOXMark );
        pSh->SetModified();
    }
    // zur naechsten wandern
    pCurTOXMark = pNext;
}
/* -----------------20.08.99 10:48-------------------

 --------------------------------------------------*/
void    SwTOXMgr::InsertTOXMark(const SwTOXMarkDescription& rDesc)
{
    SwTOXMark* pMark = 0;
    switch(rDesc.GetTOXType())
    {
        case  TOX_CONTENT:
        {
            ASSERT(rDesc.GetLevel() > 0 && rDesc.GetLevel() <= MAXLEVEL,
                                            ungueltiger Level InsertTOCMark);
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_CONTENT, 0));
            pMark->SetLevel(rDesc.GetLevel());

            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
        }
        break;
        case  TOX_INDEX:
        {
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_INDEX, 0));

            if( rDesc.GetPrimKey() && rDesc.GetPrimKey()->Len() )
            {
                pMark->SetPrimaryKey( *rDesc.GetPrimKey() );
                if(rDesc.GetPhoneticReadingOfPrimKey())
                    pMark->SetPrimaryKeyReading( *rDesc.GetPhoneticReadingOfPrimKey() );

                if( rDesc.GetSecKey() && rDesc.GetSecKey()->Len() )
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
            ASSERT(rDesc.GetLevel() > 0 && rDesc.GetLevel() <= MAXLEVEL,
                                            ungueltiger Level InsertTOCMark);
            USHORT nId = rDesc.GetTOUName() ?
                GetUserTypeID(*rDesc.GetTOUName()) : 0;
            pMark = new SwTOXMark(pSh->GetTOXType(TOX_USER, nId));
            pMark->SetLevel(rDesc.GetLevel());

            if(rDesc.GetAltStr())
                pMark->SetAlternativeText(*rDesc.GetAltStr());
        }
        break;
    }
    pSh->StartAllAction();
    pSh->SwEditShell::Insert(*pMark);
    pSh->EndAllAction();
}
/*--------------------------------------------------------------------
    Beschreibung: Update eines TOXMarks
 --------------------------------------------------------------------*/


void SwTOXMgr::UpdateTOXMark(const SwTOXMarkDescription& rDesc)
{
    ASSERT(pCurTOXMark, "kein aktuelles TOXMark");

    pSh->StartAllAction();
    if(pCurTOXMark->GetTOXType()->GetType() == TOX_INDEX)
    {
        if(rDesc.GetPrimKey() && rDesc.GetPrimKey()->Len() )
        {
            pCurTOXMark->SetPrimaryKey( *rDesc.GetPrimKey() );
            if(rDesc.GetPhoneticReadingOfPrimKey())
                pCurTOXMark->SetPrimaryKeyReading( *rDesc.GetPhoneticReadingOfPrimKey() );
            else
                pCurTOXMark->SetPrimaryKeyReading( aEmptyStr );

            if( rDesc.GetSecKey() && rDesc.GetSecKey()->Len() )
            {
                pCurTOXMark->SetSecondaryKey( *rDesc.GetSecKey() );
                if(rDesc.GetPhoneticReadingOfSecKey())
                    pCurTOXMark->SetSecondaryKeyReading( *rDesc.GetPhoneticReadingOfSecKey() );
                else
                    pCurTOXMark->SetSecondaryKeyReading( aEmptyStr );
            }
            else
            {
                pCurTOXMark->SetSecondaryKey( aEmptyStr );
                pCurTOXMark->SetSecondaryKeyReading( aEmptyStr );
            }
        }
        else
        {
            pCurTOXMark->SetPrimaryKey( aEmptyStr );
            pCurTOXMark->SetPrimaryKeyReading( aEmptyStr );
            pCurTOXMark->SetSecondaryKey( aEmptyStr );
            pCurTOXMark->SetSecondaryKeyReading( aEmptyStr );
        }
        if(rDesc.GetPhoneticReadingOfAltStr())
            pCurTOXMark->SetTextReading( *rDesc.GetPhoneticReadingOfAltStr() );
        else
            pCurTOXMark->SetTextReading( aEmptyStr );
        pCurTOXMark->SetMainEntry(rDesc.IsMainEntry());
    }
    else
        pCurTOXMark->SetLevel(rDesc.GetLevel());

    if(rDesc.GetAltStr())
    {
        // JP 26.08.96: Bug 30344 - entweder der Text aus dem Doc oder
        //                          ein Alternativ-Text, beides gibts nicht!
        BOOL bReplace = pCurTOXMark->IsAlternativeText();
        if( bReplace )
            pCurTOXMark->SetAlternativeText( *rDesc.GetAltStr() );
        else
        {
            SwTOXMark aCpy( *pCurTOXMark );
            aCurMarks.Remove(0, aCurMarks.Count());
            pSh->DeleteTOXMark(pCurTOXMark);
            aCpy.SetAlternativeText( *rDesc.GetAltStr() );
            pSh->SwEditShell::Insert( aCpy );
            pCurTOXMark = 0;
        }
    }
    pSh->SetModified();
    pSh->EndAllAction();
    // Bug 36207 pCurTOXMark zeigt hier in den Wald!
    if(!pCurTOXMark)
    {
        pSh->Left(CRSR_SKIP_CHARS, FALSE, 1, FALSE );
        pSh->GetCurTOXMarks(aCurMarks);
        SetCurTOXMark(0);
    }
}


/*--------------------------------------------------------------------
    Beschreibung:   UserTypeID ermitteln
 --------------------------------------------------------------------*/


USHORT SwTOXMgr::GetUserTypeID(const String& rStr)
{
    USHORT nSize = pSh->GetTOXTypeCount(TOX_USER);
    for(USHORT i=0; i < nSize; ++i)
    {
        const SwTOXType* pTmp = pSh->GetTOXType(TOX_USER, i);
        if(pTmp && pTmp->GetTypeName() == rStr)
            return i;
    }
    SwTOXType aUserType(TOX_USER, rStr);
    pSh->InsertTOXType(aUserType);
    return nSize;
}

/*--------------------------------------------------------------------
    Beschreibung: Traveling zwischen den TOXMarks
 --------------------------------------------------------------------*/


void SwTOXMgr::NextTOXMark(BOOL bSame)
{
    ASSERT(pCurTOXMark, "kein aktuelles TOXMark");
    if( pCurTOXMark )
    {
        SwTOXSearch eDir = bSame ? TOX_SAME_NXT : TOX_NXT;
        pCurTOXMark = (SwTOXMark*)&pSh->GotoTOXMark( *pCurTOXMark, eDir );
    }
}


void SwTOXMgr::PrevTOXMark(BOOL bSame)
{
    ASSERT(pCurTOXMark, "kein aktuelles TOXMark");
    if( pCurTOXMark )
    {
        SwTOXSearch eDir = bSame ? TOX_SAME_PRV : TOX_PRV;
        pCurTOXMark = (SwTOXMark*)&pSh->GotoTOXMark(*pCurTOXMark, eDir );
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Stichwortverzeichnis einfuegen
 --------------------------------------------------------------------*/
const SwTOXBase* SwTOXMgr::GetCurTOX()
{
    return pSh->GetCurTOX();
}

const SwTOXType* SwTOXMgr::GetTOXType(TOXTypes eTyp, USHORT nId) const
{
    return pSh->GetTOXType(eTyp, nId);
}

void SwTOXMgr::SetCurTOXMark(USHORT nId)
{
    pCurTOXMark = (nId < aCurMarks.Count()) ? aCurMarks[nId] : 0;
}

/* -----------------01.07.99 16:23-------------------

 --------------------------------------------------*/

BOOL SwTOXMgr::UpdateOrInsertTOX(const SwTOXDescription& rDesc,
                                    SwTOXBase** ppBase,
                                    const SfxItemSet* pSet)
{
    SwWait aWait( *pSh->GetView().GetDocShell(), TRUE );
    BOOL bRet = TRUE;
    const SwTOXBase* pCurTOX = ppBase && *ppBase ? *ppBase : GetCurTOX();
    SwTOXBase* pTOX = (SwTOXBase*)pCurTOX;
    TOXTypes eCurTOXType = rDesc.GetTOXType();
    if(pCurTOX && !ppBase && pSh->HasSelection())
        pSh->DelRight();

    switch(eCurTOXType)
    {
        case TOX_INDEX :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pTOX = new SwTOXBase(pType, aForm, TOX_MARK, pType->GetTypeName());
            }
            pTOX->SetOptions(rDesc.GetIndexOptions());
            pTOX->SetMainEntryCharStyle(rDesc.GetMainEntryCharStyle());
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
                pTOX = new SwTOXBase(pType, aForm, rDesc.GetContentOptions(), pType->GetTypeName());
            }
            pTOX->SetCreate(rDesc.GetContentOptions());
            pTOX->SetLevel(rDesc.GetLevel());
        }
        break;
        case TOX_USER :
        {
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                USHORT nPos  = 0;
                USHORT nSize = pSh->GetTOXTypeCount(eCurTOXType);
                for(USHORT i=0; rDesc.GetTOUName() && i < nSize; ++i)
                {   const SwTOXType* pType = pSh->GetTOXType(TOX_USER, i);
                    if(pType->GetTypeName() == *rDesc.GetTOUName())
                    {   nPos = i;
                        break;
                    }
                }
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, nPos);

                SwForm aForm(eCurTOXType);
                pTOX = new SwTOXBase(pType, aForm, rDesc.GetContentOptions(), pType->GetTypeName());

            }
            else
            {
                SwTOXBase* pTOX = (SwTOXBase*)pCurTOX;
                pTOX->SetCreate(rDesc.GetContentOptions());
            }
            pTOX->SetLevelFromChapter(rDesc.IsLevelFromChapter());
        }
        break;
        case TOX_OBJECTS:
        case TOX_TABLES:
        case TOX_AUTHORITIES:
        case TOX_ILLUSTRATIONS:
        {
            //Special handling for TOX_AUTHORITY
            if(TOX_AUTHORITIES == eCurTOXType)
            {
                SwAuthorityFieldType* pFType = (SwAuthorityFieldType*)
                                                pSh->GetFldType(RES_AUTHORITY, aEmptyStr);
                if(!pFType)
                    pFType = (SwAuthorityFieldType*)pSh->InsertFldType(
                                    SwAuthorityFieldType(pSh->GetDoc()));
                pFType->SetPreSuffix(rDesc.GetAuthBrackets().GetChar(0),
                    rDesc.GetAuthBrackets().GetChar(1));
                pFType->SetSequence(rDesc.IsAuthSequence());
                SwTOXSortKey rArr[3];
                rArr[0] = rDesc.GetSortKey1();
                rArr[1] = rDesc.GetSortKey2();
                rArr[2] = rDesc.GetSortKey3();
                pFType->SetSortKeys(3, rArr);
                pFType->SetSortByDocument(rDesc.IsSortByDocument());
                pFType->SetLanguage(rDesc.GetLanguage());
                pFType->SetSortAlgorithm(rDesc.GetSortAlgorithm());

                pFType->UpdateFlds();
            }
            // TODO: consider properties of the current TOXType
            if(!pCurTOX || (ppBase && !(*ppBase)))
            {
                const SwTOXType* pType = pSh->GetTOXType(eCurTOXType, 0);
                SwForm aForm(eCurTOXType);
                pTOX = new SwTOXBase(
                    pType, aForm,
                    TOX_AUTHORITIES == eCurTOXType ? TOX_MARK : 0, pType->GetTypeName());
            }
            else
            {
                if((!ppBase || !(*ppBase)) && pSh->HasSelection())
                    pSh->DelRight();
                pTOX = (SwTOXBase*)pCurTOX;
            }
//          pTOX->SetOptions(rDesc.GetIndexOptions());
            pTOX->SetFromObjectNames(rDesc.IsCreateFromObjectNames());
            pTOX->SetOLEOptions(rDesc.GetOLEOptions());
        }
        break;
    }


    DBG_ASSERT(pTOX, "no TOXBase created!" )
    if(!pTOX)
        return FALSE;
    pTOX->SetFromChapter(rDesc.IsFromChapter());
    pTOX->SetSequenceName(rDesc.GetSequenceName());
    pTOX->SetCaptionDisplay(rDesc.GetCaptionDisplay());
    pTOX->SetProtected(rDesc.IsReadonly());

    for(USHORT nLevel = 0; nLevel < MAXLEVEL; nLevel++)
        pTOX->SetStyleNames(rDesc.GetStyleNames(nLevel), nLevel);

    if(rDesc.GetTitle())
        pTOX->SetTitle(*rDesc.GetTitle());
    if(rDesc.GetForm())
        pTOX->SetTOXForm(*rDesc.GetForm());
    pTOX->SetLanguage(rDesc.GetLanguage());
    pTOX->SetSortAlgorithm(rDesc.GetSortAlgorithm());

    if(!pCurTOX || (ppBase && !(*ppBase)) )
    {
        // wird ppBase uebergeben, dann wird das TOXBase hier nur erzeugt
        // und dann ueber den Dialog in ein Globaldokument eingefuegt
        if(ppBase)
            (*ppBase) = pTOX;
        else
        {
            pSh->InsertTableOf(*pTOX, pSet);
            delete pTOX;
        }
    }
    else
        bRet = pSh->UpdateTableOf(*pTOX, pSet);

    return bRet;
}
/* -----------------20.10.99 14:11-------------------

 --------------------------------------------------*/
void SwTOXDescription::SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3)
{
    SwTOXSortKey aArr[3];
    USHORT nPos = 0;
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

/* -----------------10.09.99 10:02-------------------

 --------------------------------------------------*/
void SwTOXDescription::ApplyTo(SwTOXBase& rTOXBase)
{
    for(USHORT i = 0; i < MAXLEVEL; i++)
        rTOXBase.SetStyleNames(GetStyleNames(i), i);
    rTOXBase.SetTitle(GetTitle() ? *GetTitle() : aEmptyStr);
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



