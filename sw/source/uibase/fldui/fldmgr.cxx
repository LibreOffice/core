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

#include <cmdid.h>
#include <hintids.hxx>
#include <svl/stritem.hxx>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/localedatawrapper.hxx>
#include <tools/resary.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/app.hxx>
#include <svx/dialogs.hrc>
#include <svx/strarray.hxx>
#include <basic/basmgr.hxx>
#include <editeng/langitem.hxx>
#include <svl/macitem.hxx>
#include <basic/sbmod.hxx>
#include <fmtrfmrk.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbx.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <vcl/mnemonic.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <charatr.hxx>
#include <fmtinfmt.hxx>
#include <cellatr.hxx>
#include <dbmgr.hxx>
#include <shellres.hxx>
#include <fldbas.hxx>
#include <docufld.hxx>
#include <chpfld.hxx>
#include <ddefld.hxx>
#include <expfld.hxx>
#include <reffld.hxx>
#include <usrfld.hxx>
#include <dbfld.hxx>
#include <authfld.hxx>
#include <flddat.hxx>
#include <fldmgr.hxx>
#include <flddropdown.hxx>
#include <strings.hrc>
#include <tox.hxx>
#include <cnttab.hxx>
#include <unotools/useroptions.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::text;
using namespace com::sun::star::style;
using namespace com::sun::star::sdbc;
using namespace ::com::sun::star;
using namespace nsSwDocInfoSubType;

// groups of fields
enum
{
    GRP_DOC_BEGIN   =  0,
    GRP_DOC_END     =  GRP_DOC_BEGIN + 12,

    GRP_FKT_BEGIN   =  GRP_DOC_END,
    GRP_FKT_END     =  GRP_FKT_BEGIN + 8,

    GRP_REF_BEGIN   =  GRP_FKT_END,
    GRP_REF_END     =  GRP_REF_BEGIN + 2,

    GRP_REG_BEGIN   =  GRP_REF_END,
    GRP_REG_END     =  GRP_REG_BEGIN + 1,

    GRP_DB_BEGIN    =  GRP_REG_END,
    GRP_DB_END      =  GRP_DB_BEGIN  + 5,

    GRP_VAR_BEGIN   =  GRP_DB_END,
    GRP_VAR_END     =  GRP_VAR_BEGIN + 9
};

enum
{
    GRP_WEB_DOC_BEGIN   =  0,
    GRP_WEB_DOC_END     =  GRP_WEB_DOC_BEGIN + 9,

    GRP_WEB_FKT_BEGIN   =  GRP_WEB_DOC_END + 2,
    GRP_WEB_FKT_END     =  GRP_WEB_FKT_BEGIN + 0,   // the group is empty!

    GRP_WEB_REF_BEGIN   =  GRP_WEB_FKT_END + 6,     // the group is empty!
    GRP_WEB_REF_END     =  GRP_WEB_REF_BEGIN + 0,

    GRP_WEB_REG_BEGIN   =  GRP_WEB_REF_END + 2,
    GRP_WEB_REG_END     =  GRP_WEB_REG_BEGIN + 1,

    GRP_WEB_DB_BEGIN    =  GRP_WEB_REG_END,         // the group is empty!
    GRP_WEB_DB_END      =  GRP_WEB_DB_BEGIN  + 0,

    GRP_WEB_VAR_BEGIN   =  GRP_WEB_DB_END + 5,
    GRP_WEB_VAR_END     =  GRP_WEB_VAR_BEGIN + 1
};

static const sal_uInt16 VF_COUNT = 1; // { 0 }
static const sal_uInt16 VF_USR_COUNT = 2; // { 0, nsSwExtendedSubType::SUB_CMD }
static const sal_uInt16 VF_DB_COUNT = 1; // { nsSwExtendedSubType::SUB_OWN_FMT }

static const char* FLD_EU_ARY[] =
{
    FLD_EU_FIRMA,
    FLD_EU_VORNAME,
    FLD_EU_NAME,
    FLD_EU_ABK,
    FLD_EU_STRASSE,
    FLD_EU_LAND,
    FLD_EU_PLZ,
    FLD_EU_ORT,
    FLD_EU_TITEL,
    FLD_EU_POS,
    FLD_EU_TELPRIV,
    FLD_EU_TELFIRMA,
    FLD_EU_FAX,
    FLD_EU_EMAIL,
    FLD_EU_STATE
};

static const char* FMT_AUTHOR_ARY[] =
{
    FMT_AUTHOR_NAME,
    FMT_AUTHOR_SCUT
};

static const char* FLD_DATE_ARY[] =
{
    FLD_DATE_FIX,
    FLD_DATE_STD,
};

static const char* FLD_TIME_ARY[] =
{
    FLD_TIME_FIX,
    FLD_TIME_STD
};

static const char* FMT_NUM_ARY[] =
{
    FMT_NUM_ABC,
    FMT_NUM_SABC,
    FMT_NUM_ABC_N,
    FMT_NUM_SABC_N,
    FMT_NUM_ROMAN,
    FMT_NUM_SROMAN,
    FMT_NUM_ARABIC,
    FMT_NUM_PAGEDESC,
    FMT_NUM_PAGESPECIAL
};

static const char* FMT_FF_ARY[] =
{
    FMT_FF_NAME,
    FMT_FF_PATHNAME,
    FMT_FF_PATH,
    FMT_FF_NAME_NOEXT,
    FMT_FF_UI_NAME,
    FMT_FF_UI_RANGE
};

static const char* FLD_STAT_ARY[] =
{
    FLD_STAT_PAGE,
    FLD_STAT_PARA,
    FLD_STAT_WORD,
    FLD_STAT_CHAR,
    FLD_STAT_TABLE,
    FLD_STAT_GRF,
    FLD_STAT_OBJ
};

static const char* FMT_CHAPTER_ARY[] =
{
    FMT_CHAPTER_NO,
    FMT_CHAPTER_NAME,
    FMT_CHAPTER_NAMENO,
    FMT_CHAPTER_NO_NOSEPARATOR
};

static const char* FLD_INPUT_ARY[] =
{
    FLD_INPUT_TEXT
};

static const char* FMT_MARK_ARY[] =
{
    FMT_MARK_TEXT,
    FMT_MARK_TABLE,
    FMT_MARK_FRAME,
    FMT_MARK_GRAFIC,
    FMT_MARK_OLE
};

static const char* FMT_REF_ARY[] =
{
    FMT_REF_PAGE,
    FMT_REF_CHAPTER,
    FMT_REF_TEXT,
    FMT_REF_UPDOWN,
    FMT_REF_PAGE_PGDSC,
    FMT_REF_ONLYNUMBER,
    FMT_REF_ONLYCAPTION,
    FMT_REF_ONLYSEQNO,
    FMT_REF_NUMBER,
    FMT_REF_NUMBER_NO_CONTEXT,
    FMT_REF_NUMBER_FULL_CONTEXT
};

static const char* FMT_REG_ARY[] =
{
    FMT_REG_AUTHOR,
    FMT_REG_TIME,
    FMT_REG_DATE
};

static const char* FMT_DBFLD_ARY[] =
{
    FMT_DBFLD_DB,
    FMT_DBFLD_SYS
};

static const char* FMT_SETVAR_ARY[] =
{
    FMT_SETVAR_SYS,
    FMT_SETVAR_TEXT
};

static const char* FMT_GETVAR_ARY[] =
{
    FMT_GETVAR_TEXT,
    FMT_GETVAR_NAME
};

static const char* FMT_DDE_ARY[] =
{
    FMT_DDE_NORMAL,
    FMT_DDE_HOT
};

static const char* FLD_PAGEREF_ARY[] =
{
    FLD_PAGEREF_OFF,
    FLD_PAGEREF_ON
};

static const char* FMT_USERVAR_ARY[] =
{
    FMT_USERVAR_TEXT,
    FMT_USERVAR_CMD
};

// field types and subtypes
struct SwFieldPack
{
    sal_uInt16 const   nTypeId;

    const char**       pSubTypeResIds;
    size_t const       nSubTypeLength;

    const char**       pFormatResIds;
    size_t const       nFormatLength;
};

