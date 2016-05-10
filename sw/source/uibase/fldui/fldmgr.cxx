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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/app.hxx>
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
#include <crsskip.hxx>
#include <flddropdown.hxx>
#include <fldui.hrc>
#include <tox.hxx>
#include <misc.hrc>
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
    GRP_DOC_END     =  GRP_DOC_BEGIN + 11,

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

// field types and subtypes
struct SwFieldPack
{
    sal_uInt16  nTypeId;

    sal_uInt16  nSubTypeStart;
    sal_uInt16  nSubTypeEnd;

    sal_uLong   nFormatBegin;
    sal_uLong   nFormatEnd;
};

// strings and formats
static const SwFieldPack aSwFields[] =
{
    // Document
    { TYP_EXTUSERFLD,       FLD_EU_BEGIN,       FLD_EU_END,     0,                  0 },
    { TYP_AUTHORFLD,        0,                  0,              FMT_AUTHOR_BEGIN,   FMT_AUTHOR_END },
    { TYP_DATEFLD,          FLD_DATE_BEGIN,     FLD_DATE_END,   0,                  0 },
    { TYP_TIMEFLD,          FLD_TIME_BEGIN,     FLD_TIME_END,   0,                  0 },
    { TYP_PAGENUMBERFLD,    0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-1 },
    { TYP_NEXTPAGEFLD,      0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END },
    { TYP_PREVPAGEFLD,      0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END },
    { TYP_FILENAMEFLD,      0,                  0,              FMT_FF_BEGIN,       FMT_FF_END },
    { TYP_DOCSTATFLD,       FLD_STAT_BEGIN,     FLD_STAT_END,   FMT_NUM_BEGIN,      FMT_NUM_END-1 },

    { TYP_CHAPTERFLD,       0,                  0,              FMT_CHAPTER_BEGIN,  FMT_CHAPTER_END },
    { TYP_TEMPLNAMEFLD,     0,                  0,              FMT_FF_BEGIN,       FMT_FF_END },

    // Functions
    { TYP_CONDTXTFLD,       0,                  0,              0,                  0 },
    { TYP_DROPDOWN,         0,                  0,              0,                  0 },
    { TYP_INPUTFLD,         FLD_INPUT_BEGIN,    FLD_INPUT_END,  0,                  0 },
    { TYP_MACROFLD,         0,                  0,              0,                  0 },
    { TYP_JUMPEDITFLD,      0,                  0,              FMT_MARK_BEGIN,     FMT_MARK_END },
    { TYP_COMBINED_CHARS,   0,                  0,              0,                  0 },
    { TYP_HIDDENTXTFLD,     0,                  0,              0,                  0 },
    { TYP_HIDDENPARAFLD,    0,                  0,              0,                  0 },

    // Cross-References
    { TYP_SETREFFLD,        0,                  0,              0,                  0 },
    { TYP_GETREFFLD,        0,                  0,              FMT_REF_BEGIN,      FMT_REF_END },

    // DocInformation
    { TYP_DOCINFOFLD,       0,                  0,              FMT_REG_BEGIN,      FMT_REG_END },

    // Database
    { TYP_DBFLD,            0,                  0,              FMT_DBFLD_BEGIN,    FMT_DBFLD_END },
    { TYP_DBNEXTSETFLD,     0,                  0,              0,                  0 },
    { TYP_DBNUMSETFLD,      0,                  0,              0,                  0 },
    { TYP_DBSETNUMBERFLD,   0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-2 },
    { TYP_DBNAMEFLD,        0,                  0,              0,                  0 },

    // Variables
    { TYP_SETFLD,           0,                  0,              FMT_SETVAR_BEGIN,   FMT_SETVAR_END },

    { TYP_GETFLD,           0,                  0,              FMT_GETVAR_BEGIN,   FMT_GETVAR_END },
    { TYP_DDEFLD,           0,                  0,              FMT_DDE_BEGIN,      FMT_DDE_END },
    { TYP_FORMELFLD,        0,                  0,              FMT_GETVAR_BEGIN,   FMT_GETVAR_END },
    { TYP_INPUTFLD,         FLD_INPUT_BEGIN,    FLD_INPUT_END,  0,                  0 },
    { TYP_SEQFLD,           0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-2 },
    { TYP_SETREFPAGEFLD,    FLD_PAGEREF_BEGIN,  FLD_PAGEREF_END,0,                  0 },
    { TYP_GETREFPAGEFLD,    0,                  0,              FMT_NUM_BEGIN,      FMT_NUM_END-1 },
    { TYP_USERFLD,          0,                  0,              FMT_USERVAR_BEGIN,  FMT_USERVAR_END }
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

inline sal_uInt16 GetPackCount() {  return sizeof(aSwFields) / sizeof(SwFieldPack); }

// FieldManager controls inserting and updating of fields
SwFieldMgr::SwFieldMgr(SwWrtShell* pSh ) :
    pMacroItem(nullptr),
    pWrtShell(pSh),
    bEvalExp(true)
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
    SwWrtShell *pSh = pWrtShell ? pWrtShell : lcl_GetShell();
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
void SwFieldMgr::RemoveFieldType(sal_uInt16 nResId, const OUString& rName )
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if( pSh )
        pSh->RemoveFieldType(nResId, rName);
}

size_t SwFieldMgr::GetFieldTypeCount(sal_uInt16 nResId) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFieldTypeCount(nResId) : 0;
}

SwFieldType* SwFieldMgr::GetFieldType(sal_uInt16 nResId, size_t nField) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFieldType(nField, nResId) : nullptr;
}

