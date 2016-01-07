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

#include <sal/config.h>

#include <cstdlib>

#include <svl/smplhint.hxx>
#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svl/eitem.hxx>
#include <unotools/syslocale.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/lrspitem.hxx>
#include <drawdoc.hxx>
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
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
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
#include <svx/xdef.hxx>
#include <SwRewriter.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflftrit.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>

// The Format names in the list of all names have the
// following family as their first character:

#define cCHAR       (sal_Unicode)'c'
#define cPARA       (sal_Unicode)'p'
#define cFRAME      (sal_Unicode)'f'
#define cPAGE       (sal_Unicode)'g'
#define cNUMRULE    (sal_Unicode)'n'

using namespace com::sun::star;

// At the names' publication, this character is removed again and the
// family is newly generated.

// In addition now there is the Bit bPhysical. In case this Bit is
// TRUE, the Pool-Formatnames are not being submitted.

class SwImplShellAction
{
    SwWrtShell* pSh;
    CurrShell* pCurrSh;
public:
    explicit SwImplShellAction( SwDoc& rDoc );
    ~SwImplShellAction();
};

SwImplShellAction::SwImplShellAction( SwDoc& rDoc )
    : pCurrSh( nullptr )
{
    if( rDoc.GetDocShell() )
        pSh = rDoc.GetDocShell()->GetWrtShell();
    else
        pSh = nullptr;

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
static SwCharFormat* lcl_FindCharFormat( SwDoc& rDoc,
                            const OUString& rName,
                            SwDocStyleSheet* pStyle = nullptr,
                            bool bCreate = true )
{
    SwCharFormat*  pFormat = nullptr;
    if (!rName.isEmpty())
    {
        pFormat = rDoc.FindCharFormatByName( rName );
        if( !pFormat && rName == SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ] )
        {
            // Standard-Character template
            pFormat = rDoc.GetDfltCharFormat();
        }

        if( !pFormat && bCreate )
        {   // explore Pool
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
            if(nId != USHRT_MAX)
                pFormat = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool(nId);
        }
    }
    if(pStyle)
    {
        if(pFormat)
        {
            pStyle->SetPhysical(true);
            SwFormat* p = pFormat->DerivedFrom();
            if( p && !p->IsDefault() )
                pStyle->PresetParent( p->GetName() );
            else
                pStyle->PresetParent( OUString() );
        }
        else
            pStyle->SetPhysical(false);
    }
    return pFormat;
}

// find/create ParaFormats
// fill Style
static SwTextFormatColl* lcl_FindParaFormat(  SwDoc& rDoc,
                                const OUString& rName,
                                SwDocStyleSheet* pStyle = nullptr,
                                bool bCreate = true )
{
    SwTextFormatColl*   pColl = nullptr;

    if (!rName.isEmpty())
    {
        pColl = rDoc.FindTextFormatCollByName( rName );
        if( !pColl && bCreate )
        {   // explore Pool
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
            if(nId != USHRT_MAX)
                pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pColl)
        {
            pStyle->SetPhysical(true);
            if( pColl->DerivedFrom() && !pColl->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pColl->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( OUString() );

            SwTextFormatColl& rNext = pColl->GetNextTextFormatColl();
            pStyle->PresetFollow(rNext.GetName());
        }
        else
            pStyle->SetPhysical(false);
    }
    return pColl;
}

// Border formats
static SwFrameFormat* lcl_FindFrameFormat(   SwDoc& rDoc,
                            const OUString& rName,
                            SwDocStyleSheet* pStyle = nullptr,
                            bool bCreate = true )
{
    SwFrameFormat* pFormat = nullptr;
    if( !rName.isEmpty() )
    {
        pFormat = rDoc.FindFrameFormatByName( rName );
        if( !pFormat && bCreate )
        {   // explore Pool
            const sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT);
            if(nId != USHRT_MAX)
                pFormat = rDoc.getIDocumentStylePoolAccess().GetFrameFormatFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pFormat)
        {
            pStyle->SetPhysical(true);
            if( pFormat->DerivedFrom() && !pFormat->DerivedFrom()->IsDefault() )
                pStyle->PresetParent( pFormat->DerivedFrom()->GetName() );
            else
                pStyle->PresetParent( OUString() );
        }
        else
            pStyle->SetPhysical(false);
    }
    return pFormat;
}

// Page descriptors
static const SwPageDesc* lcl_FindPageDesc( SwDoc&  rDoc,
                                    const OUString& rName,
                                    SwDocStyleSheet* pStyle = nullptr,
                                    bool bCreate = true )
{
    const SwPageDesc* pDesc = nullptr;

    if (!rName.isEmpty())
    {
        pDesc = rDoc.FindPageDesc(rName);
        if( !pDesc && bCreate )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC);
            if(nId != USHRT_MAX)
                pDesc = rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pDesc)
        {
            pStyle->SetPhysical(true);
            if(pDesc->GetFollow())
                pStyle->PresetFollow(pDesc->GetFollow()->GetName());
            else
                pStyle->PresetParent( OUString() );
        }
        else
            pStyle->SetPhysical(false);
    }
    return pDesc;
}

static const SwNumRule* lcl_FindNumRule(   SwDoc&  rDoc,
                                    const OUString& rName,
                                    SwDocStyleSheet* pStyle = nullptr,
                                    bool bCreate = true )
{
    const SwNumRule* pRule = nullptr;

    if (!rName.isEmpty())
    {
        pRule = rDoc.FindNumRulePtr( rName );
        if( !pRule && bCreate )
        {
            sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE);
            if(nId != USHRT_MAX)
                pRule = rDoc.getIDocumentStylePoolAccess().GetNumRuleFromPool(nId);
        }
    }

    if(pStyle)
    {
        if(pRule)
        {
            pStyle->SetPhysical(true);
            pStyle->PresetParent( OUString() );
        }
        else
            pStyle->SetPhysical(false);
    }
    return pRule;
}

sal_uInt32 SwStyleSheetIterator::SwPoolFormatList::FindName(SfxStyleFamily eFam,
                                                         const OUString &rName)
{
    if(!maImpl.empty())
    {
        sal_Unicode cStyle(0);
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:
            cStyle = cCHAR;
            break;
        case SFX_STYLE_FAMILY_PARA:
            cStyle = cPARA;
            break;
        case SFX_STYLE_FAMILY_FRAME:
            cStyle = cFRAME;
            break;
        case SFX_STYLE_FAMILY_PAGE:
            cStyle = cPAGE;
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            cStyle = cNUMRULE;
            break;
        default:
            cStyle = ' ';
            break;
        }
        const OUString sSrch = OUString(cStyle) + rName;

        UniqueHash::const_iterator it = maUnique.find(sSrch);
        if (it != maUnique.end())
        {
            sal_uInt32 nIdx = it->second;
            assert (nIdx < maImpl.size());
            assert (maImpl.size() == maUnique.size());
            return nIdx;
        }
    }
    return SAL_MAX_UINT32;
}

void SwStyleSheetIterator::SwPoolFormatList::rehash()
{
    maUnique.clear();
    for (size_t i = 0; i < maImpl.size(); i++)
        maUnique[maImpl[i]] = i;
    assert (maImpl.size() == maUnique.size());
}

void SwStyleSheetIterator::SwPoolFormatList::RemoveName(SfxStyleFamily eFam,
                                                     const OUString &rName)
{
    sal_uInt32 nTmpPos = FindName( eFam, rName );
    if( nTmpPos < maImpl.size() )
        maImpl.erase(maImpl.begin() + nTmpPos);

    // assumption: this seldom occurs, the iterator is built, then emptied.
    rehash();
    assert (maImpl.size() == maUnique.size());
}

// Add Strings to the list of templates
void SwStyleSheetIterator::SwPoolFormatList::Append( char cChar, const OUString& rStr )
{
    const OUString aStr = OUString(cChar) + rStr;

    UniqueHash::const_iterator it = maUnique.find(aStr);
    if (it != maUnique.end())
        return;

    maUnique[aStr] = (sal_uInt32)maImpl.size();
    maImpl.push_back(aStr);
}