// strings and formats
static const SwFieldPack aSwFields[] =
{
    // Document
    { TYP_EXTUSERFLD,       FLD_EU_ARY,         SAL_N_ELEMENTS(FLD_EU_ARY),     nullptr,          0 },
    { TYP_AUTHORFLD,        nullptr,            0,                              FMT_AUTHOR_ARY,   SAL_N_ELEMENTS(FMT_AUTHOR_ARY) },
    { TYP_DATEFLD,          FLD_DATE_ARY,       SAL_N_ELEMENTS(FLD_DATE_ARY),   nullptr,          0 },
    { TYP_TIMEFLD,          FLD_TIME_ARY,       SAL_N_ELEMENTS(FLD_TIME_ARY),   nullptr,          0 },
    { TYP_PAGENUMBERFLD,    nullptr,            0,                              FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) -1 },
    { TYP_NEXTPAGEFLD,      nullptr,            0,                              FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) },
    { TYP_PREVPAGEFLD,      nullptr,            0,                              FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) },
    { TYP_FILENAMEFLD,      nullptr,            0,                              FMT_FF_ARY,       SAL_N_ELEMENTS(FMT_FF_ARY) },
    { TYP_DOCSTATFLD,       FLD_STAT_ARY,       SAL_N_ELEMENTS(FLD_STAT_ARY),   FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) -1 },

    { TYP_CHAPTERFLD,       nullptr,            0,                              FMT_CHAPTER_ARY,  SAL_N_ELEMENTS(FMT_CHAPTER_ARY) },
    { TYP_TEMPLNAMEFLD,     nullptr,            0,                              FMT_FF_ARY,       SAL_N_ELEMENTS(FMT_FF_ARY) },
    { TYP_PARAGRAPHSIGFLD,  nullptr,            0,                              nullptr,          0 },

    // Functions
    { TYP_CONDTXTFLD,       nullptr,            0,                              nullptr,          0 },
    { TYP_DROPDOWN,         nullptr,            0,                              nullptr,          0 },
    { TYP_INPUTFLD,         FLD_INPUT_ARY,      SAL_N_ELEMENTS(FLD_INPUT_ARY),  nullptr,          0 },
    { TYP_MACROFLD,         nullptr,            0,                              nullptr,          0 },
    { TYP_JUMPEDITFLD,      nullptr,            0,                              FMT_MARK_ARY,     SAL_N_ELEMENTS(FMT_MARK_ARY) },
    { TYP_COMBINED_CHARS,   nullptr,            0,                              nullptr,          0 },
    { TYP_HIDDENTXTFLD,     nullptr,            0,                              nullptr,          0 },
    { TYP_HIDDENPARAFLD,    nullptr,            0,                              nullptr,          0 },

    // Cross-References
    { TYP_SETREFFLD,        nullptr,            0,                              nullptr,          0 },
    { TYP_GETREFFLD,        nullptr,            0,                              FMT_REF_ARY,      SAL_N_ELEMENTS(FMT_REF_ARY) },

    // DocInformation
    { TYP_DOCINFOFLD,       nullptr,            0,                              FMT_REG_ARY,      SAL_N_ELEMENTS(FMT_REG_ARY) },

    // Database
    { TYP_DBFLD,            nullptr,            0,                              FMT_DBFLD_ARY,    SAL_N_ELEMENTS(FMT_DBFLD_ARY) },
    { TYP_DBNEXTSETFLD,     nullptr,            0,                              nullptr,          0 },
    { TYP_DBNUMSETFLD,      nullptr,            0,                              nullptr,          0 },
    { TYP_DBSETNUMBERFLD,   nullptr,            0,                              FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) - 2 },
    { TYP_DBNAMEFLD,        nullptr,            0,                              nullptr,          0 },

    // Variables
    { TYP_SETFLD,           nullptr,            0,                              FMT_SETVAR_ARY,   SAL_N_ELEMENTS(FMT_SETVAR_ARY) },

    { TYP_GETFLD,           nullptr,            0,                              FMT_GETVAR_ARY,   SAL_N_ELEMENTS(FMT_GETVAR_ARY) },
    { TYP_DDEFLD,           nullptr,            0,                              FMT_DDE_ARY,      SAL_N_ELEMENTS(FMT_DDE_ARY) },
    { TYP_FORMELFLD,        nullptr,            0,                              FMT_GETVAR_ARY,   SAL_N_ELEMENTS(FMT_GETVAR_ARY) },
    { TYP_INPUTFLD,         FLD_INPUT_ARY,      SAL_N_ELEMENTS(FLD_INPUT_ARY),  nullptr,          0 },
    { TYP_SEQFLD,           nullptr,            0,                              FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) - 2 },
    { TYP_SETREFPAGEFLD,    FLD_PAGEREF_ARY,    SAL_N_ELEMENTS(FLD_PAGEREF_ARY),nullptr,          0 },
    { TYP_GETREFPAGEFLD,    nullptr,            0,                              FMT_NUM_ARY,      SAL_N_ELEMENTS(FMT_NUM_ARY) - 1 },
    { TYP_USERFLD,          nullptr,            0,                              FMT_USERVAR_ARY,  SAL_N_ELEMENTS(FMT_USERVAR_ARY) }
};

// access to the shell
static SwWrtShell* lcl_GetShell()
{
    SwView* pView;
    if ( nullptr != (pView = ::GetActiveView()) )
        return pView->GetWrtShellPtr();
    OSL_FAIL("no current shell found!");
    return nullptr;
}

static sal_uInt16 GetPackCount() {  return SAL_N_ELEMENTS(aSwFields); }

// FieldManager controls inserting and updating of fields
SwFieldMgr::SwFieldMgr(SwWrtShell* pSh ) :
    m_pWrtShell(pSh),
    m_bEvalExp(true)
{
    // determine current field if existing
    GetCurField();
}

SwFieldMgr::~SwFieldMgr()
{
}

// organise RefMark by names
bool  SwFieldMgr::CanInsertRefMark( const OUString& rStr )
{
    bool bRet = false;
    SwWrtShell *pSh = m_pWrtShell ? m_pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(pSh)
    {
        sal_uInt16 nCnt = pSh->GetCursorCnt();

        // the last Cursor doesn't have to be a spanned selection
        if( 1 < nCnt && !pSh->SwCursorShell::HasSelection() )
            --nCnt;

        bRet =  2 > nCnt && nullptr == pSh->GetRefMark( rStr );
    }
    return bRet;
}

// access over ResIds
void SwFieldMgr::RemoveFieldType(SwFieldIds nResId, const OUString& rName )
{
    SwWrtShell * pSh = m_pWrtShell ? m_pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if( pSh )
        pSh->RemoveFieldType(nResId, rName);
}

size_t SwFieldMgr::GetFieldTypeCount() const
{
    SwWrtShell * pSh = m_pWrtShell ? m_pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFieldTypeCount() : 0;
}

SwFieldType* SwFieldMgr::GetFieldType(SwFieldIds nResId, size_t nField) const
{
    SwWrtShell * pSh = m_pWrtShell ? m_pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFieldType(nField, nResId) : nullptr;
}

SwFieldType* SwFieldMgr::GetFieldType(SwFieldIds nResId, const OUString& rName) const
{
    SwWrtShell * pSh = m_pWrtShell ? m_pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFieldType(nResId, rName) : nullptr;
}

// determine current field
SwField* SwFieldMgr::GetCurField()
{
    SwWrtShell *pSh = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();
    if ( pSh )
        m_pCurField = pSh->GetCurField( true );
    else
        m_pCurField = nullptr;

    // initialise strings and format
    m_aCurPar1.clear();
    m_aCurPar2.clear();
    m_sCurFrame.clear();
    m_nCurFormat = 0;

    if(!m_pCurField)
        return nullptr;

    // preprocess current values; determine parameter 1 and parameter 2
    // as well as the format
    const sal_uInt16 nTypeId = m_pCurField->GetTypeId();

    m_nCurFormat     = m_pCurField->GetFormat();
    m_aCurPar1    = m_pCurField->GetPar1();
    m_aCurPar2    = m_pCurField->GetPar2();

    switch( nTypeId )
    {
        case TYP_PAGENUMBERFLD:
        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_GETREFPAGEFLD:
            if( m_nCurFormat == SVX_NUM_PAGEDESC )
                m_nCurFormat -= 2;
            break;
    }
    return m_pCurField;
}

