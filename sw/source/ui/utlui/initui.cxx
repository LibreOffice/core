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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <unotools/localedatawrapper.hxx>
#include <viewsh.hxx>
#include <initui.hxx>
#include <edtwin.hxx>
#include <shellres.hxx>
#include <fldbas.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>

#ifndef _UTLUI_HRC
#include <utlui.hrc>
#endif
#ifndef _INITUI_HRC
#include <initui.hrc>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#include <authfld.hxx>
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#include <unotools/syslocale.hxx>

#include <unomid.h>

/*--------------------------------------------------------------------
    Beschreibung:   globale Pointer
 --------------------------------------------------------------------*/

SwGlossaries*       pGlossaries = 0;

// Liefert alle benoetigten Pfade. Wird durch UI initialisiert
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

SvStringsDtor* pDBNameList = 0;

SvStringsDtor*  pAuthFieldNameList = 0;
SvStringsDtor*  pAuthFieldTypeList = 0;

/*--------------------------------------------------------------------
    Beschreibung:   UI beenden
 --------------------------------------------------------------------*/

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
/*--------------------------------------------------------------------
    Beschreibung:   Initialisierung
 --------------------------------------------------------------------*/


void _InitUI()
{
    // ShellResource gibt der CORE die Moeglichkeit mit Resourcen zu arbeiten
    ViewShell::SetShellRes( new ShellResource );
    pDBNameList = new SvStringsDtor( 5, 5 );
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
    // --> OD 2007-09-13 #i81002#
    aGetRefFld_RefItemNotFound( SW_RES( STR_GETREFFLD_REFITEMNOTFOUND ) ),
    // <--
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
    aHyperlinkClick( SW_RES( STR_HYPERLINK_CLICK)),
    pAutoFmtNameLst(0),
    sPageDescFirstName(     SW_RES(STR_PAGEDESC_FIRSTNAME)),
    sPageDescFollowName(    SW_RES(STR_PAGEDESC_FOLLOWNAME)),
    sPageDescName(          SW_RES(STR_PAGEDESC_NAME))
{
    const sal_uInt16 nCount = FLD_DOCINFO_END - FLD_DOCINFO_BEGIN;

    KeyCode aCode( KEY_SPACE );
    KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
    String aModStr( aModifiedCode.GetName() );
    aModStr.SearchAndReplace( aCode.GetName(), String() );
    aModStr.SearchAndReplaceAllAscii( "+", String() );
    aHyperlinkClick.SearchAndReplaceAllAscii( "%s", aModStr );

    for(sal_uInt16 i = 0; i < nCount; ++i)
    {
        String* pNew = new SW_RESSTR(FLD_DOCINFO_BEGIN + i);
        aDocInfoLst.Insert(pNew, aDocInfoLst.Count());
    }

    FreeResource();
}

ShellResource::~ShellResource()
{
    if( pAutoFmtNameLst )
        delete pAutoFmtNameLst, pAutoFmtNameLst = 0;
}

String ShellResource::GetPageDescName( sal_uInt16 nNo, sal_Bool bIsFirst, sal_Bool bFollow )
{
    String sRet( bIsFirst ? sPageDescFirstName
                          : bFollow ? sPageDescFollowName
                                      : sPageDescName );
    sRet.SearchAndReplaceAscii( "$(ARG1)", String::CreateFromInt32( nNo ));
    return sRet;
}


SwGlossaries* GetGlossaries()
{
    if (!pGlossaries)
        pGlossaries = new SwGlossaries;
    return (pGlossaries);
}

sal_Bool HasGlossaryList()
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
    ImpAutoFmtNameListLoader( SvStringsDtor& rLst );
};

void ShellResource::_GetAutoFmtNameLst() const
{
    SvStringsDtor** ppLst = (SvStringsDtor**)&pAutoFmtNameLst;
    *ppLst = new SvStringsDtor( STR_AUTOFMTREDL_END );
    ImpAutoFmtNameListLoader aTmp( **ppLst );
}

ImpAutoFmtNameListLoader::ImpAutoFmtNameListLoader( SvStringsDtor& rLst )
    : Resource( ResId(RID_SHELLRES_AUTOFMTSTRS, *pSwResMgr) )
{
    for( sal_uInt16 n = 0; n < STR_AUTOFMTREDL_END; ++n )
    {
        String* p = new String( ResId( n + 1, *pSwResMgr) );
        if(STR_AUTOFMTREDL_TYPO == n)
        {
#ifdef WNT
            //fuer Windows Sonderbehandlung, da MS hier ein paar Zeichen im Dialogfont vergessen hat
            p->SearchAndReplace(C2S("%1"), C2S(",,"));
            p->SearchAndReplace(C2S("%2"), C2S("''"));
#else
            const SvtSysLocale aSysLocale;
            const LocaleDataWrapper& rLclD = aSysLocale.GetLocaleData();
            //unter richtigen Betriebssystemen funktioniert es auch so
            p->SearchAndReplace(C2S("%1"), rLclD.getDoubleQuotationMarkStart());
            p->SearchAndReplace(C2S("%2"), rLclD.getDoubleQuotationMarkEnd());
#endif
        }
        rLst.Insert( p, n );
    }
    FreeResource();
}
/* -----------------16.09.99 12:28-------------------

 --------------------------------------------------*/
const String&   SwAuthorityFieldType::GetAuthFieldName(ToxAuthorityField eType)
{
    if(!pAuthFieldNameList)
    {
        pAuthFieldNameList = new SvStringsDtor(AUTH_FIELD_END, 1);
        for(sal_uInt16 i = 0; i < AUTH_FIELD_END; i++)
        {
            String*  pTmp = new String(SW_RES(STR_AUTH_FIELD_START + i));
            pAuthFieldNameList->Insert(pTmp, pAuthFieldNameList->Count());
        }
    }
    return *pAuthFieldNameList->GetObject( static_cast< sal_uInt16 >(eType) );
}
/* -----------------16.09.99 12:29-------------------

 --------------------------------------------------*/
const String&   SwAuthorityFieldType::GetAuthTypeName(ToxAuthorityType eType)
{
    if(!pAuthFieldTypeList)
    {
        pAuthFieldTypeList = new SvStringsDtor(AUTH_TYPE_END, 1);
        for(sal_uInt16 i = 0; i < AUTH_TYPE_END; i++)
            pAuthFieldTypeList->Insert(
                new String(SW_RES(STR_AUTH_TYPE_START + i)),
                                    pAuthFieldTypeList->Count());
    }
    return *pAuthFieldTypeList->GetObject( static_cast< sal_uInt16 >(eType) );
}