// UI-sided implementation of StyleSheets
// uses the Core-Engine
SwDocStyleSheet::SwDocStyleSheet(   SwDoc&          rDocument,
                                    const OUString&           rName,
                                    SwDocStyleSheetPool*    _rPool,
                                    SfxStyleFamily          eFam,
                                    sal_uInt16                  _nMask) :

    SfxStyleSheetBase( rName, _rPool, eFam, _nMask ),
    pCharFormat(nullptr),
    pColl(nullptr),
    pFrameFormat(nullptr),
    pDesc(nullptr),
    pNumRule(nullptr),

    rDoc(rDocument),
    aCoreSet(GetPool().GetPool(),   //UUUU sorted by indices, one double removed
            RES_CHRATR_BEGIN,       RES_CHRATR_END - 1,             // [1
            RES_PARATR_BEGIN,       RES_PARATR_END - 1,             // [60
            RES_PARATR_LIST_BEGIN,  RES_PARATR_LIST_END - 1,        // [77
            RES_FRMATR_BEGIN,       RES_FRMATR_END - 1,             // [82
            RES_UNKNOWNATR_BEGIN,   RES_UNKNOWNATR_END-1,           // [143

            //UUUU FillAttribute support
            XATTR_FILL_FIRST, XATTR_FILL_LAST,                      // [1014

            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,          // [10023
            SID_ATTR_PAGE,          SID_ATTR_PAGE_EXT1,             // [10050
            SID_ATTR_PAGE_HEADERSET,SID_ATTR_PAGE_FOOTERSET,        // [10058
            SID_ATTR_PARA_MODEL,    SID_ATTR_PARA_MODEL,            // [10065

            //UUUU items to hand over XPropertyList things like
            // XColorList, XHatchList, XGradientList and XBitmapList
            // to the Area TabPage
            SID_COLOR_TABLE,        SID_BITMAP_LIST,                // [10179

            SID_SWREGISTER_COLLECTION, SID_SWREGISTER_COLLECTION,   // [10451
            SID_ATTR_PARA_PAGENUM, SID_ATTR_PARA_PAGENUM,           // [10457
            SID_SWREGISTER_MODE,    SID_SWREGISTER_MODE,            // [10467
            SID_ATTR_BRUSH_CHAR,  SID_ATTR_BRUSH_CHAR,              // [10590
            SID_ATTR_NUMBERING_RULE,    SID_ATTR_NUMBERING_RULE,    // [10855
            SID_ATTR_AUTO_STYLE_UPDATE, SID_ATTR_AUTO_STYLE_UPDATE, // [12065
            FN_PARAM_FTN_INFO,      FN_PARAM_FTN_INFO,              // [21123
            FN_COND_COLL,           FN_COND_COLL,                   // [22401
            0),
    bPhysical(false)
{
    nHelpId = UCHAR_MAX;
}

SwDocStyleSheet::SwDocStyleSheet( const SwDocStyleSheet& rOrg) :
    SfxStyleSheetBase(rOrg),
    pCharFormat(rOrg.pCharFormat),
    pColl(rOrg.pColl),
    pFrameFormat(rOrg.pFrameFormat),
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

void  SwDocStyleSheet::Reset()
{
    aName.clear();
    aFollow.clear();
    aParent.clear();
    SetPhysical(false);
}

void SwDocStyleSheet::SetGrabBagItem(const uno::Any& rVal)
{
    bool bChg = false;
    if (!bPhysical)
        FillStyleSheet(FillPhysical);

    SwFormat* pFormat = nullptr;
    switch (nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pFormat = rDoc.FindCharFormatByName(aName);
            if (pFormat)
            {
                pFormat->SetGrabBagItem(rVal);
                bChg = true;
            }
            break;
        case SFX_STYLE_FAMILY_PARA:
            pFormat = rDoc.FindTextFormatCollByName(aName);
            if (pFormat)
            {
                pFormat->SetGrabBagItem(rVal);
                bChg = true;
            }
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            {
                SwNumRule* pRule = rDoc.FindNumRulePtr(aName);
                if (pRule)
                {
                    pRule->SetGrabBagItem(rVal);
                    bChg = true;
                }
            }
            break;
        default:
            break;
    }

    if (bChg)
    {
        dynamic_cast<SwDocStyleSheetPool&>(*pPool).InvalidateIterator();
        pPool->Broadcast(SfxStyleSheetHint(SfxStyleSheetHintId::MODIFIED, *this));
        SwEditShell* pSh = rDoc.GetEditShell();
        if (pSh)
            pSh->CallChgLnk();
    }
}

void SwDocStyleSheet::GetGrabBagItem(uno::Any& rVal) const
{
    SwFormat* pFormat = nullptr;
    switch (nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pFormat = rDoc.FindCharFormatByName(aName);
            if (pFormat)
                pFormat->GetGrabBagItem(rVal);
            break;
        case SFX_STYLE_FAMILY_PARA:
            pFormat = rDoc.FindTextFormatCollByName(aName);
            if (pFormat)
                pFormat->GetGrabBagItem(rVal);
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
            {
                SwNumRule* pRule = rDoc.FindNumRulePtr(aName);
                if (pRule)
                    pRule->GetGrabBagItem(rVal);
            }
            break;
        default:
            break;
    }
}
// virtual methods
void SwDocStyleSheet::SetHidden( bool bValue )
{
    bool bChg = false;
    if(!bPhysical)
        FillStyleSheet( FillPhysical );

    SwFormat* pFormat = nullptr;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pFormat = rDoc.FindCharFormatByName( aName );
            if ( pFormat )
            {
                pFormat->SetHidden( bValue );
                bChg = true;
            }
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFormat = rDoc.FindTextFormatCollByName( aName );
            if ( pFormat )
            {
                pFormat->SetHidden( bValue );
                bChg = true;
            }
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFormat = rDoc.FindFrameFormatByName( aName );
            if ( pFormat )
            {
                pFormat->SetHidden( bValue );
                bChg = true;
            }
            break;

        case SFX_STYLE_FAMILY_PAGE:
            {
                SwPageDesc* pPgDesc = rDoc.FindPageDesc(aName);
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
        // calling pPool->First() here would be quite slow...
        dynamic_cast<SwDocStyleSheetPool&>(*pPool).InvalidateIterator(); // internal list has to be updated
        pPool->Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::MODIFIED, *this ) );
        SwEditShell* pSh = rDoc.GetEditShell();
        if( pSh )
            pSh->CallChgLnk();
    }
}

bool SwDocStyleSheet::IsHidden( ) const
{
    bool bRet = false;

    SwFormat* pFormat = nullptr;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            pFormat = rDoc.FindCharFormatByName( aName );
            bRet = pFormat && pFormat->IsHidden( );
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFormat = rDoc.FindTextFormatCollByName( aName );
            bRet = pFormat && pFormat->IsHidden( );
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFormat = rDoc.FindFrameFormatByName( aName );
            bRet = pFormat && pFormat->IsHidden( );
            break;

        case SFX_STYLE_FAMILY_PAGE:
            {
                SwPageDesc* pPgDesc = rDoc.FindPageDesc(aName);
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
        SwFormat* pFormat = nullptr;
        SwGetPoolIdFromName eGetType;
        switch(nFamily)
        {
        case SFX_STYLE_FAMILY_CHAR:
            pFormat = rDoc.FindCharFormatByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_CHRFMT;
            break;

        case SFX_STYLE_FAMILY_PARA:
            pFormat = rDoc.FindTextFormatCollByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL;
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFormat = rDoc.FindFrameFormatByName( aName );
            eGetType = nsSwGetPoolIdFromName::GET_POOLID_FRMFMT;
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
        default:
            return aEmptyOUStr;       // there's no parent
        }

        OUString sTmp;
        if( !pFormat )         // not yet there, so default Parent
        {
            sal_uInt16 i = SwStyleNameMapper::GetPoolIdFromUIName( aName, eGetType );
            i = ::GetPoolParent( i );
            if( i && USHRT_MAX != i )
                SwStyleNameMapper::FillUIName( i, sTmp );
        }
        else
        {
            SwFormat* p = pFormat->DerivedFrom();
            if( p && !p->IsDefault() )
                sTmp = p->GetName();
        }
        SwDocStyleSheet* pThis = const_cast<SwDocStyleSheet*>(this);
        pThis->aParent = sTmp;
    }
    return aParent;
}

