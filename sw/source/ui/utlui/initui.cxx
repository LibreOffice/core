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

SwGlossaries*       pGlossaries = 0;

// Provides all needed paths. Is initialized by UI.
SwGlossaryList*     pGlossaryList = 0;

String* pOldGrfCat = 0;
String* pOldTabCat = 0;
String* pOldFrmCat = 0;
String* pOldDrwCat = 0;
String* pCurrGlosGroup = 0;

String* GetOldGrfCat()
{
    return pOldGrfCat;
}
String* GetOldTabCat()
{
    return pOldTabCat;
}
String* GetOldFrmCat()
{
    return pOldFrmCat;
}
String* GetOldDrwCat()
{
    return pOldDrwCat;
}
String* GetCurrGlosGroup()
{
    return pCurrGlosGroup;
}
void SetCurrGlosGroup(String* pStr)
{
    pCurrGlosGroup = pStr;
}

std::vector<String>* pDBNameList = 0;

namespace
{

std::vector<OUString>* pAuthFieldNameList = 0;
std::vector<OUString>* pAuthFieldTypeList = 0;

}

// Finish UI

void _FinitUI()
{
    SwNewDBMgr::RemoveDbtoolsClient();
    delete ViewShell::GetShellRes();
    ViewShell::SetShellRes( 0 );

    SwEditWin::_FinitStaticData();

    DELETEZ(pGlossaries);

    delete SwFieldType::pFldNames;

    delete pOldGrfCat;
    delete pOldTabCat;
    delete pOldFrmCat;
    delete pOldDrwCat;
    delete pCurrGlosGroup;
    delete pDBNameList;
    delete pGlossaryList;
    delete pAuthFieldNameList;
    delete pAuthFieldTypeList;

}

// Initialise

void _InitUI()
{
    // ShellResource gives the CORE the possibility to work with resources.
    ViewShell::SetShellRes( new ShellResource );
    pDBNameList = new std::vector<String>;
    SwEditWin::_InitStaticData();
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

    aGetRefFld_Up( SW_RES( STR_GETREFFLD_UP ) ),
    aGetRefFld_Down( SW_RES( STR_GETREFFLD_DOWN ) ),
    // #i81002#
    aGetRefFld_RefItemNotFound( SW_RES( STR_GETREFFLD_REFITEMNOTFOUND ) ),
    aStrAllPageHeadFoot( SW_RES( STR_ALLPAGE_HEADFOOT ) ),
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
    aHyperlinkClick(SW_RESSTR(STR_HYPERLINK_CLICK)),
    pAutoFmtNameLst(0),
    sPageDescFirstName(     SW_RES(STR_PAGEDESC_FIRSTNAME)),
    sPageDescFollowName(    SW_RES(STR_PAGEDESC_FOLLOWNAME)),
    sPageDescName(          SW_RES(STR_PAGEDESC_NAME))
{
    const sal_uInt16 nCount = FLD_DOCINFO_END - FLD_DOCINFO_BEGIN;

    KeyCode aCode( KEY_SPACE );
    KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
    OUString aModStr( aModifiedCode.GetName() );
    aModStr = aModStr.replaceFirst(aCode.GetName(), OUString());
    aModStr = aModStr.replaceAll("+", OUString());
    aHyperlinkClick = aHyperlinkClick.replaceAll("%s", aModStr);

    for(sal_uInt16 i = 0; i < nCount; ++i)
        aDocInfoLst.push_back(String(SW_RESSTR(FLD_DOCINFO_BEGIN + i)));

    FreeResource();
}

ShellResource::~ShellResource()
{
    delete pAutoFmtNameLst;
}

String ShellResource::GetPageDescName( sal_uInt16 nNo, PageNameMode eMode )
{
    String sRet;

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

    sRet.SearchAndReplaceAscii( "$(ARG1)", OUString::number( nNo ));
    return sRet;
}


SwGlossaries* GetGlossaries()
{
    if (!pGlossaries)
        pGlossaries = new SwGlossaries;
    return (pGlossaries);
}

bool HasGlossaryList()
{
    return pGlossaryList != 0;
}

SwGlossaryList* GetGlossaryList()
{
    if(!pGlossaryList)
        pGlossaryList = new SwGlossaryList();

    return pGlossaryList;
}

struct ImpAutoFmtNameListLoader : public Resource
{
    ImpAutoFmtNameListLoader( std::vector<String>& rLst );
};

void ShellResource::_GetAutoFmtNameLst() const
{
    assert(!pAutoFmtNameLst);
    pAutoFmtNameLst = new std::vector<String>;
    pAutoFmtNameLst->reserve(STR_AUTOFMTREDL_END);
    ImpAutoFmtNameListLoader aTmp(*pAutoFmtNameLst);
}

ImpAutoFmtNameListLoader::ImpAutoFmtNameListLoader( std::vector<String>& rLst )
    : Resource( ResId(RID_SHELLRES_AUTOFMTSTRS, *pSwResMgr) )
{
    for( sal_uInt16 n = 0; n < STR_AUTOFMTREDL_END; ++n )
    {
        String p(ResId(n + 1, *pSwResMgr));
        if(STR_AUTOFMTREDL_TYPO == n)
        {
#ifdef WNT
            // For Windows, a special treatment is necessary because MS has
            // forgotten some characters in the dialog font here.
            p.SearchAndReplace(OUString("%1"), OUString(",,"));
            p.SearchAndReplace(OUString("%2"), OUString("''"));
#else
            const SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLclD = aSysLocale.GetLocaleData();
            // With real operating systems it also works without special handling.
            p.SearchAndReplace(OUString("%1"), rLclD.getDoubleQuotationMarkStart());
            p.SearchAndReplace(OUString("%2"), rLclD.getDoubleQuotationMarkEnd());
#endif
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