// provide group range
const SwFieldGroupRgn& SwFieldMgr::GetGroupRange(bool bHtmlMode, sal_uInt16 nGrpId)
{
    static SwFieldGroupRgn const aRanges[] =
    {
        { /* Document   */  GRP_DOC_BEGIN,  GRP_DOC_END },
        { /* Functions  */  GRP_FKT_BEGIN,  GRP_FKT_END },
        { /* Cross-Refs */  GRP_REF_BEGIN,  GRP_REF_END },
        { /* DocInfos   */  GRP_REG_BEGIN,  GRP_REG_END },
        { /* Database   */  GRP_DB_BEGIN,   GRP_DB_END  },
        { /* User       */  GRP_VAR_BEGIN,  GRP_VAR_END }
    };
    static SwFieldGroupRgn const aWebRanges[] =
    {
        { /* Document    */  GRP_WEB_DOC_BEGIN,  GRP_WEB_DOC_END },
        { /* Functions   */  GRP_WEB_FKT_BEGIN,  GRP_WEB_FKT_END },
        { /* Cross-Refs  */  GRP_WEB_REF_BEGIN,  GRP_WEB_REF_END },
        { /* DocInfos    */  GRP_WEB_REG_BEGIN,  GRP_WEB_REG_END },
        { /* Database    */  GRP_WEB_DB_BEGIN,   GRP_WEB_DB_END  },
        { /* User        */  GRP_WEB_VAR_BEGIN,  GRP_WEB_VAR_END }
    };

    if (bHtmlMode)
        return aWebRanges[nGrpId];
    else
        return aRanges[nGrpId];
}

// determine GroupId
sal_uInt16 SwFieldMgr::GetGroup(sal_uInt16 nTypeId, sal_uInt16 nSubType)
{
    if (nTypeId == TYP_SETINPFLD)
        nTypeId = TYP_SETFLD;

    if (nTypeId == TYP_INPUTFLD && (nSubType & INP_USR))
        nTypeId = TYP_USERFLD;

    if (nTypeId == TYP_FIXDATEFLD)
        nTypeId = TYP_DATEFLD;

    if (nTypeId == TYP_FIXTIMEFLD)
        nTypeId = TYP_TIMEFLD;

    for (sal_uInt16 i = GRP_DOC; i <= GRP_VAR; i++)
    {
        const SwFieldGroupRgn& rRange = GetGroupRange(false/*bHtmlMode*/, i);
        for (sal_uInt16 nPos = rRange.nStart; nPos < rRange.nEnd; nPos++)
        {
            if (aSwFields[nPos].nTypeId == nTypeId)
                return i;
        }
    }
    return USHRT_MAX;
}

// determine names to TypeId
//  ACCESS over TYP_....
sal_uInt16 SwFieldMgr::GetTypeId(sal_uInt16 nPos)
{
    OSL_ENSURE(nPos < ::GetPackCount(), "forbidden Pos");
    return aSwFields[ nPos ].nTypeId;
}

OUString SwFieldMgr::GetTypeStr(sal_uInt16 nPos)
{
    OSL_ENSURE(nPos < ::GetPackCount(), "forbidden TypeId");

    sal_uInt16 nFieldWh = aSwFields[ nPos ].nTypeId;

    // special treatment for date/time fields (without var/fix)
    if( TYP_DATEFLD == nFieldWh )
    {
        static OUString g_aDate( SwResId( STR_DATEFLD ) );
        return g_aDate;
    }
    if( TYP_TIMEFLD == nFieldWh )
    {
        static OUString g_aTime( SwResId( STR_TIMEFLD ) );
        return g_aTime;
    }

    return SwFieldType::GetTypeStr( nFieldWh );
}

// determine Pos in the list
sal_uInt16 SwFieldMgr::GetPos(sal_uInt16 nTypeId)
{
    switch( nTypeId )
    {
        case TYP_FIXDATEFLD:        nTypeId = TYP_DATEFLD;      break;
        case TYP_FIXTIMEFLD:        nTypeId = TYP_TIMEFLD;      break;
        case TYP_SETINPFLD:         nTypeId = TYP_SETFLD;       break;
        case TYP_USRINPFLD:         nTypeId = TYP_USERFLD;      break;
    }

    for(sal_uInt16 i = 0; i < GetPackCount(); i++)
        if(aSwFields[i].nTypeId == nTypeId)
            return i;

    return USHRT_MAX;
}