// Follower
const OUString&  SwDocStyleSheet::GetFollow() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = const_cast<SwDocStyleSheet*>(this);
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
            OSL_ENSURE(false, "unknown style family");
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

    const OUString sPlus(" + ");
    if ( SFX_STYLE_FAMILY_PAGE == nFamily )
    {
        if( !pSet )
            GetItemSet();

        SfxItemIter aIter( *pSet );
        OUString aDesc;

        for (const SfxPoolItem* pItem = aIter.FirstItem(); pItem; pItem = aIter.NextItem())
        {
            if(!IsInvalidItem(pItem))
            {
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
                                *pItem, eUnit, aItemPresentation, &aIntlWrapper ) )
                        {
                            if ( !aDesc.isEmpty() && !aItemPresentation.isEmpty() )
                                aDesc += sPlus;
                            aDesc += aItemPresentation;
                        }
                    }
                }
            }
        }
        return aDesc;
    }

    if ( SFX_STYLE_FAMILY_FRAME == nFamily || SFX_STYLE_FAMILY_PARA == nFamily)
    {
        if( !pSet )
            GetItemSet();

        SfxItemIter aIter( *pSet );
        OUString aDesc;
        OUString sPageNum;
        OUString sModel;
        OUString sBreak;
        bool bHasWesternFontPrefix = false;
        bool bHasCJKFontPrefix = false;
        SvtCJKOptions aCJKOptions;

        //UUUU Get currently used FillStyle and remember, also need the XFillFloatTransparenceItem
        // to decide if gradient transparence is used
        const drawing::FillStyle eFillStyle(static_cast< const XFillStyleItem& >(pSet->Get(XATTR_FILLSTYLE)).GetValue());
        const bool bUseFloatTransparence(static_cast< const XFillFloatTransparenceItem& >(pSet->Get(XATTR_FILLFLOATTRANSPARENCE)).IsEnabled());

        for ( const SfxPoolItem* pItem = aIter.FirstItem(); pItem; pItem = aIter.NextItem() )
        {
            if(!IsInvalidItem(pItem))
            {
                switch ( pItem->Which() )
                {
                    case SID_ATTR_AUTO_STYLE_UPDATE:
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
                                *pItem, eUnit, aItemPresentation, &aIntlWrapper ) )
                        {
                            bool bIsDefault = false;
                            switch ( pItem->Which() )
                            {
                                //UUUU
                                case XATTR_FILLCOLOR:
                                {
                                    // only use active FillStyle information
                                    bIsDefault = (drawing::FillStyle_SOLID == eFillStyle);
                                    break;
                                }
                                case XATTR_FILLGRADIENT:
                                {
                                    // only use active FillStyle information
                                    bIsDefault = (drawing::FillStyle_GRADIENT == eFillStyle);
                                    break;
                                }
                                case XATTR_FILLHATCH:
                                {
                                    // only use active FillStyle information
                                    bIsDefault = (drawing::FillStyle_HATCH == eFillStyle);
                                    break;
                                }
                                case XATTR_FILLBITMAP:
                                {
                                    // only use active FillStyle information
                                    bIsDefault = (drawing::FillStyle_BITMAP == eFillStyle);
                                    break;
                                }
                                case XATTR_FILLTRANSPARENCE:
                                {
                                    // only active when not FloatTransparence
                                    bIsDefault = !bUseFloatTransparence;
                                    break;
                                }
                                case XATTR_FILLFLOATTRANSPARENCE:
                                {
                                    // only active when FloatTransparence
                                    bIsDefault = bUseFloatTransparence;
                                    break;
                                }

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
                                if ( !aDesc.isEmpty() && !aItemPresentation.isEmpty() )
                                    aDesc += sPlus;
                                aDesc += aItemPresentation;
                            }
                        }
                    }
                }
            }
        }
        // Special treatment for Break, Page template and Site offset
        if (!sModel.isEmpty())
        {
            if (!aDesc.isEmpty())
                aDesc += sPlus;
            aDesc += SW_RESSTR(STR_PAGEBREAK) + sPlus + sModel;
            if (sPageNum != "0")
            {
                aDesc += sPlus + SW_RESSTR(STR_PAGEOFFSET) + sPageNum;
            }
        }
        else if (!sBreak.isEmpty()) // Break can be valid only when NO Model
        {
            if (!aDesc.isEmpty())
                aDesc += sPlus;
            aDesc += sBreak;
        }
        return aDesc;
    }

    if( SFX_STYLE_FAMILY_PSEUDO == nFamily )
    {
        return OUString();
    }

    return SfxStyleSheetBase::GetDescription(eUnit);
}

// Set names
bool  SwDocStyleSheet::SetName(const OUString& rStr, bool bReindexNow)
{
    if( rStr.isEmpty() )
        return false;

    if( aName != rStr )
    {
        if( !SfxStyleSheetBase::SetName(rStr, bReindexNow))
            return false;
    }
    else if(!bPhysical)
        FillStyleSheet( FillPhysical );

    bool bChg = false;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
        {
            OSL_ENSURE(pCharFormat, "SwCharFormat missing!");
            if( pCharFormat && pCharFormat->GetName() != rStr )
            {
                if (!pCharFormat->GetName().isEmpty())
                    rDoc.RenameFormat(*pCharFormat, rStr);
                else
                    pCharFormat->SetName(rStr);

                bChg = true;
            }
            break;
        }
        case SFX_STYLE_FAMILY_PARA :
        {
            OSL_ENSURE(pColl, "Collection missing!");
            if( pColl && pColl->GetName() != rStr )
            {
                if (!pColl->GetName().isEmpty())
                    rDoc.RenameFormat(*pColl, rStr);
                else
                    pColl->SetName(rStr);

                bChg = true;
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            OSL_ENSURE(pFrameFormat, "FrameFormat missing!");
            if( pFrameFormat && pFrameFormat->GetName() != rStr )
            {
                if (!pFrameFormat->GetName().isEmpty())
                    rDoc.RenameFormat(*pFrameFormat, rStr);
                else
                    pFrameFormat->SetName( rStr );

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
                SwPageDesc aPageDesc(*const_cast<SwPageDesc*>(pDesc));
                const OUString aOldName(aPageDesc.GetName());

                aPageDesc.SetName( rStr );
                bool const bDoesUndo = rDoc.GetIDocumentUndoRedo().DoesUndo();

                rDoc.GetIDocumentUndoRedo().DoUndo(!aOldName.isEmpty());
                rDoc.ChgPageDesc(aOldName, aPageDesc);
                rDoc.GetIDocumentUndoRedo().DoUndo(bDoesUndo);

                rDoc.getIDocumentState().SetModified();
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
                        rDoc.getIDocumentState().SetModified();

                        bChg = true;
                    }
                }
                else
                {
                    // #i91400#
                    const_cast<SwNumRule*>(pNumRule)->SetName( rStr, rDoc.getIDocumentListsAccess() );
                    rDoc.getIDocumentState().SetModified();

                    bChg = true;
                }
            }

            break;

        default:
            OSL_ENSURE(false, "unknown style family");
    }

    if( bChg )
    {
        pPool->First();  // internal list has to be updated
        pPool->Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::MODIFIED, *this ) );
        SwEditShell* pSh = rDoc.GetEditShell();
        if( pSh )
            pSh->CallChgLnk();
    }
    return true;
}

// hierarchy of deduction
bool   SwDocStyleSheet::SetParent( const OUString& rStr)
{
    SwFormat* pFormat = nullptr, *pParent = nullptr;
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            OSL_ENSURE( pCharFormat, "SwCharFormat missing!" );
            if( nullptr != ( pFormat = pCharFormat ) && !rStr.isEmpty() )
                pParent = lcl_FindCharFormat(rDoc, rStr);
            break;

        case SFX_STYLE_FAMILY_PARA :
            OSL_ENSURE( pColl, "Collection missing!");
            if( nullptr != ( pFormat = pColl ) && !rStr.isEmpty() )
                pParent = lcl_FindParaFormat( rDoc, rStr );
            break;

        case SFX_STYLE_FAMILY_FRAME:
            OSL_ENSURE(pFrameFormat, "FrameFormat missing!");
            if( nullptr != ( pFormat = pFrameFormat ) && !rStr.isEmpty() )
                pParent = lcl_FindFrameFormat( rDoc, rStr );
            break;

        case SFX_STYLE_FAMILY_PAGE:
        case SFX_STYLE_FAMILY_PSEUDO:
            break;
        default:
            OSL_ENSURE(false, "unknown style family");
    }

    bool bRet = false;
    if( pFormat && pFormat->DerivedFrom() &&
        pFormat->DerivedFrom()->GetName() != rStr )
    {
        {
            SwImplShellAction aTmp( rDoc );
            bRet = pFormat->SetDerivedFrom( pParent );
        }

        if( bRet )
        {
            aParent = rStr;
            pPool->Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::MODIFIED,
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
            SwTextFormatColl* pFollow = pColl;
            if( !rStr.isEmpty() && nullptr == (pFollow = lcl_FindParaFormat(rDoc, rStr) ))
                pFollow = pColl;

            pColl->SetNextTextFormatColl(*pFollow);
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
                                            : nullptr;
            size_t nId = 0;
            if (pFollowDesc != pDesc->GetFollow() && rDoc.FindPageDesc(pDesc->GetName(), &nId))
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
        OSL_ENSURE(false, "unknown style family");
    }

    return true;
}

static
std::unique_ptr<SfxItemSet> lcl_SwFormatToFlatItemSet(SwFormat *const pFormat)
{
    // note: we don't add the odd items that GetItemSet() would add
    // because they don't seem relevant for preview
    std::vector<SfxItemSet const*> sets;
    sets.push_back(&pFormat->GetAttrSet());
    while (SfxItemSet const*const pParent = sets.back()->GetParent())
    {
        sets.push_back(pParent);
    }
    // start by copying top-level parent set
    std::unique_ptr<SfxItemSet> pRet(new SfxItemSet(*sets.back()));
    sets.pop_back();
    for (auto iter = sets.rbegin(); iter != sets.rend(); ++iter)
    {   // in reverse so child overrides parent
        pRet->Put(**iter);
    }
    return pRet;
}

