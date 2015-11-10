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

#include <svl/cjkoptions.hxx>

#include <svl/languageoptions.hxx>
#include <i18nlangtag/lang.h>
#include <unotools/configitem.hxx>
#include <tools/solar.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

#include "itemholder2.hxx"

using namespace ::com::sun::star::uno;

#define CFG_READONLY_DEFAULT false

class SvtCJKOptions_Impl : public utl::ConfigItem
{
    bool        bIsLoaded;
    bool        bCJKFont;
    bool        bVerticalText;
    bool        bAsianTypography;
    bool        bJapaneseFind;
    bool        bRuby;
    bool        bChangeCaseMap;
    bool        bDoubleLines;
    bool        bEmphasisMarks;
    bool        bVerticalCallOut;

    bool        bROCJKFont;
    bool        bROVerticalText;
    bool        bROAsianTypography;
    bool        bROJapaneseFind;
    bool        bRORuby;
    bool        bROChangeCaseMap;
    bool        bRODoubleLines;
    bool        bROEmphasisMarks;
    bool        bROVerticalCallOut;

    virtual void    ImplCommit() override;

public:
    SvtCJKOptions_Impl();
    virtual ~SvtCJKOptions_Impl();

    virtual void    Notify( const css::uno::Sequence< OUString >& rPropertyNames ) override;
    void            Load();

    bool IsLoaded()                         { return bIsLoaded;         }

    bool IsCJKFontEnabled() const           { return bCJKFont;          }
    bool IsVerticalTextEnabled() const      { return bVerticalText;     }
    bool IsAsianTypographyEnabled() const   { return bAsianTypography;  }
    bool IsJapaneseFindEnabled() const      { return bJapaneseFind;     }
    bool IsRubyEnabled() const              { return bRuby;             }
    bool IsChangeCaseMapEnabled() const     { return bChangeCaseMap;    }
    bool IsDoubleLinesEnabled() const       { return bDoubleLines;      }

    bool IsAnyEnabled() const {
        return  bCJKFont||bVerticalText||bAsianTypography||bJapaneseFind||
                bRuby||bChangeCaseMap||bDoubleLines||bEmphasisMarks||bVerticalCallOut;   }
    void    SetAll(bool bSet);
    bool IsReadOnly(SvtCJKOptions::EOption eOption) const;
};

namespace
{
    struct PropertyNames
        : public rtl::Static< Sequence<OUString>, PropertyNames > {};
}

SvtCJKOptions_Impl::SvtCJKOptions_Impl() :
    utl::ConfigItem("Office.Common/I18N/CJK"),
    bIsLoaded(false),
    bCJKFont(true),
    bVerticalText(true),
    bAsianTypography(true),
    bJapaneseFind(true),
    bRuby(true),
    bChangeCaseMap(true),
    bDoubleLines(true),
    bEmphasisMarks(true),
    bVerticalCallOut(true),
    bROCJKFont(CFG_READONLY_DEFAULT),
    bROVerticalText(CFG_READONLY_DEFAULT),
    bROAsianTypography(CFG_READONLY_DEFAULT),
    bROJapaneseFind(CFG_READONLY_DEFAULT),
    bRORuby(CFG_READONLY_DEFAULT),
    bROChangeCaseMap(CFG_READONLY_DEFAULT),
    bRODoubleLines(CFG_READONLY_DEFAULT),
    bROEmphasisMarks(CFG_READONLY_DEFAULT),
    bROVerticalCallOut(CFG_READONLY_DEFAULT)
{
}

SvtCJKOptions_Impl::~SvtCJKOptions_Impl()
{
}

void    SvtCJKOptions_Impl::SetAll(bool bSet)
{
    if (
        !bROCJKFont          &&
        !bROVerticalText     &&
        !bROAsianTypography  &&
        !bROJapaneseFind     &&
        !bRORuby             &&
        !bROChangeCaseMap    &&
        !bRODoubleLines      &&
        !bROEmphasisMarks    &&
        !bROVerticalCallOut
       )
    {
        bCJKFont=bSet;
        bVerticalText=bSet;
        bAsianTypography=bSet;
        bJapaneseFind=bSet;
        bRuby=bSet;
        bChangeCaseMap=bSet;
        bDoubleLines=bSet;
        bEmphasisMarks=bSet;
        bVerticalCallOut=bSet;

        SetModified();
        Commit();
        NotifyListeners(0);
    }
}