// localise subtypes of a field
void SwFieldMgr::GetSubTypes(sal_uInt16 nTypeId, std::vector<OUString>& rToFill)
{
    SwWrtShell *pSh = m_pWrtShell ? m_pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(pSh)
    {
        const sal_uInt16 nPos = GetPos(nTypeId);

        switch(nTypeId)
        {
            case TYP_SETREFFLD:
            case TYP_GETREFFLD:
            {
                // references are no fields
                pSh->GetRefMarks( &rToFill );
                break;
            }
            case TYP_MACROFLD:
            {
                break;
            }
            case TYP_INPUTFLD:
            {
                rToFill.push_back(SwResId(aSwFields[nPos].pSubTypeResIds[0]));
                [[fallthrough]]; // move on at generic types
            }
            case TYP_DDEFLD:
            case TYP_SEQFLD:
            case TYP_FORMELFLD:
            case TYP_GETFLD:
            case TYP_SETFLD:
            case TYP_USERFLD:
            {

                const size_t nCount = pSh->GetFieldTypeCount();
                for(size_t i = 0; i < nCount; ++i)
                {
                    SwFieldType* pFieldType = pSh->GetFieldType( i );
                    const SwFieldIds nWhich = pFieldType->Which();

                    if((nTypeId == TYP_DDEFLD && pFieldType->Which() == SwFieldIds::Dde) ||

                       (nTypeId == TYP_USERFLD && nWhich == SwFieldIds::User) ||

                       (nTypeId == TYP_GETFLD && nWhich == SwFieldIds::SetExp &&
                        !(static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       (nTypeId == TYP_SETFLD && nWhich == SwFieldIds::SetExp &&
                        !(static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       (nTypeId == TYP_SEQFLD && nWhich == SwFieldIds::SetExp  &&
                       (static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       ((nTypeId == TYP_INPUTFLD || nTypeId == TYP_FORMELFLD) &&
                         (nWhich == SwFieldIds::User ||
                          (nWhich == SwFieldIds::SetExp &&
                          !(static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ))) ) )
                    {
                        rToFill.push_back(pFieldType->GetName());
                    }
                }
                break;
            }
            case TYP_DBNEXTSETFLD:
            case TYP_DBNUMSETFLD:
            case TYP_DBNAMEFLD:
            case TYP_DBSETNUMBERFLD:
                break;

            default:
            {
                // static SubTypes
                if(nPos != USHRT_MAX)
                {
                    sal_uInt16 nCount;
                    if (nTypeId == TYP_DOCINFOFLD)
                        nCount = DI_SUBTYPE_END - DI_SUBTYPE_BEGIN;
                    else
                        nCount = aSwFields[nPos].nSubTypeLength;

                    for(sal_uInt16 i = 0; i < nCount; ++i)
                    {
                        OUString sNew;
                        if (nTypeId == TYP_DOCINFOFLD)
                        {
                            if ( i == DI_CUSTOM )
                                sNew = SwResId(STR_CUSTOM_FIELD);
                            else
                                sNew = SwViewShell::GetShellRes()->aDocInfoLst[i];
                        }
                        else
                            sNew = SwResId(aSwFields[nPos].pSubTypeResIds[i]);

                        rToFill.push_back(sNew);
                    }
                }
            }
        }
    }
}

// determine format
//  ACCESS over TYP_....
sal_uInt16 SwFieldMgr::GetFormatCount(sal_uInt16 nTypeId, bool bHtmlMode) const
{
    OSL_ENSURE(nTypeId < TYP_END, "forbidden TypeId");
    {
        const sal_uInt16 nPos = GetPos(nTypeId);

        if (nPos == USHRT_MAX || (bHtmlMode && nTypeId == TYP_SETFLD))
            return 0;

        sal_uInt16 nCount = aSwFields[nPos].nFormatLength;

        if (nTypeId == TYP_FILENAMEFLD)
            nCount -= 2;  // no range or template

        const char** pStart = aSwFields[nPos].pFormatResIds;
        if (!pStart)
            return nCount;

        if (strcmp(*pStart, FMT_GETVAR_ARY[0]) == 0 || strcmp(*pStart, FMT_SETVAR_ARY[0]) == 0)
            return VF_COUNT;
        else if (strcmp(*pStart, FMT_USERVAR_ARY[0]) == 0)
            return VF_USR_COUNT;
        else if (strcmp(*pStart, FMT_DBFLD_ARY[0]) == 0)
            return VF_DB_COUNT;
        else if (strcmp(*pStart, FMT_NUM_ARY[0]) == 0)
        {
            GetNumberingInfo();
            if(m_xNumberingInfo.is())
            {
                Sequence<sal_Int16> aTypes = m_xNumberingInfo->getSupportedNumberingTypes();
                const sal_Int16* pTypes = aTypes.getConstArray();
                for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
                {
                    sal_Int16 nCurrent = pTypes[nType];
                    //skip all values below or equal to CHARS_LOWER_LETTER_N
                    if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                    {
                        // #i28073# it's not necessarily a sorted sequence
                        ++nCount;
                    }
                }
            }
            return nCount;
        }

        return nCount;
    }
}

// determine FormatString to a type
OUString SwFieldMgr::GetFormatStr(sal_uInt16 nTypeId, sal_uInt32 nFormatId) const
{
    OSL_ENSURE(nTypeId < TYP_END, "forbidden TypeId");
    const sal_uInt16 nPos = GetPos(nTypeId);

    if (nPos == USHRT_MAX)
        return OUString();

    const char** pStart = aSwFields[nPos].pFormatResIds;
    if (!pStart)
        return OUString();

    if (TYP_AUTHORFLD == nTypeId|| TYP_FILENAMEFLD == nTypeId)
        nFormatId &= ~static_cast<sal_uInt32>(FF_FIXED); // mask out Fixed-Flag

    if (nFormatId < aSwFields[nPos].nFormatLength)
        return SwResId(pStart[nFormatId]);

    OUString aRet;
    if (*pStart == FMT_NUM_ARY[0])
    {
        if (m_xNumberingInfo.is())
        {
            Sequence<sal_Int16> aTypes = m_xNumberingInfo->getSupportedNumberingTypes();
            const sal_Int16* pTypes = aTypes.getConstArray();
            sal_Int32 nOffset = aSwFields[nPos].nFormatLength;
            sal_uInt32 nValidEntry = 0;
            for (sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N &&
                        (nCurrent != (NumberingType::BITMAP | LINK_TOKEN)))
                {
                    if (nValidEntry == nFormatId - nOffset)
                    {
                        sal_uInt32 n = SvxNumberingTypeTable::FindIndex(pTypes[nType]);
                        if (n != RESARRAY_INDEX_NOTFOUND)
                        {
                            aRet = SvxNumberingTypeTable::GetString(n);
                        }
                        else
                        {
                            aRet = m_xNumberingInfo->getNumberingIdentifier( pTypes[nType] );
                        }
                        break;
                    }
                    ++nValidEntry;
                }
            }
        }
    }

    return aRet;
}

// determine FormatId from Pseudo-ID
sal_uInt16 SwFieldMgr::GetFormatId(sal_uInt16 nTypeId, sal_uInt32 nFormatId) const
{
    sal_uInt16 nId = static_cast<sal_uInt16>(nFormatId);
    switch( nTypeId )
    {
        case TYP_DOCINFOFLD:
        {
            const OString sId(aSwFields[GetPos(nTypeId)].pFormatResIds[nFormatId]);
            if (sId == FMT_REG_AUTHOR)
                nId = DI_SUB_AUTHOR;
            else if (sId == FMT_REG_TIME)
                nId = DI_SUB_TIME;
            else if (sId == FMT_REG_DATE)
                nId = DI_SUB_DATE;
            break;
        }
        case TYP_PAGENUMBERFLD:
        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_DOCSTATFLD:
        case TYP_DBSETNUMBERFLD:
        case TYP_SEQFLD:
        case TYP_GETREFPAGEFLD:
        {
            sal_uInt16 nPos = GetPos(nTypeId);
            if (nFormatId < aSwFields[nPos].nFormatLength)
            {
                const OString sId(aSwFields[nPos].pFormatResIds[nFormatId]);
                if (sId == FMT_NUM_ABC)
                    nId = SVX_NUM_CHARS_UPPER_LETTER;
                else if (sId == FMT_NUM_SABC)
                    nId = SVX_NUM_CHARS_LOWER_LETTER;
                else if (sId == FMT_NUM_ROMAN)
                    nId = SVX_NUM_ROMAN_UPPER;
                else if (sId == FMT_NUM_SROMAN)
                    nId = SVX_NUM_ROMAN_LOWER;
                else if (sId == FMT_NUM_ARABIC)
                    nId = SVX_NUM_ARABIC;
                else if (sId == FMT_NUM_PAGEDESC)
                    nId = SVX_NUM_PAGEDESC;
                else if (sId == FMT_NUM_PAGESPECIAL)
                    nId = SVX_NUM_CHAR_SPECIAL;
                else if (sId == FMT_NUM_ABC_N)
                    nId = SVX_NUM_CHARS_UPPER_LETTER_N;
                else if (sId == FMT_NUM_SABC_N)
                    nId = SVX_NUM_CHARS_LOWER_LETTER_N;
            }
            else if (m_xNumberingInfo.is())
            {
                Sequence<sal_Int16> aTypes = m_xNumberingInfo->getSupportedNumberingTypes();
                const sal_Int16* pTypes = aTypes.getConstArray();
                sal_Int32 nOffset = aSwFields[nPos].nFormatLength;
                sal_Int32 nValidEntry = 0;
                for (sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
                {
                    sal_Int16 nCurrent = pTypes[nType];
                    if (nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                    {
                        if (nValidEntry == static_cast<sal_Int32>(nFormatId) - nOffset)
                        {
                            nId = pTypes[nType];
                            break;
                        }
                        ++nValidEntry;
                    }
                }
            }
            break;
        }
        case TYP_DDEFLD:
        {
            const OString sId(aSwFields[GetPos(nTypeId)].pFormatResIds[nFormatId]);
            if (sId == FMT_DDE_NORMAL)
                nId = static_cast<sal_uInt16>(SfxLinkUpdateMode::ONCALL);
            else if (sId == FMT_DDE_HOT)
                nId = static_cast<sal_uInt16>(SfxLinkUpdateMode::ALWAYS);
            break;
        }
    }
    return nId;
}

// Traveling
bool SwFieldMgr::GoNextPrev( bool bNext, SwFieldType* pTyp )
{
    SwWrtShell* pSh = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();
    if(!pSh)
        return false;

    if( !pTyp && m_pCurField )
    {
        const sal_uInt16 nTypeId = m_pCurField->GetTypeId();
        if( TYP_SETINPFLD == nTypeId || TYP_USRINPFLD == nTypeId )
            pTyp = pSh->GetFieldType( 0, SwFieldIds::Input );
        else
            pTyp = m_pCurField->GetTyp();
    }

    if (pTyp && pTyp->Which() == SwFieldIds::Database)
    {
        // for fieldcommand-edit (hop to all DB fields)
        return pSh->MoveFieldType( nullptr, bNext, SwFieldIds::Database );
    }

    return pTyp && pSh->MoveFieldType(pTyp, bNext);
}

// insert field types
void SwFieldMgr::InsertFieldType(SwFieldType const & rType)
{
    SwWrtShell* pSh = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(pSh)
        pSh->InsertFieldType(rType);
}

// determine current TypeId
sal_uInt16 SwFieldMgr::GetCurTypeId() const
{
    return m_pCurField ? m_pCurField->GetTypeId() : USHRT_MAX;
}

// Over string  insert field or update
bool SwFieldMgr::InsertField(
    const SwInsertField_Data& rData)
{
    std::unique_ptr<SwField> pField;
    bool bExp = false;
    bool bTable = false;
    bool bPageVar = false;
    sal_uInt32 nFormatId = rData.m_nFormatId;
    sal_uInt16 nSubType = rData.m_nSubType;
    sal_Unicode cSeparator = rData.m_cSeparator;
    SwWrtShell* pCurShell = rData.m_pSh;
    if(!pCurShell)
        pCurShell = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pCurShell, "no SwWrtShell found");
    if(!pCurShell)
        return false;

    switch (rData.m_nTypeId)
    {   // ATTENTION this field is inserted by a separate dialog
        case TYP_POSTITFLD:
        {
            SvtUserOptions aUserOpt;
            SwPostItFieldType* pType = static_cast<SwPostItFieldType*>(pCurShell->GetFieldType(0, SwFieldIds::Postit));
            pField.reset(
                new SwPostItField(
                    pType,
                    rData.m_sPar1, // author
                    rData.m_sPar2, // content
                    aUserOpt.GetID(), // author's initials
                    OUString(), // name
                    DateTime(DateTime::SYSTEM) ));
        }
        break;
        case TYP_SCRIPTFLD:
        {
            SwScriptFieldType* pType =
                static_cast<SwScriptFieldType*>(pCurShell->GetFieldType(0, SwFieldIds::Script));
            pField.reset(new SwScriptField(pType, rData.m_sPar1, rData.m_sPar2, static_cast<bool>(nFormatId)));
            break;
        }

    case TYP_COMBINED_CHARS:
        {
            SwCombinedCharFieldType* pType = static_cast<SwCombinedCharFieldType*>(
                pCurShell->GetFieldType( 0, SwFieldIds::CombinedChars ));
            pField.reset(new SwCombinedCharField( pType, rData.m_sPar1 ));
        }
        break;

    case TYP_AUTHORITY:
        {
            SwAuthorityFieldType* pType =
                static_cast<SwAuthorityFieldType*>(pCurShell->GetFieldType(0, SwFieldIds::TableOfAuthorities));
            if (!pType)
            {
                SwAuthorityFieldType const type(pCurShell->GetDoc());
                pType = static_cast<SwAuthorityFieldType*>(
                            pCurShell->InsertFieldType(type));
            }
            pField.reset(new SwAuthorityField(pType, rData.m_sPar1));
        }
        break;

    case TYP_DATEFLD:
    case TYP_TIMEFLD:
        {
            sal_uInt16 nSub = static_cast< sal_uInt16 >(rData.m_nTypeId == TYP_DATEFLD ? DATEFLD : TIMEFLD);
            nSub |= nSubType == DATE_VAR ? 0 : FIXEDFLD;

            SwDateTimeFieldType* pTyp =
                static_cast<SwDateTimeFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::DateTime) );
            pField.reset(new SwDateTimeField(pTyp, nSub, nFormatId));
            pField->SetPar2(rData.m_sPar2);
            break;
        }

    case TYP_FILENAMEFLD:
        {
            SwFileNameFieldType* pTyp =
                static_cast<SwFileNameFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::Filename) );
            pField.reset(new SwFileNameField(pTyp, nFormatId));
            break;
        }

    case TYP_TEMPLNAMEFLD:
        {
            SwTemplNameFieldType* pTyp =
                static_cast<SwTemplNameFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::TemplateName) );
            pField.reset(new SwTemplNameField(pTyp, nFormatId));
            break;
        }

    case TYP_CHAPTERFLD:
        {
            sal_uInt16 nByte = static_cast<sal_uInt16>(rData.m_sPar2.toInt32());
            SwChapterFieldType* pTyp =
                static_cast<SwChapterFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::Chapter) );
            pField.reset(new SwChapterField(pTyp, nFormatId));
            nByte = std::max(sal_uInt16(1), nByte);
            nByte = std::min(nByte, sal_uInt16(MAXLEVEL));
            nByte -= 1;
            static_cast<SwChapterField*>(pField.get())->SetLevel(static_cast<sal_uInt8>(nByte));
            break;
        }

    case TYP_NEXTPAGEFLD:
    case TYP_PREVPAGEFLD:
    case TYP_PAGENUMBERFLD:
        {
            short nOff = static_cast<short>(rData.m_sPar2.toInt32());

            if(rData.m_nTypeId == TYP_NEXTPAGEFLD)
            {
                if( SVX_NUM_CHAR_SPECIAL == nFormatId )
                    nOff = 1;
                else
                    nOff += 1;
                nSubType = PG_NEXT;
            }
            else if(rData.m_nTypeId == TYP_PREVPAGEFLD)
            {
                if( SVX_NUM_CHAR_SPECIAL == nFormatId )
                    nOff = -1;
                else
                    nOff -= 1;
                nSubType =  PG_PREV;
            }
            else
                nSubType = PG_RANDOM;

            SwPageNumberFieldType* pTyp =
                static_cast<SwPageNumberFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::PageNumber) );
            pField.reset(new SwPageNumberField(pTyp, nSubType, nFormatId, nOff));

            if( SVX_NUM_CHAR_SPECIAL == nFormatId &&
                ( PG_PREV == nSubType || PG_NEXT == nSubType ) )
                static_cast<SwPageNumberField*>(pField.get())->SetUserString( rData.m_sPar2 );
            break;
        }

    case TYP_DOCSTATFLD:
        {
            SwDocStatFieldType* pTyp =
                static_cast<SwDocStatFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::DocStat) );
            pField.reset(new SwDocStatField(pTyp, nSubType, nFormatId));
            break;
        }

    case TYP_AUTHORFLD:
        {
            SwAuthorFieldType* pTyp =
                static_cast<SwAuthorFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::Author) );
            pField.reset(new SwAuthorField(pTyp, nFormatId));
            break;
        }

    case TYP_CONDTXTFLD:
    case TYP_HIDDENTXTFLD:
        {
            SwHiddenTextFieldType* pTyp =
                static_cast<SwHiddenTextFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::HiddenText) );
            pField.reset(new SwHiddenTextField(pTyp, true, rData.m_sPar1, rData.m_sPar2, false, rData.m_nTypeId));
            bExp = true;
            break;
        }

    case TYP_HIDDENPARAFLD:
        {
            SwHiddenParaFieldType* pTyp =
                static_cast<SwHiddenParaFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::HiddenPara) );
            pField.reset(new SwHiddenParaField(pTyp, rData.m_sPar1));
            bExp = true;
            break;
        }

    case TYP_SETREFFLD:
        {
            if( !rData.m_sPar1.isEmpty() && CanInsertRefMark( rData.m_sPar1 ) )
            {
                pCurShell->SetAttrItem( SwFormatRefMark( rData.m_sPar1 ) );
                return true;
            }
            return false;
        }

    case TYP_GETREFFLD:
        {
            SwGetRefFieldType* pTyp =
                static_cast<SwGetRefFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::GetRef) );
            sal_uInt16 nSeqNo = static_cast<sal_uInt16>(rData.m_sPar2.toInt32());
            OUString sReferenceLanguage;
            // handle language-variant formats
            if (nFormatId >= SAL_N_ELEMENTS(FMT_REF_ARY))
            {
                LanguageType nLang = GetCurrLanguage();
                if (nLang == LANGUAGE_HUNGARIAN)
                {
                    if (nFormatId >= SAL_N_ELEMENTS(FMT_REF_ARY) * 2)
                        sReferenceLanguage = "Hu";
                    else
                        sReferenceLanguage = "hu";
                }
                nFormatId %= SAL_N_ELEMENTS(FMT_REF_ARY);
            }
            pField.reset(new SwGetRefField(pTyp, rData.m_sPar1, sReferenceLanguage, nSubType, nSeqNo, nFormatId));
            bExp = true;
            break;
        }

    case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
            //              That's not yet considered here.
            sal_Int32 nIndex = 0;
            OUString sCmd = rData.m_sPar2.replaceFirst(" ", OUStringLiteral1(sfx2::cTokenSeparator), &nIndex);
            if (nIndex>=0 && ++nIndex<sCmd.getLength())
            {
                sCmd = sCmd.replaceFirst(" ", OUStringLiteral1(sfx2::cTokenSeparator), &nIndex);
            }

            SwDDEFieldType aType( rData.m_sPar1, sCmd, static_cast<SfxLinkUpdateMode>(nFormatId) );
            SwDDEFieldType* pTyp = static_cast<SwDDEFieldType*>( pCurShell->InsertFieldType( aType ) );
            pField.reset(new SwDDEField( pTyp ));
            break;
        }

    case TYP_MACROFLD:
        {
            SwMacroFieldType* pTyp =
                static_cast<SwMacroFieldType*>(pCurShell->GetFieldType(0, SwFieldIds::Macro));

            pField.reset(new SwMacroField(pTyp, rData.m_sPar1, rData.m_sPar2));

            break;
        }

    case TYP_INTERNETFLD:
        {
            SwFormatINetFormat aFormat( rData.m_sPar1, m_sCurFrame );
            return pCurShell->InsertURL( aFormat, rData.m_sPar2 );
        }

    case TYP_JUMPEDITFLD:
        {
            SwJumpEditFieldType* pTyp =
                static_cast<SwJumpEditFieldType*>(pCurShell->GetFieldType(0, SwFieldIds::JumpEdit));

            pField.reset(new SwJumpEditField(pTyp, nFormatId, rData.m_sPar1, rData.m_sPar2));
            break;
        }

    case TYP_DOCINFOFLD:
        {
            SwDocInfoFieldType* pTyp = static_cast<SwDocInfoFieldType*>( pCurShell->GetFieldType(
                0, SwFieldIds::DocInfo ) );
            pField.reset(new SwDocInfoField(pTyp, nSubType, rData.m_sPar1, nFormatId));
            break;
        }

    case TYP_EXTUSERFLD:
        {
            SwExtUserFieldType* pTyp = static_cast<SwExtUserFieldType*>( pCurShell->GetFieldType(
                0, SwFieldIds::ExtUser) );
            pField.reset(new SwExtUserField(pTyp, nSubType, nFormatId));
            break;
        }

    case TYP_DBFLD:
        {
#if HAVE_FEATURE_DBCONNECTIVITY
            SwDBData aDBData;
            OUString sPar1;

            if (rData.m_sPar1.indexOf(DB_DELIM)<0)
            {
                aDBData = pCurShell->GetDBData();
                sPar1 = rData.m_sPar1;
            }
            else
            {
                sal_Int32 nIdx{ 0 };
                aDBData.sDataSource = rData.m_sPar1.getToken(0, DB_DELIM, nIdx);
                aDBData.sCommand = rData.m_sPar1.getToken(0, DB_DELIM, nIdx);
                aDBData.nCommandType = rData.m_sPar1.getToken(0, DB_DELIM, nIdx).toInt32();
                sPar1 = rData.m_sPar1.getToken(0, DB_DELIM, nIdx);
            }

            if(!aDBData.sDataSource.isEmpty() && pCurShell->GetDBData() != aDBData)
                pCurShell->ChgDBData(aDBData);

            SwDBFieldType* pTyp = static_cast<SwDBFieldType*>(pCurShell->InsertFieldType(
                SwDBFieldType(pCurShell->GetDoc(), sPar1, aDBData) ) );
            pField.reset(new SwDBField(pTyp));
            pField->SetSubType(nSubType);

            if( !(nSubType & nsSwExtendedSubType::SUB_OWN_FMT) ) // determine database format
            {
                Reference< XDataSource> xSource;
                rData.m_aDBDataSource >>= xSource;
                Reference<XConnection> xConnection;
                rData.m_aDBConnection >>= xConnection;
                Reference<XPropertySet> xColumn;
                rData.m_aDBColumn >>= xColumn;
                if(xColumn.is())
                {
                    nFormatId = SwDBManager::GetColumnFormat(xSource, xConnection, xColumn,
                        pCurShell->GetNumberFormatter(), GetCurrLanguage() );
                }
                else
                    nFormatId = pCurShell->GetDBManager()->GetColumnFormat(
                    aDBData.sDataSource, aDBData.sCommand, sPar1,
                    pCurShell->GetNumberFormatter(), GetCurrLanguage() );
            }
            pField->ChangeFormat( nFormatId );

            bExp = true;
#endif
            break;
        }

    case TYP_DBSETNUMBERFLD:
    case TYP_DBNUMSETFLD:
    case TYP_DBNEXTSETFLD:
    case TYP_DBNAMEFLD:
        {
#if HAVE_FEATURE_DBCONNECTIVITY
            SwDBData aDBData;

            // extract DBName from rData.m_sPar1. Format: DBName.TableName.CommandType.ExpStrg
            sal_Int32 nTablePos = rData.m_sPar1.indexOf(DB_DELIM);
            sal_Int32 nCmdTypePos = -1;
            sal_Int32 nExpPos = -1;

            if (nTablePos>=0)
            {
                aDBData.sDataSource = rData.m_sPar1.copy(0, nTablePos++);
                nCmdTypePos = rData.m_sPar1.indexOf(DB_DELIM, nTablePos);
                if (nCmdTypePos>=0)
                {
                    aDBData.sCommand = rData.m_sPar1.copy(nTablePos, nCmdTypePos++ - nTablePos);
                    nExpPos = rData.m_sPar1.indexOf(DB_DELIM, nCmdTypePos);
                    if (nExpPos>=0)
                    {
                        aDBData.nCommandType = rData.m_sPar1.copy(nCmdTypePos, nExpPos++ - nCmdTypePos).toInt32();
                    }
                }
            }

            sal_Int32 nPos = 0;
            if (nExpPos>=0)
                nPos = nExpPos;
            else if (nTablePos>=0)
                nPos = nTablePos;

            OUString sPar1 = rData.m_sPar1.copy(nPos);

            if (!aDBData.sDataSource.isEmpty() && pCurShell->GetDBData() != aDBData)
                pCurShell->ChgDBData(aDBData);

            switch(rData.m_nTypeId)
            {
            case TYP_DBNAMEFLD:
                {
                    SwDBNameFieldType* pTyp =
                        static_cast<SwDBNameFieldType*>(pCurShell->GetFieldType(0, SwFieldIds::DatabaseName));
                    pField.reset(new SwDBNameField(pTyp, aDBData));

                    break;
                }
            case TYP_DBNEXTSETFLD:
                {
                    SwDBNextSetFieldType* pTyp = static_cast<SwDBNextSetFieldType*>(pCurShell->GetFieldType(
                        0, SwFieldIds::DbNextSet) );
                    pField.reset(new SwDBNextSetField(pTyp, sPar1, aDBData));
                    bExp = true;
                    break;
                }
            case TYP_DBNUMSETFLD:
                {
                    SwDBNumSetFieldType* pTyp = static_cast<SwDBNumSetFieldType*>( pCurShell->GetFieldType(
                        0, SwFieldIds::DbNumSet) );
                    pField.reset(new SwDBNumSetField( pTyp, sPar1, rData.m_sPar2, aDBData));
                    bExp = true;
                    break;
                }
            case TYP_DBSETNUMBERFLD:
                {
                    SwDBSetNumberFieldType* pTyp = static_cast<SwDBSetNumberFieldType*>(
                        pCurShell->GetFieldType(0, SwFieldIds::DbSetNumber) );
                    pField.reset(new SwDBSetNumberField( pTyp, aDBData, nFormatId));
                    bExp = true;
                    break;
                }
            }
#endif
            break;
        }

    case TYP_USERFLD:
        {
            SwUserFieldType* pTyp =
                static_cast<SwUserFieldType*>( pCurShell->GetFieldType(SwFieldIds::User, rData.m_sPar1) );

            // only if existing
            if(!pTyp)
            {
                pTyp = static_cast<SwUserFieldType*>( pCurShell->InsertFieldType(
                    SwUserFieldType(pCurShell->GetDoc(), rData.m_sPar1)) );
            }
            if (pTyp->GetContent(nFormatId) != rData.m_sPar2)
                pTyp->SetContent(rData.m_sPar2, nFormatId);
            pField.reset(new SwUserField(pTyp, 0, nFormatId));
            if (pField->GetSubType() != nSubType)
                pField->SetSubType(nSubType);
            bTable = true;
            break;
        }

    case TYP_INPUTFLD:
        {
            if ((nSubType & 0x00ff) == INP_VAR)
            {
                SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>(
                    pCurShell->GetFieldType(SwFieldIds::SetExp, rData.m_sPar1) );

                // no Expression Type with this name existing -> create
                if(pTyp)
                {
                    std::unique_ptr<SwSetExpField> pExpField(
                        new SwSetExpField(pTyp, OUString(), nFormatId));

                    // Don't change type of SwSetExpFieldType:
                    sal_uInt16 nOldSubType = pExpField->GetSubType();
                    pExpField->SetSubType(nOldSubType | (nSubType & 0xff00));

                    pExpField->SetPromptText(rData.m_sPar2);
                    pExpField->SetInputFlag(true) ;
                    bExp = true;
                    pField = std::move(pExpField);
                }
                else
                    return false;
            }
            else
            {
                SwInputFieldType* pTyp =
                    static_cast<SwInputFieldType*>( pCurShell->GetFieldType(0, SwFieldIds::Input) );

                pField.reset(
                    new SwInputField( pTyp, rData.m_sPar1, rData.m_sPar2, nSubType|nsSwExtendedSubType::SUB_INVISIBLE, nFormatId));
            }
            break;
        }

    case TYP_SETFLD:
        {
            if (rData.m_sPar2.isEmpty())   // empty variables are not allowed
                return false;

            SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>( pCurShell->InsertFieldType(
                SwSetExpFieldType(pCurShell->GetDoc(), rData.m_sPar1) ) );

            std::unique_ptr<SwSetExpField> pExpField(new SwSetExpField( pTyp, rData.m_sPar2, nFormatId));
            pExpField->SetSubType(nSubType);
            pExpField->SetPar2(rData.m_sPar2);
            bExp = true;
            pField = std::move(pExpField);
            break;
        }

    case TYP_SEQFLD:
        {
            SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>( pCurShell->InsertFieldType(
                SwSetExpFieldType(pCurShell->GetDoc(), rData.m_sPar1, nsSwGetSetExpType::GSE_SEQ)));

            sal_uInt8 nLevel = static_cast< sal_uInt8 >(nSubType & 0xff);

            pTyp->SetOutlineLvl(nLevel);
            if (nLevel != 0x7f && cSeparator == 0)
                cSeparator = '.';

            pTyp->SetDelimiter(OUString(cSeparator));
            pField.reset(new SwSetExpField(pTyp, rData.m_sPar2, nFormatId));
            bExp = true;
            break;
        }

    case TYP_GETFLD:
        {
            // is there a corresponding SetField
            SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                pCurShell->GetFieldType(SwFieldIds::SetExp, rData.m_sPar1));

            if(pSetTyp)
            {
                SwGetExpFieldType* pTyp = static_cast<SwGetExpFieldType*>( pCurShell->GetFieldType(
                    0, SwFieldIds::GetExp) );
                pField.reset( new SwGetExpField(pTyp, rData.m_sPar1, pSetTyp->GetType(), nFormatId) );
                pField->SetSubType(nSubType | pSetTyp->GetType());
                bExp = true;
            }
            else
                return false;
            break;
        }

    case TYP_FORMELFLD:
        {
            if(pCurShell->GetFrameType(nullptr,false) & FrameTypeFlags::TABLE)
            {
                pCurShell->StartAllAction();

                SvNumberFormatter* pFormatter = pCurShell->GetDoc()->GetNumberFormatter();
                const SvNumberformat* pEntry = pFormatter->GetEntry(nFormatId);

                if (pEntry)
                {
                    SfxStringItem aFormat(FN_NUMBER_FORMAT, pEntry->GetFormatstring());
                    pCurShell->GetView().GetViewFrame()->GetDispatcher()->
                        ExecuteList(FN_NUMBER_FORMAT, SfxCallMode::SYNCHRON,
                                { &aFormat });
                }

                SfxItemSet aBoxSet( pCurShell->GetAttrPool(),
                    svl::Items<RES_BOXATR_FORMULA, RES_BOXATR_FORMULA>{} );

                OUString sFormula(comphelper::string::stripStart(rData.m_sPar2, ' '));
                if ( sFormula.startsWith("=") )
                {
                    sFormula = sFormula.copy(1);
                }

                aBoxSet.Put( SwTableBoxFormula( sFormula ));
                pCurShell->SetTableBoxFormulaAttrs( aBoxSet );
                pCurShell->UpdateTable();

                pCurShell->EndAllAction();
                return true;

            }
            else
            {
                SwGetExpFieldType* pTyp = static_cast<SwGetExpFieldType*>(
                    pCurShell->GetFieldType(0, SwFieldIds::GetExp) );
                pField.reset( new SwGetExpField(pTyp, rData.m_sPar2, nsSwGetSetExpType::GSE_FORMULA, nFormatId) );
                pField->SetSubType(nSubType);
                bExp = true;
            }
            break;
        }
        case TYP_SETREFPAGEFLD:
            pField.reset( new SwRefPageSetField( static_cast<SwRefPageSetFieldType*>(
                                pCurShell->GetFieldType( 0, SwFieldIds::RefPageSet ) ),
                                static_cast<short>(rData.m_sPar2.toInt32()), 0 != nSubType  ) );
            bPageVar = true;
            break;

        case TYP_GETREFPAGEFLD:
            pField.reset( new SwRefPageGetField( static_cast<SwRefPageGetFieldType*>(
                            pCurShell->GetFieldType( 0, SwFieldIds::RefPageGet ) ), nFormatId ) );
            bPageVar = true;
            break;
        case TYP_DROPDOWN :
        {
            pField.reset( new SwDropDownField(pCurShell->GetFieldType( 0, SwFieldIds::Dropdown )) );
            const sal_Int32 nTokenCount = comphelper::string::getTokenCount(rData.m_sPar2, DB_DELIM);
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(sal_Int32 nToken = 0, nIdx = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = rData.m_sPar2.getToken(0, DB_DELIM, nIdx);
            static_cast<SwDropDownField*>(pField.get())->SetItems(aEntries);
            static_cast<SwDropDownField*>(pField.get())->SetName(rData.m_sPar1);
        }
        break;

        // Insert Paragraph Signature field by signing the paragraph.
        // The resulting field is really a metadata field, created and added via signing.
        case TYP_PARAGRAPHSIGFLD:
            pCurShell->SignParagraph();
            return true;
        break;

        default:
        {   OSL_ENSURE(false, "wrong field type");
            return false;
        }
    }
    OSL_ENSURE(pField, "field not available");

    //the auto language flag has to be set prior to the language!
    pField->SetAutomaticLanguage(rData.m_bIsAutomaticLanguage);
    LanguageType nLang = GetCurrLanguage();
    pField->SetLanguage(nLang);

    // insert
    pCurShell->StartAllAction();

    pCurShell->Insert( *pField );

    if (TYP_INPUTFLD == rData.m_nTypeId)
    {
        pCurShell->Push();

        // start dialog, not before the field is inserted tdf#99529
        pCurShell->Left(CRSR_SKIP_CHARS,
                false, (INP_VAR == (nSubType & 0xff)) ? 1 : 2, false );
        pCurShell->StartInputFieldDlg(pField.get(), false, true, rData.m_pParent ? rData.m_pParent->GetFrameWeld() : nullptr);

        pCurShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    }

    if(bExp && m_bEvalExp)
        pCurShell->UpdateExpFields(true);

    if(bTable)
    {
        pCurShell->Left(CRSR_SKIP_CHARS, false, 1, false );
        pCurShell->UpdateOneField(*pField);
        pCurShell->Right(CRSR_SKIP_CHARS, false, 1, false );
    }
    else if( bPageVar )
        static_cast<SwRefPageGetFieldType*>(pCurShell->GetFieldType( 0, SwFieldIds::RefPageGet ))->UpdateFields();
    else if( TYP_GETREFFLD == rData.m_nTypeId )
        pField->GetTyp()->ModifyNotification( nullptr, nullptr );

    // delete temporary field
    pField.reset();

    pCurShell->EndAllAction();
    return true;
}

