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

#ifndef _TOXMGR_HXX
#define _TOXMGR_HXX

#include "swdllapi.h"
#include "tox.hxx"
#include <authfld.hxx>

class SwWrtShell;
class SwForm;

/*--------------------------------------------------------------------
    Beschreibung: Manager fuer Verzeichnisfunktionalitaet
 --------------------------------------------------------------------*/
//one single method will be sufficient to insert AND upate indexes
class SW_DLLPUBLIC SwTOXDescription
{
    TOXTypes            eTOXType;
    String              aStyleNames[MAXLEVEL];
    String              sSequenceName;
    String              sMainEntryCharStyle;
    String              sAutoMarkURL;
    String*             pTitle;
    String*             pTOUName;
    SwForm*             pForm;
    USHORT              nContent;
    USHORT              nIndexOptions;
    USHORT              nOLEOptions;
    LanguageType        eLanguage;
    String              sSortAlgorithm;

    String              sAuthBrackets;
    SwCaptionDisplay    eCaptionDisplay;
    SwTOXSortKey        eSortKey1;
    SwTOXSortKey        eSortKey2;
    SwTOXSortKey        eSortKey3;
    BYTE                nLevel;
    BOOL                bFromObjectNames : 1;
    BOOL                bFromChapter : 1;
    BOOL                bReadonly: 1;
    BOOL                bLevelFromChapter : 1;
    BOOL                bIsAuthSequence :1;
    BOOL                bSortByDocument :1;

    //TODO: TemplateNames
    //const String* pTemplateName = 0, ???

    // forbidden and not implemented.
    SwTOXDescription();
    SwTOXDescription(SwTOXDescription&);
    SwTOXDescription & operator= (SwTOXDescription&);

public:
    // single argument ctors shall be explicit.
    explicit SwTOXDescription(TOXTypes eType) :
        eTOXType(eType),
        pTitle(0),
        pTOUName(0),
        pForm(0),
        nContent(nsSwTOXElement::TOX_MARK | nsSwTOXElement::TOX_OUTLINELEVEL),
        nIndexOptions(nsSwTOIOptions::TOI_SAME_ENTRY|nsSwTOIOptions::TOI_FF|nsSwTOIOptions::TOI_CASE_SENSITIVE),
        nOLEOptions(0),
        eLanguage((LanguageType)::GetAppLanguage()),
        eCaptionDisplay(CAPTION_COMPLETE),
        nLevel(MAXLEVEL),
        bFromObjectNames(FALSE),
        bFromChapter(FALSE),
        bReadonly(TRUE),
        bLevelFromChapter(FALSE),
        bIsAuthSequence(FALSE),
        bSortByDocument(TRUE)
        {}
    ~SwTOXDescription()
        {
            delete pTitle;
            delete pForm;
            delete pTOUName;
        }

    void            SetTOXType(TOXTypes eSet) { eTOXType = eSet;}
    TOXTypes        GetTOXType() const { return eTOXType;}

    const String&   GetStyleNames(USHORT nLvl) const
                                {return aStyleNames[nLvl];}
    void            SetStyleNames(const String& rSet, USHORT nLvl)
                                {aStyleNames[nLvl] = rSet; }

    const String&   GetAutoMarkURL() const { return sAutoMarkURL;}
    void            SetAutoMarkURL(const String& rSet) {sAutoMarkURL = rSet;}

    void            SetTitle(const String& pSet) {delete pTitle; pTitle = new String(pSet);}
    const String*   GetTitle() const {return pTitle; }

    void            SetTOUName(const String& pSet) {delete pTOUName; pTOUName = new String(pSet);}
    const String*   GetTOUName() const {return pTOUName; }

    void            SetForm(const SwForm& rSet) {delete pForm; pForm = new SwForm(rSet);}
    const SwForm*   GetForm() const {return pForm;}

    void            SetContentOptions(USHORT nSet) { nContent = nSet;}
    USHORT          GetContentOptions() const { return nContent;}

