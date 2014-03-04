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

#ifndef INCLUDED_SW_SOURCE_UI_INC_TOXMGR_HXX
#define INCLUDED_SW_SOURCE_UI_INC_TOXMGR_HXX

#include "swdllapi.h"
#include "tox.hxx"
#include <authfld.hxx>

class SwWrtShell;
class SwForm;

/*--------------------------------------------------------------------
    Description: manager for directory functionality
 --------------------------------------------------------------------*/
//one single method will be sufficient to insert AND upate indexes
class SW_DLLPUBLIC SwTOXDescription
{
    TOXTypes            eTOXType;
    OUString            aStyleNames[MAXLEVEL];
    OUString            sSequenceName;
    OUString            sMainEntryCharStyle;
    OUString            sAutoMarkURL;
    OUString*           pTitle;
    OUString*           pTOUName;
    SwForm*             pForm;
    sal_uInt16          nContent;
    sal_uInt16          nIndexOptions;
    sal_uInt16          nOLEOptions;
    LanguageType        eLanguage;
    OUString            sSortAlgorithm;

    OUString            sAuthBrackets;
    SwCaptionDisplay    eCaptionDisplay;
    SwTOXSortKey        eSortKey1;
    SwTOXSortKey        eSortKey2;
    SwTOXSortKey        eSortKey3;
    sal_uInt8           nLevel;
    sal_Bool                bFromObjectNames : 1;
    sal_Bool                bFromChapter : 1;
    sal_Bool                bReadonly: 1;
    sal_Bool                bLevelFromChapter : 1;
    sal_Bool                bIsAuthSequence :1;
    sal_Bool                bSortByDocument :1;

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
        bFromObjectNames(sal_False),
        bFromChapter(sal_False),
        bReadonly(sal_True),
        bLevelFromChapter(sal_False),
        bIsAuthSequence(sal_False),
        bSortByDocument(sal_True)
        {}
    ~SwTOXDescription()
        {
            delete pTitle;
            delete pForm;
            delete pTOUName;
        }

    void            SetTOXType(TOXTypes eSet) { eTOXType = eSet;}
    TOXTypes        GetTOXType() const { return eTOXType;}

    const OUString& GetStyleNames(sal_uInt16 nLvl) const
                                {return aStyleNames[nLvl];}
    void            SetStyleNames(const OUString& rSet, sal_uInt16 nLvl)
                                {aStyleNames[nLvl] = rSet; }

    const OUString& GetAutoMarkURL() const { return sAutoMarkURL;}
    void            SetAutoMarkURL(const OUString& rSet) {sAutoMarkURL = rSet;}

    void            SetTitle(const OUString& pSet) {delete pTitle; pTitle = new OUString(pSet);}
    const OUString* GetTitle() const {return pTitle; }

    void            SetTOUName(const OUString& pSet) {delete pTOUName; pTOUName = new OUString(pSet);}
    const OUString* GetTOUName() const {return pTOUName; }

    void            SetForm(const SwForm& rSet) {delete pForm; pForm = new SwForm(rSet);}
    const SwForm*   GetForm() const {return pForm;}

    void            SetContentOptions(sal_uInt16 nSet) { nContent = nSet;}
    sal_uInt16          GetContentOptions() const { return nContent;}

    void            SetIndexOptions(sal_uInt16 nSet) { nIndexOptions = nSet;}
    sal_uInt16          GetIndexOptions() const { return nIndexOptions;}

    const OUString& GetMainEntryCharStyle() const {return sMainEntryCharStyle;}
    void            SetMainEntryCharStyle(const OUString& rSet)  {sMainEntryCharStyle = rSet;}

    void            SetLevel(sal_uInt8 nSet) {nLevel = nSet;}
    sal_uInt8           GetLevel()const  {return nLevel; }

    void            SetCreateFromObjectNames(sal_Bool bSet) { bFromObjectNames = bSet;}
    sal_Bool            IsCreateFromObjectNames() const {return bFromObjectNames;}

    const OUString& GetSequenceName() const {return sSequenceName;}
    void            SetSequenceName(const OUString& rSet) {sSequenceName = rSet;}

    SwCaptionDisplay    GetCaptionDisplay() const { return eCaptionDisplay;}
    void                SetCaptionDisplay(SwCaptionDisplay eSet) {eCaptionDisplay = eSet;}

    void            SetFromChapter(sal_Bool bSet) { bFromChapter = bSet;}
    sal_Bool            IsFromChapter() const {return bFromChapter;}

    void            SetReadonly(sal_Bool bSet){bReadonly = bSet;}
    sal_Bool            IsReadonly() const {return bReadonly;}

    sal_uInt16          GetOLEOptions() const {return nOLEOptions;}
    void            SetOLEOptions(sal_uInt16 nOpt) {nOLEOptions = nOpt;}

    sal_Bool            IsLevelFromChapter() const {return bLevelFromChapter;}
    void            SetLevelFromChapter(sal_Bool bSet) {bLevelFromChapter = bSet;}

    OUString        GetAuthBrackets() const {return sAuthBrackets;}
    void            SetAuthBrackets(const OUString& rSet) {sAuthBrackets = rSet;}

