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
struct SwFldPack
{
    sal_uInt16  nTypeId;

    sal_uInt16  nSubTypeStart;
    sal_uInt16  nSubTypeEnd;

    sal_uLong   nFmtBegin;
    sal_uLong   nFmtEnd;
};

// strings and formats
static const SwFldPack aSwFlds[] =
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
    if ( 0 != (pView = ::GetActiveView()) )
        return pView->GetWrtShellPtr();
    OSL_FAIL("no current shell found!");
    return 0;
}

inline sal_uInt16 GetPackCount() {  return sizeof(aSwFlds) / sizeof(SwFldPack); }

// FieldManager controls inserting and updating of fields
SwFldMgr::SwFldMgr(SwWrtShell* pSh ) :
    pModule(0),
    pMacroItem(0),
    pWrtShell(pSh),
    bEvalExp(true)
{
    // determine current field if existing
    GetCurFld();
}

SwFldMgr::~SwFldMgr()
{
}

// organise RefMark by names
bool  SwFldMgr::CanInsertRefMark( const OUString& rStr )
{
    bool bRet = false;
    SwWrtShell *pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(pSh)
    {
        sal_uInt16 nCnt = pSh->GetCrsrCnt();

        // the last Crsr doesn't have to be a spanned selection
        if( 1 < nCnt && !pSh->SwCrsrShell::HasSelection() )
            --nCnt;

        bRet =  2 > nCnt && 0 == pSh->GetRefMark( rStr );
    }
    return bRet;
}

// access over ResIds
void SwFldMgr::RemoveFldType(sal_uInt16 nResId, const OUString& rName )
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if( pSh )
        pSh->RemoveFldType(nResId, rName);
}

sal_uInt16 SwFldMgr::GetFldTypeCount(sal_uInt16 nResId) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFldTypeCount(nResId) : 0;
}

SwFieldType* SwFldMgr::GetFldType(sal_uInt16 nResId, sal_uInt16 nId) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFldType(nId, nResId) : 0;
}

SwFieldType* SwFldMgr::GetFldType(sal_uInt16 nResId, const OUString& rName) const
{
    SwWrtShell * pSh = pWrtShell ? pWrtShell : lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    return pSh ? pSh->GetFldType(nResId, rName) : 0;
}

// determine current field
SwField* SwFldMgr::GetCurFld()
{
    SwWrtShell *pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if ( pSh )
        pCurFld = pSh->GetCurFld( true );
    else
        pCurFld = NULL;

    // initialise strings and format
    aCurPar1.clear();
    aCurPar2.clear();
    sCurFrame.clear();
    nCurFmt = 0;

    if(!pCurFld)
        return 0;

    // preprocess current values; determine parameter 1 and parameter 2
    // as well as the format
    const sal_uInt16 nTypeId = pCurFld->GetTypeId();

    nCurFmt     = pCurFld->GetFormat();
    aCurPar1    = pCurFld->GetPar1();
    aCurPar2    = pCurFld->GetPar2();

    switch( nTypeId )
    {
        case TYP_PAGENUMBERFLD:
        case TYP_NEXTPAGEFLD:
        case TYP_PREVPAGEFLD:
        case TYP_GETREFPAGEFLD:
            if( nCurFmt == SVX_NUM_PAGEDESC )
                nCurFmt -= 2;
            break;
    }
    return pCurFld;
}

// provide group range
const SwFldGroupRgn& SwFldMgr::GetGroupRange(bool bHtmlMode, sal_uInt16 nGrpId)
{
static SwFldGroupRgn const aRanges[] =
{
    { /* Document   */  GRP_DOC_BEGIN,  GRP_DOC_END },
    { /* Functions  */  GRP_FKT_BEGIN,  GRP_FKT_END },
    { /* Cross-Refs */  GRP_REF_BEGIN,  GRP_REF_END },
    { /* DocInfos   */  GRP_REG_BEGIN,  GRP_REG_END },
    { /* Database   */  GRP_DB_BEGIN,   GRP_DB_END  },
    { /* User       */  GRP_VAR_BEGIN,  GRP_VAR_END }
};
static SwFldGroupRgn const aWebRanges[] =
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
sal_uInt16 SwFldMgr::GetGroup(bool bHtmlMode, sal_uInt16 nTypeId, sal_uInt16 nSubType)
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
        const SwFldGroupRgn& rRange = GetGroupRange(bHtmlMode, i);
        for (sal_uInt16 nPos = rRange.nStart; nPos < rRange.nEnd; nPos++)
        {
            if (aSwFlds[nPos].nTypeId == nTypeId)
                return i;
        }
    }
    return USHRT_MAX;
}

