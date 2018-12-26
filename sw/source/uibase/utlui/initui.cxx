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
#include <comcore.hxx>
#include <strings.hrc>
#include <utlui.hrc>
#include <authfld.hxx>
#include <dbmgr.hxx>
#include <unotools/syslocale.hxx>
#include <vcl/keycod.hxx>

#include <unomid.h>

// Global Pointer

static std::unique_ptr<SwGlossaries> pGlossaries;

// Provides all needed paths. Is initialized by UI.
static SwGlossaryList* pGlossaryList = nullptr;

namespace
{
OUString CurrGlosGroup;
}

const OUString& GetCurrGlosGroup()
{
    return CurrGlosGroup;
}

void SetCurrGlosGroup(const OUString& sStr)
{
    CurrGlosGroup = sStr;
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

    pGlossaries.reset();

    delete SwFieldType::s_pFieldNames;

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

static const char* FLD_DOCINFO_ARY[] =
{
    FLD_DOCINFO_TITEL,
    FLD_DOCINFO_THEMA,
    FLD_DOCINFO_KEYS,
    FLD_DOCINFO_COMMENT,
    FLD_DOCINFO_CREATE,
    FLD_DOCINFO_CHANGE,
    FLD_DOCINFO_PRINT,
    FLD_DOCINFO_DOCNO,
    FLD_DOCINFO_EDIT
};

ShellResource::ShellResource()
    : aPostItAuthor( SwResId( STR_POSTIT_AUTHOR ) ),
    aPostItPage( SwResId( STR_POSTIT_PAGE ) ),
    aPostItLine( SwResId( STR_POSTIT_LINE ) ),

    aCalc_Syntax( SwResId( STR_CALC_SYNTAX ) ),
    aCalc_ZeroDiv( SwResId( STR_CALC_ZERODIV ) ),
    aCalc_Brack( SwResId( STR_CALC_BRACK ) ),
    aCalc_Pow( SwResId( STR_CALC_POW ) ),
    aCalc_Overflow( SwResId( STR_CALC_OVERFLOW ) ),
    aCalc_Default( SwResId( STR_CALC_DEFAULT ) ),
    aCalc_Error( SwResId( STR_CALC_ERROR ) ),

    // #i81002#
    aGetRefField_RefItemNotFound( SwResId( STR_GETREFFLD_REFITEMNOTFOUND ) ),
    aStrNone( SwResId( STR_TEMPLATE_NONE )),
    aFixedStr( SwResId( STR_FIELD_FIXED )),
    sDurationFormat( SwResId( STR_DURATION_FORMAT )),

    aTOXIndexName(          SwResId(STR_TOI)),
    aTOXUserName(           SwResId(STR_TOU)),
    aTOXContentName(        SwResId(STR_TOC)),
    aTOXIllustrationsName(  SwResId(STR_TOX_ILL)),
    aTOXObjectsName(        SwResId(STR_TOX_OBJ)),
    aTOXTablesName(         SwResId(STR_TOX_TBL)),
    aTOXAuthoritiesName(    SwResId(STR_TOX_AUTH)),
    aTOXCitationName(    SwResId(STR_TOX_CITATION)),
    aLinkCtrlClick(SwResId(STR_LINK_CTRL_CLICK)),
    aLinkClick(SwResId(STR_LINK_CLICK)),
    sPageDescFirstName(     SwResId(STR_PAGEDESC_FIRSTNAME)),
    sPageDescFollowName(    SwResId(STR_PAGEDESC_FOLLOWNAME)),
    sPageDescName(          SwResId(STR_PAGEDESC_NAME))
{
    vcl::KeyCode aCode( KEY_SPACE );
    vcl::KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
    OUString aModStr( aModifiedCode.GetName() );
    aModStr = aModStr.replaceFirst(aCode.GetName(), "");
    aModStr = aModStr.replaceAll("+", "");
    aLinkCtrlClick = aLinkCtrlClick.replaceAll("%s", aModStr);

    for (size_t i = 0; i < SAL_N_ELEMENTS(FLD_DOCINFO_ARY); ++i)
        aDocInfoLst.push_back(SwResId(FLD_DOCINFO_ARY[i]));
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
        pGlossaries.reset( new SwGlossaries );
    return pGlossaries.get();
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

void ShellResource::GetAutoFormatNameLst_() const
{
    assert(!pAutoFormatNameLst);
    pAutoFormatNameLst.reset( new std::vector<OUString> );
    pAutoFormatNameLst->reserve(STR_AUTOFMTREDL_END);

    assert(SAL_N_ELEMENTS(RID_SHELLRES_AUTOFMTSTRS) == STR_AUTOFMTREDL_END);
    for (sal_uInt16 n = 0; n < STR_AUTOFMTREDL_END; ++n)
    {
        OUString p(SwResId(RID_SHELLRES_AUTOFMTSTRS[n]));
        if (STR_AUTOFMTREDL_TYPO == n)
        {
            const SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLclD = aSysLocale.GetLocaleData();
            p = p.replaceFirst("%1", rLclD.getDoubleQuotationMarkStart());
            p = p.replaceFirst("%2", rLclD.getDoubleQuotationMarkEnd());
        }
        pAutoFormatNameLst->push_back(p);
    }
}

namespace
{
    const char* STR_AUTH_FIELD_ARY[] =
    {
        STR_AUTH_FIELD_IDENTIFIER,
        STR_AUTH_FIELD_AUTHORITY_TYPE,
        STR_AUTH_FIELD_ADDRESS,
        STR_AUTH_FIELD_ANNOTE,
        STR_AUTH_FIELD_AUTHOR,
        STR_AUTH_FIELD_BOOKTITLE,
        STR_AUTH_FIELD_CHAPTER,
        STR_AUTH_FIELD_EDITION,
        STR_AUTH_FIELD_EDITOR,
        STR_AUTH_FIELD_HOWPUBLISHED,
        STR_AUTH_FIELD_INSTITUTION,
        STR_AUTH_FIELD_JOURNAL,
        STR_AUTH_FIELD_MONTH,
        STR_AUTH_FIELD_NOTE,
        STR_AUTH_FIELD_NUMBER,
        STR_AUTH_FIELD_ORGANIZATIONS,
        STR_AUTH_FIELD_PAGES,
        STR_AUTH_FIELD_PUBLISHER,
        STR_AUTH_FIELD_SCHOOL,
        STR_AUTH_FIELD_SERIES,
        STR_AUTH_FIELD_TITLE,
        STR_AUTH_FIELD_TYPE,
        STR_AUTH_FIELD_VOLUME,
        STR_AUTH_FIELD_YEAR,
        STR_AUTH_FIELD_URL,
        STR_AUTH_FIELD_CUSTOM1,
        STR_AUTH_FIELD_CUSTOM2,
        STR_AUTH_FIELD_CUSTOM3,
        STR_AUTH_FIELD_CUSTOM4,
        STR_AUTH_FIELD_CUSTOM5,
        STR_AUTH_FIELD_ISBN
    };
}

OUString const & SwAuthorityFieldType::GetAuthFieldName(ToxAuthorityField eType)
{
    if(!pAuthFieldNameList)
    {
        pAuthFieldNameList = new std::vector<OUString>;
        pAuthFieldNameList->reserve(AUTH_FIELD_END);
        for (sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i)
            pAuthFieldNameList->push_back(SwResId(STR_AUTH_FIELD_ARY[i]));
    }
    return (*pAuthFieldNameList)[static_cast< sal_uInt16 >(eType)];
}

static const char* STR_AUTH_TYPE_ARY[] =
{
    STR_AUTH_TYPE_ARTICLE,
    STR_AUTH_TYPE_BOOK,
    STR_AUTH_TYPE_BOOKLET,
    STR_AUTH_TYPE_CONFERENCE,
    STR_AUTH_TYPE_INBOOK,
    STR_AUTH_TYPE_INCOLLECTION,
    STR_AUTH_TYPE_INPROCEEDINGS,
    STR_AUTH_TYPE_JOURNAL,
    STR_AUTH_TYPE_MANUAL,
    STR_AUTH_TYPE_MASTERSTHESIS,
    STR_AUTH_TYPE_MISC,
    STR_AUTH_TYPE_PHDTHESIS,
    STR_AUTH_TYPE_PROCEEDINGS,
    STR_AUTH_TYPE_TECHREPORT,
    STR_AUTH_TYPE_UNPUBLISHED,
    STR_AUTH_TYPE_EMAIL,
    STR_AUTH_TYPE_WWW,
    STR_AUTH_TYPE_CUSTOM1,
    STR_AUTH_TYPE_CUSTOM2,
    STR_AUTH_TYPE_CUSTOM3,
    STR_AUTH_TYPE_CUSTOM4,
    STR_AUTH_TYPE_CUSTOM5
};

OUString const & SwAuthorityFieldType::GetAuthTypeName(ToxAuthorityType eType)
{
    if(!pAuthFieldTypeList)
    {
        pAuthFieldTypeList = new std::vector<OUString>;
        pAuthFieldTypeList->reserve(AUTH_TYPE_END);
        for (sal_uInt16 i = 0; i < AUTH_TYPE_END; ++i)
            pAuthFieldTypeList->push_back(SwResId(STR_AUTH_TYPE_ARY[i]));
    }
    return (*pAuthFieldTypeList)[static_cast< sal_uInt16 >(eType)];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