std::unique_ptr<SfxItemSet> SwDocStyleSheet::GetItemSetForPreview()
{
    if (SFX_STYLE_FAMILY_PAGE == nFamily || SFX_STYLE_FAMILY_PSEUDO == nFamily)
    {
        SAL_WARN("sw.ui", "GetItemSetForPreview not implemented for page or number style");
        return std::unique_ptr<SfxItemSet>();
    }
    if (!bPhysical)
    {
        // because not only this style, but also any number of its parents
        // (or follow style) may not actually exist in the document at this
        // time, return one "flattened" item set that contains all items from
        // all parents.
        std::unique_ptr<SfxItemSet> pRet;
        FillStyleSheet(FillPreview, &pRet);
        assert(pRet);
        return pRet;
    }
    else
    {
        switch (nFamily)
        {
            case SFX_STYLE_FAMILY_CHAR:
                return lcl_SwFormatToFlatItemSet(pCharFormat);
            case SFX_STYLE_FAMILY_PARA:
                return lcl_SwFormatToFlatItemSet(pColl);
            case SFX_STYLE_FAMILY_FRAME:
                return lcl_SwFormatToFlatItemSet(pFrameFormat);
            default:
                std::abort();
        }
    }
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
                aBoxInfo.SetTable( false );
                aBoxInfo.SetDist( true );   // always show gap field
                aBoxInfo.SetMinDist( true );// set minimum size in tables and paragraphs
                aBoxInfo.SetDefDist( MIN_BORDER_DIST );// always set Default-Gap
                    // Single lines can only have DontCare-Status in tables
                aBoxInfo.SetValid( SvxBoxInfoItemValidFlags::DISABLE );

                if( nFamily == SFX_STYLE_FAMILY_CHAR )
                {
                    SAL_WARN_IF(!pCharFormat, "sw.ui", "Where's SwCharFormat");
                    aCoreSet.Put(pCharFormat->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );

                    if(pCharFormat->DerivedFrom())
                        aCoreSet.SetParent(&pCharFormat->DerivedFrom()->GetAttrSet());
                }
                else if ( nFamily == SFX_STYLE_FAMILY_PARA )
                {
                    OSL_ENSURE(pColl, "Where's Collection");
                    aCoreSet.Put(pColl->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );
                    aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pColl->IsAutoUpdateFormat()));

                    if(pColl->DerivedFrom())
                        aCoreSet.SetParent(&pColl->DerivedFrom()->GetAttrSet());
                }
                else
                {
                    OSL_ENSURE(pFrameFormat, "Where's FrameFormat");
                    aCoreSet.Put(pFrameFormat->GetAttrSet());
                    aCoreSet.Put( aBoxInfo );
                    aCoreSet.Put(SfxBoolItem(SID_ATTR_AUTO_STYLE_UPDATE, pFrameFormat->IsAutoUpdateFormat()));

                    if(pFrameFormat->DerivedFrom())
                        aCoreSet.SetParent(&pFrameFormat->DerivedFrom()->GetAttrSet());

                    //UUUU create needed items for XPropertyList entries from the DrawModel so that
                    // the Area TabPage can access them
                    const SwDrawModel* pDrawModel = rDoc.getIDocumentDrawModelAccess().GetDrawModel();

                    aCoreSet.Put(SvxColorListItem(pDrawModel->GetColorList(), SID_COLOR_TABLE));
                    aCoreSet.Put(SvxGradientListItem(pDrawModel->GetGradientList(), SID_GRADIENT_LIST));
                    aCoreSet.Put(SvxHatchListItem(pDrawModel->GetHatchList(), SID_HATCH_LIST));
                    aCoreSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapList(), SID_BITMAP_LIST));
                }
            }
            break;

        case SFX_STYLE_FAMILY_PAGE :
            {
                //UUUU set correct parent to get the drawing::FillStyle_NONE FillStyle as needed
                if(!aCoreSet.GetParent())
                {
                    aCoreSet.SetParent(&rDoc.GetDfltFrameFormat()->GetAttrSet());
                }

                OSL_ENSURE(pDesc, "No PageDescriptor");
                ::PageDescToItemSet(*const_cast<SwPageDesc*>(pDesc), aCoreSet);
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
            OSL_ENSURE(false, "unknown style family");
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
        OSL_ENSURE( pColl->GetItemState( RES_PARATR_NUMRULE ) == SfxItemState::SET,
                "<SwDocStyleSheet::MergeIndentAttrsOfListStyle(..)> - list level indents are applicable at paragraph style, but no list style found. Serious defect." );
        const OUString sNumRule = pColl->GetNumRule().GetValue();
        if (!sNumRule.isEmpty())
        {
            const SwNumRule* pRule = rDoc.FindNumRulePtr( sNumRule );
            if( pRule )
            {
                const SwNumFormat& rFormat = pRule->Get( 0 );
                if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    SvxLRSpaceItem aLR( RES_LR_SPACE );
                    aLR.SetTextLeft( rFormat.GetIndentAt() );
                    aLR.SetTextFirstLineOfst( static_cast<short>(rFormat.GetFirstLineIndent()) );
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

    SwFormat* pFormat = nullptr;
    SwPageDesc* pNewDsc = nullptr;
    size_t nPgDscPos = 0;

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            {
                OSL_ENSURE(pCharFormat, "Where's CharFormat");
                pFormat = pCharFormat;
            }
            break;

        case SFX_STYLE_FAMILY_PARA :
        {
            OSL_ENSURE(pColl, "Where's Collection");
            const SfxPoolItem* pAutoUpdate;
            if(SfxItemState::SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,false, &pAutoUpdate ))
            {
                pColl->SetAutoUpdateFormat(static_cast<const SfxBoolItem*>(pAutoUpdate)->GetValue());
            }

            const SwCondCollItem* pCondItem;
            if( SfxItemState::SET != rSet.GetItemState( FN_COND_COLL, false,
                reinterpret_cast<const SfxPoolItem**>(&pCondItem) ))
                pCondItem = nullptr;

            if( RES_CONDTXTFMTCOLL == pColl->Which() && pCondItem )
            {
                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for(sal_uInt16 i = 0; i < COND_COMMAND_COUNT; i++)
                {
                    SwCollCondition aCond( nullptr, pCmds[ i ].nCnd, pCmds[ i ].nSubCond );
                    static_cast<SwConditionTextFormatColl*>(pColl)->RemoveCondition( aCond );
                    const OUString sStyle = pCondItem->GetStyle( i );
                    if (sStyle.isEmpty())
                        continue;
                    SwFormat *const pFindFormat = lcl_FindParaFormat( rDoc, sStyle );
                    if (pFindFormat)
                    {
                        aCond.RegisterToFormat( *pFindFormat );
                        static_cast<SwConditionTextFormatColl*>(pColl)->InsertCondition( aCond );
                    }
                }

                // Update document to new conditions
                SwCondCollCondChg aMsg( pColl );
                pColl->ModifyNotification( &aMsg, &aMsg );
            }
            else if( pCondItem && !pColl->HasWriterListeners() )
            {
                // no conditional template, then first create and adopt
                // all important values
                SwConditionTextFormatColl* pCColl = rDoc.MakeCondTextFormatColl(
                        pColl->GetName(), static_cast<SwTextFormatColl*>(pColl->DerivedFrom()) );
                if( pColl != &pColl->GetNextTextFormatColl() )
                    pCColl->SetNextTextFormatColl( pColl->GetNextTextFormatColl() );

                if( pColl->IsAssignedToListLevelOfOutlineStyle())
                    pCColl->AssignToListLevelOfOutlineStyle(pColl->GetAssignedOutlineStyleLevel());
                else
                    pCColl->DeleteAssignmentToListLevelOfOutlineStyle();

                const CommandStruct* pCmds = SwCondCollItem::GetCmds();
                for( sal_uInt16 i = 0; i < COND_COMMAND_COUNT; ++i )
                {
                    const OUString sStyle = pCondItem->GetStyle( i );
                    if (sStyle.isEmpty())
                        continue;
                    SwTextFormatColl *const pFindFormat = lcl_FindParaFormat( rDoc, sStyle );
                    if (pFindFormat)
                    {
                        pCColl->InsertCondition( SwCollCondition( pFindFormat,
                                    pCmds[ i ].nCnd, pCmds[ i ].nSubCond ) );
                    }
                }

                rDoc.DelTextFormatColl( pColl );
                pColl = pCColl;
            }
            if ( bResetIndentAttrsAtParagraphStyle &&
                 rSet.GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET &&
                 rSet.GetItemState( RES_LR_SPACE, false ) != SfxItemState::SET &&
                 pColl->GetItemState( RES_LR_SPACE, false ) == SfxItemState::SET )
            {
                rDoc.ResetAttrAtFormat( RES_LR_SPACE, *pColl );
            }

            // #i56252: If a standard numbering style is assigned to a standard paragraph style
            // we have to create a physical instance of the numbering style. If we do not and
            // neither the paragraph style nor the numbering style is used in the document
            // the numbering style will not be saved with the document and the assignment got lost.
            const SfxPoolItem* pNumRuleItem = nullptr;
            if( SfxItemState::SET == rSet.GetItemState( RES_PARATR_NUMRULE, false, &pNumRuleItem ) )
            {   // Setting a numbering rule?
                const OUString sNumRule = static_cast<const SwNumRuleItem*>(pNumRuleItem)->GetValue();
                if (!sNumRule.isEmpty())
                {
                    SwNumRule* pRule = rDoc.FindNumRulePtr( sNumRule );
                    if( !pRule )
                    {   // Numbering rule not in use yet.
                        sal_uInt16 nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sNumRule, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
                        if( USHRT_MAX != nPoolId ) // It's a standard numbering rule
                        {
                            rDoc.getIDocumentStylePoolAccess().GetNumRuleFromPool( nPoolId ); // Create numbering rule (physical)
                        }
                    }
                }
            }

            pFormat = pColl;

            sal_uInt16 nId = pColl->GetPoolFormatId() &
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
            pColl->SetPoolFormatId( nId );
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            OSL_ENSURE(pFrameFormat, "Where's FrameFormat");
            const SfxPoolItem* pAutoUpdate;
            if(SfxItemState::SET == rSet.GetItemState(SID_ATTR_AUTO_STYLE_UPDATE,false, &pAutoUpdate ))
            {
                pFrameFormat->SetAutoUpdateFormat(static_cast<const SfxBoolItem*>(pAutoUpdate)->GetValue());
            }
            pFormat = pFrameFormat;
        }
        break;

        case SFX_STYLE_FAMILY_PAGE :
            {
                OSL_ENSURE(pDesc, "Where's PageDescriptor");

                if (rDoc.FindPageDesc(pDesc->GetName(), &nPgDscPos))
                {
                    pNewDsc = new SwPageDesc( *pDesc );
                    // #i48949# - no undo actions for the
                    // copy of the page style
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    rDoc.CopyPageDesc(*pDesc, *pNewDsc); // #i7983#

                    pFormat = &pNewDsc->GetMaster();
                }
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            {
                OSL_ENSURE(pNumRule, "Where's NumRule");

                if (!pNumRule)
                    break;

                const SfxPoolItem* pItem;
                switch( rSet.GetItemState( SID_ATTR_NUMBERING_RULE, false, &pItem ))
                {
                case SfxItemState::SET:
                {
                    SvxNumRule* pSetRule = static_cast<const SvxNumBulletItem*>(pItem)->GetNumRule();
                    pSetRule->UnLinkGraphics();
                    SwNumRule aSetRule(*pNumRule);
                    aSetRule.SetSvxRule(*pSetRule, &rDoc);
                    rDoc.ChgNumRuleFormats( aSetRule );
                }
                break;
                case SfxItemState::DONTCARE:
                // set NumRule to default values
                // what are the default values?
                {
                    SwNumRule aRule( pNumRule->GetName(),
                                     // #i89178#
                                     numfunc::GetDefaultPositionAndSpaceMode() );
                    rDoc.ChgNumRuleFormats( aRule );
                }
                break;
                default: break;
                }
            }
            break;

        default:
            OSL_ENSURE(false, "unknown style family");
    }

    if( pFormat && rSet.Count())
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
                                        *pFormat );
            }

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }
        SfxItemSet aSet(rSet);
        aSet.ClearInvalidItems();

        if(SFX_STYLE_FAMILY_FRAME == nFamily)
        {
            //UUUU Need to check for unique item for DrawingLayer items of type NameOrIndex
            // and evtl. correct that item to ensure unique names for that type. This call may
            // modify/correct entries inside of the given SfxItemSet
            rDoc.CheckForUniqueItemForLineFillNameOrIndex(aSet);
        }

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
            rDoc.ChgFormat(*pFormat, aSet);       // put all that is set
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
        rDoc.GetIDocumentUndoRedo().EndUndo(UNDO_END, nullptr);
    }
}