// determine names to TypeId
//  ACCESS over TYP_....
sal_uInt16 SwFldMgr::GetTypeId(sal_uInt16 nPos)
{
    OSL_ENSURE(nPos < ::GetPackCount(), "forbidden Pos");
    return aSwFlds[ nPos ].nTypeId;
}

OUString SwFldMgr::GetTypeStr(sal_uInt16 nPos)
{
    OSL_ENSURE(nPos < ::GetPackCount(), "forbidden TypeId");

    sal_uInt16 nFldWh = aSwFlds[ nPos ].nTypeId;

    // special treatment for date/time fields (without var/fix)
    if( TYP_DATEFLD == nFldWh )
    {
        static OUString g_aDate( SW_RES( STR_DATEFLD ) );
        return g_aDate;
    }
    if( TYP_TIMEFLD == nFldWh )
    {
        static OUString g_aTime( SW_RES( STR_TIMEFLD ) );
        return g_aTime;
    }

    return SwFieldType::GetTypeStr( nFldWh );
}

// determine Pos in the list
sal_uInt16 SwFldMgr::GetPos(sal_uInt16 nTypeId)
{
    switch( nTypeId )
    {
        case TYP_FIXDATEFLD:        nTypeId = TYP_DATEFLD;      break;
        case TYP_FIXTIMEFLD:        nTypeId = TYP_TIMEFLD;      break;
        case TYP_SETINPFLD:         nTypeId = TYP_SETFLD;       break;
        case TYP_USRINPFLD:         nTypeId = TYP_USERFLD;      break;
    }

    for(sal_uInt16 i = 0; i < GetPackCount(); i++)
        if(aSwFlds[i].nTypeId == nTypeId)
            return i;

    return USHRT_MAX;
}

