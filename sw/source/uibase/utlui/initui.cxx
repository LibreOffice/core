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

#include <config_features.h>

#include <unotools/localedatawrapper.hxx>
#include <viewsh.hxx>
#include <initui.hxx>
#include <edtwin.hxx>
#include <shellres.hxx>
#include <fldbas.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>

#include <utlui.hrc>
#include <initui.hrc>
#include <comcore.hrc>
#include <authfld.hxx>
#include <dbmgr.hxx>
#include <unotools/syslocale.hxx>

#include <unomid.h>

// Global Pointer

static SwGlossaries* pGlossaries = nullptr;

// Provides all needed paths. Is initialized by UI.
static SwGlossaryList* pGlossaryList = nullptr;

namespace
{

enum CachedStringID
{
    OldGrfCat,
    OldTabCat,
    OldFrameCat,
    OldDrwCat,
    CurrGlosGroup,
    CachedStrings
};

OUString *StringCache[CachedStrings] = {nullptr};

inline OUString GetCachedString(CachedStringID id)
{
    return StringCache[id] ? *StringCache[id] : OUString();
}

inline void SetCachedString(CachedStringID id, const OUString& sStr)
{
    if (StringCache[id])
    {
        *StringCache[id] = sStr;
    }
    else
    {
        StringCache[id] = new OUString(sStr);
    }
}

void ClearStringCache()
{
    for (OUString* p : StringCache)
    {
        delete p;
    }
}

}

OUString GetOldGrfCat()
{
    return GetCachedString(OldGrfCat);
}

void SetOldGrfCat(const OUString& sStr)
{
    SetCachedString(OldGrfCat, sStr);
}

OUString GetOldTabCat()
{
    return GetCachedString(OldTabCat);
}

void SetOldTabCat(const OUString& sStr)
{
    SetCachedString(OldTabCat, sStr);
}

OUString GetOldFrameCat()
{
    return GetCachedString(OldFrameCat);
}

void SetOldFrameCat(const OUString& sStr)
{
    SetCachedString(OldFrameCat, sStr);
}

OUString GetOldDrwCat()
{
    return GetCachedString(OldDrwCat);
}

void SetOldDrwCat(const OUString& sStr)
{
    SetCachedString(OldDrwCat, sStr);
}

OUString GetCurrGlosGroup()
{
    return GetCachedString(CurrGlosGroup);
}

void SetCurrGlosGroup(const OUString& sStr)
{
    SetCachedString(CurrGlosGroup, sStr);
}

namespace
{

std::vector<OUString>* pAuthFieldNameList = nullptr;
std::vector<OUString>* pAuthFieldTypeList = nullptr;

}

// Finish UI

void FinitUI()
{
    delete SwViewShell::GetShellRes();
    SwViewShell::SetShellRes( nullptr );

    SwEditWin::FinitStaticData();

    DELETEZ(pGlossaries);

    delete SwFieldType::s_pFieldNames;

    ClearStringCache();
    delete pGlossaryList;
    delete pAuthFieldNameList;
    delete pAuthFieldTypeList;

}

// Initialise

void InitUI()
{
    // ShellResource gives the CORE the possibility to work with resources.
    SwViewShell::SetShellRes( new ShellResource );
    SwEditWin::InitStaticData();
}

ShellResource::ShellResource()
    : Resource( SW_RES(RID_SW_SHELLRES) ),
    aPostItAuthor( SW_RES( STR_POSTIT_AUTHOR ) ),
    aPostItPage( SW_RES( STR_POSTIT_PAGE ) ),
    aPostItLine( SW_RES( STR_POSTIT_LINE ) ),

    aCalc_Syntax( SW_RES( STR_CALC_SYNTAX ) ),
    aCalc_ZeroDiv( SW_RES( STR_CALC_ZERODIV ) ),
    aCalc_Brack( SW_RES( STR_CALC_BRACK ) ),
    aCalc_Pow( SW_RES( STR_CALC_POW ) ),
    aCalc_VarNFnd( SW_RES( STR_CALC_VARNFND ) ),
    aCalc_Overflow( SW_RES( STR_CALC_OVERFLOW ) ),
    aCalc_WrongTime( SW_RES( STR_CALC_WRONGTIME ) ),
    aCalc_Default( SW_RES( STR_CALC_DEFAULT ) ),
    aCalc_Error( SW_RES( STR_CALC_ERROR ) ),

    // #i81002#
    aGetRefField_RefItemNotFound( SW_RES( STR_GETREFFLD_REFITEMNOTFOUND ) ),
    aStrNone( SW_RES( STR_TEMPLATE_NONE )),
    aFixedStr( SW_RES( STR_FIELD_FIXED )),
    sDurationFormat( SW_RES( STR_DURATION_FORMAT )),

    aTOXIndexName(          SW_RES(STR_TOI)),
    aTOXUserName(           SW_RES(STR_TOU)),
    aTOXContentName(        SW_RES(STR_TOC)),
    aTOXIllustrationsName(  SW_RES(STR_TOX_ILL)),
    aTOXObjectsName(        SW_RES(STR_TOX_OBJ)),
    aTOXTablesName(         SW_RES(STR_TOX_TBL)),
    aTOXAuthoritiesName(    SW_RES(STR_TOX_AUTH)),
    aTOXCitationName(    SW_RES(STR_TOX_CITATION)),
    aLinkCtrlClick(SW_RESSTR(STR_LINK_CTRL_CLICK)),
    aLinkClick(SW_RESSTR(STR_LINK_CLICK)),
    pAutoFormatNameLst(nullptr),
    sPageDescFirstName(     SW_RES(STR_PAGEDESC_FIRSTNAME)),
    sPageDescFollowName(    SW_RES(STR_PAGEDESC_FOLLOWNAME)),
    sPageDescName(          SW_RES(STR_PAGEDESC_NAME))
{
    const sal_uInt16 nCount = FLD_DOCINFO_END - FLD_DOCINFO_BEGIN;

    vcl::KeyCode aCode( KEY_SPACE );
    vcl::KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
    OUString aModStr( aModifiedCode.GetName() );
    aModStr = aModStr.replaceFirst(aCode.GetName(), "");
    aModStr = aModStr.replaceAll("+", "");
    aLinkCtrlClick = aLinkCtrlClick.replaceAll("%s", aModStr);

    for(sal_uInt16 i = 0; i < nCount; ++i)
        aDocInfoLst.push_back(OUString(SW_RESSTR(FLD_DOCINFO_BEGIN + i)));

    FreeResource();
}