static void lcl_SaveStyles( sal_uInt16 nFamily, std::vector<void*>& rArr, SwDoc& rDoc )
{
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            const SwCharFormats& rTable = *rDoc.GetCharFormats();
            for( size_t n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTable[ n ] );
            }
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            const SwTextFormatColls& rTable = *rDoc.GetTextFormatColls();
            for( size_t n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTable[ n ] );
            }
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            const SwFrameFormats& rTable = *rDoc.GetFrameFormats();
            for( size_t n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTable[ n ] );
            }
        }
        break;

    case SFX_STYLE_FAMILY_PAGE:
        {
            for( size_t n = 0, nCnt = rDoc.GetPageDescCnt(); n < nCnt; ++n )
            {
                rArr.push_back( &rDoc.GetPageDesc( n ) );
            }
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            const SwNumRuleTable& rTable = rDoc.GetNumRuleTable();
            for( size_t n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                rArr.push_back( rTable[ n ] );
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
    size_t n, nCnt;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            std::deque<sal_uInt16> aDelArr;
            const SwCharFormats& rTable = *rDoc.GetCharFormats();
            for( n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTable[ n ] ))
                    aDelArr.push_front( n );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelCharFormat( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_PARA :
        {
            std::deque<sal_uInt16> aDelArr;
            const SwTextFormatColls& rTable = *rDoc.GetTextFormatColls();
            for( n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTable[ n ] ))
                    aDelArr.push_front( n );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelTextFormatColl( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_FRAME:
        {
            std::deque<SwFrameFormat*> aDelArr;
            const SwFrameFormats& rTable = *rDoc.GetFrameFormats();
            for( n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTable[ n ] ))
                    aDelArr.push_front( rTable[ n ] );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelFrameFormat( aDelArr[ n ] );
        }
        break;

    case SFX_STYLE_FAMILY_PAGE:
        {
            std::deque<size_t> aDelArr;
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
            const SwNumRuleTable& rTable = rDoc.GetNumRuleTable();
            for( n = 0, nCnt = rTable.size(); n < nCnt; ++n )
            {
                if( !lcl_Contains( rArr, rTable[ n ] ))
                    aDelArr.push_front( rTable[ n ] );
            }
            for( n = 0, nCnt = aDelArr.size(); n < nCnt; ++n )
                rDoc.DelNumRule( aDelArr[ n ]->GetName() );
        }
        break;
    }
}

// determine the format
bool SwDocStyleSheet::FillStyleSheet(
    FillStyleType const eFType, std::unique_ptr<SfxItemSet> *const o_ppFlatSet)
{
    bool bRet = false;
    sal_uInt16 nPoolId = USHRT_MAX;
    SwFormat* pFormat = nullptr;

    bool bCreate = FillPhysical == eFType;
    bool bDeleteInfo = false;
    bool bFillOnlyInfo = FillAllInfo == eFType || FillPreview == eFType;
    std::vector<void*> aDelArr;
    bool const isModified(rDoc.getIDocumentState().IsModified());

    switch(nFamily)
    {
    case SFX_STYLE_FAMILY_CHAR:
        pCharFormat = lcl_FindCharFormat(rDoc, aName, this, bCreate );
        bPhysical = nullptr != pCharFormat;
        if( bFillOnlyInfo && !bPhysical )
        {
            // create style (plus all needed parents) and clean it up
            // later - without affecting the undo/redo stack
            ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pCharFormat = lcl_FindCharFormat(rDoc, aName, this );
        }

        pFormat = pCharFormat;
        if( !bCreate && !pFormat )
        {
            if( aName == SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                            RES_POOLCOLL_TEXT_BEGIN ] )
                nPoolId = 0;
            else
                nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
        }

        bRet = nullptr != pCharFormat || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pCharFormat = nullptr;
        break;

    case SFX_STYLE_FAMILY_PARA:
        {
            pColl = lcl_FindParaFormat(rDoc, aName, this, bCreate);
            bPhysical = nullptr != pColl;
            if( bFillOnlyInfo && !bPhysical )
            {
                ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                bDeleteInfo = true;
                ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
                pColl = lcl_FindParaFormat(rDoc, aName, this );
            }

            pFormat = pColl;
            if( pColl )
                PresetFollow( pColl->GetNextTextFormatColl().GetName() );
            else if( !bCreate )
                nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );

            bRet = nullptr != pColl || USHRT_MAX != nPoolId;

            if( bDeleteInfo )
                pColl = nullptr;
        }
        break;

    case SFX_STYLE_FAMILY_FRAME:
        pFrameFormat = lcl_FindFrameFormat(rDoc,  aName, this, bCreate);
        bPhysical = nullptr != pFrameFormat;
        if (bFillOnlyInfo && !bPhysical)
        {
            ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pFrameFormat = lcl_FindFrameFormat(rDoc, aName, this );
        }
        pFormat = pFrameFormat;
        if( !bCreate && !pFormat )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT );

        bRet = nullptr != pFrameFormat || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pFrameFormat = nullptr;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        pDesc = lcl_FindPageDesc(rDoc, aName, this, bCreate);
        bPhysical = nullptr != pDesc;
        if( bFillOnlyInfo && !pDesc )
        {
            ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pDesc = lcl_FindPageDesc( rDoc, aName, this );
        }

        if( pDesc )
        {
            nPoolId = pDesc->GetPoolFormatId();
            nHelpId = pDesc->GetPoolHelpId();
            if( pDesc->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pDesc->GetPoolHlpFileId() );
            else
                aHelpFile.clear();
        }
        else if( !bCreate )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = nullptr != pDesc || USHRT_MAX != nPoolId;
        if( bDeleteInfo )
            pDesc = nullptr;
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        pNumRule = lcl_FindNumRule(rDoc, aName, this, bCreate);
        bPhysical = nullptr != pNumRule;
        if( bFillOnlyInfo && !pNumRule )
        {
            ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
            bDeleteInfo = true;
            ::lcl_SaveStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
            pNumRule = lcl_FindNumRule( rDoc, aName, this );
        }

        if( pNumRule )
        {
            nPoolId = pNumRule->GetPoolFormatId();
            nHelpId = pNumRule->GetPoolHelpId();
            if( pNumRule->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pNumRule->GetPoolHlpFileId() );
            else
                aHelpFile.clear();
        }
        else if( !bCreate )
            nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
        SetMask( USER_FMT & nPoolId ? SFXSTYLEBIT_USERDEF : 0 );

        bRet = nullptr != pNumRule || USHRT_MAX != nPoolId;

        if( bDeleteInfo )
            pNumRule = nullptr;
        break;
        default:; //prevent warning
    }

    if( SFX_STYLE_FAMILY_CHAR == nFamily ||
        SFX_STYLE_FAMILY_PARA == nFamily ||
        SFX_STYLE_FAMILY_FRAME == nFamily )
    {
        if( pFormat )
            nPoolId = pFormat->GetPoolFormatId();

        sal_uInt16 _nMask = 0;
        if( pFormat == rDoc.GetDfltCharFormat() )
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

        if( pFormat )
        {
            OSL_ENSURE( bPhysical, "Format not found" );

            nHelpId = pFormat->GetPoolHelpId();
            if( pFormat->GetPoolHlpFileId() != UCHAR_MAX )
                aHelpFile = *rDoc.GetDocPattern( pFormat->GetPoolHlpFileId() );
            else
                aHelpFile.clear();

            if( RES_CONDTXTFMTCOLL == pFormat->Which() )
                _nMask |= SWSTYLEBIT_CONDCOLL;

            if (FillPreview == eFType)
            {
                assert(o_ppFlatSet);
                *o_ppFlatSet = lcl_SwFormatToFlatItemSet(pFormat);
            }
        }

        SetMask( _nMask );
    }
    if( bDeleteInfo && bFillOnlyInfo )
    {
        ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
        ::lcl_DeleteInfoStyles( static_cast< sal_uInt16 >(nFamily), aDelArr, rDoc );
        if (!isModified)
        {
            rDoc.getIDocumentState().ResetModified();
        }
    }
    return bRet;
}

