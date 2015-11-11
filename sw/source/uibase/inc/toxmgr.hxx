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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TOXMGR_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TOXMGR_HXX

#include "swdllapi.h"
#include "tox.hxx"
#include <authfld.hxx>

class SwWrtShell;
class SwForm;

// manager for directory functionality
//one single method will be sufficient to insert AND upate indexes
class SW_DLLPUBLIC SwTOXDescription
{
    TOXTypes            m_eTOXType;
    OUString            m_aStyleNames[MAXLEVEL];
    OUString            m_sSequenceName;
    OUString            m_sMainEntryCharStyle;
    OUString            m_sAutoMarkURL;
    OUString*           m_pTitle;
    OUString*           m_pTOUName;
    SwForm*             m_pForm;
    sal_uInt16          m_nContent;
    sal_uInt16          m_nIndexOptions;
    sal_uInt16          m_nOLEOptions;
    LanguageType        m_eLanguage;
    OUString            m_sSortAlgorithm;

    OUString            m_sAuthBrackets;
    SwCaptionDisplay    m_eCaptionDisplay;
    SwTOXSortKey        m_eSortKey1;
    SwTOXSortKey        m_eSortKey2;
    SwTOXSortKey        m_eSortKey3;
    sal_uInt8           m_nLevel;
    bool                m_bFromObjectNames : 1;
    bool                m_bFromChapter : 1;
    bool                m_bReadonly: 1;
    bool                m_bLevelFromChapter : 1;
    bool                m_bIsAuthSequence :1;
    bool                m_bSortByDocument :1;

    //TODO: TemplateNames
    //const String* pTemplateName = 0, ???

    SwTOXDescription(SwTOXDescription&) = delete;
    SwTOXDescription & operator= (SwTOXDescription&) = delete;

public:
    // single argument ctors shall be explicit.
    explicit SwTOXDescription(TOXTypes eType) :
        m_eTOXType(eType),
        m_pTitle(nullptr),
        m_pTOUName(nullptr),
        m_pForm(nullptr),
        m_nContent(nsSwTOXElement::TOX_MARK | nsSwTOXElement::TOX_OUTLINELEVEL),
        m_nIndexOptions(nsSwTOIOptions::TOI_SAME_ENTRY|nsSwTOIOptions::TOI_FF|nsSwTOIOptions::TOI_CASE_SENSITIVE),
        m_nOLEOptions(0),
        m_eLanguage((LanguageType)::GetAppLanguage()),
        m_eCaptionDisplay(CAPTION_COMPLETE),
        m_nLevel(MAXLEVEL),
        m_bFromObjectNames(false),
        m_bFromChapter(false),
        m_bReadonly(true),
        m_bLevelFromChapter(false),
        m_bIsAuthSequence(false),
        m_bSortByDocument(true)
        {}
    ~SwTOXDescription()
        {
            delete m_pTitle;
            delete m_pForm;
            delete m_pTOUName;
        }

    TOXTypes        GetTOXType() const { return m_eTOXType;}

    const OUString& GetStyleNames(sal_uInt16 nLvl) const
                                {return m_aStyleNames[nLvl];}
    void            SetStyleNames(const OUString& rSet, sal_uInt16 nLvl)
                                {m_aStyleNames[nLvl] = rSet; }

    const OUString& GetAutoMarkURL() const { return m_sAutoMarkURL;}
    void            SetAutoMarkURL(const OUString& rSet) {m_sAutoMarkURL = rSet;}

    void            SetTitle(const OUString& pSet) {delete m_pTitle; m_pTitle = new OUString(pSet);}
    const OUString* GetTitle() const {return m_pTitle; }

    void            SetTOUName(const OUString& pSet) {delete m_pTOUName; m_pTOUName = new OUString(pSet);}
    const OUString* GetTOUName() const {return m_pTOUName; }

    void            SetForm(const SwForm& rSet) {delete m_pForm; m_pForm = new SwForm(rSet);}
    const SwForm*   GetForm() const {return m_pForm;}

    void            SetContentOptions(sal_uInt16 nSet) { m_nContent = nSet;}
    sal_uInt16          GetContentOptions() const { return m_nContent;}

    void            SetIndexOptions(sal_uInt16 nSet) { m_nIndexOptions = nSet;}
    sal_uInt16          GetIndexOptions() const { return m_nIndexOptions;}

    const OUString& GetMainEntryCharStyle() const {return m_sMainEntryCharStyle;}
    void            SetMainEntryCharStyle(const OUString& rSet)  {m_sMainEntryCharStyle = rSet;}