    sal_Bool            IsAuthSequence() const {return bIsAuthSequence;}
    void            SetAuthSequence(sal_Bool bSet){bIsAuthSequence = bSet;}

    sal_Bool            IsSortByDocument()const {return bSortByDocument ;}
    void            SetSortByDocument(sal_Bool bSet) {bSortByDocument = bSet;}

    void SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3);

    SwTOXSortKey GetSortKey1() const {return eSortKey1;}
    SwTOXSortKey GetSortKey2() const {return eSortKey2;}
    SwTOXSortKey GetSortKey3() const {return eSortKey3;}

    LanguageType    GetLanguage() const {return eLanguage;}
    void            SetLanguage(LanguageType nLang)  {eLanguage = nLang;}

    const OUString& GetSortAlgorithm()const {return sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {sSortAlgorithm = rSet;}

    void            ApplyTo(SwTOXBase& rTOXBase);

};

class SwTOXMarkDescription
{
    TOXTypes    eTOXType;
    int         nLevel;
    sal_Bool        bMainEntry;

    OUString*   pPrimKey;
    OUString*   pSecKey;
    OUString*   pAltStr;
    OUString*   pTOUName;

    OUString*   pPhoneticReadingOfAltStr;
    OUString*   pPhoneticReadingOfPrimKey;
    OUString*   pPhoneticReadingOfSecKey;

    // forbidden and not implemented.
    SwTOXMarkDescription();
    SwTOXMarkDescription(SwTOXMarkDescription&);
    SwTOXMarkDescription & operator= (SwTOXMarkDescription&);

public:
    // single argument ctors shall be explicit.
    explicit SwTOXMarkDescription(TOXTypes eType) :
        eTOXType(eType),
        nLevel(0),
        bMainEntry(sal_False),
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

    void            SetMainEntry(sal_Bool bSet) {bMainEntry = bSet;}
    sal_Bool            IsMainEntry() const {return bMainEntry;}

    void            SetPrimKey(const OUString& rSet)
                                {delete pPrimKey; pPrimKey = new OUString(rSet);}
    const OUString* GetPrimKey() const {return pPrimKey;}

    void            SetSecKey(const OUString& rSet)
                                {delete pSecKey;  pSecKey  = new OUString(rSet);}
    const OUString* GetSecKey() const { return pSecKey; }

    void            SetAltStr(const OUString& rSet)
                                {delete pAltStr;  pAltStr  = new OUString(rSet);}
    const OUString* GetAltStr() const { return pAltStr; }

    void            SetTOUName(const OUString& rSet)
                                {delete pTOUName; pTOUName = new OUString(rSet);}
    const OUString* GetTOUName() const {return pTOUName;}

    void            SetPhoneticReadingOfAltStr(const OUString& rSet)
                                {delete pPhoneticReadingOfAltStr;  pPhoneticReadingOfAltStr  = new OUString(rSet);}
    const OUString* GetPhoneticReadingOfAltStr() const {    return pPhoneticReadingOfAltStr; }

    void            SetPhoneticReadingOfPrimKey(const OUString& rSet)
                                {delete pPhoneticReadingOfPrimKey;  pPhoneticReadingOfPrimKey  = new OUString(rSet);}
    const OUString* GetPhoneticReadingOfPrimKey() const {   return pPhoneticReadingOfPrimKey; }

    void            SetPhoneticReadingOfSecKey(const OUString& rSet)
                                {delete pPhoneticReadingOfSecKey;  pPhoneticReadingOfSecKey  = new OUString(rSet);}
    const OUString* GetPhoneticReadingOfSecKey() const {    return pPhoneticReadingOfSecKey; }
};

class SW_DLLPUBLIC SwTOXMgr
{
    SwWrtShell*         pSh;
    SwTOXMark*          pCurTOXMark;
    SwTOXMarks          aCurMarks;

    SAL_DLLPRIVATE sal_uInt16                GetUserTypeID(const OUString& rStr);

public:
    // single argument ctors shall be explicit.
    explicit SwTOXMgr(SwWrtShell* pShell);

    // methods for directory marks

    void    InsertTOXMark(const SwTOXMarkDescription& rDesc);

    void    UpdateTOXMark(const SwTOXMarkDescription& rDesc);

    void                DeleteTOXMark();
    void                NextTOXMark(sal_Bool bSame=sal_False);
    void                PrevTOXMark(sal_Bool bSame=sal_False);

    // get current TOXmarks
    sal_uInt16              GetTOXMarks();
    sal_uInt16              GetTOXMarkCount();
    SwTOXMark*          GetTOXMark(sal_uInt16 nId);
    SwTOXMark*          GetCurTOXMark();
    void                SetCurTOXMark(sal_uInt16 nId);

    // methods for directories

    sal_Bool    UpdateOrInsertTOX(const SwTOXDescription& rDesc, SwTOXBase** ppBase = 0, const SfxItemSet* pSet = 0);

    const SwTOXType*    GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const;
    const SwTOXBase*    GetCurTOX();

};

/*--------------------------------------------------------------------
    Description: inlines
 --------------------------------------------------------------------*/
inline sal_uInt16 SwTOXMgr::GetTOXMarkCount()
    {   return aCurMarks.size();   }

inline SwTOXMark* SwTOXMgr::GetCurTOXMark()
    {   return pCurTOXMark; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