    void            SetIndexOptions(USHORT nSet) { nIndexOptions = nSet;}
    USHORT          GetIndexOptions() const { return nIndexOptions;}

    const String&   GetMainEntryCharStyle() const {return sMainEntryCharStyle;}
    void            SetMainEntryCharStyle(const String& rSet)  {sMainEntryCharStyle = rSet;}

    void            SetLevel(BYTE nSet) {nLevel = nSet;}
    BYTE            GetLevel()const  {return nLevel; }

    void            SetCreateFromObjectNames(BOOL bSet) { bFromObjectNames = bSet;}
    BOOL            IsCreateFromObjectNames() const {return bFromObjectNames;}

    const String&   GetSequenceName() const {return sSequenceName;}
    void            SetSequenceName(const String& rSet) {sSequenceName = rSet;}

    SwCaptionDisplay    GetCaptionDisplay() const { return eCaptionDisplay;}
    void                SetCaptionDisplay(SwCaptionDisplay eSet) {eCaptionDisplay = eSet;}

    void            SetFromChapter(BOOL bSet) { bFromChapter = bSet;}
    BOOL            IsFromChapter() const {return bFromChapter;}

    void            SetReadonly(BOOL bSet){bReadonly = bSet;}
    BOOL            IsReadonly() const {return bReadonly;}

    USHORT          GetOLEOptions() const {return nOLEOptions;}
    void            SetOLEOptions(USHORT nOpt) {nOLEOptions = nOpt;}

    BOOL            IsLevelFromChapter() const {return bLevelFromChapter;}
    void            SetLevelFromChapter(BOOL bSet) {bLevelFromChapter = bSet;}

    String          GetAuthBrackets() const {return sAuthBrackets;}
    void            SetAuthBrackets(const String& rSet) {sAuthBrackets = rSet;}

    BOOL            IsAuthSequence() const {return bIsAuthSequence;}
    void            SetAuthSequence(BOOL bSet){bIsAuthSequence = bSet;}

    BOOL            IsSortByDocument()const {return bSortByDocument ;}
    void            SetSortByDocument(BOOL bSet) {bSortByDocument = bSet;}

    void SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3);

    SwTOXSortKey GetSortKey1() const {return eSortKey1;}
    SwTOXSortKey GetSortKey2() const {return eSortKey2;}
    SwTOXSortKey GetSortKey3() const {return eSortKey3;}

    LanguageType    GetLanguage() const {return eLanguage;}
    void            SetLanguage(LanguageType nLang)  {eLanguage = nLang;}

    const String&   GetSortAlgorithm()const {return sSortAlgorithm;}
    void            SetSortAlgorithm(const String& rSet) {sSortAlgorithm = rSet;}

    void            ApplyTo(SwTOXBase& rTOXBase);

};

class SwTOXMarkDescription
{
    TOXTypes    eTOXType;
    int         nLevel;
    BOOL        bMainEntry;

    String*     pPrimKey;
    String*     pSecKey;
    String*     pAltStr;
    String*     pTOUName;

    String*     pPhoneticReadingOfAltStr;
    String*     pPhoneticReadingOfPrimKey;
    String*     pPhoneticReadingOfSecKey;

    // forbidden and not implemented.
    SwTOXMarkDescription();
    SwTOXMarkDescription(SwTOXMarkDescription&);
    SwTOXMarkDescription & operator= (SwTOXMarkDescription&);

public:
    // single argument ctors shall be explicit.
    explicit SwTOXMarkDescription(TOXTypes eType) :
        eTOXType(eType),
        nLevel(0),
        bMainEntry(FALSE),
        pPrimKey(0),
        pSecKey(0),
        pAltStr(0),
        pTOUName(0),
        pPhoneticReadingOfAltStr(0),
        pPhoneticReadingOfPrimKey(0),
        pPhoneticReadingOfSecKey(0)
        {
        }
    ~SwTOXMarkDescription()
    {
        delete pPrimKey;
        delete pSecKey;
        delete pAltStr;
        delete pTOUName;
        delete pPhoneticReadingOfAltStr;
        delete pPhoneticReadingOfPrimKey;
        delete pPhoneticReadingOfSecKey;
    }