    void            SetLevel(sal_uInt8 nSet) {m_nLevel = nSet;}
    sal_uInt8           GetLevel()const  {return m_nLevel; }

    void            SetCreateFromObjectNames(bool bSet) { m_bFromObjectNames = bSet;}
    bool            IsCreateFromObjectNames() const {return m_bFromObjectNames;}

    const OUString& GetSequenceName() const {return m_sSequenceName;}
    void            SetSequenceName(const OUString& rSet) {m_sSequenceName = rSet;}

    SwCaptionDisplay    GetCaptionDisplay() const { return m_eCaptionDisplay;}
    void                SetCaptionDisplay(SwCaptionDisplay eSet) {m_eCaptionDisplay = eSet;}

    void            SetFromChapter(bool bSet) { m_bFromChapter = bSet;}
    bool            IsFromChapter() const {return m_bFromChapter;}

    void            SetReadonly(bool bSet){m_bReadonly = bSet;}
    bool            IsReadonly() const {return m_bReadonly;}

    sal_uInt16          GetOLEOptions() const {return m_nOLEOptions;}
    void            SetOLEOptions(sal_uInt16 nOpt) {m_nOLEOptions = nOpt;}

    bool            IsLevelFromChapter() const {return m_bLevelFromChapter;}
    void            SetLevelFromChapter(bool bSet) {m_bLevelFromChapter = bSet;}

    OUString        GetAuthBrackets() const {return m_sAuthBrackets;}
    void            SetAuthBrackets(const OUString& rSet) {m_sAuthBrackets = rSet;}

    bool            IsAuthSequence() const {return m_bIsAuthSequence;}
    void            SetAuthSequence(bool bSet){m_bIsAuthSequence = bSet;}

    bool            IsSortByDocument()const {return m_bSortByDocument ;}
    void            SetSortByDocument(bool bSet) {m_bSortByDocument = bSet;}

    void SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3);

    SwTOXSortKey GetSortKey1() const {return m_eSortKey1;}
    SwTOXSortKey GetSortKey2() const {return m_eSortKey2;}
    SwTOXSortKey GetSortKey3() const {return m_eSortKey3;}

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const OUString& GetSortAlgorithm()const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {m_sSortAlgorithm = rSet;}

    void            ApplyTo(SwTOXBase& rTOXBase);

};

class SwTOXMarkDescription
{
    TOXTypes    eTOXType;
    int         nLevel;
    bool        bMainEntry;

    OUString*   pPrimKey;
    OUString*   pSecKey;
    OUString*   pAltStr;
    OUString*   pTOUName;

    OUString*   pPhoneticReadingOfAltStr;
    OUString*   pPhoneticReadingOfPrimKey;
    OUString*   pPhoneticReadingOfSecKey;

    SwTOXMarkDescription(SwTOXMarkDescription&) = delete;
    SwTOXMarkDescription & operator= (SwTOXMarkDescription&) = delete;

public:
    // single argument ctors shall be explicit.
    explicit SwTOXMarkDescription(TOXTypes eType) :
        eTOXType(eType),
        nLevel(0),
        bMainEntry(false),
        pPrimKey(nullptr),
        pSecKey(nullptr),
        pAltStr(nullptr),
        pTOUName(nullptr),
        pPhoneticReadingOfAltStr(nullptr),
        pPhoneticReadingOfPrimKey(nullptr),
        pPhoneticReadingOfSecKey(nullptr)
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

    void            SetMainEntry(bool bSet) {bMainEntry = bSet;}
    bool            IsMainEntry() const {return bMainEntry;}

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
    void                NextTOXMark(bool bSame=false);
    void                PrevTOXMark(bool bSame=false);

    // get current TOXmarks
    sal_uInt16              GetTOXMarks();
    sal_uInt16              GetTOXMarkCount();
    SwTOXMark*          GetTOXMark(sal_uInt16 nId);
    SwTOXMark*          GetCurTOXMark();
    void                SetCurTOXMark(sal_uInt16 nId);

    // methods for directories

    bool    UpdateOrInsertTOX(const SwTOXDescription& rDesc, SwTOXBase** ppBase = nullptr, const SfxItemSet* pSet = nullptr);

    const SwTOXType*    GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const;
    const SwTOXBase*    GetCurTOX();

};

// inlines
inline sal_uInt16 SwTOXMgr::GetTOXMarkCount()
    {   return aCurMarks.size();   }

inline SwTOXMark* SwTOXMgr::GetCurTOXMark()
    {   return pCurTOXMark; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