// fields update
void SwFieldMgr::UpdateCurField(sal_uInt32 nFormat,
                            const OUString& rPar1,
                            const OUString& rPar2,
                            std::unique_ptr<SwField> pTmpField)
{
    // change format
    OSL_ENSURE(m_pCurField, "no field at CursorPos");

    if (!pTmpField)
        pTmpField = m_pCurField->CopyField();

    SwFieldType* pType   = pTmpField->GetTyp();
    const sal_uInt16 nTypeId = pTmpField->GetTypeId();

    SwWrtShell* pSh = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(!pSh)
        return;
    pSh->StartAllAction();

    bool bSetPar2 = true;
    bool bSetPar1 = true;
    OUString sPar2( rPar2 );

    // Order to Format
    switch( nTypeId )
    {
        case TYP_DDEFLD:
        {
            // DDE-Topics/-Items can have blanks in their names!
            //  That's not yet considered here!
            sal_Int32 nIndex = 0;
            sPar2 = sPar2.replaceFirst(" ", OUStringLiteral1(sfx2::cTokenSeparator), &nIndex );
            if (nIndex>=0 && ++nIndex<sPar2.getLength())
            {
                sPar2 = sPar2.replaceFirst(" ", OUStringLiteral1(sfx2::cTokenSeparator), &nIndex);
            }
            break;
        }

        case TYP_CHAPTERFLD:
        {
            sal_uInt16 nByte = static_cast<sal_uInt16>(rPar2.toInt32());
            nByte = std::max(sal_uInt16(1), nByte);
            nByte = std::min(nByte, sal_uInt16(MAXLEVEL));
            nByte -= 1;
            static_cast<SwChapterField*>(pTmpField.get())->SetLevel(static_cast<sal_uInt8>(nByte));
            bSetPar2 = false;
            break;
        }

        case TYP_SCRIPTFLD:
            static_cast<SwScriptField*>(pTmpField.get())->SetCodeURL(static_cast<bool>(nFormat));
            break;

        case TYP_NEXTPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                static_cast<SwPageNumberField*>(m_pCurField)->SetUserString( sPar2 );
                sPar2 = "1";
            }
            else
            {
                if( nFormat + 2 == SVX_NUM_PAGEDESC )
                    nFormat = SVX_NUM_PAGEDESC;
                short nOff = static_cast<short>(sPar2.toInt32());
                nOff += 1;
                sPar2 = OUString::number(nOff);
            }
            break;

        case TYP_PREVPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                static_cast<SwPageNumberField*>(m_pCurField)->SetUserString( sPar2 );
                sPar2 = "-1";
            }
            else
            {
                if( nFormat + 2 == SVX_NUM_PAGEDESC )
                    nFormat = SVX_NUM_PAGEDESC;
                short nOff = static_cast<short>(sPar2.toInt32());
                nOff -= 1;
                sPar2 = OUString::number(nOff);
            }
            break;

        case TYP_PAGENUMBERFLD:
        case TYP_GETREFPAGEFLD:
            if( nFormat + 2 == SVX_NUM_PAGEDESC )
                nFormat = SVX_NUM_PAGEDESC;
            break;

        case TYP_GETREFFLD:
            {
                bSetPar2 = false;
                static_cast<SwGetRefField*>(pTmpField.get())->SetSubType( static_cast<sal_uInt16>(rPar2.toInt32()) );
                const sal_Int32 nPos = rPar2.indexOf( '|' );
                if( nPos>=0 )
                    static_cast<SwGetRefField*>(pTmpField.get())->SetSeqNo( static_cast<sal_uInt16>(rPar2.copy( nPos + 1 ).toInt32()));
            }
            break;
        case TYP_DROPDOWN:
        {
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sPar2, DB_DELIM);
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(sal_Int32 nToken = 0, nIdx = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = sPar2.getToken(0, DB_DELIM, nIdx);
            static_cast<SwDropDownField*>(pTmpField.get())->SetItems(aEntries);
            static_cast<SwDropDownField*>(pTmpField.get())->SetName(rPar1);
            bSetPar1 = bSetPar2 = false;
        }
        break;
        case TYP_AUTHORITY :
        {
            //#i99069# changes to a bibliography field should change the field type
            SwAuthorityField* pAuthorityField = static_cast<SwAuthorityField*>(pTmpField.get());
            SwAuthorityFieldType* pAuthorityType = static_cast<SwAuthorityFieldType*>(pType);
            rtl::Reference<SwAuthEntry> xTempEntry(new SwAuthEntry);
            for( sal_Int32 i = 0, nIdx = 0; i < AUTH_FIELD_END; ++i )
                xTempEntry->SetAuthorField( static_cast<ToxAuthorityField>(i),
                                rPar1.getToken( 0, TOX_STYLE_DELIMITER, nIdx ));
            if( pAuthorityType->ChangeEntryContent( xTempEntry.get() ) )
            {
                pType->UpdateFields();
                pSh->SetModified();
            }

            if( xTempEntry->GetAuthorField( AUTH_FIELD_IDENTIFIER ) ==
                pAuthorityField->GetFieldText( AUTH_FIELD_IDENTIFIER ) )
                bSetPar1 = false; //otherwise it's a new or changed entry, the field needs to be updated
            bSetPar2 = false;
        }
        break;
    }

    // set format
    // setup format before SetPar2 because of NumberFormatter!
    pTmpField->ChangeFormat(nFormat);

    if( bSetPar1 )
        pTmpField->SetPar1( rPar1 );
    if( bSetPar2 )
        pTmpField->SetPar2( sPar2 );

    // kick off update
    if(nTypeId == TYP_DDEFLD ||
       nTypeId == TYP_USERFLD ||
       nTypeId == TYP_USRINPFLD)
    {
        pType->UpdateFields();
        pSh->SetModified();
    }
    else {
        // mb: #32157
        pSh->SwEditShell::UpdateOneField(*pTmpField);
        GetCurField();
    }

    pTmpField.reset();

    pSh->EndAllAction();
}