ShellResource::~ShellResource()
{
}

OUString ShellResource::GetPageDescName(sal_uInt16 nNo, PageNameMode eMode)
{
    OUString sRet;

    switch (eMode)
    {
        case NORMAL_PAGE:
            sRet = sPageDescName;
            break;
        case FIRST_PAGE:
            sRet = sPageDescFirstName;
            break;
        case FOLLOW_PAGE:
            sRet = sPageDescFollowName;
            break;
    }

    return sRet.replaceFirst( "$(ARG1)", OUString::number( nNo ));
}

SwGlossaries* GetGlossaries()
{
    if (!pGlossaries)
        pGlossaries = new SwGlossaries;
    return pGlossaries;
}

bool HasGlossaryList()
{
    return pGlossaryList != nullptr;
}

SwGlossaryList* GetGlossaryList()
{
    if(!pGlossaryList)
        pGlossaryList = new SwGlossaryList();

    return pGlossaryList;
}

struct ImpAutoFormatNameListLoader : public Resource
{
    explicit ImpAutoFormatNameListLoader( std::vector<OUString>& rLst );
};

void ShellResource::GetAutoFormatNameLst_() const
{
    assert(!pAutoFormatNameLst);
    pAutoFormatNameLst.reset( new std::vector<OUString> );
    pAutoFormatNameLst->reserve(STR_AUTOFMTREDL_END);
    ImpAutoFormatNameListLoader aTmp(*pAutoFormatNameLst);
}

ImpAutoFormatNameListLoader::ImpAutoFormatNameListLoader( std::vector<OUString>& rLst )
    : Resource( ResId(RID_SHELLRES_AUTOFMTSTRS, *pSwResMgr) )
{
    for( sal_uInt16 n = 0; n < STR_AUTOFMTREDL_END; ++n )
    {
        OUString p(ResId(n + 1, *pSwResMgr));
        if(STR_AUTOFMTREDL_TYPO == n)
        {
            const SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLclD = aSysLocale.GetLocaleData();
            p = p.replaceFirst("%1", rLclD.getDoubleQuotationMarkStart());
            p = p.replaceFirst("%2", rLclD.getDoubleQuotationMarkEnd());
        }
        rLst.insert(rLst.begin() + n, p);
    }
    FreeResource();
}

OUString SwAuthorityFieldType::GetAuthFieldName(ToxAuthorityField eType)
{
    if(!pAuthFieldNameList)
    {
        pAuthFieldNameList = new std::vector<OUString>;
        pAuthFieldNameList->reserve(AUTH_FIELD_END);
        for(sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
            pAuthFieldNameList->push_back(SW_RES(STR_AUTH_FIELD_START + i));
    }
    return (*pAuthFieldNameList)[static_cast< sal_uInt16 >(eType)];
}

OUString SwAuthorityFieldType::GetAuthTypeName(ToxAuthorityType eType)
{
    if(!pAuthFieldTypeList)
    {
        pAuthFieldTypeList = new std::vector<OUString>;
        pAuthFieldTypeList->reserve(AUTH_TYPE_END);
        for(sal_uInt16 i = 0; i < AUTH_TYPE_END; ++i)
            pAuthFieldTypeList->push_back(SW_RES(STR_AUTH_TYPE_START + i));
    }
    return (*pAuthFieldTypeList)[static_cast< sal_uInt16 >(eType)];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
