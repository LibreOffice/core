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


#include <svl/smplhint.hxx>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svl/eitem.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/lrspitem.hxx>
#include <fmtcol.hxx>
#include <uitool.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <docstyle.hxx>
#include <docary.hxx>
#include <ccoll.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <cmdid.h>
#include <swstyle.h>
#include <app.hrc>
#include <paratr.hxx>
#include <SwStyleNameMapper.hxx>
#include <svl/cjkoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/intlwrapper.hxx>
#include <numrule.hxx>
#include <fmthdft.hxx>
#include <svx/svxids.hrc>
#include <SwRewriter.hxx>

// The Format names in the list of all names have the
// following family as their first character:

#define cCHAR       (sal_Unicode)'c'
#define cPARA       (sal_Unicode)'p'
#define cFRAME      (sal_Unicode)'f'
#define cPAGE       (sal_Unicode)'g'
#define cNUMRULE    (sal_Unicode)'n'

// At the names' publication, this character is removed again and the
// family is newly generated.

// In addition now there is the Bit bPhysical. In case this Bit is
// TRUE, the Pool-Formatnames are not being submitted.

class SwImplShellAction
{
    SwWrtShell* pSh;
    CurrShell* pCurrSh;
public:
    SwImplShellAction( SwDoc& rDoc );
    ~SwImplShellAction();

    SwWrtShell* GetSh() { return pSh; }
};

SwImplShellAction::SwImplShellAction( SwDoc& rDoc )
    : pCurrSh( 0 )
{
    if( rDoc.GetDocShell() )
        pSh = rDoc.GetDocShell()->GetWrtShell();
    else
        pSh = 0;

    if( pSh )
    {
        pCurrSh = new CurrShell( pSh );
        pSh->StartAllAction();
    }
}

SwImplShellAction::~SwImplShellAction()
{
    if( pCurrSh )
    {
        pSh->EndAllAction();
        delete pCurrSh;
    }
}

// find/create SwCharFormate
// possibly fill Style
static SwCharFmt* lcl_FindCharFmt( SwDoc& rDoc,
                            const String& rName,
                            SwDocStyleSheet* pStyle = 0,
                            bool bCreate = true )
{
    SwCharFmt*  pFmt = 0;
    if( rName.Len() )
    {
        pFmt = rDoc.FindCharFmtByName( rName );
        if( !pFmt && rName == SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] )
        {
            // Standard-Character template
            pFmt = (SwCharFmt*)rDoc.GetDfltCharFmt();
        }

        if( !pFmt && bCreate )
        {   // explore Pool
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
            if(nId != USHRT_MAX)
                pFmt = rDoc.GetCharFmtFromPool(nId);
        }
    }
    if(pStyle)
    {
        if(pFmt)
        {
            pStyle->SetPhysical(sal_True);
            SwFmt* p = pFmt->DerivedFrom();
            if( p && !p->IsDefault() )
                pStyle->PresetParent( p->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pFmt;
}

// find/create ParaFormats
// fill Style
static SwTxtFmtColl* lcl_FindParaFmt(  SwDoc& rDoc,
                                const String& rName,
                                SwDocStyleSheet* pStyle = 0,
                                bool bCreate = true )
{
    SwTxtFmtColl*   pColl = 0;

    if( rName.Len() )
    {
        pColl = rDoc.FindTxtFmtCollByName( rName );
        if( !pColl && bCreate )
        {   // explore Pool
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                pColl = rDoc.GetTxtCollFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pColl)
        {
            pStyle->SetPhysical(sal_True);
            if( pColl->DerivedFrom() && !pColl->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pColl->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( aEmptyStr );

            SwTxtFmtColl& rNext = pColl->GetNextTxtFmtColl();
            pStyle->PresetFollow(rNext.GetName());
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pColl;
}

// Border formats
static SwFrmFmt* lcl_FindFrmFmt(   SwDoc& rDoc,
                            const OUString& rName,
                            SwDocStyleSheet* pStyle = 0,
                            bool bCreate = true )
{
    SwFrmFmt* pFmt = 0;
    if( !rName.isEmpty() )
    {
        pFmt = rDoc.FindFrmFmtByName( rName );
        if( !pFmt && bCreate )
        {   // explore Pool
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT);
            if(nId != USHRT_MAX)
                pFmt = rDoc.GetFrmFmtFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pFmt)
        {
            pStyle->SetPhysical(sal_True);
            if( pFmt->DerivedFrom() && !pFmt->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pFmt->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( aEmptyOUStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pFmt;
}

// Page descriptors
static const SwPageDesc* lcl_FindPageDesc( SwDoc&  rDoc,
                                    const String&    rName,
                                    SwDocStyleSheet* pStyle = 0,
                                    bool bCreate = true )
{
    const SwPageDesc* pDesc = 0;

    if( rName.Len() )
    {
        pDesc = rDoc.FindPageDescByName( rName );
        if( !pDesc && bCreate )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC);
            if(nId != USHRT_MAX)
                pDesc = rDoc.GetPageDescFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pDesc)
        {
            pStyle->SetPhysical(sal_True);
            if(pDesc->GetFollow())
                pStyle->PresetFollow(pDesc->GetFollow()->GetName());
            else
                pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pDesc;
}

static const SwNumRule* lcl_FindNumRule(   SwDoc&  rDoc,
                                    const String&    rName,
                                    SwDocStyleSheet* pStyle = 0,
                                    bool bCreate = true )
{
    const SwNumRule* pRule = 0;

    if( rName.Len() )
    {
        pRule = rDoc.FindNumRulePtr( rName );
        if( !pRule && bCreate )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE);
            if(nId != USHRT_MAX)
                pRule = rDoc.GetNumRuleFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pRule)
        {
            pStyle->SetPhysical(sal_True);
            pStyle->PresetParent( aEmptyStr );
        }
        else
            pStyle->SetPhysical(sal_False);
    }
    return pRule;
}


static sal_uInt16 lcl_FindName(const SwPoolFmtList& rLst, SfxStyleFamily eFam,
    const OUString& rName)
{
    if(!rLst.empty())
    {
        String sSrch = OUString(' ');
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:  sSrch = cCHAR;     break;
        case SFX_STYLE_FAMILY_PARA:  sSrch = cPARA;     break;
        case SFX_STYLE_FAMILY_FRAME: sSrch = cFRAME;    break;
        case SFX_STYLE_FAMILY_PAGE:  sSrch = cPAGE;     break;
        case SFX_STYLE_FAMILY_PSEUDO: sSrch = cNUMRULE; break;
        default:; //prevent warning
        }
        sSrch += rName;
        for(size_t i = 0; i < rLst.size(); ++i)
            if(rLst[i] == sSrch)
                return i;
    }
    return USHRT_MAX;
}

bool FindPhyStyle( SwDoc& rDoc, const String& rName, SfxStyleFamily eFam )
{
    switch( eFam )
    {
    case SFX_STYLE_FAMILY_CHAR :
        return 0 != lcl_FindCharFmt( rDoc, rName, 0, false );
    case SFX_STYLE_FAMILY_PARA :
        return 0 != lcl_FindParaFmt( rDoc, rName, 0, false );
    case SFX_STYLE_FAMILY_FRAME:
        return 0 != lcl_FindFrmFmt( rDoc, rName, 0, false );
    case SFX_STYLE_FAMILY_PAGE :
        return 0 != lcl_FindPageDesc( rDoc, rName, 0, false );
    case SFX_STYLE_FAMILY_PSEUDO:
        return 0 != lcl_FindNumRule( rDoc, rName, 0, false );
    default:; //prevent warning
    }
    return false;
}

// Add Strings to the list of templates
void SwPoolFmtList::Append( char cChar, const String& rStr )
{
    String aStr = OUString(cChar);
    aStr += rStr;
    for(std::vector<String>::const_iterator i = begin(); i != end(); ++i)
        if(*i == aStr)
            return;
    push_back(aStr);
}

// Erase the list completely
void SwPoolFmtList::Erase()
{
    clear();
}

// UI-sided implementation of StyleSheets
// uses the Core-Engine
SwDocStyleSheet::SwDocStyleSheet(   SwDoc&          rDocument,
                                    const OUString&           rName,
                                    SwDocStyleSheetPool*    _rPool,
                                    SfxStyleFamily          eFam,
                                    sal_uInt16                  _nMask) :

    SfxStyleSheetBase( rName, _rPool, eFam, _nMask ),
    pCharFmt(0),
    pColl(0),
    pFrmFmt(0),
    pDesc(0),
    pNumRule(0),

    rDoc(rDocument),
    aCoreSet(GetPool().GetPool(),
            RES_CHRATR_BEGIN,       RES_CHRATR_END - 1,
            RES_PARATR_BEGIN,       RES_PARATR_END - 1,
            RES_PARATR_LIST_BEGIN,  RES_PARATR_LIST_END - 1,
            RES_FRMATR_BEGIN,       RES_FRMATR_END - 1,
            RES_UNKNOWNATR_BEGIN,   RES_UNKNOWNATR_END-1,
            SID_ATTR_PAGE,          SID_ATTR_PAGE_EXT1,
            SID_ATTR_PAGE_HEADERSET,SID_ATTR_PAGE_FOOTERSET,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            FN_PARAM_FTN_INFO,      FN_PARAM_FTN_INFO,
            SID_ATTR_PARA_MODEL,    SID_ATTR_PARA_MODEL,
            SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM,
            SID_SWREGISTER_MODE,    SID_SWREGISTER_MODE,
            SID_SWREGISTER_COLLECTION, SID_SWREGISTER_COLLECTION,
            FN_COND_COLL,           FN_COND_COLL,
            SID_ATTR_AUTO_STYLE_UPDATE, SID_ATTR_AUTO_STYLE_UPDATE,
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,
            SID_PARA_BACKGRND_DESTINATION,  SID_ATTR_BRUSH_CHAR,
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,
            0),
    bPhysical(sal_False)
{
    nHelpId = UCHAR_MAX;
}


SwDocStyleSheet::SwDocStyleSheet( const SwDocStyleSheet& rOrg) :
    SfxStyleSheetBase(rOrg),
    pCharFmt(rOrg.pCharFmt),
    pColl(rOrg.pColl),
    pFrmFmt(rOrg.pFrmFmt),
    pDesc(rOrg.pDesc),
    pNumRule(rOrg.pNumRule),
    rDoc(rOrg.rDoc),
    aCoreSet(rOrg.aCoreSet),
    bPhysical(rOrg.bPhysical)
{
}


 SwDocStyleSheet::~SwDocStyleSheet()
{
}

// Reset
void  SwDocStyleSheet::Reset()
{
    aName = "";
    aFollow = "";
    aParent = "";
    SetPhysical(sal_False);
}

// virtual methods
void SwDocStyleSheet::SetHidden( sal_Bool bValue )
{
    bool bChg = false;
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    SwFmt* pFmt = 0;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pFmt = rDoc.FindCharFmtByName( aName );
            if ( pFmt )
            {
                pFmt->SetHidden( bValue );
                bChg = true;
            }
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFmt = rDoc.FindTxtFmtCollByName( aName );
            if ( pFmt )
            {
                pFmt->SetHidden( bValue );
                bChg = true;
            }
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFmt = rDoc.FindFrmFmtByName( aName );
            if ( pFmt )
            {
                pFmt->SetHidden( bValue );
                bChg = true;
            }
            break;

        case SFX_STYLE_FAMILY_PAGE:
            {
                SwPageDesc* pPgDesc = rDoc.FindPageDescByName( aName );
                if ( pPgDesc )
                {
                    pPgDesc->SetHidden( bValue );
                    bChg = true;
                }
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                SwNumRule* pRule = rDoc.FindNumRulePtr( aName );
                if ( pRule )
                {
                    pRule->SetHidden( bValue );
                    bChg = true;
                }
            }
        default:;
    }

    if( bChg )
    {
        pPool->First();  // internal list has to be updated
        pPool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
        SwEditShell* pSh = rDoc.GetEditShell();
        if( pSh )
            pSh->CallChgLnk();
    }
}

sal_Bool SwDocStyleSheet::IsHidden( ) const
{
    sal_Bool bRet = sal_False;

    SwFmt* pFmt = 0;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pFmt = rDoc.FindCharFmtByName( aName );
            bRet = pFmt && pFmt->IsHidden( );
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFmt = rDoc.FindTxtFmtCollByName( aName );
            bRet = pFmt && pFmt->IsHidden( );
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFmt = rDoc.FindFrmFmtByName( aName );
            bRet = pFmt && pFmt->IsHidden( );
            break;

        case SFX_STYLE_FAMILY_PAGE:
            {
                SwPageDesc* pPgDesc = rDoc.FindPageDescByName( aName );
                bRet = pPgDesc && pPgDesc->IsHidden( );
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            {
                SwNumRule* pRule = rDoc.FindNumRulePtr( aName );
                bRet = pRule && pRule->IsHidden( );
            }
        default:;
    }

    return bRet;
}

const OUString&  SwDocStyleSheet::GetParent() const
{
    if( !bPhysical )
    {
        // check if it's already in document
        SwFmt* pFmt = 0;
        SwGetPoolIdFromName eGetType;
        switch(nFamily)
        {
        case SFX_STYLE_FAMILY_CHAR:
            pFmt = rDoc.FindCharFmtByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_CHRFMT;
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFmt = rDoc.FindTxtFmtCollByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL;
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFmt = rDoc.FindFrmFmtByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_FRMFMT;
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
        default:
            return aEmptyOUStr;       // there's no parent
        }

        String sTmp;
        if( !pFmt )         // not yet there, so default Parent
        {
            sal_uInt16 i = SwStyleNameMapper::GetPoolIdFromUIName( aName, eGetType );
            i = ::GetPoolParent( i );
            if( i && USHRT_MAX != i )
                SwStyleNameMapper::FillUIName( i, sTmp );
        }
        else
        {
            SwFmt* p = pFmt->DerivedFrom();
            if( p && !p->IsDefault() )
                sTmp = p->GetName();
        }
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->aParent = sTmp;
    }
    return aParent;
}

// Follower
const OUString&  SwDocStyleSheet::GetFollow() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->FillStyleSheet( FillAllInfo );
    }
    return aFollow;
}

// What Linkage is possible
bool  SwDocStyleSheet::HasFollowSupport() const
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_PAGE : return true;
        case SFX_STYLE_FAMILY_FRAME:
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_PSEUDO: return false;
        default:
            OSL_ENSURE(!this, "unknown style family");
    }
    return false;
}