void SvtCJKOptions_Impl::Load()
{
    Sequence<OUString> &rPropertyNames = PropertyNames::get();
    if(!rPropertyNames.getLength())
    {
        rPropertyNames.realloc(9);
        OUString* pNames = rPropertyNames.getArray();

        pNames[0] = "CJKFont";
        pNames[1] = "VerticalText";
        pNames[2] = "AsianTypography";
        pNames[3] = "JapaneseFind";
        pNames[4] = "Ruby";
        pNames[5] = "ChangeCaseMap";
        pNames[6] = "DoubleLines";
        pNames[7] = "EmphasisMarks";
        pNames[8] = "VerticalCallOut";

        EnableNotification( rPropertyNames );
    }
    Sequence< Any > aValues = GetProperties(rPropertyNames);
    Sequence< sal_Bool > aROStates = GetReadOnlyStates(rPropertyNames);
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
    assert(aValues.getLength() == rPropertyNames.getLength() && "GetProperties failed");
    assert(aROStates.getLength() == rPropertyNames.getLength() && "GetReadOnlyStates failed");
    if ( aValues.getLength() == rPropertyNames.getLength() && aROStates.getLength() == rPropertyNames.getLength() )
    {
        for ( int nProp = 0; nProp < rPropertyNames.getLength(); nProp++ )
        {
            if( pValues[nProp].hasValue() )
            {
                bool bValue = *static_cast<sal_Bool const *>(pValues[nProp].getValue());
                switch ( nProp )
                {
                    case 0: { bCJKFont = bValue; bROCJKFont = pROStates[nProp]; } break;
                    case 1: { bVerticalText = bValue; bROVerticalText = pROStates[nProp]; } break;
                    case 2: { bAsianTypography = bValue; bROAsianTypography = pROStates[nProp]; } break;
                    case 3: { bJapaneseFind = bValue; bROJapaneseFind = pROStates[nProp]; } break;
                    case 4: { bRuby = bValue; bRORuby = pROStates[nProp]; } break;
                    case 5: { bChangeCaseMap = bValue; bROChangeCaseMap = pROStates[nProp]; } break;
                    case 6: { bDoubleLines = bValue; bRODoubleLines = pROStates[nProp]; } break;
                    case 7: { bEmphasisMarks = bValue; bROEmphasisMarks = pROStates[nProp]; } break;
                    case 8: { bVerticalCallOut = bValue; bROVerticalCallOut = pROStates[nProp]; } break;
                }
            }
        }
    }

    if (!bCJKFont)
    {
        SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage(LANGUAGE_SYSTEM);
        //system locale is CJK
        bool bAutoEnableCJK = bool(nScriptType & SvtScriptType::ASIAN);

        if (!bAutoEnableCJK)
        {
            SvtSystemLanguageOptions aSystemLocaleSettings;

            //windows secondary system locale is CJK
            LanguageType eSystemLanguage = aSystemLocaleSettings.GetWin16SystemLanguage();
            if (eSystemLanguage != LANGUAGE_SYSTEM)
            {
                SvtScriptType nWinScript = SvtLanguageOptions::GetScriptTypeOfLanguage( eSystemLanguage );
                bAutoEnableCJK = bool(nWinScript & SvtScriptType::ASIAN);
            }

            //CJK keyboard is installed
            if (!bAutoEnableCJK)
                bAutoEnableCJK = aSystemLocaleSettings.isCJKKeyboardLayoutInstalled();
        }

        if (bAutoEnableCJK)
        {
            SetAll(true);
        }
    }
    bIsLoaded = true;
}

void    SvtCJKOptions_Impl::Notify( const Sequence< OUString >& )
{
    Load();
    NotifyListeners(0);
}

