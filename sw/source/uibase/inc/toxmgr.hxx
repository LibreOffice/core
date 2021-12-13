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

#include <swdllapi.h>
#include <tox.hxx>
#include <authfld.hxx>
#include <memory>
#include <optional>

class SwWrtShell;
class SwForm;

// manager for directory functionality
//one single method will be sufficient to insert AND update indexes
class SW_DLLPUBLIC SwTOXDescription
{
    TOXTypes            m_eTOXType;
    OUString            m_aStyleNames[MAXLEVEL];
    OUString            m_sSequenceName;
    OUString            m_sMainEntryCharStyle;
    OUString            m_sAutoMarkURL;
    std::optional<OUString>
                        m_aTitle;
    std::optional<OUString>
                        m_aTOUName;
    std::unique_ptr<SwForm>
                        m_pForm;
    SwTOXElement        m_nContent;
    SwTOIOptions        m_nIndexOptions;
    SwTOOElements       m_nOLEOptions;
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

    SwTOXDescription(SwTOXDescription const &) = delete;
    SwTOXDescription & operator= (SwTOXDescription const &) = delete;

public:
    // single argument ctors shall be explicit.
    explicit SwTOXDescription(TOXTypes eType) :
        m_eTOXType(eType),
        m_nContent(SwTOXElement::Mark | SwTOXElement::OutlineLevel),
        m_nIndexOptions(SwTOIOptions::SameEntry|SwTOIOptions::FF|SwTOIOptions::CaseSensitive),
        m_nOLEOptions(SwTOOElements::NONE),
        m_eLanguage(::GetAppLanguage()),
        m_eCaptionDisplay(CAPTION_COMPLETE),
        m_nLevel(MAXLEVEL),
        m_bFromObjectNames(false),
        m_bFromChapter(false),
        m_bReadonly(true),
        m_bLevelFromChapter(false),
        m_bIsAuthSequence(false),
        m_bSortByDocument(true)
        {}

    TOXTypes        GetTOXType() const { return m_eTOXType;}

    const OUString& GetStyleNames(sal_uInt16 nLvl) const
                                {return m_aStyleNames[nLvl];}
    void            SetStyleNames(const OUString& rSet, sal_uInt16 nLvl)
                                {m_aStyleNames[nLvl] = rSet; }

    const OUString& GetAutoMarkURL() const { return m_sAutoMarkURL;}
    void            SetAutoMarkURL(const OUString& rSet) {m_sAutoMarkURL = rSet;}

    void            SetTitle(const OUString& rSet) { m_aTitle = rSet; }
    std::optional<OUString> const & GetTitle() const {return m_aTitle; }

    void            SetTOUName(const OUString& rSet) { m_aTOUName = rSet; }
    std::optional<OUString> const & GetTOUName() const { return m_aTOUName; }

    void            SetForm(const SwForm& rSet) { m_pForm.reset( new SwForm(rSet) );}
    const SwForm*   GetForm() const {return m_pForm.get();}

    void            SetContentOptions(SwTOXElement nSet) { m_nContent = nSet;}
    SwTOXElement    GetContentOptions() const { return m_nContent;}

    void            SetIndexOptions(SwTOIOptions nSet) { m_nIndexOptions = nSet;}
    SwTOIOptions    GetIndexOptions() const { return m_nIndexOptions;}

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

    SwTOOElements   GetOLEOptions() const {return m_nOLEOptions;}
    void            SetOLEOptions(SwTOOElements nOpt) {m_nOLEOptions = nOpt;}

    bool            IsLevelFromChapter() const {return m_bLevelFromChapter;}
    void            SetLevelFromChapter(bool bSet) {m_bLevelFromChapter = bSet;}

    const OUString& GetAuthBrackets() const {return m_sAuthBrackets;}
    void            SetAuthBrackets(const OUString& rSet) {m_sAuthBrackets = rSet;}

    bool            IsAuthSequence() const {return m_bIsAuthSequence;}
    void            SetAuthSequence(bool bSet){m_bIsAuthSequence = bSet;}

    bool            IsSortByDocument()const {return m_bSortByDocument ;}
    void            SetSortByDocument(bool bSet) {m_bSortByDocument = bSet;}

    void SetSortKeys(SwTOXSortKey eKey1,
                        SwTOXSortKey eKey2,
                            SwTOXSortKey eKey3);