// Parent ?
bool  SwDocStyleSheet::HasParentSupport() const
{
    bool bRet = false;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_FRAME: bRet = true;
        default:; //prevent warning
    }
    return bRet;
}


bool  SwDocStyleSheet::HasClearParentSupport() const
{
    bool bRet = false;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA :
        case SFX_STYLE_FAMILY_CHAR :
        case SFX_STYLE_FAMILY_FRAME: bRet = true;
        default:; //prevent warning
    }
    return bRet;
}

// determine textual description
OUString  SwDocStyleSheet::GetDescription(SfxMapUnit eUnit)
{
    IntlWrapper aIntlWrapper( SvtSysLocale().GetLanguageTag() );

    OUString sPlus(" + ");
    if ( SFX_STYLE_FAMILY_PAGE == nFamily )
    {
        if( !pSet )
            GetItemSet();

        SfxItemIter aIter( *pSet );
        String aDesc;
        const SfxPoolItem* pItem = aIter.FirstItem();

        while ( pItem )
        {
            if(!IsInvalidItem(pItem))
                switch ( pItem->Which() )
                {
                    case RES_LR_SPACE:
                    case SID_ATTR_PAGE_SIZE:
                    case SID_ATTR_PAGE_MAXSIZE:
                    case SID_ATTR_PAGE_PAPERBIN:
                    case SID_ATTR_PAGE_APP:
                    case SID_ATTR_BORDER_INNER:
                        break;
                    default:
                    {
                        OUString aItemPresentation;
                        if ( !IsInvalidItem( pItem ) &&
                             pPool->GetPool().GetPresentation(
                                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                                eUnit, aItemPresentation, &aIntlWrapper ) )
                        {
                            if ( aDesc.Len() && aItemPresentation.getLength() )
                                aDesc += sPlus;
                            aDesc += aItemPresentation;
                        }
                    }
                }
            pItem = aIter.NextItem();
        }
        return aDesc;
    }
    else if ( SFX_STYLE_FAMILY_FRAME == nFamily ||
              SFX_STYLE_FAMILY_PARA == nFamily)
    {
        if( !pSet )
            GetItemSet();

        SfxItemIter aIter( *pSet );
        String aDesc;
        const SfxPoolItem* pItem = aIter.FirstItem();

        OUString sPageNum;
        String sModel, sBreak;
        bool bHasWesternFontPrefix = false;
        bool bHasCJKFontPrefix = false;
        SvtCJKOptions aCJKOptions;

        while ( pItem )
        {
            if(!IsInvalidItem(pItem))
                switch ( pItem->Which() )
                {
                    case SID_ATTR_AUTO_STYLE_UPDATE:
                    case SID_PARA_BACKGRND_DESTINATION:
                    case RES_PAGEDESC:
                    //CTL not yet supported
                    case RES_CHRATR_CTL_FONT:
                    case RES_CHRATR_CTL_FONTSIZE:
                    case RES_CHRATR_CTL_LANGUAGE:
                    case RES_CHRATR_CTL_POSTURE:
                    case RES_CHRATR_CTL_WEIGHT:
                        break;
                    default:
                    {
                        OUString aItemPresentation;
                        if ( !IsInvalidItem( pItem ) &&
                             pPool->GetPool().GetPresentation(
                                *pItem, SFX_ITEM_PRESENTATION_COMPLETE,
                                eUnit, aItemPresentation, &aIntlWrapper ) )
                        {
                            bool bIsDefault = false;
                            switch ( pItem->Which() )
                            {
                                case SID_ATTR_PARA_PAGENUM:
                                    sPageNum = aItemPresentation;
                                    break;
                                case SID_ATTR_PARA_MODEL:
                                    sModel = aItemPresentation;
                                    break;
                                case RES_BREAK:
                                    sBreak = aItemPresentation;
                                    break;
                                case RES_CHRATR_CJK_FONT:
                                case RES_CHRATR_CJK_FONTSIZE:
                                case RES_CHRATR_CJK_LANGUAGE:
                                case RES_CHRATR_CJK_POSTURE:
                                case RES_CHRATR_CJK_WEIGHT:
                                if(aCJKOptions.IsCJKFontEnabled())
                                    bIsDefault = true;
                                if(!bHasCJKFontPrefix)
                                {
                                    aItemPresentation = SW_RESSTR(STR_CJK_FONT) + aItemPresentation;
                                    bHasCJKFontPrefix = true;
                                }
                                break;
                                case RES_CHRATR_FONT:
                                case RES_CHRATR_FONTSIZE:
                                case RES_CHRATR_LANGUAGE:
                                case RES_CHRATR_POSTURE:
                                case RES_CHRATR_WEIGHT:
                                if(!bHasWesternFontPrefix)
                                {
                                    aItemPresentation = SW_RESSTR(STR_WESTERN_FONT) + aItemPresentation;
                                    bHasWesternFontPrefix = true;
                                    bIsDefault = true;
                                }
                                // no break;
                                default:
                                    bIsDefault = true;
                            }
                            if(bIsDefault)
                            {
                                if ( aDesc.Len() && aItemPresentation.getLength() )
                                    aDesc += sPlus;
                                aDesc += aItemPresentation;
                            }
                        }
                    }
                }
            pItem = aIter.NextItem();
        }
        // Special treatment for Break, Page template and Site offset
        if(sBreak.Len() && !sModel.Len())  // when Model, break is invalid
        {
            if(aDesc.Len())
                aDesc += sPlus;
            aDesc += sBreak;
        }
        if(sModel.Len())
        {
            if(aDesc.Len())
                aDesc += sPlus;
            aDesc += SW_RESSTR(STR_PAGEBREAK);
            aDesc += sPlus;
            aDesc += sModel;
            if (sPageNum != "0")
            {
                aDesc += sPlus;
                aDesc += SW_RESSTR(STR_PAGEOFFSET);
                aDesc += sPageNum;
            }
        }
        return aDesc;
    }
    else if( SFX_STYLE_FAMILY_PSEUDO == nFamily )
    {
        return aEmptyStr;
    }

    return SfxStyleSheetBase::GetDescription(eUnit);
}


OUString  SwDocStyleSheet::GetDescription()
{
    return GetDescription(SFX_MAPUNIT_CM);
}