// Create new format in Core
void SwDocStyleSheet::Create()
{
    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR :
            pCharFormat = lcl_FindCharFormat( rDoc, aName );
            if( !pCharFormat )
                pCharFormat = rDoc.MakeCharFormat(aName,
                                            rDoc.GetDfltCharFormat());
            pCharFormat->SetAuto(false);
            break;

        case SFX_STYLE_FAMILY_PARA :
            pColl = lcl_FindParaFormat( rDoc, aName );
            if( !pColl )
            {
                SwTextFormatColl *pPar = (*rDoc.GetTextFormatColls())[0];
                if( nMask & SWSTYLEBIT_CONDCOLL )
                    pColl = rDoc.MakeCondTextFormatColl( aName, pPar );
                else
                    pColl = rDoc.MakeTextFormatColl( aName, pPar );
            }
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pFrameFormat = lcl_FindFrameFormat( rDoc, aName );
            if( !pFrameFormat )
                pFrameFormat = rDoc.MakeFrameFormat(aName, rDoc.GetDfltFrameFormat(), false, false);

            break;

        case SFX_STYLE_FAMILY_PAGE :
            pDesc = lcl_FindPageDesc( rDoc, aName );
            if( !pDesc )
            {
                pDesc = rDoc.MakePageDesc(aName);
            }
            break;

        case SFX_STYLE_FAMILY_PSEUDO:
            pNumRule = lcl_FindNumRule( rDoc, aName );
            if( !pNumRule )
            {
                const OUString sTmpNm( aName.isEmpty() ? rDoc.GetUniqueNumRuleName() : aName );
                SwNumRule* pRule = rDoc.GetNumRuleTable()[
                    rDoc.MakeNumRule( sTmpNm, nullptr, false,
                                      // #i89178#
                                      numfunc::GetDefaultPositionAndSpaceMode() ) ];
                pRule->SetAutoRule( false );
                if( aName.isEmpty() )
                {
                    // #i91400#
                    pRule->SetName( aName, rDoc.getIDocumentListsAccess() );
                }
                pNumRule = pRule;
            }
            break;
        default:; //prevent warning
    }
    bPhysical = true;
    aCoreSet.ClearItem();
}

SwCharFormat* SwDocStyleSheet::GetCharFormat()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pCharFormat;
}

SwTextFormatColl* SwDocStyleSheet::GetCollection()
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
    rDoc.ChgNumRuleFormats( rRule );
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
void SwDocStyleSheet::SetPhysical(bool bPhys)
{
    bPhysical = bPhys;

    if(!bPhys)
    {
        pCharFormat = nullptr;
        pColl    = nullptr;
        pFrameFormat  = nullptr;
        pDesc    = nullptr;
    }
}

SwFrameFormat* SwDocStyleSheet::GetFrameFormat()
{
    if(!bPhysical)
        FillStyleSheet( FillPhysical );
    return pFrameFormat;
}

bool  SwDocStyleSheet::IsUsed() const
{
    if( !bPhysical )
    {
        SwDocStyleSheet* pThis = const_cast<SwDocStyleSheet*>(this);
        pThis->FillStyleSheet( FillOnlyName );
    }

    if( !bPhysical )
        return false;

    const SwModify* pMod;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pMod = pCharFormat;   break;
    case SFX_STYLE_FAMILY_PARA : pMod = pColl;      break;
    case SFX_STYLE_FAMILY_FRAME: pMod = pFrameFormat;    break;
    case SFX_STYLE_FAMILY_PAGE : pMod = pDesc;      break;

    case SFX_STYLE_FAMILY_PSEUDO:
            return pNumRule && SwDoc::IsUsed( *pNumRule );

    default:
        OSL_ENSURE(false, "unknown style family");
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

    const SwFormat* pTmpFormat = nullptr;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( !pCharFormat &&
            nullptr == (pCharFormat = lcl_FindCharFormat( rDoc, aName, nullptr, false )) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFormat = pCharFormat;
        break;

    case SFX_STYLE_FAMILY_PARA:
        if( !pColl &&
            nullptr == ( pColl = lcl_FindParaFormat( rDoc, aName, nullptr, false )) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFormat = pColl;
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( !pFrameFormat &&
            nullptr == ( pFrameFormat = lcl_FindFrameFormat( rDoc, aName, nullptr, false ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT );
            return USHRT_MAX == nId ? 0 : nId;
        }
        pTmpFormat = pFrameFormat;
        break;

    case SFX_STYLE_FAMILY_PAGE:
        if( !pDesc &&
            nullptr == ( pDesc = lcl_FindPageDesc( rDoc, aName, nullptr, false ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pDesc->GetPoolHelpId();
        nFileId = pDesc->GetPoolHlpFileId();
        nPoolId = pDesc->GetPoolFormatId();
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        if( !pNumRule &&
            nullptr == ( pNumRule = lcl_FindNumRule( rDoc, aName, nullptr, false ) ) )
        {
            nId = SwStyleNameMapper::GetPoolIdFromUIName( aName, nsSwGetPoolIdFromName::GET_POOLID_NUMRULE );
            return USHRT_MAX == nId ? 0 : nId;
        }

        nId = pNumRule->GetPoolHelpId();
        nFileId = pNumRule->GetPoolHlpFileId();
        nPoolId = pNumRule->GetPoolFormatId();
        break;

    default:
        OSL_ENSURE(false, "unknown style family");
        return 0;
    }

    if( pTmpFormat )
    {
        nId = pTmpFormat->GetPoolHelpId();
        nFileId = pTmpFormat->GetPoolHlpFileId();
        nPoolId = pTmpFormat->GetPoolFormatId();
    }

    if( UCHAR_MAX != nFileId )
    {
        const OUString *pTemplate = rDoc.GetDocPattern( nFileId );
        if( pTemplate )
        {
            rFile = *pTemplate;
        }
    }
    else if( !IsPoolUserFormat( nPoolId ) )
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

    SwFormat* pTmpFormat = nullptr;
    switch( nFamily )
    {
    case SFX_STYLE_FAMILY_CHAR : pTmpFormat = pCharFormat;    break;
    case SFX_STYLE_FAMILY_PARA : pTmpFormat = pColl;       break;
    case SFX_STYLE_FAMILY_FRAME: pTmpFormat = pFrameFormat;     break;
    case SFX_STYLE_FAMILY_PAGE :
        const_cast<SwPageDesc*>(pDesc)->SetPoolHelpId( nHId );
        const_cast<SwPageDesc*>(pDesc)->SetPoolHlpFileId( nFileId );
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        const_cast<SwNumRule*>(pNumRule)->SetPoolHelpId( nHId );
        const_cast<SwNumRule*>(pNumRule)->SetPoolHlpFileId( nFileId );
        break;

    default:
        OSL_ENSURE(false, "unknown style family");
        return ;
    }
    if( pTmpFormat )
    {
        pTmpFormat->SetPoolHelpId( nHId );
        pTmpFormat->SetPoolHlpFileId( nFileId );
    }
}

// methods for DocStyleSheetPool
SwDocStyleSheetPool::SwDocStyleSheetPool( SwDoc& rDocument, bool bOrg )
: SfxStyleSheetBasePool( rDocument.GetAttrPool() )
, mxStyleSheet( new SwDocStyleSheet( rDocument, OUString(), this, SFX_STYLE_FAMILY_CHAR, 0 ) )
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
                                                sal_uInt16          _nMask)
{
    mxStyleSheet->PresetName(rName);
    mxStyleSheet->PresetParent(OUString());
    mxStyleSheet->PresetFollow(OUString());
    mxStyleSheet->SetMask(_nMask) ;
    mxStyleSheet->SetFamily(eFam);
    mxStyleSheet->SetPhysical(true);
    mxStyleSheet->Create();

    return *mxStyleSheet.get();
}

SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const SfxStyleSheetBase& /*rOrg*/)
{
    OSL_ENSURE(false , "Create im SW-Stylesheet-Pool geht nicht" );
    return nullptr;
}

SfxStyleSheetBase*   SwDocStyleSheetPool::Create( const OUString &,
                                                  SfxStyleFamily, sal_uInt16 )
{
    OSL_ENSURE( false, "Create im SW-Stylesheet-Pool geht nicht" );
    return nullptr;
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
    const OUString sName = pStyle->GetName();
    switch( pStyle->GetFamily() )
    {
    case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFormat* pFormat = lcl_FindCharFormat(rDoc, sName, nullptr, false );
            if(pFormat)
                rDoc.DelCharFormat(pFormat);
        }
        break;
    case SFX_STYLE_FAMILY_PARA:
        {
            SwTextFormatColl* pColl = lcl_FindParaFormat(rDoc, sName, nullptr, false );
            if(pColl)
                rDoc.DelTextFormatColl(pColl);
        }
        break;
    case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrameFormat* pFormat = lcl_FindFrameFormat(rDoc, sName, nullptr, false );
            if(pFormat)
                rDoc.DelFrameFormat(pFormat);
        }
        break;
    case SFX_STYLE_FAMILY_PAGE :
        {
            rDoc.DelPageDesc(sName);
        }
        break;

    case SFX_STYLE_FAMILY_PSEUDO:
        {
            if( !rDoc.DelNumRule( sName ) )
                // Only send Broadcast, when something was deleted
                bBroadcast = false;
        }
        break;

    default:
        OSL_ENSURE(false, "unknown style family");
        bBroadcast = false;
    }

    if( bBroadcast )
        Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::ERASED, *pStyle ) );
}