    const SwTOXSortKey& GetSortKey1() const {return m_eSortKey1;}
    const SwTOXSortKey& GetSortKey2() const {return m_eSortKey2;}
    const SwTOXSortKey& GetSortKey3() const {return m_eSortKey3;}

    LanguageType    GetLanguage() const {return m_eLanguage;}
    void            SetLanguage(LanguageType nLang)  {m_eLanguage = nLang;}

    const OUString& GetSortAlgorithm()const {return m_sSortAlgorithm;}
    void            SetSortAlgorithm(const OUString& rSet) {m_sSortAlgorithm = rSet;}

    void            ApplyTo(SwTOXBase& rTOXBase);

};

class SwTOXMarkDescription
{
    TOXTypes    meTOXType;
    int         mnLevel;
    bool        mbMainEntry;

    std::optional<OUString>  maPrimKey;
    std::optional<OUString>  maSecKey;
    std::optional<OUString>  maAltStr;
    std::optional<OUString>  maTOUName;

    std::optional<OUString>  maPhoneticReadingOfAltStr;
    std::optional<OUString>  maPhoneticReadingOfPrimKey;
    std::optional<OUString>  maPhoneticReadingOfSecKey;

    SwTOXMarkDescription(SwTOXMarkDescription const &) = delete;
    SwTOXMarkDescription & operator= (SwTOXMarkDescription const &) = delete;

public:
    // single argument ctors shall be explicit.
    explicit SwTOXMarkDescription(TOXTypes eType) :
        meTOXType(eType),
        mnLevel(0),
        mbMainEntry(false)
        {
        }

    TOXTypes        GetTOXType()const {return meTOXType;}

    void            SetLevel(int nSet) {mnLevel = nSet;}
    int             GetLevel() const {return mnLevel;}

    void            SetMainEntry(bool bSet) {mbMainEntry = bSet;}
    bool            IsMainEntry() const {return mbMainEntry;}

    void            SetPrimKey(const OUString& rSet) { maPrimKey = rSet; }
    std::optional<OUString> const & GetPrimKey() const { return maPrimKey; }

    void            SetSecKey(const OUString& rSet) { maSecKey = rSet; }
    std::optional<OUString> const & GetSecKey() const { return maSecKey; }

    void            SetAltStr(const OUString& rSet) { maAltStr = rSet; }
    std::optional<OUString> const & GetAltStr() const { return maAltStr; }

    void            SetTOUName(const OUString& rSet) { maTOUName = rSet; }
    std::optional<OUString> const & GetTOUName() const { return maTOUName; }

    void            SetPhoneticReadingOfAltStr(const OUString& rSet) { maPhoneticReadingOfAltStr = rSet; }
    std::optional<OUString> const & GetPhoneticReadingOfAltStr() const { return maPhoneticReadingOfAltStr; }

    void            SetPhoneticReadingOfPrimKey(const OUString& rSet) { maPhoneticReadingOfPrimKey = rSet; }
    std::optional<OUString> const & GetPhoneticReadingOfPrimKey() const { return maPhoneticReadingOfPrimKey; }

    void            SetPhoneticReadingOfSecKey(const OUString& rSet) { maPhoneticReadingOfSecKey = rSet; }
    std::optional<OUString> const & GetPhoneticReadingOfSecKey() const { return maPhoneticReadingOfSecKey; }
};

class SW_DLLPUBLIC SwTOXMgr
{
    SwWrtShell*         m_pSh;
    SwTOXMark*          m_pCurTOXMark;
    SwTOXMarks          m_aCurMarks;

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
    sal_uInt16              GetTOXMarkCount() const;
    SwTOXMark*          GetTOXMark(sal_uInt16 nId);
    SwTOXMark*          GetCurTOXMark();
    void                SetCurTOXMark(sal_uInt16 nId);

    // methods for directories

    bool    UpdateOrInsertTOX(const SwTOXDescription& rDesc, SwTOXBase** ppBase, const SfxItemSet* pSet);

    const SwTOXType*    GetTOXType(TOXTypes eTyp) const;
    SwWrtShell * GetShell() { return m_pSh; }
};

// inlines
inline sal_uInt16 SwTOXMgr::GetTOXMarkCount() const
    {   return m_aCurMarks.size();   }

inline SwTOXMark* SwTOXMgr::GetCurTOXMark()
    {   return m_pCurTOXMark; }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