// Set names
bool  SwDocStyleSheet::SetName( const OUString& rStr)
{
    if( rStr.isEmpty() )
        return false;

    if( aName != rStr )
    {
        if( !SfxStyleSheetBase::SetName( rStr ))
            return false;
    }
    else if(!bPhysical)
        FillStyleSheet( FillPhysical );

    int bChg = false;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        {
            OSL_ENSURE(pCharFmt, "SwCharFormat missing!");
            if( pCharFmt && pCharFmt->GetName() != rStr )
            {
                pCharFmt->SetName( rStr );
                bChg = true;
            }
            break;
        }
        case SFX_STYLE_FAMILY_PARA :
        {
            OSL_ENSURE(pColl, "Collection missing!");
            if( pColl && pColl->GetName() != rStr )
            {
                if (pColl->GetName().Len() > 0)
                    rDoc.RenameFmt(*pColl, rStr);
                else
                    pColl->SetName(rStr);

                bChg = true;
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            OSL_ENSURE(pFrmFmt, "FrmFmt missing!");
            if( pFrmFmt && pFrmFmt->GetName() != rStr )
            {
                if (pFrmFmt->GetName().Len() > 0)
                    rDoc.RenameFmt(*pFrmFmt, rStr);
                else
                    pFrmFmt->SetName( rStr );

                bChg = true;
            }
            break;
        }
        case SFX_STYLE_FAMILY_PAGE :
            OSL_ENSURE(pDesc, "PageDesc missing!");
            if( pDesc && pDesc->GetName() != rStr )
            {
                // Set PageDesc - copy with earlier one - probably not
                // necessary for setting the name. So here we allow a
                // cast.
                SwPageDesc aPageDesc(*((SwPageDesc*)pDesc));
                String aOldName(aPageDesc.GetName());

                aPageDesc.SetName( rStr );
                bool const bDoesUndo = rDoc.GetIDocumentUndoRedo().DoesUndo();

                rDoc.GetIDocumentUndoRedo().DoUndo(aOldName.Len() > 0);
                rDoc.ChgPageDesc(aOldName, aPageDesc);
                rDoc.GetIDocumentUndoRedo().DoUndo(bDoesUndo);

                rDoc.SetModified();
                bChg = true;
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            OSL_ENSURE(pNumRule, "NumRule missing!");

            if (pNumRule)
            {
                OUString aOldName = pNumRule->GetName();

                if (!aOldName.isEmpty())
                {
                    if ( aOldName != rStr &&
                         rDoc.RenameNumRule(aOldName, rStr))
                    {
                        pNumRule = rDoc.FindNumRulePtr(rStr);
                        rDoc.SetModified();

                        bChg = true;
                    }
                }
                else
                {
                    // #i91400#
                    ((SwNumRule*)pNumRule)->SetName( rStr, rDoc );
                    rDoc.SetModified();

                    bChg = true;
                }
            }

            break;

        default:
            OSL_ENSURE(!this, "unknown style family");
    }

    if( bChg )
    {
        pPool->First();  // internal list has to be updated
        pPool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED, *this ) );
        SwEditShell* pSh = rDoc.GetEditShell();
        if( pSh )
            pSh->CallChgLnk();
    }
    return true;
}

// hierarchy of deduction
bool   SwDocStyleSheet::SetParent( const OUString& rStr)
{
    SwFmt* pFmt = 0, *pParent = 0;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            OSL_ENSURE( pCharFmt, "SwCharFormat missing!" );
            if( 0 != ( pFmt = pCharFmt ) && !rStr.isEmpty() )
                pParent = lcl_FindCharFmt(rDoc, rStr);
            break;

        case SFX_STYLE_FAMILY_PARA :
            OSL_ENSURE( pColl, "Collektion missing!");
            if( 0 != ( pFmt = pColl ) && !rStr.isEmpty() )
                pParent = lcl_FindParaFmt( rDoc, rStr );
            break;

        case SFX_STYLE_FAMILY_FRAME:
            OSL_ENSURE(pFrmFmt, "FrameFormat missing!");
            if( 0 != ( pFmt = pFrmFmt ) && !rStr.isEmpty() )
                pParent = lcl_FindFrmFmt( rDoc, rStr );
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
            break;
        default:
            OSL_ENSURE(!this, "unknown style family");
    }

    bool bRet = false;
    if( pFmt && pFmt->DerivedFrom() &&
        pFmt->DerivedFrom()->GetName() != rStr )
    {
        {
            SwImplShellAction aTmp( rDoc );
            bRet = pFmt->SetDerivedFrom( pParent );
        }

        if( bRet )
        {
            aParent = rStr;
            pPool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED,
                            *this ) );
        }
    }

    return bRet;
}

// Set Follower
bool   SwDocStyleSheet::SetFollow( const OUString& rStr)
{
    if( !rStr.isEmpty() && !SfxStyleSheetBase::SetFollow( rStr ))
        return false;

    SwImplShellAction aTmpSh( rDoc );
    switch(nFamily)
    {
    case SFX_STYLE_FAMILY_PARA :
    {
        OSL_ENSURE(pColl, "Collection missing!");
        if( pColl )
        {
            SwTxtFmtColl* pFollow = pColl;
            if( !rStr.isEmpty() && 0 == (pFollow = lcl_FindParaFmt(rDoc, rStr) ))
                pFollow = pColl;

            pColl->SetNextTxtFmtColl(*pFollow);
        }
        break;
    }
    case SFX_STYLE_FAMILY_PAGE :
    {
        OSL_ENSURE(pDesc, "PageDesc missing!");
        if( pDesc )
        {
            const SwPageDesc* pFollowDesc = !rStr.isEmpty()
                                            ? lcl_FindPageDesc(rDoc, rStr)
                                            : 0;
            sal_uInt16 nId;
            if( pFollowDesc != pDesc->GetFollow() &&
                rDoc.FindPageDescByName( pDesc->GetName(), &nId ) )
            {
                SwPageDesc aDesc( *pDesc );
                aDesc.SetFollow( pFollowDesc );
                rDoc.ChgPageDesc( nId, aDesc );
                pDesc = &rDoc.GetPageDesc( nId );
            }
        }
        break;
    }
    case SFX_STYLE_FAMILY_CHAR:
    case SFX_STYLE_FAMILY_FRAME:
    case SFX_STYLE_FAMILY_PSEUDO:
        break;
    default:
        OSL_ENSURE(!this, "unknwown style family");
    }

    return true;
}

// extract ItemSet to Name and Family, Mask
SfxItemSet&   SwDocStyleSheet::GetItemSet()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
        case SFX_STYLE_FAMILY_PARA:
        case SFX_STYLE_FAMILY_FRAME:
            {
                SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                aBoxInfo.SetTable( sal_False );
                aBoxInfo.SetDist( sal_True );   // always show gap field
                aBoxInfo.SetMinDist( sal_True );// set minimum size in tables and paragraphs
                aBoxInfo.SetDefDist( MIN_BORDER_DIST );// always set Default-Gap
                    // Single lines can only have DontCare-Status in tables
                aBoxInfo.SetValid( VALID_DISABLE, sal_True );

                if( nFamily == SFX_STYLE_FAMILY_CHAR )
                {
                    SAL_WARN_IF(!pCharFmt, "sw.app.docstyle", "Where's SwCharFmt");
                    aCoreSet.Put(pCharFmt->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );

                    if(pCharFmt->DerivedFrom())
                        aCoreSet.SetParent(&pCharFmt->DerivedFrom()->GetAttrSet());
                }
                else if ( nFamily == SFX_STYLE_FAMILY_PARA )
                {
                    OSL_ENSURE(pColl, "Where's Collection");
                    aCoreSet.Put(pColl->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );
                    aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pColl->IsAutoUpdateFmt()));

                    if(pColl->DerivedFrom())
                        aCoreSet.SetParent(&pColl->DerivedFrom()->GetAttrSet());
                }
                else
                {
                    OSL_ENSURE(pFrmFmt, "Where's FrmFmt");
                    aCoreSet.Put(pFrmFmt->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );
                    aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pFrmFmt->IsAutoUpdateFmt()));

                    if(pFrmFmt->DerivedFrom())
                        aCoreSet.SetParent(&pFrmFmt->DerivedFrom()->GetAttrSet());
                }
            }
            break;

        case SFX_STYLE_FAMILY_PAGE :
            {
                OSL_ENSURE(pDesc, "No PageDescriptor");
                ::PageDescToItemSet(*((SwPageDesc*)pDesc), aCoreSet);
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                OSL_ENSURE(pNumRule, "No NumRule");
                SvxNumRule aRule = pNumRule->MakeSvxNumRule();
                aCoreSet.Put(SvxNumBulletItem(aRule));
            }
            break;

        default:
            OSL_ENSURE(!this, "unknown style family");
    }
    // Member of Baseclass
    pSet = &aCoreSet;

    return aCoreSet;
}

void SwDocStyleSheet::MergeIndentAttrsOfListStyle( SfxItemSet& rSet )
{
    if ( nFamily != SFX_STYLE_FAMILY_PARA )
    {
        return;
    }

    OSL_ENSURE( pColl, "<SwDocStyleSheet::MergeIndentAttrsOfListStyle(..)> - missing paragraph style");
    if ( pColl->AreListLevelIndentsApplicable() )
    {
        OSL_ENSURE( pColl->GetItemState( RES_PARATR_NUMRULE ) == SFX_ITEM_SET,
                "<SwDocStyleSheet::MergeIndentAttrsOfListStyle(..)> - list level indents are applicable at paragraph style, but no list style found. Serious defect -> please inform OD." );
        const String sNumRule = pColl->GetNumRule().GetValue();
        if( sNumRule.Len() )
        {
            const SwNumRule* pRule = rDoc.FindNumRulePtr( sNumRule );
            if( pRule )
            {
                const SwNumFmt& rFmt = pRule->Get( 0 );
                if ( rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    SvxLRSpaceItem aLR( RES_LR_SPACE );
                    aLR.SetTxtLeft( rFmt.GetIndentAt() );
                    aLR.SetTxtFirstLineOfst( static_cast<short>(rFmt.GetFirstLineIndent()) );
                    rSet.Put( aLR );
                }
            }
        }
    }
}