void    SvtCJKOptions_Impl::ImplCommit()
{
    Sequence<OUString> &rPropertyNames = PropertyNames::get();
    OUString* pOrgNames = rPropertyNames.getArray();
    sal_Int32 nOrgCount = rPropertyNames.getLength();

    Sequence< OUString > aNames(nOrgCount);
    Sequence< Any > aValues(nOrgCount);

    OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    const Type& rType = cppu::UnoType<bool>::get();
    for(int nProp = 0; nProp < nOrgCount; nProp++)
    {
        switch(nProp)
        {
            case  0:
                {
                    if (!bROCJKFont)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bCJKFont, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  1:
                {
                    if (!bROVerticalText)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bVerticalText, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  2:
                {
                    if (!bROAsianTypography)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bAsianTypography, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  3:
                {
                    if (!bROJapaneseFind)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bJapaneseFind, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  4:
                {
                    if (!bRORuby)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bRuby, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  5:
                {
                    if (!bROChangeCaseMap)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bChangeCaseMap, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  6:
                {
                    if (!bRODoubleLines)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bDoubleLines, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  7:
                {
                    if (!bROEmphasisMarks)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bEmphasisMarks, rType);
                        ++nRealCount;
                    }
                }
                break;

            case  8:
                {
                    if (!bROVerticalCallOut)
                    {
                        pNames[nRealCount] = pOrgNames[nProp];
                        pValues[nRealCount].setValue(&bVerticalCallOut, rType);
                        ++nRealCount;
                    }
                }
                break;
        }
    }
    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties(aNames, aValues);
}

bool SvtCJKOptions_Impl::IsReadOnly(SvtCJKOptions::EOption eOption) const
{
    bool bReadOnly = CFG_READONLY_DEFAULT;
    switch(eOption)
    {
        case SvtCJKOptions::E_CJKFONT : bReadOnly = bROCJKFont; break;
        case SvtCJKOptions::E_VERTICALTEXT : bReadOnly = bROVerticalText; break;
        case SvtCJKOptions::E_ASIANTYPOGRAPHY : bReadOnly = bROAsianTypography; break;
        case SvtCJKOptions::E_JAPANESEFIND : bReadOnly = bROJapaneseFind; break;
        case SvtCJKOptions::E_RUBY : bReadOnly = bRORuby; break;
        case SvtCJKOptions::E_CHANGECASEMAP : bReadOnly = bROChangeCaseMap; break;
        case SvtCJKOptions::E_DOUBLELINES : bReadOnly = bRODoubleLines; break;
        case SvtCJKOptions::E_EMPHASISMARKS : bReadOnly = bROEmphasisMarks; break;
        case SvtCJKOptions::E_VERTICALCALLOUT : bReadOnly = bROVerticalCallOut; break;
        case SvtCJKOptions::E_ALL : if (bROCJKFont || bROVerticalText || bROAsianTypography || bROJapaneseFind || bRORuby || bROChangeCaseMap || bRODoubleLines || bROEmphasisMarks || bROVerticalCallOut)
                                        bReadOnly = true;
                                break;
    }
    return bReadOnly;
}

// global

static SvtCJKOptions_Impl*  pCJKOptions = nullptr;
static sal_Int32            nCJKRefCount = 0;
namespace { struct theCJKOptionsMutex : public rtl::Static< ::osl::Mutex , theCJKOptionsMutex >{}; }

SvtCJKOptions::SvtCJKOptions(bool bDontLoad)
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( theCJKOptionsMutex::get() );
    if ( !pCJKOptions )
    {
        pCJKOptions = new SvtCJKOptions_Impl;
        ItemHolder2::holdConfigItem(E_CJKOPTIONS);
    }
    if( !bDontLoad && !pCJKOptions->IsLoaded())
        pCJKOptions->Load();

    ++nCJKRefCount;
    pImp = pCJKOptions;
}



SvtCJKOptions::~SvtCJKOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( theCJKOptionsMutex::get() );
    if ( !--nCJKRefCount )
        DELETEZ( pCJKOptions );
}

bool SvtCJKOptions::IsCJKFontEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsCJKFontEnabled();
}

bool SvtCJKOptions::IsVerticalTextEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsVerticalTextEnabled();
}

bool SvtCJKOptions::IsAsianTypographyEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsAsianTypographyEnabled();
}

bool SvtCJKOptions::IsJapaneseFindEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsJapaneseFindEnabled();
}

bool SvtCJKOptions::IsRubyEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsRubyEnabled();
}

bool SvtCJKOptions::IsChangeCaseMapEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsChangeCaseMapEnabled();
}

bool SvtCJKOptions::IsDoubleLinesEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsDoubleLinesEnabled();
}

void        SvtCJKOptions::SetAll(bool bSet)
{
    assert(pCJKOptions->IsLoaded());
    pCJKOptions->SetAll(bSet);
}

bool    SvtCJKOptions::IsAnyEnabled() const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsAnyEnabled();
}

bool    SvtCJKOptions::IsReadOnly(EOption eOption) const
{
    assert(pCJKOptions->IsLoaded());
    return pCJKOptions->IsReadOnly(eOption);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
