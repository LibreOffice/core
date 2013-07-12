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
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

#include <itemholder2.hxx>

using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define CFG_READONLY_DEFAULT sal_False

class SvtCJKOptions_Impl : public utl::ConfigItem
{
    sal_Bool        bIsLoaded;
    sal_Bool        bCJKFont;
    sal_Bool        bVerticalText;
    sal_Bool        bAsianTypography;
    sal_Bool        bJapaneseFind;
    sal_Bool        bRuby;
    sal_Bool        bChangeCaseMap;
    sal_Bool        bDoubleLines;
    sal_Bool        bEmphasisMarks;
    sal_Bool        bVerticalCallOut;

    sal_Bool        bROCJKFont;
    sal_Bool        bROVerticalText;
    sal_Bool        bROAsianTypography;
    sal_Bool        bROJapaneseFind;
    sal_Bool        bRORuby;
    sal_Bool        bROChangeCaseMap;
    sal_Bool        bRODoubleLines;
    sal_Bool        bROEmphasisMarks;
    sal_Bool        bROVerticalCallOut;

public:
    SvtCJKOptions_Impl();
    ~SvtCJKOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< OUString >& rPropertyNames );
    virtual void    Commit();
    void            Load();

    sal_Bool IsLoaded()                         { return bIsLoaded;         }

    sal_Bool IsCJKFontEnabled() const           { return bCJKFont;          }
    sal_Bool IsVerticalTextEnabled() const      { return bVerticalText;     }
    sal_Bool IsAsianTypographyEnabled() const   { return bAsianTypography;  }
    sal_Bool IsJapaneseFindEnabled() const      { return bJapaneseFind;     }
    sal_Bool IsRubyEnabled() const              { return bRuby;             }
    sal_Bool IsChangeCaseMapEnabled() const     { return bChangeCaseMap;    }
    sal_Bool IsDoubleLinesEnabled() const       { return bDoubleLines;      }

    sal_Bool IsAnyEnabled() const {
        return  bCJKFont||bVerticalText||bAsianTypography||bJapaneseFind||
                bRuby||bChangeCaseMap||bDoubleLines||bEmphasisMarks||bVerticalCallOut;   }
    void    SetAll(sal_Bool bSet);
    sal_Bool IsReadOnly(SvtCJKOptions::EOption eOption) const;
};

namespace
{
    struct PropertyNames
        : public rtl::Static< Sequence<OUString>, PropertyNames > {};
}

SvtCJKOptions_Impl::SvtCJKOptions_Impl() :
    utl::ConfigItem("Office.Common/I18N/CJK"),
    bIsLoaded(sal_False),
    bCJKFont(sal_True),
    bVerticalText(sal_True),
    bAsianTypography(sal_True),
    bJapaneseFind(sal_True),
    bRuby(sal_True),
    bChangeCaseMap(sal_True),
    bDoubleLines(sal_True),
    bEmphasisMarks(sal_True),
    bVerticalCallOut(sal_True),
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

void    SvtCJKOptions_Impl::SetAll(sal_Bool bSet)
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
    DBG_ASSERT( aValues.getLength() == rPropertyNames.getLength(), "GetProperties failed" );
    DBG_ASSERT( aROStates.getLength() == rPropertyNames.getLength(), "GetReadOnlyStates failed" );
    if ( aValues.getLength() == rPropertyNames.getLength() && aROStates.getLength() == rPropertyNames.getLength() )
    {
        for ( int nProp = 0; nProp < rPropertyNames.getLength(); nProp++ )
        {
            if( pValues[nProp].hasValue() )
            {
                sal_Bool bValue = *(sal_Bool*)pValues[nProp].getValue();
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
        bool bAutoEnableCJK = false;

        sal_uInt16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage(LANGUAGE_SYSTEM);
        //system locale is CJK
        bAutoEnableCJK = (nScriptType & SCRIPTTYPE_ASIAN);

        if (!bAutoEnableCJK)
        {
            SvtSystemLanguageOptions aSystemLocaleSettings;

            //windows secondary system locale is CJK
            LanguageType eSystemLanguage = aSystemLocaleSettings.GetWin16SystemLanguage();
            if (eSystemLanguage != LANGUAGE_SYSTEM)
            {
                sal_uInt16 nWinScript = SvtLanguageOptions::GetScriptTypeOfLanguage( eSystemLanguage );
                bAutoEnableCJK = (nWinScript & SCRIPTTYPE_ASIAN);
            }

            //CJK keyboard is installed
            if (!bAutoEnableCJK)
                bAutoEnableCJK = aSystemLocaleSettings.isCJKKeyboardLayoutInstalled();
        }

        if (bAutoEnableCJK)
        {
            SetAll(sal_True);
        }
    }
    bIsLoaded = sal_True;
}

void    SvtCJKOptions_Impl::Notify( const Sequence< OUString >& )
{
    Load();
    NotifyListeners(0);
}

void    SvtCJKOptions_Impl::Commit()
{
    Sequence<OUString> &rPropertyNames = PropertyNames::get();
    OUString* pOrgNames = rPropertyNames.getArray();
    sal_Int32 nOrgCount = rPropertyNames.getLength();

    Sequence< OUString > aNames(nOrgCount);
    Sequence< Any > aValues(nOrgCount);

    OUString* pNames = aNames.getArray();
    Any* pValues = aValues.getArray();
    sal_Int32 nRealCount = 0;

    const Type& rType = ::getBooleanCppuType();
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

sal_Bool SvtCJKOptions_Impl::IsReadOnly(SvtCJKOptions::EOption eOption) const
{
    sal_Bool bReadOnly = CFG_READONLY_DEFAULT;
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
                                        bReadOnly = sal_True;
                                break;
    }
    return bReadOnly;
}

// global ----------------------------------------------------------------

static SvtCJKOptions_Impl*  pCJKOptions = NULL;
static sal_Int32            nCJKRefCount = 0;
namespace { struct theCJKOptionsMutex : public rtl::Static< ::osl::Mutex , theCJKOptionsMutex >{}; }


// class SvtCJKOptions --------------------------------------------------

SvtCJKOptions::SvtCJKOptions(sal_Bool bDontLoad)
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

// -----------------------------------------------------------------------

SvtCJKOptions::~SvtCJKOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( theCJKOptionsMutex::get() );
    if ( !--nCJKRefCount )
        DELETEZ( pCJKOptions );
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsCJKFontEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsCJKFontEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsVerticalTextEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsVerticalTextEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsAsianTypographyEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsAsianTypographyEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsJapaneseFindEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsJapaneseFindEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsRubyEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsRubyEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsChangeCaseMapEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsChangeCaseMapEnabled();
}
// -----------------------------------------------------------------------
sal_Bool SvtCJKOptions::IsDoubleLinesEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsDoubleLinesEnabled();
}

void        SvtCJKOptions::SetAll(sal_Bool bSet)
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    pCJKOptions->SetAll(bSet);
}

sal_Bool    SvtCJKOptions::IsAnyEnabled() const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsAnyEnabled();
}

sal_Bool    SvtCJKOptions::IsReadOnly(EOption eOption) const
{
    DBG_ASSERT(pCJKOptions->IsLoaded(), "CJK options not loaded");
    return pCJKOptions->IsReadOnly(eOption);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