// handling of parameter <bResetIndentAttrsAtParagraphStyle>
void SwDocStyleSheet::SetItemSet( const SfxItemSet& rSet,
                                  const bool bResetIndentAttrsAtParagraphStyle )
{
    // if applicable determine format first
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    SwImplShellAction aTmpSh( rDoc );

    OSL_ENSURE( &rSet != &aCoreSet, "SetItemSet with own Set is not allowed" );

    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        SwRewriter aRewriter;
        aRewriter.AddRule( UndoArg1, GetName() );
        rDoc.GetIDocumentUndoRedo().StartUndo( UNDO_INSFMTATTR, &aRewriter );
    }

    SwFmt* pFmt = 0;
    SwPageDesc* pNewDsc = 0;
    sal_uInt16 nPgDscPos = 0;

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            {
                OSL_ENSURE(pCharFmt, "Where's CharFormat");
                pFmt = pCharFmt;
            }
            break;

        case SFX_STYLE_FAMILY_PARA :
        {
            OSL_ENSURE(pColl, "Where's Collection");
            const SfxPoolItem* pAutoUpdate;
            if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,sal_False, &pAutoUpdate ))
            {
                pColl->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
            }

            const SwCondCollItem* pCondItem;
            if( SFX_ITEM_SET != rSet.GetItemState( FN_COND_COLL, sal_False,
                (const SfxPoolItem**)&pCondItem ))
                pCondItem = 0;

            if( RES_CONDTXTFMTCOLL == pColl->Which() && pCondItem )
            {
                SwFmt* pFindFmt;
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for(sal_uInt16 i = 0; i < COND_COMMAND_COUNT; i++)
                {
                    SwCollCondition aCond( 0, pCmds[ i ].nCnd, pCmds[ i ].nSubCond );
                    ((SwConditionTxtFmtColl*)pColl)->RemoveCondition( aCond );
                    const String& rStyle = pCondItem->GetStyle( i );
                    if( rStyle.Len() &&
                        0 != ( pFindFmt = lcl_FindParaFmt( rDoc, rStyle, 0, true )))
                    {
                        aCond.RegisterToFormat( *pFindFmt );
                        ((SwConditionTxtFmtColl*)pColl)->InsertCondition( aCond );
                    }
                }

                // Update document to new conditions
                SwCondCollCondChg aMsg( pColl );
                pColl->ModifyNotification( &aMsg, &aMsg );
            }
            else if( pCondItem && !pColl->GetDepends() )
            {
                // no conditional template, then first create and adopt
                // all important values
                SwConditionTxtFmtColl* pCColl = rDoc.MakeCondTxtFmtColl(
                        pColl->GetName(), (SwTxtFmtColl*)pColl->DerivedFrom() );
                if( pColl != &pColl->GetNextTxtFmtColl() )
                    pCColl->SetNextTxtFmtColl( pColl->GetNextTxtFmtColl() );

                if( pColl->IsAssignedToListLevelOfOutlineStyle())
                    pCColl->AssignToListLevelOfOutlineStyle(pColl->GetAssignedOutlineStyleLevel());
                else
                    pCColl->DeleteAssignmentToListLevelOfOutlineStyle();



                SwTxtFmtColl* pFindFmt;
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for( sal_uInt16 i = 0; i < COND_COMMAND_COUNT; ++i )
                {
                    const String& rStyle = pCondItem->GetStyle( i );
                    if( rStyle.Len() &&
                        0 != ( pFindFmt = lcl_FindParaFmt( rDoc, rStyle, 0, true )))
                    {
                        pCColl->InsertCondition( SwCollCondition( pFindFmt,
                                    pCmds[ i ].nCnd, pCmds[ i ].nSubCond ) );
                    }
                }

                rDoc.DelTxtFmtColl( pColl );
                pColl = pCColl;
            }
            if ( bResetIndentAttrsAtParagraphStyle &&
                 rSet.GetItemState( RES_PARATR_NUMRULE, sal_False, 0 ) == SFX_ITEM_SET &&
                 rSet.GetItemState( RES_LR_SPACE, sal_False, 0 ) != SFX_ITEM_SET &&
                 pColl->GetItemState( RES_LR_SPACE, sal_False, 0 ) == SFX_ITEM_SET )
            {
                rDoc.ResetAttrAtFormat( RES_LR_SPACE, *pColl );
            }

            // #i56252: If a standard numbering style is assigned to a standard paragraph style
            // we have to create a physical instance of the numbering style. If we do not and
            // neither the paragraph style nor the numbering style is used in the document
            // the numbering style will not be saved with the document and the assignment got lost.
            const SfxPoolItem* pNumRuleItem = 0;
            if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, sal_False, &pNumRuleItem ) )
            {   // Setting a numbering rule?
                String sNumRule = ((SwNumRuleItem*)pNumRuleItem)->GetValue();
                if( sNumRule.Len() )
                {
                    SwNumRule* pRule = rDoc.FindNumRulePtr( sNumRule );
                    if( !pRule )
                    {   // Numbering rule not in use yet.
                        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sNumRule, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
                        if( USHRT_MAX != nPoolId ) // It's a standard numbering rule
                        {
                            pRule = rDoc.GetNumRuleFromPool( nPoolId ); // Create numbering rule (physical)
                        }
                    }
                }
            }

            pFmt = pColl;

            sal_uInt16 nId = pColl->GetPoolFmtId() &
                            ~ ( COLL_GET_RANGE_BITS | POOLGRP_NOCOLLID );
            switch( GetMask() & ( 0x0fff & ~SWSTYLEBIT_CONDCOLL ) )
            {
                case SWSTYLEBIT_TEXT:
                    nId |= COLL_TEXT_BITS;
                    break;
                case SWSTYLEBIT_CHAPTER:
                    nId |= COLL_DOC_BITS;
                    break;
                case SWSTYLEBIT_LIST:
                    nId |= COLL_LISTS_BITS;
                    break;
                case SWSTYLEBIT_IDX:
                    nId |= COLL_REGISTER_BITS;
                    break;
                case SWSTYLEBIT_EXTRA:
                    nId |= COLL_EXTRA_BITS;
                    break;
                case SWSTYLEBIT_HTML:
                    nId |= COLL_HTML_BITS;
                    break;
            }
            pColl->SetPoolFmtId( nId );
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            OSL_ENSURE(pFrmFmt, "Where's FrmFmt");
            const SfxPoolItem* pAutoUpdate;
            if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,sal_False, &pAutoUpdate ))
            {
                pFrmFmt->SetAutoUpdateFmt(((const SfxBoolItem*)pAutoUpdate)->GetValue());
            }
            pFmt = pFrmFmt;
        }
        break;

        case SFX_STYLE_FAMILY_PAGE :
            {
                OSL_ENSURE(pDesc, "Where's PageDescriptor");

                if( rDoc.FindPageDescByName( pDesc->GetName(), &nPgDscPos ))
                {
                    pNewDsc = new SwPageDesc( *pDesc );
                    // #i48949# - no undo actions for the
                    // copy of the page style
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    rDoc.CopyPageDesc(*pDesc, *pNewDsc); // #i7983#

                    pFmt = &pNewDsc->GetMaster();
                }
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                OSL_ENSURE(pNumRule, "Where's NumRule");

                if (!pNumRule)
                    break;

                const SfxPoolItem* pItem;
                switch( rSet.GetItemState( SID_ATTR_NUMBERING_RULE, sal_False, &pItem ))
                {
                case SFX_ITEM_SET:
                {
                    SvxNumRule* pSetRule = ((SvxNumBulletItem*)pItem)->GetNumRule();
                    pSetRule->UnLinkGraphics();
                    SwNumRule aSetRule(*pNumRule);
                    aSetRule.SetSvxRule(*pSetRule, &rDoc);
                    rDoc.ChgNumRuleFmts( aSetRule );
                }
                break;
                case SFX_ITEM_DONTCARE:
                    // set NumRule to default values
                    // what are the default values?
                    {
                        SwNumRule aRule( pNumRule->GetName(),
                                         // #i89178#
                                         numfunc::GetDefaultPositionAndSpaceMode() );
                        rDoc.ChgNumRuleFmts( aRule );
                    }
                    break;
                }
            }
            break;

        default:
            OSL_ENSURE(!this, "unknown style family");
    }

    if( pFmt && rSet.Count())
    {
        SfxItemIter aIter( rSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( true )
        {
            if( IsInvalidItem( pItem ) )            // Clear
            {
                // use method <SwDoc::ResetAttrAtFormat(..)> in order to
                // create an Undo object for the attribute reset.
                rDoc.ResetAttrAtFormat( rSet.GetWhichByPos(aIter.GetCurPos()),
                                        *pFmt );
            }

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
        SfxItemSet aSet(rSet);
        aSet.ClearInvalidItems();

        aCoreSet.ClearItem();

        if( pNewDsc )
        {
            ::ItemSetToPageDesc( aSet, *pNewDsc );
            rDoc.ChgPageDesc( nPgDscPos, *pNewDsc );
            pDesc = &rDoc.GetPageDesc( nPgDscPos );
            rDoc.PreDelPageDesc(pNewDsc); // #i7983#
            delete pNewDsc;
        }
        else
            rDoc.ChgFmt(*pFmt, aSet);       // put all that is set
    }
    else
    {
        aCoreSet.ClearItem();
        if( pNewDsc )       // we still need to delete it
        {
            rDoc.PreDelPageDesc(pNewDsc); // #i7983#
            delete pNewDsc;
        }
    }

    if (rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_END, 0);
    }
}

static void lcl_SaveStyles( sal_uInt16 nFamily, std::vector<void*>& rArr, SwDoc& rDoc )
{
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            const SwCharFmts& rTbl = *rDoc.GetCharFmts();
            for( sal_uInt16 n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTbl[ n ] );
            }
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            const SwTxtFmtColls& rTbl = *rDoc.GetTxtFmtColls();
            for( sal_uInt16 n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTbl[ n ] );
            }
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            const SwFrmFmts& rTbl = *rDoc.GetFrmFmts();
            for( sal_uInt16 n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTbl[ n ] );
            }
        }
        break;

    case SFX_STYLE_FAMILY_PAGE:
        {
            for( sal_uInt16 n = 0, nCnt = rDoc.GetPageDescCnt(); n < nCnt; ++n )
            {
                rArr.push_back( &rDoc.GetPageDesc( n ) );
            }
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            const SwNumRuleTbl& rTbl = rDoc.GetNumRuleTbl();
            for( sal_uInt16 n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTbl[ n ] );
            }
        }
        break;
    }
}

static bool lcl_Contains(const std::vector<void*>& rArr, const void* p)
{
    return std::find( rArr.begin(), rArr.end(), p ) != rArr.end();
}

static void lcl_DeleteInfoStyles( sal_uInt16 nFamily, std::vector<void*>& rArr, SwDoc& rDoc )
{
    sal_uInt16 n, nCnt;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            std::deque<sal_uInt16> aDelArr;
            const SwCharFmts& rTbl = *rDoc.GetCharFmts();
            for( n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTbl[ n ] ))
                    aDelArr.push_front( n );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelCharFmt( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_PARA :
        {
            std::deque<sal_uInt16> aDelArr;
            const SwTxtFmtColls& rTbl = *rDoc.GetTxtFmtColls();
            for( n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTbl[ n ] ))
                    aDelArr.push_front( n );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelTxtFmtColl( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_FRAME:
        {
            std::deque<SwFrmFmt*> aDelArr;
            const SwFrmFmts& rTbl = *rDoc.GetFrmFmts();
            for( n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTbl[ n ] ))
                    aDelArr.push_front( rTbl[ n ] );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelFrmFmt( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_PAGE:
        {
            std::deque<sal_uInt16> aDelArr;
            for( n = 0, nCnt = rDoc.GetPageDescCnt(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, &rDoc.GetPageDesc( n ) ))
                    aDelArr.push_front( n );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelPageDesc( aDelArr[ n ] );
        }
        break;


    case SFX_STYLE_FAMILY_PSEUDO:
        {
            std::deque<SwNumRule*> aDelArr;
            const SwNumRuleTbl& rTbl = rDoc.GetNumRuleTbl();
            for( n = 0, nCnt = rTbl.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTbl[ n ] ))
                    aDelArr.push_front( rTbl[ n ] );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelNumRule( aDelArr[ n ]->GetName() );
        }
        break;
    }
}

