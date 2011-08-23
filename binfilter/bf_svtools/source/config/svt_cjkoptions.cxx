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

#include <bf_svtools/cjkoptions.hxx>

#include <bf_svtools/languageoptions.hxx>

#include <i18npool/lang.h>

#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>

#include <itemholder2.hxx>

using namespace ::com::sun::star::uno;
using namespace ::rtl;

namespace binfilter
{

#define C2U(cChar) OUString::createFromAscii(cChar)
#define CFG_READONLY_DEFAULT sal_False
/* -----------------------------10.04.01 12:39--------------------------------

 ---------------------------------------------------------------------------*/
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

    virtual void 	Notify( const com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames );
    virtual void	Commit();
    void            Load();

    sal_Bool IsLoaded()                         { return bIsLoaded;         }

    sal_Bool IsCJKFontEnabled() const           { return bCJKFont;          }
    sal_Bool IsVerticalTextEnabled() const      { return bVerticalText;     }
    sal_Bool IsAsianTypographyEnabled() const   { return bAsianTypography;  }
    sal_Bool IsJapaneseFindEnabled() const      { return bJapaneseFind;     }
    sal_Bool IsRubyEnabled() const              { return bRuby;             }
    sal_Bool IsChangeCaseMapEnabled() const     { return bChangeCaseMap;    }
    sal_Bool IsDoubleLinesEnabled() const       { return bDoubleLines;      }
    sal_Bool IsEmphasisMarksEnabled() const     { return bEmphasisMarks;    }
    sal_Bool IsVerticalCallOutEnabled() const   { return bVerticalCallOut;  }

    sal_Bool IsAnyEnabled() const {
        return  bCJKFont||bVerticalText||bAsianTypography||bJapaneseFind||
                bRuby||bChangeCaseMap||bDoubleLines||bEmphasisMarks||bVerticalCallOut;   }
    void    SetAll(sal_Bool bSet);
};
/*-- 10.04.01 12:41:57---------------------------------------------------

  -----------------------------------------------------------------------*/
namespace
{ 
    struct PropertyNames 
        : public rtl::Static< Sequence<OUString>, PropertyNames > {}; 
}

SvtCJKOptions_Impl::SvtCJKOptions_Impl() :
    utl::ConfigItem(C2U("Office.Common/I18N/CJK")),
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
/*-- 10.04.01 12:41:57---------------------------------------------------

  -----------------------------------------------------------------------*/
SvtCJKOptions_Impl::~SvtCJKOptions_Impl()
{
}
/* -----------------------------20.04.01 14:34--------------------------------

 ---------------------------------------------------------------------------*/
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
    }
}
/*-- 10.04.01 12:41:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SvtCJKOptions_Impl::Load()
{
    Sequence<OUString> &rPropertyNames = PropertyNames::get();
    if(!rPropertyNames.getLength())
    {
        rPropertyNames.realloc(9);
        OUString* pNames = rPropertyNames.getArray();

        pNames[0] = C2U("CJKFont");
        pNames[1] = C2U("VerticalText");
        pNames[2] = C2U("AsianTypography");
        pNames[3] = C2U("JapaneseFind");
        pNames[4] = C2U("Ruby");
        pNames[5] = C2U("ChangeCaseMap");
        pNames[6] = C2U("DoubleLines");
        pNames[7] = C2U("EmphasisMarks");
        pNames[8] = C2U("VerticalCallOut");

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

    SvtSystemLanguageOptions aSystemLocaleSettings;
    LanguageType eSystemLanguage = aSystemLocaleSettings.GetWin16SystemLanguage();
    sal_uInt16 nWinScript = SvtLanguageOptions::GetScriptTypeOfLanguage( eSystemLanguage );
    
    sal_uInt16 nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage(LANGUAGE_SYSTEM);
    if ( !bCJKFont && (( nScriptType & SCRIPTTYPE_ASIAN )||
             ((eSystemLanguage != LANGUAGE_SYSTEM)  && ( nWinScript & SCRIPTTYPE_ASIAN ))))
    {
        SetAll(sal_True);
    }
    bIsLoaded = sal_True;
}
/*-- 10.04.01 12:41:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SvtCJKOptions_Impl::Notify( const Sequence< OUString >& )
{
    Load();
}
/*-- 10.04.01 12:41:57---------------------------------------------------

  -----------------------------------------------------------------------*/
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

// global ----------------------------------------------------------------

static SvtCJKOptions_Impl*  pCJKOptions = NULL;
static sal_Int32            nCJKRefCount = 0;
namespace { struct CJKMutex : public rtl::Static< ::osl::Mutex , CJKMutex >{}; }


// class SvtCJKOptions --------------------------------------------------

SvtCJKOptions::SvtCJKOptions(sal_Bool bDontLoad)
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( CJKMutex::get() );
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
    ::osl::MutexGuard aGuard( CJKMutex::get() );
    if ( !--nCJKRefCount )
        DELETEZ( pCJKOptions );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