// localise subtypes of a field
bool SwFldMgr::GetSubTypes(sal_uInt16 nTypeId, std::vector<OUString>& rToFill)
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
                rToFill.push_back(SW_RES(aSwFlds[nPos].nSubTypeStart));
                // move on at generic types
            }
            case TYP_DDEFLD:
            case TYP_SEQFLD:
            case TYP_FORMELFLD:
            case TYP_GETFLD:
            case TYP_SETFLD:
            case TYP_USERFLD:
            {

                const sal_uInt16 nCount = pSh->GetFldTypeCount();
                for(sal_uInt16 i = 0; i < nCount; ++i)
                {
                    SwFieldType* pFldType = pSh->GetFldType( i );
                    const sal_uInt16 nWhich = pFldType->Which();

                    if((nTypeId == TYP_DDEFLD && pFldType->Which() == RES_DDEFLD) ||

                       (nTypeId == TYP_USERFLD && nWhich == RES_USERFLD) ||

                       (nTypeId == TYP_GETFLD && nWhich == RES_SETEXPFLD &&
                        !(static_cast<SwSetExpFieldType*>(pFldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       (nTypeId == TYP_SETFLD && nWhich == RES_SETEXPFLD &&
                        !(static_cast<SwSetExpFieldType*>(pFldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       (nTypeId == TYP_SEQFLD && nWhich == RES_SETEXPFLD  &&
                       (static_cast<SwSetExpFieldType*>(pFldType)->GetType() & nsSwGetSetExpType::GSE_SEQ)) ||

                       ((nTypeId == TYP_INPUTFLD || nTypeId == TYP_FORMELFLD) &&
                         (nWhich == RES_USERFLD ||
                          (nWhich == RES_SETEXPFLD &&
                          !(static_cast<SwSetExpFieldType*>(pFldType)->GetType() & nsSwGetSetExpType::GSE_SEQ))) ) )
                    {
                        rToFill.push_back(pFldType->GetName());
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
                        nCount = aSwFlds[nPos].nSubTypeEnd - aSwFlds[nPos].nSubTypeStart;

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
                            sNew = SW_RES(aSwFlds[nPos].nSubTypeStart + i);

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
sal_uInt16 SwFldMgr::GetFormatCount(sal_uInt16 nTypeId, bool bIsText, bool bHtmlMode) const
{
    OSL_ENSURE(nTypeId < TYP_END, "forbidden TypeId");
    {
        const sal_uInt16 nPos = GetPos(nTypeId);

        if(nPos == USHRT_MAX || (bHtmlMode && nTypeId == TYP_SETFLD))
            return 0;

        sal_uLong nStart = aSwFlds[nPos].nFmtBegin;
        sal_uLong nEnd   = aSwFlds[nPos].nFmtEnd;

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
OUString SwFldMgr::GetFormatStr(sal_uInt16 nTypeId, sal_uLong nFormatId) const
{
    OSL_ENSURE(nTypeId < TYP_END, "forbidden TypeId");
    const sal_uInt16 nPos = GetPos(nTypeId);

    if(nPos == USHRT_MAX)
        return OUString();

    sal_uLong nStart;

    nStart = aSwFlds[nPos].nFmtBegin;

    if (TYP_AUTHORFLD == nTypeId|| TYP_FILENAMEFLD == nTypeId)
        nFormatId &= ~FF_FIXED;     // mask out Fixed-Flag

    if((nStart + nFormatId) < aSwFlds[nPos].nFmtEnd)
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
            sal_Int32 nOffset = aSwFlds[nPos].nFmtEnd - nStart;
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
sal_uInt16 SwFldMgr::GetFormatId(sal_uInt16 nTypeId, sal_uLong nFormatId) const
{
    sal_uInt16 nId = (sal_uInt16)nFormatId;
    switch( nTypeId )
    {
    case TYP_DOCINFOFLD:
        switch( aSwFlds[ GetPos( nTypeId ) ].nFmtBegin + nFormatId )
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
        sal_uLong nBegin = aSwFlds[ nPos ].nFmtBegin;
        sal_uLong nEnd = aSwFlds[nPos].nFmtEnd;
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
        switch ( aSwFlds[ GetPos( nTypeId ) ].nFmtBegin + nFormatId )
        {
        case FMT_DDE_NORMAL:    nId = static_cast<sal_uInt16>(SfxLinkUpdateMode::ONCALL); break;
        case FMT_DDE_HOT:       nId = static_cast<sal_uInt16>(SfxLinkUpdateMode::ALWAYS); break;
        }
        break;
    }

    return nId;

}

// Traveling
bool SwFldMgr::GoNextPrev( bool bNext, SwFieldType* pTyp )
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if(!pSh)
        return false;

    if( !pTyp && pCurFld )
    {
        const sal_uInt16 nTypeId = pCurFld->GetTypeId();
        if( TYP_SETINPFLD == nTypeId || TYP_USRINPFLD == nTypeId )
            pTyp = pSh->GetFldType( 0, RES_INPUTFLD );
        else
            pTyp = pCurFld->GetTyp();
    }

    if (pTyp && pTyp->Which() == RES_DBFLD)
    {
        // for fieldcommand-edit (hop to all DB fields)
        return pSh->MoveFldType( 0, bNext, RES_DBFLD );
    }

    return pTyp && pSh && pSh->MoveFldType( pTyp, bNext );
}

// insert field types
void SwFldMgr::InsertFldType(SwFieldType& rType)
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    OSL_ENSURE(pSh, "no SwWrtShell found");
    if(pSh)
        pSh->InsertFldType(rType);
}

// determine current TypeId
sal_uInt16 SwFldMgr::GetCurTypeId() const
{
    return pCurFld ? pCurFld->GetTypeId() : USHRT_MAX;
}

// Over string  insert field or update
bool SwFldMgr::InsertFld(
    const SwInsertFld_Data& rData)
{
    SwField* pFld   = 0;
    bool bExp = false;
    bool bTbl = false;
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
            SwPostItFieldType* pType = static_cast<SwPostItFieldType*>(pCurShell->GetFldType(0, RES_POSTITFLD));
            SwPostItField* pPostItField =
                new SwPostItField(
                    pType,
                    rData.m_sPar1, // author
                    rData.m_sPar2, // content
                    aUserOpt.GetID(), // author's initials
                    OUString(), // name
                    DateTime(DateTime::SYSTEM) );
            pFld = pPostItField;
        }
        break;
        case TYP_SCRIPTFLD:
        {
            SwScriptFieldType* pType =
                static_cast<SwScriptFieldType*>(pCurShell->GetFldType(0, RES_SCRIPTFLD));
            pFld = new SwScriptField(pType, rData.m_sPar1, rData.m_sPar2, (bool)nFormatId);
            break;
        }

    case TYP_COMBINED_CHARS:
        {
            SwCombinedCharFieldType* pType = static_cast<SwCombinedCharFieldType*>(
                pCurShell->GetFldType( 0, RES_COMBINED_CHARS ));
            pFld = new SwCombinedCharField( pType, rData.m_sPar1 );
        }
        break;

    case TYP_AUTHORITY:
        {
            SwAuthorityFieldType* pType =
                static_cast<SwAuthorityFieldType*>(pCurShell->GetFldType(0, RES_AUTHORITY));
            if (!pType)
            {
                SwAuthorityFieldType const type(pCurShell->GetDoc());
                pType = static_cast<SwAuthorityFieldType*>(
                            pCurShell->InsertFldType(type));
            }
            pFld = new SwAuthorityField(pType, rData.m_sPar1);
        }
        break;

    case TYP_DATEFLD:
    case TYP_TIMEFLD:
        {
            sal_uInt16 nSub = static_cast< sal_uInt16 >(rData.m_nTypeId == TYP_DATEFLD ? DATEFLD : TIMEFLD);
            nSub |= nSubType == DATE_VAR ? 0 : FIXEDFLD;

            SwDateTimeFieldType* pTyp =
                static_cast<SwDateTimeFieldType*>( pCurShell->GetFldType(0, RES_DATETIMEFLD) );
            pFld = new SwDateTimeField(pTyp, nSub, nFormatId);
            pFld->SetPar2(rData.m_sPar2);
            break;
        }

    case TYP_FILENAMEFLD:
        {
            SwFileNameFieldType* pTyp =
                static_cast<SwFileNameFieldType*>( pCurShell->GetFldType(0, RES_FILENAMEFLD) );
            pFld = new SwFileNameField(pTyp, nFormatId);
            break;
        }

    case TYP_TEMPLNAMEFLD:
        {
            SwTemplNameFieldType* pTyp =
                static_cast<SwTemplNameFieldType*>( pCurShell->GetFldType(0, RES_TEMPLNAMEFLD) );
            pFld = new SwTemplNameField(pTyp, nFormatId);
            break;
        }

    case TYP_CHAPTERFLD:
        {
            sal_uInt16 nByte = (sal_uInt16)rData.m_sPar2.toInt32();
            SwChapterFieldType* pTyp =
                static_cast<SwChapterFieldType*>( pCurShell->GetFldType(0, RES_CHAPTERFLD) );
            pFld = new SwChapterField(pTyp, nFormatId);
            nByte = std::max(sal_uInt16(1), nByte);
            nByte = std::min(nByte, sal_uInt16(MAXLEVEL));
            nByte -= 1;
            static_cast<SwChapterField*>(pFld)->SetLevel((sal_uInt8)nByte);
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
                static_cast<SwPageNumberFieldType*>( pCurShell->GetFldType(0, RES_PAGENUMBERFLD) );
            pFld = new SwPageNumberField(pTyp, nSubType, nFormatId, nOff);

            if( SVX_NUM_CHAR_SPECIAL == nFormatId &&
                ( PG_PREV == nSubType || PG_NEXT == nSubType ) )
                static_cast<SwPageNumberField*>(pFld)->SetUserString( rData.m_sPar2 );
            break;
        }

    case TYP_DOCSTATFLD:
        {
            SwDocStatFieldType* pTyp =
                static_cast<SwDocStatFieldType*>( pCurShell->GetFldType(0, RES_DOCSTATFLD) );
            pFld = new SwDocStatField(pTyp, nSubType, nFormatId);
            break;
        }

    case TYP_AUTHORFLD:
        {
            SwAuthorFieldType* pTyp =
                static_cast<SwAuthorFieldType*>( pCurShell->GetFldType(0, RES_AUTHORFLD) );
            pFld = new SwAuthorField(pTyp, nFormatId);
            break;
        }

    case TYP_CONDTXTFLD:
    case TYP_HIDDENTXTFLD:
        {
            SwHiddenTxtFieldType* pTyp =
                static_cast<SwHiddenTxtFieldType*>( pCurShell->GetFldType(0, RES_HIDDENTXTFLD) );
            pFld = new SwHiddenTxtField(pTyp, true, rData.m_sPar1, rData.m_sPar2, false, rData.m_nTypeId);
            bExp = true;
            break;
        }

    case TYP_HIDDENPARAFLD:
        {
            SwHiddenParaFieldType* pTyp =
                static_cast<SwHiddenParaFieldType*>( pCurShell->GetFldType(0, RES_HIDDENPARAFLD) );
            pFld = new SwHiddenParaField(pTyp, rData.m_sPar1);
            bExp = true;
            break;
        }

    case TYP_SETREFFLD:
        {
            if( !rData.m_sPar1.isEmpty() && CanInsertRefMark( rData.m_sPar1 ) )
            {
                pCurShell->SetAttrItem( SwFmtRefMark( rData.m_sPar1 ) );
                return true;
            }
            return false;
        }

    case TYP_GETREFFLD:
        {
            SwGetRefFieldType* pTyp =
                static_cast<SwGetRefFieldType*>( pCurShell->GetFldType(0, RES_GETREFFLD) );
            sal_uInt16 nSeqNo = (sal_uInt16)rData.m_sPar2.toInt32();
            pFld = new SwGetRefField(pTyp, rData.m_sPar1, nSubType, nSeqNo, nFormatId);
            bExp = true;
            break;
        }

    case TYP_DDEFLD:
        {
            //JP 28.08.95: DDE-Topics/-Items can have blanks in their names!
            //              That's not yet considered here.
            sal_Int32 nIndex = 0;
            OUString sCmd = rData.m_sPar2.replaceFirst(OUString(' '), OUString(sfx2::cTokenSeparator), &nIndex);
            if (nIndex>=0 && ++nIndex<sCmd.getLength())
            {
                sCmd = sCmd.replaceFirst(OUString(' '), OUString(sfx2::cTokenSeparator), &nIndex);
            }

            SwDDEFieldType aType( rData.m_sPar1, sCmd, static_cast<SfxLinkUpdateMode>(nFormatId) );
            SwDDEFieldType* pTyp = static_cast<SwDDEFieldType*>( pCurShell->InsertFldType( aType ) );
            pFld = new SwDDEField( pTyp );
            break;
        }

    case TYP_MACROFLD:
        {
            SwMacroFieldType* pTyp =
                static_cast<SwMacroFieldType*>(pCurShell->GetFldType(0, RES_MACROFLD));

            pFld = new SwMacroField(pTyp, rData.m_sPar1, rData.m_sPar2);

            break;
        }

    case TYP_INTERNETFLD:
        {
            SwFmtINetFmt aFmt( rData.m_sPar1, sCurFrame );
            if( pMacroItem )
                aFmt.SetMacroTbl( &pMacroItem->GetMacroTable() );
            return pCurShell->InsertURL( aFmt, rData.m_sPar2 );
        }

    case TYP_JUMPEDITFLD:
        {
            SwJumpEditFieldType* pTyp =
                static_cast<SwJumpEditFieldType*>(pCurShell->GetFldType(0, RES_JUMPEDITFLD));

            pFld = new SwJumpEditField(pTyp, nFormatId, rData.m_sPar1, rData.m_sPar2);
            break;
        }

    case TYP_DOCINFOFLD:
        {
            SwDocInfoFieldType* pTyp = static_cast<SwDocInfoFieldType*>( pCurShell->GetFldType(
                0, RES_DOCINFOFLD ) );
            pFld = new SwDocInfoField(pTyp, nSubType, rData.m_sPar1, nFormatId);
            break;
        }

    case TYP_EXTUSERFLD:
        {
            SwExtUserFieldType* pTyp = static_cast<SwExtUserFieldType*>( pCurShell->GetFldType(
                0, RES_EXTUSERFLD) );
            pFld = new SwExtUserField(pTyp, nSubType, nFormatId);
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

            SwDBFieldType* pTyp = static_cast<SwDBFieldType*>(pCurShell->InsertFldType(
                SwDBFieldType(pCurShell->GetDoc(), sPar1, aDBData) ) );
            pFld = new SwDBField(pTyp);
            pFld->SetSubType(nSubType);

            if( !(nSubType & nsSwExtendedSubType::SUB_OWN_FMT) ) // determinee database format
            {
                Reference< XDataSource> xSource;
                rData.m_aDBDataSource >>= xSource;
                Reference<XConnection> xConnection;
                rData.m_aDBConnection >>= xConnection;
                Reference<XPropertySet> xColumn;
                rData.m_aDBColumn >>= xColumn;
                if(xColumn.is())
                {
                    nFormatId = SwDBManager::GetColumnFmt(xSource, xConnection, xColumn,
                        pCurShell->GetNumberFormatter(), GetCurrLanguage() );
                }
                else
                    nFormatId = pCurShell->GetDBManager()->GetColumnFmt(
                    aDBData.sDataSource, aDBData.sCommand, sPar1,
                    pCurShell->GetNumberFormatter(), GetCurrLanguage() );
            }
            pFld->ChangeFormat( nFormatId );

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

            // excract DBName from rData.m_sPar1. Format: DBName.TableName.CommandType.ExpStrg
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
                        static_cast<SwDBNameFieldType*>(pCurShell->GetFldType(0, RES_DBNAMEFLD));
                    pFld = new SwDBNameField(pTyp, aDBData);

                    break;
                }
            case TYP_DBNEXTSETFLD:
                {
                    SwDBNextSetFieldType* pTyp = static_cast<SwDBNextSetFieldType*>(pCurShell->GetFldType(
                        0, RES_DBNEXTSETFLD) );
                    pFld = new SwDBNextSetField(pTyp, sPar1, rData.m_sPar2, aDBData);
                    bExp = true;
                    break;
                }
            case TYP_DBNUMSETFLD:
                {
                    SwDBNumSetFieldType* pTyp = static_cast<SwDBNumSetFieldType*>( pCurShell->GetFldType(
                        0, RES_DBNUMSETFLD) );
                    pFld = new SwDBNumSetField( pTyp, sPar1, rData.m_sPar2, aDBData);
                    bExp = true;
                    break;
                }
            case TYP_DBSETNUMBERFLD:
                {
                    SwDBSetNumberFieldType* pTyp = static_cast<SwDBSetNumberFieldType*>(
                        pCurShell->GetFldType(0, RES_DBSETNUMBERFLD) );
                    pFld = new SwDBSetNumberField( pTyp, aDBData, nFormatId);
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
                static_cast<SwUserFieldType*>( pCurShell->GetFldType(RES_USERFLD, rData.m_sPar1) );

            // only if existing
            if(!pTyp)
            {
                pTyp = static_cast<SwUserFieldType*>( pCurShell->InsertFldType(
                    SwUserFieldType(pCurShell->GetDoc(), rData.m_sPar1)) );
            }
            if (pTyp->GetContent(nFormatId) != rData.m_sPar2)
                pTyp->SetContent(rData.m_sPar2, nFormatId);
            pFld = new SwUserField(pTyp, 0, nFormatId);
            if (pFld->GetSubType() != nSubType)
                pFld->SetSubType(nSubType);
            bTbl = true;
            break;
        }

    case TYP_INPUTFLD:
        {
            if ((nSubType & 0x00ff) == INP_VAR)
            {
                SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>(
                    pCurShell->GetFldType(RES_SETEXPFLD, rData.m_sPar1) );

                // no Experssion Type with this name existing -> create
                if(pTyp)
                {
                    SwSetExpField* pExpFld =
                        new SwSetExpField(pTyp, OUString(), nFormatId);

                    // Don't change type of SwSetExpFieldType:
                    sal_uInt16 nOldSubType = pExpFld->GetSubType();
                    pExpFld->SetSubType(nOldSubType | (nSubType & 0xff00));

                    pExpFld->SetPromptText(rData.m_sPar2);
                    pExpFld->SetInputFlag(true) ;
                    bExp = true;
                    pFld = pExpFld;
                }
                else
                    return false;
            }
            else
            {
                SwInputFieldType* pTyp =
                    static_cast<SwInputFieldType*>( pCurShell->GetFldType(0, RES_INPUTFLD) );

                SwInputField* pInpFld =
                    new SwInputField( pTyp, rData.m_sPar1, rData.m_sPar2, nSubType|nsSwExtendedSubType::SUB_INVISIBLE, nFormatId);
                pFld = pInpFld;
            }

            // start dialog
            pCurShell->StartInputFldDlg(pFld, false, rData.m_pParent);
            break;
        }

    case TYP_SETFLD:
        {
            if (rData.m_sPar2.isEmpty())   // empty variables are not allowed
                return false;

            SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>( pCurShell->InsertFldType(
                SwSetExpFieldType(pCurShell->GetDoc(), rData.m_sPar1) ) );

            SwSetExpField* pExpFld = new SwSetExpField( pTyp, rData.m_sPar2, nFormatId);
            pExpFld->SetSubType(nSubType);
            pExpFld->SetPar2(rData.m_sPar2);
            bExp = true;
            pFld = pExpFld;
            break;
        }

    case TYP_SEQFLD:
        {
            SwSetExpFieldType* pTyp = static_cast<SwSetExpFieldType*>( pCurShell->InsertFldType(
                SwSetExpFieldType(pCurShell->GetDoc(), rData.m_sPar1, nsSwGetSetExpType::GSE_SEQ)));

            sal_uInt8 nLevel = static_cast< sal_uInt8 >(nSubType & 0xff);

            pTyp->SetOutlineLvl(nLevel);
            if (nLevel != 0x7f && cSeparator == 0)
                cSeparator = '.';

            pTyp->SetDelimiter(OUString(cSeparator));
            SwSetExpField* pExpFld = new SwSetExpField(pTyp, rData.m_sPar2, nFormatId);
            bExp = true;
            pFld = pExpFld;
            nSubType = nsSwGetSetExpType::GSE_SEQ;
            break;
        }

    case TYP_GETFLD:
        {
            // is there a corresponding SetField
            SwSetExpFieldType* pSetTyp = static_cast<SwSetExpFieldType*>(
                pCurShell->GetFldType(RES_SETEXPFLD, rData.m_sPar1));

            if(pSetTyp)
            {
                SwGetExpFieldType* pTyp = static_cast<SwGetExpFieldType*>( pCurShell->GetFldType(
                    0, RES_GETEXPFLD) );
                pFld = new SwGetExpField(pTyp, rData.m_sPar1, pSetTyp->GetType(), nFormatId);
                pFld->SetSubType(nSubType | pSetTyp->GetType());
                bExp = true;
            }
            else
                return false;
            break;
        }

    case TYP_FORMELFLD:
        {
            if(pCurShell->GetFrmType(0,false) & FrmTypeFlags::TABLE)
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

                OUString sFml(comphelper::string::stripStart(rData.m_sPar2, ' '));
                if ( sFml.startsWith("=") )
                {
                    sFml = sFml.copy(1);
                }

                aBoxSet.Put( SwTblBoxFormula( sFml ));
                pCurShell->SetTblBoxFormulaAttrs( aBoxSet );
                pCurShell->UpdateTable();

                pCurShell->EndAllAction();
                return true;

            }
            else
            {
                SwGetExpFieldType* pTyp = static_cast<SwGetExpFieldType*>(
                    pCurShell->GetFldType(0, RES_GETEXPFLD) );
                pFld = new SwGetExpField(pTyp, rData.m_sPar2, nsSwGetSetExpType::GSE_FORMULA, nFormatId);
                pFld->SetSubType(nSubType);
                bExp = true;
            }
            break;
        }
        case TYP_SETREFPAGEFLD:
            pFld = new SwRefPageSetField( static_cast<SwRefPageSetFieldType*>(
                                pCurShell->GetFldType( 0, RES_REFPAGESETFLD ) ),
                                (short)rData.m_sPar2.toInt32(), 0 != nSubType  );
            bPageVar = true;
            break;

        case TYP_GETREFPAGEFLD:
            pFld = new SwRefPageGetField( static_cast<SwRefPageGetFieldType*>(
                            pCurShell->GetFldType( 0, RES_REFPAGEGETFLD ) ), nFormatId );
            bPageVar = true;
            break;
        case TYP_DROPDOWN :
        {
            pFld = new SwDropDownField(pCurShell->GetFldType( 0, RES_DROPDOWN ));
            const sal_Int32 nTokenCount = comphelper::string::getTokenCount(rData.m_sPar2, DB_DELIM);
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(sal_Int32 nToken = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = rData.m_sPar2.getToken(nToken, DB_DELIM);
            static_cast<SwDropDownField*>(pFld)->SetItems(aEntries);
            static_cast<SwDropDownField*>(pFld)->SetName(rData.m_sPar1);
        }
        break;
        default:
        {   OSL_ENSURE(false, "wrong field type");
            return false;
        }
    }
    OSL_ENSURE(pFld, "field not available");

    //the auto language flag has to be set prior to the language!
    pFld->SetAutomaticLanguage(rData.m_bIsAutomaticLanguage);
    sal_uInt16 nLang = GetCurrLanguage();
    pFld->SetLanguage(nLang);

    // insert
    pCurShell->StartAllAction();

    pCurShell->Insert( *pFld );

    if(bExp && bEvalExp)
        pCurShell->UpdateExpFlds(true);

    if(bTbl)
    {
        pCurShell->Left(CRSR_SKIP_CHARS, false, 1, false );
        pCurShell->UpdateFlds(*pFld);
        pCurShell->Right(CRSR_SKIP_CHARS, false, 1, false );
    }
    else if( bPageVar )
        static_cast<SwRefPageGetFieldType*>(pCurShell->GetFldType( 0, RES_REFPAGEGETFLD ))->UpdateFlds();
    else if( TYP_GETREFFLD == rData.m_nTypeId )
        pFld->GetTyp()->ModifyNotification( 0, 0 );

    // delete temporary field
    delete pFld;

    pCurShell->EndAllAction();
    return true;
}

// fields update
void SwFldMgr::UpdateCurFld(sal_uLong nFormat,
                            const OUString& rPar1,
                            const OUString& rPar2,
                            SwField * _pTmpFld) // #111840#
{
    // change format
    OSL_ENSURE(pCurFld, "no field at CursorPos");

    bool bDelete = false;
    SwField *pTmpFld;       // mb: fixed memory leak
    if (NULL != _pTmpFld)
    {
        pTmpFld = _pTmpFld;
    }
    else
    {
        pTmpFld = pCurFld->CopyField();
        bDelete = true;
    }

    SwFieldType* pType   = pTmpFld->GetTyp();
    const sal_uInt16 nTypeId = pTmpFld->GetTypeId();

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
            sPar2 = sPar2.replaceFirst(OUString(' '), OUString(sfx2::cTokenSeparator), &nIndex );
            if (nIndex>=0 && ++nIndex<sPar2.getLength())
            {
                sPar2 = sPar2.replaceFirst(OUString(' '), OUString(sfx2::cTokenSeparator), &nIndex);
            }
            break;
        }

        case TYP_CHAPTERFLD:
        {
            sal_uInt16 nByte = (sal_uInt16)rPar2.toInt32();
            nByte = std::max(sal_uInt16(1), nByte);
            nByte = std::min(nByte, sal_uInt16(MAXLEVEL));
            nByte -= 1;
            static_cast<SwChapterField*>(pTmpFld)->SetLevel((sal_uInt8)nByte);
            bSetPar2 = false;
            break;
        }

        case TYP_SCRIPTFLD:
            static_cast<SwScriptField*>(pTmpFld)->SetCodeURL((bool)nFormat);
            break;

        case TYP_NEXTPAGEFLD:
            if( SVX_NUM_CHAR_SPECIAL == nFormat )
            {
                static_cast<SwPageNumberField*>(pCurFld)->SetUserString( sPar2 );
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
                static_cast<SwPageNumberField*>(pCurFld)->SetUserString( sPar2 );
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
                static_cast<SwGetRefField*>(pTmpFld)->SetSubType( (sal_uInt16)rPar2.toInt32() );
                const sal_Int32 nPos = rPar2.indexOf( '|' );
                if( nPos>=0 )
                    static_cast<SwGetRefField*>(pTmpFld)->SetSeqNo( (sal_uInt16)rPar2.copy( nPos + 1 ).toInt32());
            }
            break;
        case TYP_DROPDOWN:
        {
            sal_Int32 nTokenCount = comphelper::string::getTokenCount(sPar2, DB_DELIM);
            Sequence<OUString> aEntries(nTokenCount);
            OUString* pArray = aEntries.getArray();
            for(sal_Int32 nToken = 0; nToken < nTokenCount; nToken++)
                pArray[nToken] = sPar2.getToken(nToken, DB_DELIM);
            static_cast<SwDropDownField*>(pTmpFld)->SetItems(aEntries);
            static_cast<SwDropDownField*>(pTmpFld)->SetName(sPar1);
            bSetPar1 = bSetPar2 = false;
        }
        break;
        case TYP_AUTHORITY :
        {
            //#i99069# changes to a bibliography field should change the field type
            SwAuthorityField* pAuthorityField = static_cast<SwAuthorityField*>(pTmpFld);
            SwAuthorityFieldType* pAuthorityType = static_cast<SwAuthorityFieldType*>(pType);
            SwAuthEntry aTempEntry;
            for( sal_uInt16 i = 0; i < AUTH_FIELD_END; ++i )
                aTempEntry.SetAuthorField( (ToxAuthorityField)i,
                                rPar1.getToken( i, TOX_STYLE_DELIMITER ));
            if( pAuthorityType->ChangeEntryContent( &aTempEntry ) )
            {
                pType->UpdateFlds();
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
    pTmpFld->ChangeFormat(nFormat);

    if(bSetPar1)
        pTmpFld->SetPar1( sPar1 );
    if( bSetPar2 )
        pTmpFld->SetPar2( sPar2 );

    // kick off update
    if(nTypeId == TYP_DDEFLD ||
       nTypeId == TYP_USERFLD ||
       nTypeId == TYP_USRINPFLD)
    {
        pType->UpdateFlds();
        pSh->SetModified();
    }
    else {
        // mb: #32157
        pSh->SwEditShell::UpdateFlds(*pTmpFld);
        GetCurFld();
    }

    if (bDelete)
        delete pTmpFld;

    pSh->EndAllAction();
}

// explicitly evaluate ExpressionFields
void SwFldMgr::EvalExpFlds(SwWrtShell* pSh)
{
    if (pSh == NULL)
        pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();

    if(pSh)
    {
        pSh->StartAllAction();
        pSh->UpdateExpFlds(true);
        pSh->EndAllAction();
    }
}
sal_uInt16 SwFldMgr::GetCurrLanguage() const
{
    SwWrtShell* pSh = pWrtShell ? pWrtShell : ::lcl_GetShell();
    if( pSh )
        return pSh->GetCurLang();
    return SvtSysLocale().GetLanguageTag().getLanguageType();
}

void SwFieldType::_GetFldName()
{
    static const sal_uInt16 coFldNms[] = {
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
    SwFieldType::s_pFldNames = new std::vector<OUString>;
    SwFieldType::s_pFldNames->reserve(SAL_N_ELEMENTS(coFldNms));
    for( sal_uInt16 nIdx = 0; nIdx < SAL_N_ELEMENTS(coFldNms); ++nIdx )
    {
        const OUString aTmp(SW_RES( coFldNms[ nIdx ] ));
        SwFieldType::s_pFldNames->push_back(MnemonicGenerator::EraseAllMnemonicChars( aTmp ));
    }
}

bool SwFldMgr::ChooseMacro(const OUString&)
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

void SwFldMgr::SetMacroPath(const OUString& rPath)
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

sal_uLong SwFldMgr::GetDefaultFormat(sal_uInt16 nTypeId, bool bIsText, SvNumberFormatter* pFormatter, double* pVal)
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

            sal_uLong nNumFmtTime = (sal_uLong)aTime.GetSec() + (sal_uLong)aTime.GetMin() * 60L +
                          (sal_uLong)aTime.GetHour() * 3600L;

            fValue += (double)nNumFmtTime / 86400.0;

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

Reference<XNumberingTypeInfo> SwFldMgr::GetNumberingInfo() const
{
    if(!xNumberingInfo.is())
    {
        Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
        Reference<XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);
        const_cast<SwFldMgr*>(this)->xNumberingInfo = Reference<XNumberingTypeInfo>(xDefNum, UNO_QUERY);
    }
    return xNumberingInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