// determine the format
sal_Bool SwDocStyleSheet::FillStyleSheet( FillStyleType eFType )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nPoolId = USHRT_MAX;
    SwFmt* pFmt = 0;

    bool bCreate = FillPhysical == eFType;
    bool bDeleteInfo = false;
    bool bFillOnlyInfo = FillAllInfo == eFType;
    std::vector<void*> aDelArr;

    switch(nFamily)
    {
    case SFX_STYLE_FAMILY_CHAR:
        pCharFmt = lcl_FindCharFmt(rDoc, aName, this, bCreate );
        bPhysical = 0 != pCharFmt;
        if( bFillOnlyInfo && !bPhysical )
        {
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pCharFmt = lcl_FindCharFmt(rDoc, aName, this, true );
        }

        pFmt = pCharFmt;
        if( !bCreate && !pFmt )
        {
            if( aName == SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                            RES_POOLCOLL_TEXT_BEGIN ] )
                nPoolId = 0;
            else
                nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
        }

        bRet = 0 != pCharFmt || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pCharFmt = 0;
        break;

    case SFX_STYLE_FAMILY_PARA:
        {
            pColl = lcl_FindParaFmt(rDoc, aName, this, bCreate);
            bPhysical = 0 != pColl;
            if( bFillOnlyInfo && !bPhysical )
            {
                bDeleteInfo = true;
                ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
                pColl = lcl_FindParaFmt(rDoc, aName, this, true );
            }

            pFmt = pColl;
            if( pColl )
                PresetFollow( pColl->GetNextTxtFmtColl().GetName() );
            else if( !bCreate )
                nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );

            bRet = 0 != pColl || USHRT_MAX != nPoolId;

            if( bDeleteInfo )
                pColl = 0;
        }
        break;

    case SFX_STYLE_FAMILY_FRAME:
        pFrmFmt = lcl_FindFrmFmt(rDoc,  aName, this, bCreate);
        bPhysical = 0 != pFrmFmt;
        if( bFillOnlyInfo && bPhysical )
        {
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pFrmFmt = lcl_FindFrmFmt(rDoc, aName, this, true );
        }
        pFmt = pFrmFmt;
        if( !bCreate && !pFmt )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT );

        bRet = 0 != pFrmFmt || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pFrmFmt = 0;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        pDesc = lcl_FindPageDesc(rDoc, aName, this, bCreate);
        bPhysical = 0 != pDesc;
        if( bFillOnlyInfo && !pDesc )
        {
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pDesc = lcl_FindPageDesc( rDoc, aName, this, true );
        }

        if( pDesc )
        {
            nPoolId = pDesc->GetPoolFmtId();
            nHelpId = pDesc->GetPoolHelpId();
            if( pDesc->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pDesc->GetPoolHlpFileId() );
            else
                aHelpFile = "";
        }
        else if( !bCreate )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = 0 != pDesc || USHRT_MAX != nPoolId;
        if( bDeleteInfo )
            pDesc = 0;
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        pNumRule = lcl_FindNumRule(rDoc, aName, this, bCreate);
        bPhysical = 0 != pNumRule;
        if( bFillOnlyInfo && !pNumRule )
        {
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pNumRule = lcl_FindNumRule( rDoc, aName, this, true );
        }

        if( pNumRule )
        {
            nPoolId = pNumRule->GetPoolFmtId();
            nHelpId = pNumRule->GetPoolHelpId();
            if( pNumRule->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pNumRule->GetPoolHlpFileId() );
            else
                aHelpFile = "";
        }
        else if( !bCreate )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = 0 != pNumRule || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pNumRule = 0;
        break;
        default:; //prevent warning
    }

    if( SFX_STYLE_FAMILY_CHAR == nFamily ||
        SFX_STYLE_FAMILY_PARA == nFamily ||
        SFX_STYLE_FAMILY_FRAME == nFamily )
    {
        if( pFmt )
            nPoolId = pFmt->GetPoolFmtId();

        sal_uInt16 _nMask = 0;
        if( pFmt == rDoc.GetDfltCharFmt() )
            _nMask |= SFXSTYLEBIT_READONLY;
        else if( USER_FMT & nPoolId )
            _nMask |= SFXSTYLEBIT_USERDEF;

        switch ( COLL_GET_RANGE_BITS & nPoolId )
        {
        case COLL_TEXT_BITS:     _nMask |= SWSTYLEBIT_TEXT;   break;
        case COLL_DOC_BITS :     _nMask |= SWSTYLEBIT_CHAPTER; break;
        case COLL_LISTS_BITS:    _nMask |= SWSTYLEBIT_LIST;   break;
        case COLL_REGISTER_BITS: _nMask |= SWSTYLEBIT_IDX;    break;
        case COLL_EXTRA_BITS:    _nMask |= SWSTYLEBIT_EXTRA;      break;
        case COLL_HTML_BITS:     _nMask |= SWSTYLEBIT_HTML;   break;
        }

        if( pFmt )
        {
            OSL_ENSURE( bPhysical, "Format not found" );

            nHelpId = pFmt->GetPoolHelpId();
            if( pFmt->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pFmt->GetPoolHlpFileId() );
            else
                aHelpFile = "";

            if( RES_CONDTXTFMTCOLL == pFmt->Which() )
                _nMask |= SWSTYLEBIT_CONDCOLL;
        }

        SetMask( _nMask );
    }
    if( bDeleteInfo && bFillOnlyInfo )
        ::lcl_DeleteInfoStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
    return bRet;
}

// Create new format in Core
void SwDocStyleSheet::Create()
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            pCharFmt = lcl_FindCharFmt( rDoc, aName );
            if( !pCharFmt )
                pCharFmt = rDoc.MakeCharFmt(aName,
                                            rDoc.GetDfltCharFmt());
            pCharFmt->SetAuto( false );
            break;

        case SFX_STYLE_FAMILY_PARA :
            pColl = lcl_FindParaFmt( rDoc, aName );
            if( !pColl )
            {
                SwTxtFmtColl *pPar = (*rDoc.GetTxtFmtColls())[0];
                if( nMask & SWSTYLEBIT_CONDCOLL )
                    pColl = rDoc.MakeCondTxtFmtColl( aName, pPar );
                else
                    pColl = rDoc.MakeTxtFmtColl( aName, pPar );
            }
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFrmFmt = lcl_FindFrmFmt( rDoc, aName );
            if( !pFrmFmt )
                pFrmFmt = rDoc.MakeFrmFmt(aName, rDoc.GetDfltFrmFmt(), false, false);

            break;

        case SFX_STYLE_FAMILY_PAGE :
            pDesc = lcl_FindPageDesc( rDoc, aName );
            if( !pDesc )
            {
                sal_uInt16 nId = rDoc.MakePageDesc(aName);
                pDesc = &rDoc.GetPageDesc(nId);
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            pNumRule = lcl_FindNumRule( rDoc, aName );
            if( !pNumRule )
            {
                String sTmpNm( aName );
                if( aName.isEmpty() )
                    sTmpNm = rDoc.GetUniqueNumRuleName();

                SwNumRule* pRule = rDoc.GetNumRuleTbl()[
                    rDoc.MakeNumRule( sTmpNm, 0, false,
                                      // #i89178#
                                      numfunc::GetDefaultPositionAndSpaceMode() ) ];
                pRule->SetAutoRule( sal_False );
                if( aName.isEmpty() )
                {
                    // #i91400#
                    pRule->SetName( aName, rDoc );
                }
                pNumRule = pRule;
            }
            break;
        default:; //prevent warning
    }
    bPhysical = sal_True;
    aCoreSet.ClearItem();
}

SwCharFmt* SwDocStyleSheet::GetCharFmt()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pCharFmt;
}

SwTxtFmtColl* SwDocStyleSheet::GetCollection()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pColl;
}

const SwPageDesc* SwDocStyleSheet::GetPageDesc()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pDesc;
}

const SwNumRule * SwDocStyleSheet::GetNumRule()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pNumRule;
}

void SwDocStyleSheet::SetNumRule(const SwNumRule& rRule)
{
    OSL_ENSURE(pNumRule, "Wo ist die NumRule");
    rDoc.ChgNumRuleFmts( rRule );
}

// re-generate Name AND Family from String
// First() and Next() (see below) insert an identification letter at Pos.1

void SwDocStyleSheet::PresetNameAndFamily(const OUString& rName)
{
    switch( rName[0] )
    {
    case cPARA:     nFamily = SFX_STYLE_FAMILY_PARA; break;
    case cFRAME:    nFamily = SFX_STYLE_FAMILY_FRAME; break;
    case cPAGE:     nFamily = SFX_STYLE_FAMILY_PAGE; break;
    case cNUMRULE:  nFamily = SFX_STYLE_FAMILY_PSEUDO; break;
    default:        nFamily = SFX_STYLE_FAMILY_CHAR; break;
    }
    aName = rName.copy(1);
}

// Is the format physically present yet
void SwDocStyleSheet::SetPhysical(sal_Bool bPhys)
{
    bPhysical = bPhys;

    if(!bPhys)
    {
        pCharFmt = 0;
        pColl    = 0;
        pFrmFmt  = 0;
        pDesc    = 0;
    }
}

SwFrmFmt* SwDocStyleSheet::GetFrmFmt()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pFrmFmt;
}

bool  SwDocStyleSheet::IsUsed() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = (SwDocStyleSheet*)this;
        pThis->FillStyleSheet( FillOnlyName );
    }

    if( !bPhysical )
        return false;

    const SwModify* pMod;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pMod = pCharFmt;   break;
    case SFX_STYLE_FAMILY_PARA : pMod = pColl;      break;
    case SFX_STYLE_FAMILY_FRAME: pMod = pFrmFmt;    break;
    case SFX_STYLE_FAMILY_PAGE : pMod = pDesc;      break;

    case SFX_STYLE_FAMILY_PSEUDO:
            return pNumRule ? rDoc.IsUsed( *pNumRule ) : false;

    default:
        OSL_ENSURE(!this, "unknown style family");
        return false;
    }
    return rDoc.IsUsed( *pMod );
}