// explicitly evaluate ExpressionFields
void SwFieldMgr::EvalExpFields(SwWrtShell* pSh)
{
    if (pSh == nullptr)
        pSh = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();

    if(pSh)
    {
        pSh->StartAllAction();
        pSh->UpdateExpFields(true);
        pSh->EndAllAction();
    }
}
LanguageType SwFieldMgr::GetCurrLanguage() const
{
    SwWrtShell* pSh = m_pWrtShell ? m_pWrtShell : ::lcl_GetShell();
    if( pSh )
        return pSh->GetCurLang();
    return SvtSysLocale().GetLanguageTag().getLanguageType();
}

void SwFieldType::GetFieldName_()
{
    static const char* coFieldNms[] =
    {
        FLD_DATE_STD,
        FLD_TIME_STD,
        STR_FILENAMEFLD,
        STR_DBNAMEFLD,
        STR_CHAPTERFLD,
        STR_PAGENUMBERFLD,
        STR_DOCSTATFLD,
        STR_AUTHORFLD,
        STR_SETFLD,
        STR_GETFLD,
        STR_FORMELFLD,
        STR_HIDDENTXTFLD,
        STR_SETREFFLD,
        STR_GETREFFLD,
        STR_DDEFLD,
        STR_MACROFLD,
        STR_INPUTFLD,
        STR_HIDDENPARAFLD,
        STR_DOCINFOFLD,
        STR_DBFLD,
        STR_USERFLD,
        STR_POSTITFLD,
        STR_TEMPLNAMEFLD,
        STR_SEQFLD,
        STR_DBNEXTSETFLD,
        STR_DBNUMSETFLD,
        STR_DBSETNUMBERFLD,
        STR_CONDTXTFLD,
        STR_NEXTPAGEFLD,
        STR_PREVPAGEFLD,
        STR_EXTUSERFLD,
        FLD_DATE_FIX,
        FLD_TIME_FIX,
        STR_SETINPUTFLD,
        STR_USRINPUTFLD,
        STR_SETREFPAGEFLD,
        STR_GETREFPAGEFLD,
        STR_INTERNETFLD,
        STR_JUMPEDITFLD,
        STR_SCRIPTFLD,
        STR_AUTHORITY,
        STR_COMBINED_CHARS,
        STR_DROPDOWN,
        STR_CUSTOM_FIELD,
        STR_PARAGRAPH_SIGNATURE
    };

    // insert infos for fields
    SwFieldType::s_pFieldNames = new std::vector<OUString>;
    SwFieldType::s_pFieldNames->reserve(SAL_N_ELEMENTS(coFieldNms));
    for (const char* id : coFieldNms)
    {
        const OUString aTmp(SwResId(id));
        SwFieldType::s_pFieldNames->push_back(MnemonicGenerator::EraseAllMnemonicChars( aTmp ));
    }
}