bool  SwDocStyleSheetPool::SetParent( SfxStyleFamily eFam,
                                      const OUString &rStyle, const OUString &rParent )
{
    SwFormat* pFormat = nullptr, *pParent = nullptr;
    switch( eFam )
    {
    case SFX_STYLE_FAMILY_CHAR :
        if( nullptr != ( pFormat = lcl_FindCharFormat( rDoc, rStyle ) ) && !rParent.isEmpty() )
            pParent = lcl_FindCharFormat(rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_PARA :
        if( nullptr != ( pFormat = lcl_FindParaFormat( rDoc, rStyle ) ) && !rParent.isEmpty() )
            pParent = lcl_FindParaFormat( rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_FRAME:
        if( nullptr != ( pFormat = lcl_FindFrameFormat( rDoc, rStyle ) ) && !rParent.isEmpty() )
            pParent = lcl_FindFrameFormat( rDoc, rParent );
        break;

    case SFX_STYLE_FAMILY_PAGE:
    case SFX_STYLE_FAMILY_PSEUDO:
        break;

    default:
        OSL_ENSURE(false, "unknown style family");
    }

    bool bRet = false;
    if( pFormat && pFormat->DerivedFrom() &&
        pFormat->DerivedFrom()->GetName() != rParent )
    {
        {
            SwImplShellAction aTmpSh( rDoc );
            bRet = pFormat->SetDerivedFrom( pParent );
        }

        if( bRet )
        {
            // only for Broadcasting
            mxStyleSheet->PresetName( rStyle );
            mxStyleSheet->PresetParent( rParent );
            if( SFX_STYLE_FAMILY_PARA == eFam )
                mxStyleSheet->PresetFollow( static_cast<SwTextFormatColl*>(pFormat)->
                        GetNextTextFormatColl().GetName() );
            else
                mxStyleSheet->PresetFollow( OUString() );

            Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::MODIFIED,
                                            *(mxStyleSheet.get()) ) );
        }
    }

    return bRet;
}

SfxStyleSheetBase* SwDocStyleSheetPool::Find( const OUString& rName,
                                              SfxStyleFamily eFam, sal_uInt16 n )
{
    sal_uInt16 nSMask = n;
    if( SFX_STYLE_FAMILY_PARA == eFam &&  rDoc.getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) )
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
    const SwModify* pMod = nullptr;

    mxStyleSheet->SetPhysical( false );
    mxStyleSheet->PresetName( rName );
    mxStyleSheet->SetFamily( eFam );
    bool bFnd = mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    if( mxStyleSheet->IsPhysical() )
    {
        switch( eFam )
        {
        case SFX_STYLE_FAMILY_CHAR:
            pMod = mxStyleSheet->GetCharFormat();
            break;

        case SFX_STYLE_FAMILY_PARA:
            pMod = mxStyleSheet->GetCollection();
            break;

        case SFX_STYLE_FAMILY_FRAME:
            pMod = mxStyleSheet->GetFrameFormat();
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
                            ? !(pRule->GetPoolFormatId() & USER_FMT)
                                // searched for used and found none
                            : bSearchUsed ))
                    bFnd = false;
            }
            break;

        default:
            OSL_ENSURE(false, "unknown style family");
        }
    }

    // then evaluate the mask:
    if( pMod && !bSearchUsed )
    {
        const sal_uInt16 nId = SFX_STYLE_FAMILY_PAGE == eFam
                        ? static_cast<const SwPageDesc*>(pMod)->GetPoolFormatId()
                        : static_cast<const SwFormat*>(pMod)->GetPoolFormatId();

        if( ( nSMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
            ? !(nId & USER_FMT)
                // searched for used and found none
            : bSearchUsed )
            bFnd = false;
    }
    return bFnd ? mxStyleSheet.get() : nullptr;
}

SwStyleSheetIterator::SwStyleSheetIterator( SwDocStyleSheetPool* pBase,
                                SfxStyleFamily eFam, sal_uInt16 n )
    : SfxStyleSheetIterator( pBase, eFam, n ),
    mxIterSheet( new SwDocStyleSheet( pBase->GetDoc(), OUString(), pBase, SFX_STYLE_FAMILY_CHAR, 0 ) ),
    mxStyleSheet( new SwDocStyleSheet( pBase->GetDoc(), OUString(), pBase, SFX_STYLE_FAMILY_CHAR, 0 ) )
{
    bFirstCalled = false;
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

SfxStyleSheetBase* SwStyleSheetIterator::operator[]( sal_uInt16 nIdx )
{
    // found
    if( !bFirstCalled )
        First();
    mxStyleSheet->PresetNameAndFamily( aLst[ nIdx ] );
    mxStyleSheet->SetPhysical( false );
    mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );

    return mxStyleSheet.get();
}