sal_uLong  SwDocStyleSheet::GetHelpId( OUString& rFile )
{
    sal_uInt16 nId = 0;
    sal_uInt16 nPoolId = 0;
    unsigned char nFileId = UCHAR_MAX;

    rFile = "swrhlppi.hlp";

    const SwFmt* pTmpFmt = 0;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( !pCharFmt &&
            0 == (pCharFmt = lcl_FindCharFmt( rDoc, aName, 0, false )) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pCharFmt;
        break;

    case SFX_STYLE_FAMILY_PARA:
        if( !pColl &&
            0 == ( pColl = lcl_FindParaFmt( rDoc, aName, 0, false )) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pColl;
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( !pFrmFmt &&
            0 == ( pFrmFmt = lcl_FindFrmFmt( rDoc, aName, 0, false ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFmt = pFrmFmt;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        if( !pDesc &&
            0 == ( pDesc = lcl_FindPageDesc( rDoc, aName, 0, false ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pDesc->GetPoolHelpId();
        nFileId = pDesc->GetPoolHlpFileId();
        nPoolId = pDesc->GetPoolFmtId();
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        if( !pNumRule &&
            0 == ( pNumRule = lcl_FindNumRule( rDoc, aName, 0, false ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pNumRule->GetPoolHelpId();
        nFileId = pNumRule->GetPoolHlpFileId();
        nPoolId = pNumRule->GetPoolFmtId();
        break;

    default:
        OSL_ENSURE(!this, "unknown style family");
        return 0;
    }

    if( pTmpFmt )
    {
        nId = pTmpFmt->GetPoolHelpId();
        nFileId = pTmpFmt->GetPoolHlpFileId();
        nPoolId = pTmpFmt->GetPoolFmtId();
    }

    if( UCHAR_MAX != nFileId )
    {
        const String *pTemplate = rDoc.GetDocPattern( nFileId );
        if( pTemplate )
        {
            rFile = *pTemplate;
        }
    }
    else if( !IsPoolUserFmt( nPoolId ) )
    {
        nId = nPoolId;
    }

    // because SFX acts like that, with HelpId:
    if( USHRT_MAX == nId )
        nId = 0;        // don't show Help accordingly

    return nId;
}


void  SwDocStyleSheet::SetHelpId( const OUString& r, sal_uLong nId )
{
    sal_uInt8 nFileId = static_cast< sal_uInt8 >(rDoc.SetDocPattern( r ));
    sal_uInt16 nHId = static_cast< sal_uInt16 >(nId); //!! SFX changed over to ULONG arbitrarily!

    SwFmt* pTmpFmt = 0;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pTmpFmt = pCharFmt;    break;
    case SFX_STYLE_FAMILY_PARA : pTmpFmt = pColl;       break;
    case SFX_STYLE_FAMILY_FRAME: pTmpFmt = pFrmFmt;     break;
    case SFX_STYLE_FAMILY_PAGE :
        ((SwPageDesc*)pDesc)->SetPoolHelpId( nHId );
        ((SwPageDesc*)pDesc)->SetPoolHlpFileId( nFileId );
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        ((SwNumRule*)pNumRule)->SetPoolHelpId( nHId );
        ((SwNumRule*)pNumRule)->SetPoolHlpFileId( nFileId );
        break;

    default:
        OSL_ENSURE(!this, "unknown style family");
        return ;
    }
    if( pTmpFmt )
    {
        pTmpFmt->SetPoolHelpId( nHId );
        pTmpFmt->SetPoolHlpFileId( nFileId );
    }
}

// methods for DocStyleSheetPool
SwDocStyleSheetPool::SwDocStyleSheetPool( SwDoc& rDocument, sal_Bool bOrg )
: SfxStyleSheetBasePool( rDocument.GetAttrPool() )
, mxStyleSheet( new SwDocStyleSheet( rDocument, aEmptyStr, this, SFX_STYLE_FAMILY_CHAR, 0 ) )
, rDoc( rDocument )
{
    bOrganizer = bOrg;
}

 SwDocStyleSheetPool::~SwDocStyleSheetPool()
{
}

void SAL_CALL SwDocStyleSheetPool::acquire(  ) throw ()
{
    comphelper::OWeakTypeObject::acquire();
}

void SAL_CALL SwDocStyleSheetPool::release(  ) throw ()
{
    comphelper::OWeakTypeObject::release();
}

SfxStyleSheetBase&   SwDocStyleSheetPool::Make( const OUString&   rName,
                                                SfxStyleFamily  eFam,
                                                sal_uInt16          _nMask,
                                                sal_uInt16          /*nPos*/ )
{
    mxStyleSheet->PresetName(rName);
    mxStyleSheet->PresetParent(aEmptyStr);
    mxStyleSheet->PresetFollow(aEmptyStr);
    mxStyleSheet->SetMask(_nMask) ;
    mxStyleSheet->SetFamily(eFam);
    mxStyleSheet->SetPhysical(sal_True);
    mxStyleSheet->Create();

    return *mxStyleSheet.get();
}


SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const SfxStyleSheetBase& /*rOrg*/)
{
    OSL_ENSURE(!this , "Create im SW-Stylesheet-Pool geht nicht" );
    return NULL;
}


SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const OUString &,
                                                  SfxStyleFamily, sal_uInt16 )
{
    OSL_ENSURE( !this, "Create im SW-Stylesheet-Pool geht nicht" );
    return NULL;
}

void  SwDocStyleSheetPool::Replace( SfxStyleSheetBase& rSource,
                                    SfxStyleSheetBase& rTarget )
{
    SfxStyleFamily eFamily( rSource.GetFamily() );
    if( rSource.HasParentSupport())
    {
        const String& rParentName = rSource.GetParent();
        if( 0 != rParentName.Len() )
        {
            SfxStyleSheetBase* pParentOfNew = Find( rParentName, eFamily );
            if( pParentOfNew )
                rTarget.SetParent( rParentName );
        }
    }
    if( rSource.HasFollowSupport())
    {
        const String& rFollowName = rSource.GetFollow();
        if( 0 != rFollowName.Len() )
        {
            SfxStyleSheetBase* pFollowOfNew = Find( rFollowName, eFamily );
            if( pFollowOfNew )
                rTarget.SetFollow( rFollowName );
        }
    }

    SwImplShellAction aTmpSh( rDoc );

    bool bSwSrcPool = GetAppName() == rSource.GetPool().GetAppName();
    if( SFX_STYLE_FAMILY_PAGE == eFamily && bSwSrcPool )
    {
        // deal with separately!
        SwPageDesc* pDestDsc =
            (SwPageDesc*)((SwDocStyleSheet&)rTarget).GetPageDesc();
        SwPageDesc* pCpyDsc =
            (SwPageDesc*)((SwDocStyleSheet&)rSource).GetPageDesc();
        rDoc.CopyPageDesc( *pCpyDsc, *pDestDsc );
    }
    else
    {
        const SwFmt *pSourceFmt = 0;
        SwFmt *pTargetFmt = 0;
        sal_uInt16 nPgDscPos = USHRT_MAX;
        switch( eFamily )
        {
        case SFX_STYLE_FAMILY_CHAR :
            if( bSwSrcPool )
                pSourceFmt = ((SwDocStyleSheet&)rSource).GetCharFmt();
            pTargetFmt = ((SwDocStyleSheet&)rTarget).GetCharFmt();
            break;
        case SFX_STYLE_FAMILY_PARA :
            if( bSwSrcPool )
                pSourceFmt = ((SwDocStyleSheet&)rSource).GetCollection();
            pTargetFmt = ((SwDocStyleSheet&)rTarget).GetCollection();
            break;
        case SFX_STYLE_FAMILY_FRAME:
            if( bSwSrcPool )
                pSourceFmt = ((SwDocStyleSheet&)rSource).GetFrmFmt();
            pTargetFmt = ((SwDocStyleSheet&)rTarget).GetFrmFmt();
            break;
        case SFX_STYLE_FAMILY_PAGE:
            if( bSwSrcPool )
                pSourceFmt = &((SwDocStyleSheet&)rSource).GetPageDesc()
                                ->GetMaster();
            {
                SwPageDesc *pDesc = rDoc.FindPageDescByName(
                    ((SwDocStyleSheet&)rTarget).GetPageDesc()->GetName(),
                    &nPgDscPos );

                if( pDesc )
                    pTargetFmt = &pDesc->GetMaster();
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            // A NumRule only consists of one Item, so nothing has
            // to be deleted here.
            break;
        default:; //prevent warning
        }
        if( pTargetFmt )
        {
            if( pSourceFmt )
                pTargetFmt->DelDiffs( *pSourceFmt );
            else if( USHRT_MAX != nPgDscPos )
                pTargetFmt->ResetFmtAttr( RES_PAGEDESC, RES_FRMATR_END-1 );
            else
            {
                // #i73790# - method renamed
                pTargetFmt->ResetAllFmtAttr();
            }

            if( USHRT_MAX != nPgDscPos )
                rDoc.ChgPageDesc( nPgDscPos,
                                  rDoc.GetPageDesc(nPgDscPos) );
        }
        ((SwDocStyleSheet&)rTarget).SetItemSet( rSource.GetItemSet() );
    }
}

SfxStyleSheetIteratorPtr SwDocStyleSheetPool::CreateIterator( SfxStyleFamily eFam, sal_uInt16 _nMask )
{
    return SfxStyleSheetIteratorPtr(new SwStyleSheetIterator( this, eFam, _nMask ));
}

void SwDocStyleSheetPool::dispose()
{
    mxStyleSheet.clear();
}

void SwDocStyleSheetPool::Remove( SfxStyleSheetBase* pStyle)
{
    if( !pStyle )
        return;

    bool bBroadcast = true;
    SwImplShellAction aTmpSh( rDoc );
    const String& rName = pStyle->GetName();
    switch( pStyle->GetFamily() )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFmt* pFmt = lcl_FindCharFmt(rDoc, rName, 0, false );
            if(pFmt)
                rDoc.DelCharFmt(pFmt);
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            SwTxtFmtColl* pColl = lcl_FindParaFmt(rDoc, rName, 0, false );
            if(pColl)
                rDoc.DelTxtFmtColl(pColl);
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrmFmt* pFmt = lcl_FindFrmFmt(rDoc, rName, 0, false );
            if(pFmt)
                rDoc.DelFrmFmt(pFmt);
        }
        break;
    case SFX_STYLE_FAMILY_PAGE :
        {
            sal_uInt16 nPos;
            if( rDoc.FindPageDescByName( rName, &nPos ))
                rDoc.DelPageDesc( nPos );
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            if( !rDoc.DelNumRule( rName ) )
                // Only send Broadcast, when something was deleted
                bBroadcast = false;
        }
        break;

    default:
        OSL_ENSURE(!this, "unknown style family");
        bBroadcast = false;
    }

    if( bBroadcast )
        Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_ERASED, *pStyle ) );
}


bool  SwDocStyleSheetPool::SetParent( SfxStyleFamily eFam,
                                      const OUString &rStyle, const OUString &rParent )
{
    SwFmt* pFmt = 0, *pParent = 0;
    switch( eFam )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( 0 != ( pFmt = lcl_FindCharFmt( rDoc, rStyle ) ) && !rParent.isEmpty() )
            pParent = lcl_FindCharFmt(rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_PARA :
        if( 0 != ( pFmt = lcl_FindParaFmt( rDoc, rStyle ) ) && !rParent.isEmpty() )
            pParent = lcl_FindParaFmt( rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( 0 != ( pFmt = lcl_FindFrmFmt( rDoc, rStyle ) ) && !rParent.isEmpty() )
            pParent = lcl_FindFrmFmt( rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_PAGE:
    case SFX_STYLE_FAMILY_PSEUDO:
        break;

    default:
        OSL_ENSURE(!this, "unknown style family");
    }

    bool bRet = false;
    if( pFmt && pFmt->DerivedFrom() &&
        pFmt->DerivedFrom()->GetName() != rParent )
    {
        {
            SwImplShellAction aTmpSh( rDoc );
            bRet = pFmt->SetDerivedFrom( pParent );
        }

        if( bRet )
        {
            // only for Broadcasting
            mxStyleSheet->PresetName( rStyle );
            mxStyleSheet->PresetParent( rParent );
            if( SFX_STYLE_FAMILY_PARA == eFam )
                mxStyleSheet->PresetFollow( ((SwTxtFmtColl*)pFmt)->
                        GetNextTxtFmtColl().GetName() );
            else
                mxStyleSheet->PresetFollow( aEmptyOUStr );

            Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_MODIFIED,
                                            *(mxStyleSheet.get()) ) );
        }
    }

    return bRet;
}

SfxStyleSheetBase* SwDocStyleSheetPool::Find( const OUString& rName,
                                              SfxStyleFamily eFam, sal_uInt16 n )
{
    sal_uInt16 nSMask = n;
    if( SFX_STYLE_FAMILY_PARA == eFam && rDoc.get(IDocumentSettingAccess::HTML_MODE) )
    {
        // then only HTML-Templates are of interest
        if( USHRT_MAX == nSMask )
            nSMask = SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED;
        else
            nSMask &= SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF |
                                SWSTYLEBIT_CONDCOLL | SWSTYLEBIT_HTML;
        if( !nSMask )
            nSMask = SWSTYLEBIT_HTML;
    }

    const bool bSearchUsed = ( n != SFXSTYLEBIT_ALL && n & SFXSTYLEBIT_USED );
    const SwModify* pMod = 0;

    mxStyleSheet->SetPhysical( sal_False );
    mxStyleSheet->PresetName( rName );
    mxStyleSheet->SetFamily( eFam );
    sal_Bool bFnd = mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    if( mxStyleSheet->IsPhysical() )
    {
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:
            pMod = mxStyleSheet->GetCharFmt();
            break;

        case SFX_STYLE_FAMILY_PARA:
            pMod = mxStyleSheet->GetCollection();
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pMod = mxStyleSheet->GetFrmFmt();
            break;

        case SFX_STYLE_FAMILY_PAGE:
            pMod = mxStyleSheet->GetPageDesc();
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                const SwNumRule* pRule = mxStyleSheet->GetNumRule();
                if( pRule &&
                    !bSearchUsed &&
                    (( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                            ? !(pRule->GetPoolFmtId() & USER_FMT)
                                // searched for used and found none
                            : bSearchUsed ))
                    bFnd = sal_False;
            }
            break;

        default:
            OSL_ENSURE(!this, "unknown style family");
        }
    }

    // then evaluate the mask:
    if( pMod && !bSearchUsed )
    {
        const sal_uInt16 nId = SFX_STYLE_FAMILY_PAGE == eFam
                        ? ((SwPageDesc*)pMod)->GetPoolFmtId()
                        : ((SwFmt*)pMod)->GetPoolFmtId();

        if( ( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
            ? !(nId & USER_FMT)
                // searched for used and found none
            : bSearchUsed )
            bFnd = sal_False;
    }
    return bFnd ? mxStyleSheet.get() : 0;
}

SwStyleSheetIterator::SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
                                SfxStyleFamily eFam, sal_uInt16 n )
    : SfxStyleSheetIterator( pBase, eFam, n ),
    mxIterSheet( new SwDocStyleSheet( pBase->GetDoc(), aEmptyStr, pBase, SFX_STYLE_FAMILY_CHAR, 0 ) ),
    mxStyleSheet( new SwDocStyleSheet( pBase->GetDoc(), aEmptyStr, pBase, SFX_STYLE_FAMILY_CHAR, 0 ) )
{
    bFirstCalled = sal_False;
    nLastPos = 0;
    StartListening( *pBase );
}

SwStyleSheetIterator::~SwStyleSheetIterator()
{
    EndListening( mxIterSheet->GetPool() );
}

sal_uInt16  SwStyleSheetIterator::Count()
{
    // let the list fill correctly!!
    if( !bFirstCalled )
        First();
    return aLst.size();
}

SfxStyleSheetBase*  SwStyleSheetIterator::operator[]( sal_uInt16 nIdx )
{
    // found
    if( !bFirstCalled )
        First();
    mxStyleSheet->PresetNameAndFamily( aLst[ nIdx ] );
    mxStyleSheet->SetPhysical( sal_False );
    mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    return mxStyleSheet.get();
}

SfxStyleSheetBase*  SwStyleSheetIterator::First()
{
    // Delete old list
    bFirstCalled = sal_True;
    nLastPos = 0;
    aLst.Erase();

    // Delete current
    mxIterSheet->Reset();

    SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
    const sal_uInt16 nSrchMask = nMask;
    const sal_Bool bIsSearchUsed = SearchUsed();

    bool bSearchHidden = ( nMask & SFXSTYLEBIT_HIDDEN );
    bool bOnlyHidden = nMask == SFXSTYLEBIT_HIDDEN;

    const sal_Bool bOrganizer = ((SwDocStyleSheetPool*)pBasePool)->IsOrganizerMode();
    bool bAll = ( nSrchMask & SFXSTYLEBIT_ALL_VISIBLE ) == SFXSTYLEBIT_ALL_VISIBLE;

    if( nSearchFamily == SFX_STYLE_FAMILY_CHAR
     || nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const sal_uInt16 nArrLen = rDoc.GetCharFmts()->size();
        for( sal_uInt16 i = 0; i < nArrLen; i++ )
        {
            SwCharFmt* pFmt = (*rDoc.GetCharFmts())[ i ];

            const bool bUsed = bIsSearchUsed && (bOrganizer || rDoc.IsUsed(*pFmt));
            if( ( !bSearchHidden && pFmt->IsHidden() && !bUsed ) || ( pFmt->IsDefault() && pFmt != rDoc.GetDfltCharFmt() ) )
                continue;

            if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !pFmt->IsHidden( ) )
                continue;

            if( !bUsed )
            {
                // Standard is no User template
                const sal_uInt16 nId = rDoc.GetDfltCharFmt() == pFmt ?
                        sal_uInt16( RES_POOLCHR_INET_NORMAL ):
                                pFmt->GetPoolFmtId();
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                        // searched for used and found none
                    : bIsSearchUsed )
                {
                    continue;
                }

                if( rDoc.get(IDocumentSettingAccess::HTML_MODE) && !(nId & USER_FMT) &&
                    !( RES_POOLCHR_HTML_BEGIN <= nId &&
                          nId < RES_POOLCHR_HTML_END ) &&
                    RES_POOLCHR_INET_NORMAL != nId &&
                    RES_POOLCHR_INET_VISIT != nId &&
                    RES_POOLCHR_FOOTNOTE  != nId &&
                    RES_POOLCHR_ENDNOTE != nId )
                    continue;
            }

            aLst.Append( cCHAR, pFmt == rDoc.GetDfltCharFmt()
                        ? SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ]
                        : pFmt->GetName() );
        }

        // PoolFormate
        if( bAll )
        {
            if( !rDoc.get(IDocumentSettingAccess::HTML_MODE) )
                AppendStyleList(SwStyleNameMapper::GetChrFmtUINameArray(),
                                bIsSearchUsed, bSearchHidden, bOnlyHidden,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, cCHAR);
            else
            {
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_INET_NORMAL - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_INET_VISIT - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_ENDNOTE - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFmtUINameArray()[
                        RES_POOLCHR_FOOTNOTE - RES_POOLCHR_BEGIN ] );
            }
            AppendStyleList(SwStyleNameMapper::GetHTMLChrFmtUINameArray(),
                                bIsSearchUsed, bSearchHidden, bOnlyHidden,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, cCHAR);
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PARA ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        sal_uInt16 nSMask = nSrchMask;
        if( rDoc.get(IDocumentSettingAccess::HTML_MODE) )
        {
            // then only HTML-Template are of interest
            if( SFXSTYLEBIT_ALL_VISIBLE == ( nSMask & SFXSTYLEBIT_ALL_VISIBLE ) )
                nSMask = SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF |
                            SFXSTYLEBIT_USED;
            else
                nSMask &= SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF |
                                SWSTYLEBIT_CONDCOLL | SWSTYLEBIT_HTML;
            if( !nSMask )
                nSMask = SWSTYLEBIT_HTML;
        }

        const sal_uInt16 nArrLen = rDoc.GetTxtFmtColls()->size();
        for( sal_uInt16 i = 0; i < nArrLen; i++ )
        {
            SwTxtFmtColl* pColl = (*rDoc.GetTxtFmtColls())[ i ];

            const bool bUsed = bOrganizer || rDoc.IsUsed(*pColl);
            if ( ( !bSearchHidden && pColl->IsHidden( ) && !bUsed ) || pColl->IsDefault() )
                continue;

            if ( nSMask == SFXSTYLEBIT_HIDDEN && !pColl->IsHidden( ) )
                continue;

            if( !(bIsSearchUsed && bUsed ))
            {
                const sal_uInt16 nId = pColl->GetPoolFmtId();
                switch ( (nSMask & ~SFXSTYLEBIT_USED) )
                {
                case SFXSTYLEBIT_USERDEF:
                    if(!IsPoolUserFmt(nId)) continue;
                    break;
                case SWSTYLEBIT_TEXT:
                    if((nId & COLL_GET_RANGE_BITS) != COLL_TEXT_BITS) continue;
                    break;
                case SWSTYLEBIT_CHAPTER:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_DOC_BITS) continue;
                    break;
                case SWSTYLEBIT_LIST:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_LISTS_BITS) continue;
                    break;
                case SWSTYLEBIT_IDX:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_REGISTER_BITS) continue;
                    break;
                case SWSTYLEBIT_EXTRA:
                    if((nId  & COLL_GET_RANGE_BITS) != COLL_EXTRA_BITS) continue;
                    break;
                case SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF:
                    if(IsPoolUserFmt(nId))
                        break;
                    // otherwise move on
                case SWSTYLEBIT_HTML:
                    if( (nId  & COLL_GET_RANGE_BITS) != COLL_HTML_BITS)
                    {
                        // but some we also want to see in this section
                        bool bWeiter = true;
                        switch( nId )
                        {
                        case RES_POOLCOLL_SENDADRESS:   //  --> ADDRESS
                        case RES_POOLCOLL_TABLE_HDLN:   //  --> TH
                        case RES_POOLCOLL_TABLE:        //  --> TD
                        case RES_POOLCOLL_TEXT:         //  --> P
                        case RES_POOLCOLL_HEADLINE_BASE://  --> H
                        case RES_POOLCOLL_HEADLINE1:    //  --> H1
                        case RES_POOLCOLL_HEADLINE2:    //  --> H2
                        case RES_POOLCOLL_HEADLINE3:    //  --> H3
                        case RES_POOLCOLL_HEADLINE4:    //  --> H4
                        case RES_POOLCOLL_HEADLINE5:    //  --> H5
                        case RES_POOLCOLL_HEADLINE6:    //  --> H6
                        case RES_POOLCOLL_STANDARD:     //  --> P
                        case RES_POOLCOLL_FOOTNOTE:
                        case RES_POOLCOLL_ENDNOTE:
                            bWeiter = false;
                            break;
                        }
                        if( bWeiter )
                            continue;
                    }
                    break;
                case SWSTYLEBIT_CONDCOLL:
                    if( RES_CONDTXTFMTCOLL != pColl->Which() ) continue;
                    break;
                default:
                    // searched for used and found none
                    if( bIsSearchUsed )
                        continue;
                }
            }
            aLst.Append( cPARA, pColl->GetName() );
        }

        bAll = ( nSMask & SFXSTYLEBIT_ALL_VISIBLE ) == SFXSTYLEBIT_ALL_VISIBLE;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_TEXT )
            AppendStyleList(SwStyleNameMapper::GetTextUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA );
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CHAPTER )
            AppendStyleList(SwStyleNameMapper::GetDocUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_LIST )
            AppendStyleList(SwStyleNameMapper::GetListsUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_IDX )
            AppendStyleList(SwStyleNameMapper::GetRegisterUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_EXTRA )
            AppendStyleList(SwStyleNameMapper::GetExtraUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
        if ( bAll || (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_CONDCOLL )
        {
            if( !bIsSearchUsed ||
                rDoc.IsPoolTxtCollUsed( RES_POOLCOLL_TEXT ))
                aLst.Append( cPARA, SwStyleNameMapper::GetTextUINameArray()[
                        RES_POOLCOLL_TEXT - RES_POOLCOLL_TEXT_BEGIN ] );
        }
        if ( bAll ||
            (nSMask & ~SFXSTYLEBIT_USED) == SWSTYLEBIT_HTML ||
            (nSMask & ~SFXSTYLEBIT_USED) ==
                        (SWSTYLEBIT_HTML | SFXSTYLEBIT_USERDEF) )
        {
            AppendStyleList(SwStyleNameMapper::GetHTMLUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL, cPARA ) ;
            if( !bAll )
            {
                // then also the ones, that we are mapping:
                static sal_uInt16 aPoolIds[] = {
                    RES_POOLCOLL_SENDADRESS,    //  --> ADDRESS
                    RES_POOLCOLL_TABLE_HDLN,    //  --> TH
                    RES_POOLCOLL_TABLE,     //  --> TD
                    RES_POOLCOLL_STANDARD,      //  --> P
                    RES_POOLCOLL_TEXT,          //  --> P
                    RES_POOLCOLL_HEADLINE_BASE, //  --> H
                    RES_POOLCOLL_HEADLINE1, //  --> H1
                    RES_POOLCOLL_HEADLINE2, //  --> H2
                    RES_POOLCOLL_HEADLINE3, //  --> H3
                    RES_POOLCOLL_HEADLINE4, //  --> H4
                    RES_POOLCOLL_HEADLINE5, //  --> H5
                    RES_POOLCOLL_HEADLINE6, //  --> H6
                    RES_POOLCOLL_FOOTNOTE,
                    RES_POOLCOLL_ENDNOTE,
                    0
                    };

                sal_uInt16* pPoolIds = aPoolIds;
                String s;
                while( *pPoolIds )
                {
                    if( !bIsSearchUsed || rDoc.IsPoolTxtCollUsed( *pPoolIds ) )
                        aLst.Append( cPARA,
                            s = SwStyleNameMapper::GetUIName( *pPoolIds, s ));
                    ++pPoolIds;
                }
            }
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_FRAME ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const sal_uInt16 nArrLen = rDoc.GetFrmFmts()->size();
        for( sal_uInt16 i = 0; i < nArrLen; i++ )
        {
            SwFrmFmt* pFmt = (*rDoc.GetFrmFmts())[ i ];

            bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(*pFmt));
            if( ( !bSearchHidden && pFmt->IsHidden( ) && !bUsed ) || pFmt->IsDefault() || pFmt->IsAuto() )
                continue;

            if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !pFmt->IsHidden( ) )
                continue;

            const sal_uInt16 nId = pFmt->GetPoolFmtId();
            if( !bUsed )
            {
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                    // searched for used and found none
                    : bIsSearchUsed )
                {
                    continue;
                }
            }

            aLst.Append( cFRAME, pFmt->GetName() );
        }

        // PoolFormate
        if ( bAll )
            AppendStyleList(SwStyleNameMapper::GetFrmFmtUINameArray(),
                                    bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, cFRAME);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PAGE ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const sal_uInt16 nCount = rDoc.GetPageDescCnt();
        for(sal_uInt16 i = 0; i < nCount; ++i)
        {
            const SwPageDesc& rDesc = rDoc.GetPageDesc(i);
            const sal_uInt16 nId = rDesc.GetPoolFmtId();
            bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(rDesc));
            if( !bUsed )
            {
                if ( ( !bSearchHidden && rDesc.IsHidden() ) ||
                       ( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                    // searched for used and found none
                    : bIsSearchUsed ) )
                    continue;
            }

            if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !rDesc.IsHidden( ) )
                continue;

            aLst.Append( cPAGE, rDesc.GetName() );
        }
        if ( bAll )
            AppendStyleList(SwStyleNameMapper::GetPageDescUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, cPAGE);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PSEUDO ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const SwNumRuleTbl& rNumTbl = rDoc.GetNumRuleTbl();
        for(sal_uInt16 i = 0; i < rNumTbl.size(); ++i)
        {
            const SwNumRule& rRule = *rNumTbl[ i ];
            if( !rRule.IsAutoRule() )
            {
                if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !rRule.IsHidden( ) )
                    continue;

                bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(rRule) );
                if( !bUsed )
                {
                    if( ( !bSearchHidden && rRule.IsHidden() ) ||
                           ( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                        ? !(rRule.GetPoolFmtId() & USER_FMT)
                        // searched for used and found none
                        : bIsSearchUsed ) )
                        continue;
                }

                aLst.Append( cNUMRULE, rRule.GetName() );
            }
        }
        if ( bAll )
            AppendStyleList(SwStyleNameMapper::GetNumRuleUINameArray(),
                            bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE, cNUMRULE);
    }

    if(!aLst.empty())
    {
        nLastPos = USHRT_MAX;
        return Next();
    }
    return 0;
}