bool SwFieldMgr::ChooseMacro(weld::Window* pDialogParent)
{
    bool bRet = false;

    // choose script dialog
    OUString aScriptURL = SfxApplication::ChooseScript(pDialogParent);

    // the script selector dialog returns a valid script URL
    if ( !aScriptURL.isEmpty() )
    {
        SetMacroPath( aScriptURL );
        bRet = true;
    }

    return bRet;
}

void SwFieldMgr::SetMacroPath(const OUString& rPath)
{
    m_sMacroPath = rPath;
    m_sMacroName = rPath;

    // try to set sMacroName member variable by parsing the macro path
    // using the new URI parsing services

    Reference< XComponentContext > xContext =
        ::comphelper::getProcessComponentContext();

    Reference< uri::XUriReferenceFactory >
        xFactory = uri::UriReferenceFactory::create( xContext );

    Reference< uri::XVndSunStarScriptUrl >
        xUrl( xFactory->parse( m_sMacroPath ), UNO_QUERY );

    if ( xUrl.is() )
    {
        m_sMacroName = xUrl->getName();
    }
}

sal_uInt32 SwFieldMgr::GetDefaultFormat(sal_uInt16 nTypeId, bool bIsText, SvNumberFormatter* pFormatter)
{
    SvNumFormatType  nDefFormat;

    switch (nTypeId)
    {
        case TYP_TIMEFLD:
        case TYP_DATEFLD:
        {
            nDefFormat = (nTypeId == TYP_DATEFLD) ? SvNumFormatType::DATE : SvNumFormatType::TIME;
        }
        break;

        default:
            if (bIsText)
            {
                nDefFormat = SvNumFormatType::TEXT;
            }
            else
            {
                nDefFormat = SvNumFormatType::ALL;
            }
            break;
    }

    return pFormatter->GetStandardFormat(nDefFormat, GetCurrLanguage());
}

Reference<XNumberingTypeInfo> const & SwFieldMgr::GetNumberingInfo() const
{
    if(!m_xNumberingInfo.is())
    {
        Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
        Reference<XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);
        const_cast<SwFieldMgr*>(this)->m_xNumberingInfo.set(xDefNum, UNO_QUERY);
    }
    return m_xNumberingInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