SfxStyleSheetBase*  SwStyleSheetIterator::First()
{
    // Delete old list
    bFirstCalled = true;
    nLastPos = 0;
    aLst.clear();

    // Delete current
    mxIterSheet->Reset();

    SwDoc& rDoc = static_cast<SwDocStyleSheetPool*>(pBasePool)->GetDoc();
    const sal_uInt16 nSrchMask = nMask;
    const bool bIsSearchUsed = SearchUsed();

    bool bSearchHidden = ( nMask & SFXSTYLEBIT_HIDDEN );
    bool bOnlyHidden = nMask == SFXSTYLEBIT_HIDDEN;

    const bool bOrganizer = static_cast<SwDocStyleSheetPool*>(pBasePool)->IsOrganizerMode();
    bool bAll = ( nSrchMask & SFXSTYLEBIT_ALL_VISIBLE ) == SFXSTYLEBIT_ALL_VISIBLE;

    if( nSearchFamily == SFX_STYLE_FAMILY_CHAR
     || nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const size_t nArrLen = rDoc.GetCharFormats()->size();
        for( size_t i = 0; i < nArrLen; i++ )
        {
            SwCharFormat* pFormat = (*rDoc.GetCharFormats())[ i ];

            const bool bUsed = bIsSearchUsed && (bOrganizer || rDoc.IsUsed(*pFormat));
            if( ( !bSearchHidden && pFormat->IsHidden() && !bUsed ) || ( pFormat->IsDefault() && pFormat != rDoc.GetDfltCharFormat() ) )
                continue;

            if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !pFormat->IsHidden( ) )
                continue;

            if( !bUsed )
            {
                // Standard is no User template
                const sal_uInt16 nId = rDoc.GetDfltCharFormat() == pFormat ?
                        sal_uInt16( RES_POOLCHR_INET_NORMAL ):
                                pFormat->GetPoolFormatId();
                if( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                    ? !(nId & USER_FMT)
                        // searched for used and found none
                    : bIsSearchUsed )
                {
                    continue;
                }

                if(  rDoc.getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) && !(nId & USER_FMT) &&
                    !( RES_POOLCHR_HTML_BEGIN <= nId &&
                          nId < RES_POOLCHR_HTML_END ) &&
                    RES_POOLCHR_INET_NORMAL != nId &&
                    RES_POOLCHR_INET_VISIT != nId &&
                    RES_POOLCHR_FOOTNOTE  != nId &&
                    RES_POOLCHR_ENDNOTE != nId )
                    continue;
            }

            aLst.Append( cCHAR, pFormat == rDoc.GetDfltCharFormat()
                        ? SwStyleNameMapper::GetTextUINameArray()[ RES_POOLCOLL_STANDARD -
                                                RES_POOLCOLL_TEXT_BEGIN ]
                        : pFormat->GetName() );
        }

        // PoolFormate
        if( bAll )
        {
            if( ! rDoc.getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) )
                AppendStyleList(SwStyleNameMapper::GetChrFormatUINameArray(),
                                bIsSearchUsed, bSearchHidden, bOnlyHidden,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, cCHAR);
            else
            {
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFormatUINameArray()[
                        RES_POOLCHR_INET_NORMAL - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFormatUINameArray()[
                        RES_POOLCHR_INET_VISIT - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFormatUINameArray()[
                        RES_POOLCHR_ENDNOTE - RES_POOLCHR_BEGIN ] );
                aLst.Append( cCHAR, SwStyleNameMapper::GetChrFormatUINameArray()[
                        RES_POOLCHR_FOOTNOTE - RES_POOLCHR_BEGIN ] );
            }
            AppendStyleList(SwStyleNameMapper::GetHTMLChrFormatUINameArray(),
                                bIsSearchUsed, bSearchHidden, bOnlyHidden,
                                nsSwGetPoolIdFromName::GET_POOLID_CHRFMT, cCHAR);
        }
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PARA ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        sal_uInt16 nSMask = nSrchMask;
        if(  rDoc.getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) )
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

        const size_t nArrLen = rDoc.GetTextFormatColls()->size();
        for( size_t i = 0; i < nArrLen; i++ )
        {
            SwTextFormatColl* pColl = (*rDoc.GetTextFormatColls())[ i ];

            const bool bUsed = bOrganizer || rDoc.IsUsed(*pColl);
            if ( ( !bSearchHidden && pColl->IsHidden( ) && !bUsed ) || pColl->IsDefault() )
                continue;

            if ( nSMask == SFXSTYLEBIT_HIDDEN && !pColl->IsHidden( ) )
                continue;

            if( !(bIsSearchUsed && bUsed ))
            {
                const sal_uInt16 nId = pColl->GetPoolFormatId();
                switch ( (nSMask & ~SFXSTYLEBIT_USED) )
                {
                case SFXSTYLEBIT_USERDEF:
                    if(!IsPoolUserFormat(nId)) continue;
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
                    if(IsPoolUserFormat(nId))
                        break;
                    // otherwise move on
                case SWSTYLEBIT_HTML:
                    if( (nId  & COLL_GET_RANGE_BITS) != COLL_HTML_BITS)
                    {
                        // but some we also want to see in this section
                        bool bContinue = true;
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
                            bContinue = false;
                            break;
                        }
                        if( bContinue )
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
                rDoc.getIDocumentStylePoolAccess().IsPoolTextCollUsed( RES_POOLCOLL_TEXT ))
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
                OUString s;
                while( *pPoolIds )
                {
                    if( !bIsSearchUsed || rDoc.getIDocumentStylePoolAccess().IsPoolTextCollUsed( *pPoolIds ) )
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
        const size_t nArrLen = rDoc.GetFrameFormats()->size();
        for( size_t i = 0; i < nArrLen; i++ )
        {
            const SwFrameFormat* pFormat = (*rDoc.GetFrameFormats())[ i ];

            bool bUsed = bIsSearchUsed && ( bOrganizer || rDoc.IsUsed(*pFormat));
            if( ( !bSearchHidden && pFormat->IsHidden( ) && !bUsed ) || pFormat->IsDefault() || pFormat->IsAuto() )
                continue;

            if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !pFormat->IsHidden( ) )
                continue;

            const sal_uInt16 nId = pFormat->GetPoolFormatId();
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

            aLst.Append( cFRAME, pFormat->GetName() );
        }

        // PoolFormate
        if ( bAll )
            AppendStyleList(SwStyleNameMapper::GetFrameFormatUINameArray(),
                                    bIsSearchUsed, bSearchHidden, bOnlyHidden, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, cFRAME);
    }

    if( nSearchFamily == SFX_STYLE_FAMILY_PAGE ||
        nSearchFamily == SFX_STYLE_FAMILY_ALL )
    {
        const size_t nCount = rDoc.GetPageDescCnt();
        for(size_t i = 0; i < nCount; ++i)
        {
            const SwPageDesc& rDesc = rDoc.GetPageDesc(i);
            const sal_uInt16 nId = rDesc.GetPoolFormatId();
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
        const SwNumRuleTable& rNumTable = rDoc.GetNumRuleTable();
        for(size_t i = 0; i < rNumTable.size(); ++i)
        {
            const SwNumRule& rRule = *rNumTable[ i ];
            if( !rRule.IsAutoRule() )
            {
                if ( nSrchMask == SFXSTYLEBIT_HIDDEN && !rRule.IsHidden( ) )
                    continue;

                bool bUsed = bIsSearchUsed && ( bOrganizer || SwDoc::IsUsed(rRule) );
                if( !bUsed )
                {
                    if( ( !bSearchHidden && rRule.IsHidden() ) ||
                           ( (nSrchMask & ~SFXSTYLEBIT_USED) == SFXSTYLEBIT_USERDEF
                        ? !(rRule.GetPoolFormatId() & USER_FMT)
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
        nLastPos = SAL_MAX_UINT32;
        return Next();
    }
    return nullptr;
}

SfxStyleSheetBase* SwStyleSheetIterator::Next()
{
    assert(bFirstCalled);
    ++nLastPos;
    if(nLastPos < aLst.size())
    {
        mxIterSheet->PresetNameAndFamily(aLst[nLastPos]);
        mxIterSheet->SetPhysical( false );
        mxIterSheet->SetMask( nMask );
        if(mxIterSheet->pSet)
        {
            mxIterSheet->pSet->ClearItem();
            mxIterSheet->pSet= nullptr;
        }
        return mxIterSheet.get();
    }
    return nullptr;
}

SfxStyleSheetBase* SwStyleSheetIterator::Find(const OUString& rName)
{
    // searching
    if( !bFirstCalled )
        First();

    nLastPos = aLst.FindName( nSearchFamily, rName );
    if( SAL_MAX_UINT32 != nLastPos )
    {
        // found
        mxStyleSheet->PresetNameAndFamily(aLst[nLastPos]);
        // new name is set, so determine its Data
        mxStyleSheet->FillStyleSheet( SwDocStyleSheet::FillOnlyName );
        if( !mxStyleSheet->IsPhysical() )
            mxStyleSheet->SetPhysical( false );

        return mxStyleSheet.get();
    }
    return nullptr;
}

void SwStyleSheetIterator::AppendStyleList(const ::std::vector<OUString>& rList,
                                            bool bTestUsed, bool bTestHidden, bool bOnlyHidden,
                                            sal_uInt16 nSection, char cType )
{
    SwDoc& rDoc = static_cast<SwDocStyleSheetPool*>(pBasePool)->GetDoc();
    bool bUsed = false;
    for ( size_t i=0; i < rList.size(); ++i )
    {
        bool bHidden = false;
        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rList[i], (SwGetPoolIdFromName)nSection);
        switch ( nSection )
        {
            case nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL:
                {
                    bUsed = rDoc.getIDocumentStylePoolAccess().IsPoolTextCollUsed( nId );
                    SwFormat* pFormat = rDoc.FindTextFormatCollByName( rList[i] );
                    bHidden = pFormat && pFormat->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_CHRFMT:
                {
                    bUsed = rDoc.getIDocumentStylePoolAccess().IsPoolFormatUsed( nId );
                    SwFormat* pFormat = rDoc.FindCharFormatByName( rList[i] );
                    bHidden = pFormat && pFormat->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_FRMFMT:
                {
                    bUsed = rDoc.getIDocumentStylePoolAccess().IsPoolFormatUsed( nId );
                    SwFormat* pFormat = rDoc.FindFrameFormatByName( rList[i] );
                    bHidden = pFormat && pFormat->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC:
                {
                    bUsed = rDoc.getIDocumentStylePoolAccess().IsPoolPageDescUsed( nId );
                    SwPageDesc* pPgDesc = rDoc.FindPageDesc(rList[i]);
                    bHidden = pPgDesc && pPgDesc->IsHidden( );
                }
                break;
            case nsSwGetPoolIdFromName::GET_POOLID_NUMRULE:
                {
                    SwNumRule* pRule = rDoc.FindNumRulePtr( rList[i] );
                    bUsed = pRule && SwDoc::IsUsed( *pRule );
                    bHidden = pRule && pRule->IsHidden( );
                }
                break;
            default:
                OSL_ENSURE( false, "unknown PoolFormat-Id" );
        }

        bool bMatchHidden = ( bTestHidden && ( bHidden || !bOnlyHidden ) ) || ( !bTestHidden && ( !bHidden || bUsed ) );
        if ( ( !bTestUsed && bMatchHidden ) || ( bTestUsed && bUsed ) )
            aLst.Append( cType, rList[i] );
    }
}

void SwDocStyleSheetPool::InvalidateIterator()
{
    dynamic_cast<SwStyleSheetIterator&>(GetIterator_Impl()).InvalidateIterator();
}

void SwStyleSheetIterator::InvalidateIterator()
{
    // potentially we could send an SfxHint to Notify but currently it's
    // iterating over the vector anyway so would still be slow - why does
    // this iterator not use a map?
    bFirstCalled = false;
    nLastPos = 0;
    aLst.clear();
}

void SwStyleSheetIterator::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    // search and remove from View-List!!
    const SfxStyleSheetHint* pStyleSheetHint = dynamic_cast<const SfxStyleSheetHint*>(&rHint);
    if( pStyleSheetHint &&
        SfxStyleSheetHintId::ERASED == pStyleSheetHint->GetHint() )
    {
        SfxStyleSheetBase* pStyle = pStyleSheetHint->GetStyleSheet();

        if (pStyle)
            aLst.RemoveName(pStyle->GetFamily(), pStyle->GetName());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