    TOXTypes        GetTOXType()const {return eTOXType;}

    void            SetLevel(int nSet) {nLevel = nSet;}
    int             GetLevel() const {return nLevel;}

    void            SetMainEntry(BOOL bSet) {bMainEntry = bSet;}
    BOOL            IsMainEntry() const {return bMainEntry;}

    void            SetPrimKey(const String& rSet)
                                {delete pPrimKey; pPrimKey = new String(rSet);}
    const String*   GetPrimKey() const {return pPrimKey;}

    void            SetSecKey(const String& rSet)
                                {delete pSecKey;  pSecKey  = new String(rSet);}
    const String*   GetSecKey() const { return pSecKey; }

    void            SetAltStr(const String& rSet)
                                {delete pAltStr;  pAltStr  = new String(rSet);}
    const String*   GetAltStr() const { return pAltStr; }

    void            SetTOUName(const String& rSet)
                                {delete pTOUName; pTOUName = new String(rSet);}
    const String*   GetTOUName() const {return pTOUName;}


    void            SetPhoneticReadingOfAltStr(const String& rSet)
                                {delete pPhoneticReadingOfAltStr;  pPhoneticReadingOfAltStr  = new String(rSet);}
    const String*   GetPhoneticReadingOfAltStr() const {    return pPhoneticReadingOfAltStr; }

    void            SetPhoneticReadingOfPrimKey(const String& rSet)
                                {delete pPhoneticReadingOfPrimKey;  pPhoneticReadingOfPrimKey  = new String(rSet);}
    const String*   GetPhoneticReadingOfPrimKey() const {   return pPhoneticReadingOfPrimKey; }

    void            SetPhoneticReadingOfSecKey(const String& rSet)
                                {delete pPhoneticReadingOfSecKey;  pPhoneticReadingOfSecKey  = new String(rSet);}
    const String*   GetPhoneticReadingOfSecKey() const {    return pPhoneticReadingOfSecKey; }
};

class SW_DLLPUBLIC SwTOXMgr
{
    SwWrtShell*         pSh;
    SwTOXMark*          pCurTOXMark;
    SwTOXMarks          aCurMarks;

    SW_DLLPRIVATE USHORT                GetUserTypeID(const String& rStr);

public:
    // single argument ctors shall be explicit.
    explicit SwTOXMgr(SwWrtShell* pShell);

    //
    // Methoden fuer Verzeichnismarkierungen
    //
    void    InsertTOXMark(const SwTOXMarkDescription& rDesc);

    void    UpdateTOXMark(const SwTOXMarkDescription& rDesc);


    void                DeleteTOXMark();
    void                NextTOXMark(BOOL bSame=FALSE);
    void                PrevTOXMark(BOOL bSame=FALSE);

    // Aktuelle TOXmarks holen
    USHORT              GetTOXMarks();
    USHORT              GetTOXMarkCount();
    SwTOXMark*          GetTOXMark(USHORT nId);
    SwTOXMark*          GetCurTOXMark();
    void                SetCurTOXMark(USHORT nId);

    //
    // Methoden fuer Verzeichnisse
    //
    BOOL    UpdateOrInsertTOX(const SwTOXDescription& rDesc, SwTOXBase** ppBase = 0, const SfxItemSet* pSet = 0);

    const SwTOXType*    GetTOXType(TOXTypes eTyp, USHORT nId) const;
    const SwTOXBase*    GetCurTOX();

};

/*--------------------------------------------------------------------
    Beschreibung: Inlines
 --------------------------------------------------------------------*/
inline USHORT SwTOXMgr::GetTOXMarkCount()
    {   return aCurMarks.Count();   }

inline SwTOXMark* SwTOXMgr::GetCurTOXMark()
    {   return pCurTOXMark; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