SwFieldType* SwFieldMgr::GetFieldType(sal_uInt16 nResId, const OUString& rName) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFieldType(nResId, rName) : nullptr;
}

// determine current field
SwField* SwFieldMgr::GetCurField()
{
    SwWrtShell *pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if ( pSh )
        pCurField = pSh->GetCurField( true );
    else
        pCurField = nullptr;

    // initialise strings and format
    aCurPar1.clear();
    aCurPar2.clear();
    sCurFrame.clear();
    nCurFormat = 0;

    if(!pCurField)
        return nullptr;

    // preprocess current values; determine parameter 1 and parameter 2
    // as well as the format
    const sal_uInt16 nTypeId = pCurField->GetTypeId();

    nCurFormat     = pCurField->GetFormat();
    aCurPar1    = pCurField->GetPar1();
    aCurPar2    = pCurField->GetPar2();

    switch( nTypeId )
    {
        case TYP_PAGENUMBERFLD:
        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_GETREFPAGEFLD:
            if( nCurFormat == SVX_NUM_PAGEDESC )
                nCurFormat -= 2;
            break;
    }
    return pCurField;
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
        return aWebRanges[(sal_uInt16)nGrpId];
    else
        return aRanges[(sal_uInt16)nGrpId];
}

// determine GroupId
sal_uInt16 SwFieldMgr::GetGroup(bool bHtmlMode, sal_uInt16 nTypeId, sal_uInt16 nSubType)
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
        const SwFieldGroupRgn& rRange = GetGroupRange(bHtmlMode, i);
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
        static OUString g_aDate( SW_RES( STR_DATEFLD ) );
        return g_aDate;
    }
    if( TYP_TIMEFLD == nFieldWh )
    {
        static OUString g_aTime( SW_RES( STR_TIMEFLD ) );
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
bool SwFieldMgr::GetSubTypes(sal_uInt16 nTypeId, std::vector<OUString>& rToFill)
{
    bool bRet = false;
    SwWrtShell *pSh = pWrtShell ? pWrtShell : lcl_GetShell();
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
                rToFill.push_back(SW_RES(aSwFields[nPos].nSubTypeStart));
                SAL_FALLTHROUGH; // move on at generic types
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
                    const sal_uInt16 nWhich = pFieldType->Which();

                    if((nTypeId == TYP_DDEFLD && pFieldType->Which() == RES_DDEFLD) ||

                       (nTypeId == TYP_USERFLD && nWhich == RES_USERFLD) ||

                       (nTypeId == TYP_GETFLD && nWhich == RES_SETEXPFLD &&
                        !(static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       (nTypeId == TYP_SETFLD && nWhich == RES_SETEXPFLD &&
                        !(static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       (nTypeId == TYP_SEQFLD && nWhich == RES_SETEXPFLD  &&
                       (static_cast<SwSetExpFieldType*>(pFieldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       ((nTypeId == TYP_INPUTFLD || nTypeId == TYP_FORMELFLD) &&
                         (nWhich == RES_USERFLD ||
                          (nWhich == RES_SETEXPFLD &&
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
                        nCount = aSwFields[nPos].nSubTypeEnd - aSwFields[nPos].nSubTypeStart;

                    for(sal_uInt16 i = 0; i < nCount; ++i)
                    {
                        OUString sNew;
                        if (nTypeId == TYP_DOCINFOFLD)
                        {
                            if ( i == DI_CUSTOM )
                                sNew = SW_RES( STR_CUSTOM );
                            else
                                sNew = SwViewShell::GetShellRes()->aDocInfoLst[i];
                        }
                        else
                            sNew = SW_RES(aSwFields[nPos].nSubTypeStart + i);

                        rToFill.push_back(sNew);
                    }
                }
            }
        }
        bRet = true;
    }
    return bRet;
}

// determine format
//  ACCESS over TYP_....
sal_uInt16 SwFieldMgr::GetFormatCount(sal_uInt16 nTypeId, bool bIsText, bool bHtmlMode) const
{
    OSL_ENSURE(nTypeId < TYP_END, "forbidden TypeId");
    {
        const sal_uInt16 nPos = GetPos(nTypeId);

        if(nPos == USHRT_MAX || (bHtmlMode && nTypeId == TYP_SETFLD))
            return 0;

        sal_uLong nStart = aSwFields[nPos].nFormatBegin;
        sal_uLong nEnd   = aSwFields[nPos].nFormatEnd;

        if (bIsText && nEnd - nStart >= 2)
            return 2;

        if (nTypeId == TYP_FILENAMEFLD)
            nEnd -= 2;  // no range or template

        switch(nStart)
        {
            case FMT_GETVAR_BEGIN:
            case FMT_SETVAR_BEGIN:  return VF_COUNT;
            case FMT_USERVAR_BEGIN: return VF_USR_COUNT;
            case FMT_DBFLD_BEGIN:   return VF_DB_COUNT;
            case FMT_NUM_BEGIN:
            {
                sal_uInt16 nCount = (sal_uInt16)(nEnd - nStart);
                GetNumberingInfo();
                if(xNumberingInfo.is())
                {
                    Sequence<sal_Int16> aTypes = xNumberingInfo->getSupportedNumberingTypes();
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

        }
        return (sal_uInt16)(nEnd - nStart);
    }
}

// determine FormatString to a type
OUString SwFieldMgr::GetFormatStr(sal_uInt16 nTypeId, sal_uLong nFormatId) const
{
    OSL_ENSURE(nTypeId < TYP_END, "forbidden TypeId");
    const sal_uInt16 nPos = GetPos(nTypeId);

    if(nPos == USHRT_MAX)
        return OUString();

    sal_uLong nStart;

    nStart = aSwFields[nPos].nFormatBegin;

    if (TYP_AUTHORFLD == nTypeId|| TYP_FILENAMEFLD == nTypeId)
        nFormatId &= ~FF_FIXED;     // mask out Fixed-Flag

    if((nStart + nFormatId) < aSwFields[nPos].nFormatEnd)
        return SW_RES((sal_uInt16)(nStart + nFormatId));

    OUString aRet;
    if( FMT_NUM_BEGIN == nStart)
    {
        if(xNumberingInfo.is())
        {
            SwOLENames aNames(SW_RES(STRRES_NUMTYPES));
            ResStringArray& rNames = aNames.GetNames();

            Sequence<sal_Int16> aTypes = xNumberingInfo->getSupportedNumberingTypes();
            const sal_Int16* pTypes = aTypes.getConstArray();
            sal_Int32 nOffset = aSwFields[nPos].nFormatEnd - nStart;
            sal_Int32 nValidEntry = 0;
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                {
                    if(nValidEntry == ((sal_Int32)nFormatId) - nOffset)
                    {
                        sal_uInt32 n = rNames.FindIndex(pTypes[nType]);
                        if (n != RESARRAY_INDEX_NOTFOUND)
                        {
                            aRet = rNames.GetString(n);
                        }
                        else
                        {
                            aRet = xNumberingInfo->getNumberingIdentifier( pTypes[nType] );
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
sal_uInt16 SwFieldMgr::GetFormatId(sal_uInt16 nTypeId, sal_uLong nFormatId) const
{
    sal_uInt16 nId = (sal_uInt16)nFormatId;
    switch( nTypeId )
    {
    case TYP_DOCINFOFLD:
        switch( aSwFields[ GetPos( nTypeId ) ].nFormatBegin + nFormatId )
        {
        case FMT_REG_AUTHOR:    nId = DI_SUB_AUTHOR;    break;
        case FMT_REG_TIME:      nId = DI_SUB_TIME;      break;
        case FMT_REG_DATE:      nId = DI_SUB_DATE;      break;
        }
        break;

    case TYP_PAGENUMBERFLD:
    case TYP_NEXTPAGEFLD:
    case TYP_PREVPAGEFLD:
    case TYP_DOCSTATFLD:
    case TYP_DBSETNUMBERFLD:
    case TYP_SEQFLD:
    case TYP_GETREFPAGEFLD:
    {
        sal_uInt16 nPos = GetPos( nTypeId );
        sal_uLong nBegin = aSwFields[ nPos ].nFormatBegin;
        sal_uLong nEnd = aSwFields[nPos].nFormatEnd;
        if((nBegin + nFormatId) < nEnd)
        {
            switch( nBegin + nFormatId )
            {
            case FMT_NUM_ABC:               nId = SVX_NUM_CHARS_UPPER_LETTER;   break;
            case FMT_NUM_SABC:              nId = SVX_NUM_CHARS_LOWER_LETTER;   break;
            case FMT_NUM_ROMAN:             nId = SVX_NUM_ROMAN_UPPER;          break;
            case FMT_NUM_SROMAN:            nId = SVX_NUM_ROMAN_LOWER;          break;
            case FMT_NUM_ARABIC:            nId = SVX_NUM_ARABIC;               break;
            case FMT_NUM_PAGEDESC:          nId = SVX_NUM_PAGEDESC;             break;
            case FMT_NUM_PAGESPECIAL:       nId = SVX_NUM_CHAR_SPECIAL;         break;
            case FMT_NUM_ABC_N:             nId = SVX_NUM_CHARS_UPPER_LETTER_N; break;
            case FMT_NUM_SABC_N:            nId = SVX_NUM_CHARS_LOWER_LETTER_N; break;
            }
        }
        else if(xNumberingInfo.is())
        {
            Sequence<sal_Int16> aTypes = xNumberingInfo->getSupportedNumberingTypes();
            const sal_Int16* pTypes = aTypes.getConstArray();
            sal_Int32 nOffset = nEnd - nBegin;
            sal_Int32 nValidEntry = 0;
            for(sal_Int32 nType = 0; nType < aTypes.getLength(); nType++)
            {
                sal_Int16 nCurrent = pTypes[nType];
                if(nCurrent > NumberingType::CHARS_LOWER_LETTER_N)
                {
                    if(nValidEntry == ((sal_Int32)nFormatId) - nOffset)
                    {
                        nId = pTypes[nType];
                        break;
                    }
                    ++nValidEntry;
                }
            }
        }
    }
    break;
    case TYP_DDEFLD:
        switch ( aSwFields[ GetPos( nTypeId ) ].nFormatBegin + nFormatId )
        {
        case FMT_DDE_NORMAL:    nId = static_cast<sal_uInt16>(SfxLinkUpdateMode::ONCALL); break;
        case FMT_DDE_HOT:       nId = static_cast<sal_uInt16>(SfxLinkUpdateMode::ALWAYS); break;
        }
        break;
    }

    return nId;

}

// Traveling
bool SwFieldMgr::GoNextPrev( bool bNext, SwFieldType* pTyp )
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if(!pSh)
        return false;

    if( !pTyp && pCurField )
    {
        const sal_uInt16 nTypeId = pCurField->GetTypeId();
        if( TYP_SETINPFLD == nTypeId || TYP_USRINPFLD == nTypeId )
            pTyp = pSh->GetFieldType( 0, RES_INPUTFLD );
        else
            pTyp = pCurField->GetTyp();
    }

    if (pTyp && pTyp->Which() == RES_DBFLD)
    {
        // for fieldcommand-edit (hop to all DB fields)
        return pSh->MoveFieldType( nullptr, bNext, RES_DBFLD );
    }

    return pTyp && pSh && pSh->MoveFieldType( pTyp, bNext );
}

// insert field types
void SwFieldMgr::InsertFieldType(SwFieldType& rType)
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(pSh)
        pSh->InsertFieldType(rType);
}

// determine current TypeId
sal_uInt16 SwFieldMgr::GetCurTypeId() const
{
    return pCurField ? pCurField->GetTypeId() : USHRT_MAX;
}

// Over string  insert field or update
bool SwFieldMgr::InsertField(
    const SwInsertField_Data& rData)
{
    SwField* pField   = nullptr;
    bool bExp = false;
    bool bTable = false;
    bool bPageVar = false;
    sal_uLong nFormatId = rData.m_nFormatId;
    sal_uInt16 nSubType = rData.m_nSubType;
    sal_Unicode cSeparator = rData.m_cSeparator;
    SwWrtShell* pCurShell = rData.m_pSh;
    if(!pCurShell)
        pCurShell = pWrtShell ? pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pCurShell, "no SwWrtShell found");
    if(!pCurShell)
        return false;

    switch (rData.m_nTypeId)
    {   // ATTENTION this field is inserted by a separate dialog
        case TYP_POSTITFLD:
        {
            SvtUserOptions aUserOpt;
            SwPostItFieldType* pType = static_cast<SwPostItFieldType*>(pCurShell->GetFieldType(0, RES_POSTITFLD));
            SwPostItField* pPostItField =
                new SwPostItField(
                    pType,
                    rData.m_sPar1, // author
                    rData.m_sPar2, // content
                    aUserOpt.GetID(), // author's initials
                    OUString(), // name
                    DateTime(DateTime::SYSTEM) );
            pField = pPostItField;
        }
        break;
        case TYP_SCRIPTFLD:
        {
            SwScriptFieldType* pType =
                static_cast<SwScriptFieldType*>(pCurShell->GetFieldType(0, RES_SCRIPTFLD));
            pField = new SwScriptField(pType, rData.m_sPar1, rData.m_sPar2, (bool)nFormatId);
            break;
        }

    case TYP_COMBINED_CHARS:
        {
            SwCombinedCharFieldType* pType = static_cast<SwCombinedCharFieldType*>(
                pCurShell->GetFieldType( 0, RES_COMBINED_CHARS ));
            pField = new SwCombinedCharField( pType, rData.m_sPar1 );
        }
        break;

    case TYP_AUTHORITY:
        {
            SwAuthorityFieldType* pType =
                static_cast<SwAuthorityFieldType*>(pCurShell->GetFieldType(0, RES_AUTHORITY));
            if (!pType)
            {
                SwAuthorityFieldType const type(pCurShell->GetDoc());
                pType = static_cast<SwAuthorityFieldType*>(
                            pCurShell->InsertFieldType(type));
            }
            pField = new SwAuthorityField(pType, rData.m_sPar1);
        }
        break;

    case TYP_DATEFLD:
    case TYP_TIMEFLD:
        {
            sal_uInt16 nSub = static_cast< sal_uInt16 >(rData.m_nTypeId == TYP_DATEFLD ? DATEFLD : TIMEFLD);
            nSub |= nSubType == DATE_VAR ? 0 : FIXEDFLD;

            SwDateTimeFieldType* pTyp =
                static_cast<SwDateTimeFieldType*>( pCurShell->GetFieldType(0, RES_DATETIMEFLD) );
            pField = new SwDateTimeField(pTyp, nSub, nFormatId);
            pField->SetPar2(rData.m_sPar2);
            break;
        }

    case TYP_FILENAMEFLD:
        {
            SwFileNameFieldType* pTyp =
                static_cast<SwFileNameFieldType*>( pCurShell->GetFieldType(0, RES_FILENAMEFLD) );
            pField = new SwFileNameField(pTyp, nFormatId);
            break;
        }

    case TYP_TEMPLNAMEFLD:
        {
            SwTemplNameFieldType* pTyp =
                static_cast<SwTemplNameFieldType*>( pCurShell->GetFieldType(0, RES_TEMPLNAMEFLD) );
            pField = new SwTemplNameField(pTyp, nFormatId);
            break;
        }

    case TYP_CHAPTERFLD:
        {
            sal_uInt16 nByte = (sal_uInt16)rData.m_sPar2.toInt32();
            SwChapterFieldType* pTyp =
                static_cast<SwChapterFieldType*>( pCurShell->GetFieldType(0, RES_CHAPTERFLD) );
            pField = new SwChapterField(pTyp, nFormatId);
            nByte = std::max(sal_uInt16(1), nByte);
            nByte = std::min(nByte, sal_uInt16(MAXLEVEL));
            nByte -= 1;
            static_cast<SwChapterField*>(pField)->SetLevel((sal_uInt8)nByte);
            break;
        }

    case TYP_NEXTPAGEFLD:
    case TYP_PREVPAGEFLD:
    case TYP_PAGENUMBERFLD:
        {
            short nOff = (short)rData.m_sPar2.toInt32();

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
                static_cast<SwPageNumberFieldType*>( pCurShell->GetFieldType(0, RES_PAGENUMBERFLD) );
            pField = new SwPageNumberField(pTyp, nSubType, nFormatId, nOff);

            if( SVX_NUM_CHAR_SPECIAL == nFormatId &&
                ( PG_PREV == nSubType || PG_NEXT == nSubType ) )
                static_cast<SwPageNumberField*>(pField)->SetUserString( rData.m_sPar2 );
            break;
        }

    case TYP_DOCSTATFLD:
        {
            SwDocStatFieldType* pTyp =
                static_cast<SwDocStatFieldType*>( pCurShell->GetFieldType(0, RES_DOCSTATFLD) );
            pField = new SwDocStatField(pTyp, nSubType, nFormatId);
            break;
        }

    case TYP_AUTHORFLD:
        {
            SwAuthorFieldType* pTyp =
                static_cast<SwAuthorFieldType*>( pCurShell->GetFieldType(0, RES_AUTHORFLD) );
            pField = new SwAuthorField(pTyp, nFormatId);
            break;
        }

    case TYP_CONDTXTFLD:
    case TYP_HIDDENTXTFLD:
        {
            SwHiddenTextFieldType* pTyp =
                static_cast<SwHiddenTextFieldType*>( pCurShell->GetFieldType(0, RES_HIDDENTXTFLD) );
            pField = new SwHiddenTextField(pTyp, true, rData.m_sPar1, rData.m_sPar2, false, rData.m_nTypeId);
            bExp = true;
            break;
        }

    case TYP_HIDDENPARAFLD:
        {
            SwHiddenParaFieldType* pTyp =
                static_cast<SwHiddenParaFieldType*>( pCurShell->GetFieldType(0, RES_HIDDENPARAFLD) );
            pField = new SwHiddenParaField(pTyp, rData.m_sPar1);
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
                static_cast<SwGetRefFieldType*>( pCurShell->GetFieldType(0, RES_GETREFFLD) );
            sal_uInt16 nSeqNo = (sal_uInt16)rData.m_sPar2.toInt32();
            pField = new SwGetRefField(pTyp, rData.m_sPar1, nSubType, nSeqNo, nFormatId);
            bExp = true;
            break;
        }

    case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
            //              That's not yet considered here.
            sal_Int32 nIndex = 0;
            OUString sCmd = rData.m_sPar2.replaceFirst(" ", OUString(sfx2::cTokenSeparator), &nIndex);
            if (nIndex>=0 && ++nIndex<sCmd.getLength())
            {
                sCmd = sCmd.replaceFirst(" ", OUString(sfx2::cTokenSeparator), &nIndex);
            }

            SwDDEFieldType aType( rData.m_sPar1, sCmd, static_cast<SfxLinkUpdateMode>(nFormatId) );
            SwDDEFieldType* pTyp = static_cast<SwDDEFieldType*>( pCurShell->InsertFieldType( aType ) );
            pField = new SwDDEField( pTyp );
            break;
        }

    case TYP_MACROFLD:
        {
            SwMacroFieldType* pTyp =
                static_cast<SwMacroFieldType*>(pCurShell->GetFieldType(0, RES_MACROFLD));

            pField = new SwMacroField(pTyp, rData.m_sPar1, rData.m_sPar2);

            break;
        }

    case TYP_INTERNETFLD:
        {
            SwFormatINetFormat aFormat( rData.m_sPar1, sCurFrame );
            if( pMacroItem )
                aFormat.SetMacroTable( &pMacroItem->GetMacroTable() );
            return pCurShell->InsertURL( aFormat, rData.m_sPar2 );
        }

    case TYP_JUMPEDITFLD:
        {
            SwJumpEditFieldType* pTyp =
                static_cast<SwJumpEditFieldType*>(pCurShell->GetFieldType(0, RES_JUMPEDITFLD));

            pField = new SwJumpEditField(pTyp, nFormatId, rData.m_sPar1, rData.m_sPar2);
            break;
        }

    case TYP_DOCINFOFLD:
        {
            SwDocInfoFieldType* pTyp = static_cast<SwDocInfoFieldType*>( pCurShell->GetFieldType(
                0, RES_DOCINFOFLD ) );
            pField = new SwDocInfoField(pTyp, nSubType, rData.m_sPar1, nFormatId);
            break;
        }

    case TYP_EXTUSERFLD:
        {
            SwExtUserFieldType* pTyp = static_cast<SwExtUserFieldType*>( pCurShell->GetFieldType(
                0, RES_EXTUSERFLD) );
            pField = new SwExtUserField(pTyp, nSubType, nFormatId);
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
                aDBData.sDataSource = rData.m_sPar1.getToken(0, DB_DELIM);
                aDBData.sCommand = rData.m_sPar1.getToken(1, DB_DELIM);
                aDBData.nCommandType = rData.m_sPar1.getToken(2, DB_DELIM).toInt32();
                sPar1 = rData.m_sPar1.getToken(3, DB_DELIM);
            }

            if(!aDBData.sDataSource.isEmpty() && pCurShell->GetDBData() != aDBData)
                pCurShell->ChgDBData(aDBData);

            SwDBFieldType* pTyp = static_cast<SwDBFieldType*>(pCurShell->InsertFieldType(
                SwDBFieldType(pCurShell->GetDoc(), sPar1, aDBData) ) );
            pField = new SwDBField(pTyp);
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
                        static_cast<SwDBNameFieldType*>(pCurShell->GetFieldType(0, RES_DBNAMEFLD));
                    pField = new SwDBNameField(pTyp, aDBData);

                    break;
                }
            case TYP_DBNEXTSETFLD:
                {
                    SwDBNextSetFieldType* pTyp = static_cast<SwDBNextSetFieldType*>(pCurShell->GetFieldType(
                        0, RES_DBNEXTSETFLD) );
                    pField = new SwDBNextSetField(pTyp, sPar1, rData.m_sPar2, aDBData);
                    bExp = true;
                    break;
                }
            case TYP_DBNUMSETFLD:
                {
                    SwDBNumSetFieldType* pTyp = static_cast<SwDBNumSetFieldType*>( pCurShell->GetFieldType(
                        0, RES_DBNUMSETFLD) );
                    pField = new SwDBNumSetField( pTyp, sPar1, rData.m_sPar2, aDBData);
                    bExp = true;
                    break;
                }
            case TYP_DBSETNUMBERFLD:
                {
                    SwDBSetNumberFieldType* pTyp = static_cast<SwDBSetNumberFieldType*>(
                        pCurShell->GetFieldType(0, RES_DBSETNUMBERFLD) );
                    pField = new SwDBSetNumberField( pTyp, aDBData, nFormatId);
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
                static_cast<SwUserFieldType*>( pCurShell->GetFieldType(RES_USERFLD, rData.m_sPar1) );

            // only if existing
            if(!pTyp)
            {
                pTyp = static_cast<SwUserFieldType*>( pCurShell->InsertFieldType(
                    SwUserFieldType(pCurShell->GetDoc(), rData.m_sPar1)) );
            }
            if (pTyp->GetContent(nFormatId) != rData.m_sPar2)
                pTyp->SetContent(rData.m_sPar2, nFormatId);
            pField = new SwUserField(pTyp, 0, nFormatId);
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
                    pCurShell->GetFieldType(RES_SETEXPFLD, rData.m_sPar1) );

                // no Experssion Type with this name existing -> create
                if(pTyp)
                {
                    SwSetExpField* pExpField =
                        new SwSetExpField(pTyp, OUString(), nFormatId);

                    // Don't change type of SwSetExpFieldType:
                    sal_uInt16 nOldSubType = pExpField->GetSubType();
                    pExpField->SetSubType(nOldSubType | (nSubType & 0xff00));

                    pExpField->SetPromptText(rData.m_sPar2);
                    pExpField->SetInputFlag(true) ;
                    bExp = true;
                    pField = pExpField;
                }
                else
                    return false;
            }
            else
            {
                SwInputFieldType* pTyp =
                    static_cast<SwInputFieldType*>( pCurShell->GetFieldType(0, RES_INPUTFLD) );

                SwInputField* pInpField =
                    new SwInputField( pTyp, rData.m_sPar1, rData.m_sPar2, nSubType|nsSwExtendedSubType::SUB_INVISIBLE, nFormatId);
                pField = pInpField;
            }
            break;
        }

    case TYP_SETFLD:
        {
            if (rData.m_sPar2.isEmpty())   // empty variables are not allowed
                return false;

            SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>( pCurShell->InsertFieldType(
                SwSetExpFieldType(pCurShell->GetDoc(), rData.m_sPar1) ) );

            SwSetExpField* pExpField = new SwSetExpField( pTyp, rData.m_sPar2, nFormatId);
            pExpField->SetSubType(nSubType);
            pExpField->SetPar2(rData.m_sPar2);
            bExp = true;
            pField = pExpField;
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
            SwSetExpField* pExpField = new SwSetExpField(pTyp, rData.m_sPar2, nFormatId);
            bExp = true;
            pField = pExpField;
            nSubType = nsSwGetSetExpType::GSE_SEQ;
            break;
        }

    case TYP_GETFLD:
        {
            // is there a corresponding SetField
            SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                pCurShell->GetFieldType(RES_SETEXPFLD, rData.m_sPar1));

            if(pSetTyp)
            {
                SwGetExpFieldType* pTyp = static_cast<SwGetExpFieldType*>( pCurShell->GetFieldType(
                    0, RES_GETEXPFLD) );
                pField = new SwGetExpField(pTyp, rData.m_sPar1, pSetTyp->GetType(), nFormatId);
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
                        Execute(FN_NUMBER_FORMAT, SfxCallMode::SYNCHRON, &aFormat, 0L);
                }

                SfxItemSet aBoxSet( pCurShell->GetAttrPool(),
                    RES_BOXATR_FORMULA, RES_BOXATR_FORMULA );

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
                    pCurShell->GetFieldType(0, RES_GETEXPFLD) );
                pField = new SwGetExpField(pTyp, rData.m_sPar2, nsSwGetSetExpType::GSE_FORMULA, nFormatId);
                pField->SetSubType(nSubType);
                bExp = true;
            }
            break;
        }
        case TYP_SETREFPAGEFLD:
            pField = new SwRefPageSetField( static_cast<SwRefPageSetFieldType*>(
                                pCurShell->GetFieldType( 0, RES_REFPAGESETFLD ) ),
                                (short)rData.m_sPar2.toInt32(), 0 != nSubType  );
            bPageVar = true;
            break;

        case TYP_GETREFPAGEFLD:
            pField = new SwRefPageGetField( static_cast<SwRefPageGetFieldType*>(
                            pCurShell->GetFieldType( 0, RES_REFPAGEGETFLD ) ), nFormatId );
            bPageVar = true;
            break;
        case TYP_DROPDOWN :
        {
            pField = new SwDropDownField(pCurShell->GetFieldType( 0, RES_DROPDOWN ));
            const sal_Int32 nTokenCount = comphelper::string::getTokenCount(rData.m_sPar2, DB_DELIM);
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(sal_Int32 nToken = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = rData.m_sPar2.getToken(nToken, DB_DELIM);
            static_cast<SwDropDownField*>(pField)->SetItems(aEntries);
            static_cast<SwDropDownField*>(pField)->SetName(rData.m_sPar1);
        }
        break;
        default:
        {   OSL_ENSURE(false, "wrong field type");
            return false;
        }
    }
    OSL_ENSURE(pField, "field not available");

    //the auto language flag has to be set prior to the language!
    pField->SetAutomaticLanguage(rData.m_bIsAutomaticLanguage);
    sal_uInt16 nLang = GetCurrLanguage();
    pField->SetLanguage(nLang);

    // insert
    pCurShell->StartAllAction();

    pCurShell->Insert( *pField );

    if (TYP_INPUTFLD == rData.m_nTypeId)
    {
        // start dialog, not before the field is inserted tdf#99529
        pCurShell->Left(CRSR_SKIP_CHARS,
                false, (INP_VAR == (nSubType & 0xff)) ? 1 : 2, false );
        pCurShell->StartInputFieldDlg(pField, false, rData.m_pParent);
    }

    if(bExp && bEvalExp)
        pCurShell->UpdateExpFields(true);

    if(bTable)
    {
        pCurShell->Left(CRSR_SKIP_CHARS, false, 1, false );
        pCurShell->UpdateFields(*pField);
        pCurShell->Right(CRSR_SKIP_CHARS, false, 1, false );
    }
    else if( bPageVar )
        static_cast<SwRefPageGetFieldType*>(pCurShell->GetFieldType( 0, RES_REFPAGEGETFLD ))->UpdateFields();
    else if( TYP_GETREFFLD == rData.m_nTypeId )
        pField->GetTyp()->ModifyNotification( nullptr, nullptr );

    // delete temporary field
    delete pField;

    pCurShell->EndAllAction();
    return true;
}

// fields update
void SwFieldMgr::UpdateCurField(sal_uLong nFormat,
                            const OUString& rPar1,
                            const OUString& rPar2,
                            SwField * _pTmpField)
{
    // change format
    OSL_ENSURE(pCurField, "no field at CursorPos");

    bool bDelete = false;
    SwField *pTmpField;       // mb: fixed memory leak
    if (nullptr != _pTmpField)
    {
        pTmpField = _pTmpField;
    }
    else
    {
        pTmpField = pCurField->CopyField();
        bDelete = true;
    }

    SwFieldType* pType   = pTmpField->GetTyp();
    const sal_uInt16 nTypeId = pTmpField->GetTypeId();

    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(!pSh)
        return;
    pSh->StartAllAction();

    bool bSetPar2 = true;
    bool bSetPar1 = true;
    OUString sPar1( rPar1 );
    OUString sPar2( rPar2 );

    // Order to Format
    switch( nTypeId )
    {
        case TYP_DDEFLD:
        {
            // DDE-Topics/-Items can have blanks in their names!
            //  That's not yet considered here!
            sal_Int32 nIndex = 0;
            sPar2 = sPar2.replaceFirst(" ", OUString(sfx2::cTokenSeparator), &nIndex );
            if (nIndex>=0 && ++nIndex<sPar2.getLength())
            {
                sPar2 = sPar2.replaceFirst(" ", OUString(sfx2::cTokenSeparator), &nIndex);
            }
            break;
        }

        case TYP_CHAPTERFLD:
        {
            sal_uInt16 nByte = (sal_uInt16)rPar2.toInt32();
            nByte = std::max(sal_uInt16(1), nByte);
            nByte = std::min(nByte, sal_uInt16(MAXLEVEL));
            nByte -= 1;
            static_cast<SwChapterField*>(pTmpField)->SetLevel((sal_uInt8)nByte);
            bSetPar2 = false;
            break;
        }

        case TYP_SCRIPTFLD:
            static_cast<SwScriptField*>(pTmpField)->SetCodeURL((bool)nFormat);
            break;

        case TYP_NEXTPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                static_cast<SwPageNumberField*>(pCurField)->SetUserString( sPar2 );
                sPar2 = "1";
            }
            else
            {
                if( nFormat + 2 == SVX_NUM_PAGEDESC )
                    nFormat = SVX_NUM_PAGEDESC;
                short nOff = (short)sPar2.toInt32();
                nOff += 1;
                sPar2 = OUString::number(nOff);
            }
            break;

        case TYP_PREVPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                static_cast<SwPageNumberField*>(pCurField)->SetUserString( sPar2 );
                sPar2 = "-1";
            }
            else
            {
                if( nFormat + 2 == SVX_NUM_PAGEDESC )
                    nFormat = SVX_NUM_PAGEDESC;
                short nOff = (short)sPar2.toInt32();
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
                static_cast<SwGetRefField*>(pTmpField)->SetSubType( (sal_uInt16)rPar2.toInt32() );
                const sal_Int32 nPos = rPar2.indexOf( '|' );
                if( nPos>=0 )
                    static_cast<SwGetRefField*>(pTmpField)->SetSeqNo( (sal_uInt16)rPar2.copy( nPos + 1 ).toInt32());
            }
            break;
        case TYP_DROPDOWN:
        {
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sPar2, DB_DELIM);
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(sal_Int32 nToken = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = sPar2.getToken(nToken, DB_DELIM);
            static_cast<SwDropDownField*>(pTmpField)->SetItems(aEntries);
            static_cast<SwDropDownField*>(pTmpField)->SetName(sPar1);
            bSetPar1 = bSetPar2 = false;
        }
        break;
        case TYP_AUTHORITY :
        {
            //#i99069# changes to a bibliography field should change the field type
            SwAuthorityField* pAuthorityField = static_cast<SwAuthorityField*>(pTmpField);
            SwAuthorityFieldType* pAuthorityType = static_cast<SwAuthorityFieldType*>(pType);
            SwAuthEntry aTempEntry;
            for( sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i )
                aTempEntry.SetAuthorField( (ToxAuthorityField)i,
                                rPar1.getToken( i, TOX_STYLE_DELIMITER ));
            if( pAuthorityType->ChangeEntryContent( &aTempEntry ) )
            {
                pType->UpdateFields();
                pSh->SetModified();
            }

            if( aTempEntry.GetAuthorField( AUTH_FIELD_IDENTIFIER ) ==
                pAuthorityField->GetFieldText( AUTH_FIELD_IDENTIFIER ) )
                bSetPar1 = false; //otherwise it's a new or changed entry, the field needs to be updated
            bSetPar2 = false;
        }
        break;
    }

    // set format
    // setup format before SetPar2 because of NumberFormatter!
    pTmpField->ChangeFormat(nFormat);

    if(bSetPar1)
        pTmpField->SetPar1( sPar1 );
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
        pSh->SwEditShell::UpdateFields(*pTmpField);
        GetCurField();
    }

    if (bDelete)
        delete pTmpField;

    pSh->EndAllAction();
}

// explicitly evaluate ExpressionFields
void SwFieldMgr::EvalExpFields(SwWrtShell* pSh)
{
    if (pSh == nullptr)
        pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();

    if(pSh)
    {
        pSh->StartAllAction();
        pSh->UpdateExpFields(true);
        pSh->EndAllAction();
    }
}
sal_uInt16 SwFieldMgr::GetCurrLanguage() const
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if( pSh )
        return pSh->GetCurLang();
    return SvtSysLocale().GetLanguageTag().getLanguageType();
}

void SwFieldType::_GetFieldName()
{
    static const sal_uInt16 coFieldNms[] = {
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
        STR_CUSTOM
    };

    // insert infos for fields
    SwFieldType::s_pFieldNames = new std::vector<OUString>;
    SwFieldType::s_pFieldNames->reserve(SAL_N_ELEMENTS(coFieldNms));
    for( sal_uInt16 nIdx = 0; nIdx < SAL_N_ELEMENTS(coFieldNms); ++nIdx )
    {
        const OUString aTmp(SW_RES( coFieldNms[ nIdx ] ));
        SwFieldType::s_pFieldNames->push_back(MnemonicGenerator::EraseAllMnemonicChars( aTmp ));
    }
}

bool SwFieldMgr::ChooseMacro(const OUString&)
{
    bool bRet = false;

    // choose script dialog
    OUString aScriptURL = SfxApplication::ChooseScript();

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
    sMacroPath = rPath;
    sMacroName = rPath;

    // try to set sMacroName member variable by parsing the macro path
    // using the new URI parsing services

    Reference< XComponentContext > xContext =
        ::comphelper::getProcessComponentContext();

    Reference< uri::XUriReferenceFactory >
        xFactory = uri::UriReferenceFactory::create( xContext );

    Reference< uri::XVndSunStarScriptUrl >
        xUrl( xFactory->parse( sMacroPath ), UNO_QUERY );

    if ( xUrl.is() )
    {
        sMacroName = xUrl->getName();
    }
}

sal_uLong SwFieldMgr::GetDefaultFormat(sal_uInt16 nTypeId, bool bIsText, SvNumberFormatter* pFormatter, double* pVal)
{
    double fValue;
    short  nDefFormat;

    switch (nTypeId)
    {
        case TYP_TIMEFLD:
        case TYP_DATEFLD:
        {
            Date aDate( Date::SYSTEM );
            Date* pNullDate = pFormatter->GetNullDate();

            fValue = aDate - *pNullDate;

            tools::Time aTime( tools::Time::SYSTEM );

            sal_uLong nNumFormatTime = (sal_uLong)aTime.GetSec() + (sal_uLong)aTime.GetMin() * 60L +
                          (sal_uLong)aTime.GetHour() * 3600L;

            fValue += (double)nNumFormatTime / 86400.0;

            nDefFormat = (nTypeId == TYP_DATEFLD) ? css::util::NumberFormat::DATE : css::util::NumberFormat::TIME;
        }
        break;

        default:
            if (bIsText)
            {
                fValue = 0.0;
                nDefFormat = css::util::NumberFormat::TEXT;
            }
            else
            {
                fValue = 0.0;
                nDefFormat = css::util::NumberFormat::ALL;
            }
            break;
    }

    if (pVal)
        *pVal = fValue;

    return pFormatter->GetStandardFormat(nDefFormat, GetCurrLanguage());
}

Reference<XNumberingTypeInfo> SwFieldMgr::GetNumberingInfo() const
{
    if(!xNumberingInfo.is())
    {
        Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
        Reference<XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);
        const_cast<SwFieldMgr*>(this)->xNumberingInfo.set(xDefNum, UNO_QUERY);
    }
    return xNumberingInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