SfxStyleSheetBase*  SwStyleSheetIterator::Next()
{
    ++nLastPos;
    if(!aLst.empty() && nLastPos < aLst.size())
    {
        mxIterSheet->PresetNameAndFamily(aLst[nLastPos]);
        mxIterSheet->SetPhysical( sal_False );
        mxIterSheet->SetMask( nMask );
        if(mxIterSheet->pSet)
        {
            mxIterSheet->pSet->ClearItem(0);
            mxIterSheet->pSet= 0;
        }
        return mxIterSheet.get();
    }
    return 0;
}

SfxStyleSheetBase*  SwStyleSheetIterator::Find(const OUString& rName)
{
    // searching
    if( !bFirstCalled )
        First();

    nLastPos = lcl_FindName( aLst, nSearchFamily, rName );
    if( USHRT_MAX != nLastPos )
    {
        // found
        mxStyleSheet->PresetNameAndFamily(aLst[nLastPos]);
        // new name is set, so determine its Data
        mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );
        if( !mxStyleSheet->IsPhysical() )
            mxStyleSheet->SetPhysical( sal_False );

        return mxStyleSheet.get();
    }
    return 0;
}

void SwStyleSheetIterator::AppendStyleList(const boost::ptr_vector<String>& rList,
                                            sal_Bool bTestUsed, sal_Bool bTestHidden, bool bOnlyHidden,
                                            sal_uInt16 nSection, char cType )
{
    SwDoc& rDoc = ((SwDocStyleSheetPool*)pBasePool)->GetDoc();
    sal_Bool bUsed = sal_False;
    for ( sal_uInt16 i=0; i < rList.size(); ++i )
    {
        bool bHidden = false;
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rList[i], (SwGetPoolIdFromName)nSection);
        switch ( nSection )
        {
            case nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL:
                {
                    bUsed = rDoc.IsPoolTxtCollUsed( nId );
                    SwFmt* pFmt = rDoc.FindTxtFmtCollByName( rList[i] );
                    bHidden = pFmt && pFmt->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_CHRFMT:
                {
                    bUsed = rDoc.IsPoolFmtUsed( nId );
                    SwFmt* pFmt = rDoc.FindCharFmtByName( rList[i] );
                    bHidden = pFmt && pFmt->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_FRMFMT:
                {
                    bUsed = rDoc.IsPoolFmtUsed( nId );
                    SwFmt* pFmt = rDoc.FindFrmFmtByName( rList[i] );
                    bHidden = pFmt && pFmt->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC:
                {
                    bUsed = rDoc.IsPoolPageDescUsed( nId );
                    SwPageDesc* pPgDesc = rDoc.FindPageDescByName( rList[i] );
                    bHidden = pPgDesc && pPgDesc->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_NUMRULE:
                {
                    SwNumRule* pRule = rDoc.FindNumRulePtr( rList[i] );
                    bUsed = pRule && rDoc.IsUsed( *pRule );
                    bHidden = pRule && pRule->IsHidden( );
                }
                break;
            default:
                OSL_ENSURE( !this, "unknown PoolFmt-Id" );
        }

        bool bMatchHidden = ( bTestHidden && ( bHidden || !bOnlyHidden ) ) || ( !bTestHidden && ( !bHidden || bUsed ) );
        if ( ( !bTestUsed && bMatchHidden ) || ( bTestUsed && bUsed ) )
            aLst.Append( cType, rList[i] );
    }
}

void  SwStyleSheetIterator::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // search and remove from View-List!!
    if( rHint.ISA( SfxStyleSheetHint ) &&
        SFX_STYLESHEET_ERASED == ((SfxStyleSheetHint&) rHint).GetHint() )
    {
        SfxStyleSheetBase* pStyle = ((SfxStyleSheetHint&)rHint).GetStyleSheet();

        if (pStyle)
        {
            sal_uInt16 nTmpPos = lcl_FindName( aLst, pStyle->GetFamily(),
                                           pStyle->GetName() );
            if( nTmpPos < aLst.size() )
                aLst.erase(aLst.begin() + nTmpPos);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
