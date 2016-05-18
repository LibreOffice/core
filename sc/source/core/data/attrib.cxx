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

#include <com/sun/star/util/CellProtection.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>

#include "attrib.hxx"
#include "global.hxx"
#include "editutil.hxx"
#include "sc.hrc"
#include "globstr.hrc"

#include "textuno.hxx"

using namespace com::sun::star;


SfxPoolItem* ScProtectionAttr::CreateDefault() { return new ScProtectionAttr; }
SfxPoolItem* ScDoubleItem::CreateDefault() { DBG_ASSERT(false, "No ScDoubleItem factory available"); return nullptr; }

/**
 * General Help Function
 */
bool ScHasPriority( const ::editeng::SvxBorderLine* pThis, const ::editeng::SvxBorderLine* pOther )
{

    if (!pThis)
        return false;
    if (!pOther)
        return true;

    sal_uInt16 nThisSize = pThis->GetScaledWidth();
    sal_uInt16 nOtherSize = pOther->GetScaledWidth();

    if (nThisSize > nOtherSize)
        return true;
    else if (nThisSize < nOtherSize)
        return false;
    else
    {
        if ( pOther->GetInWidth() && !pThis->GetInWidth() )
            return true;
        else if ( pThis->GetInWidth() && !pOther->GetInWidth() )
            return false;
        else
        {
            return true; // FIXME: What is this?
        }
    }
}

/** Item - Implementations */

/**
 * Merge
 */
ScMergeAttr::ScMergeAttr():
    SfxPoolItem(ATTR_MERGE),
    nColMerge(0),
    nRowMerge(0)
{}

ScMergeAttr::ScMergeAttr( SCsCOL nCol, SCsROW nRow):
    SfxPoolItem(ATTR_MERGE),
    nColMerge(nCol),
    nRowMerge(nRow)
{}

ScMergeAttr::ScMergeAttr(const ScMergeAttr& rItem):
    SfxPoolItem(ATTR_MERGE)
{
    nColMerge = rItem.nColMerge;
    nRowMerge = rItem.nRowMerge;
}

ScMergeAttr::~ScMergeAttr()
{
}

bool ScMergeAttr::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( Which() != rItem.Which() || typeid(*this) == typeid(rItem), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (nColMerge == static_cast<const ScMergeAttr&>(rItem).nColMerge)
             && (nRowMerge == static_cast<const ScMergeAttr&>(rItem).nRowMerge);
}

SfxPoolItem* ScMergeAttr::Clone( SfxItemPool * ) const
{
    return new ScMergeAttr(*this);
}

SfxPoolItem* ScMergeAttr::Create( SvStream& rStream, sal_uInt16 /* nVer */ ) const
{
    sal_Int16   nCol;
    sal_Int16   nRow;
    rStream.ReadInt16( nCol );
    rStream.ReadInt16( nRow );
    return new ScMergeAttr(static_cast<SCCOL>(nCol),static_cast<SCROW>(nRow));
}

/**
 * MergeFlag
 */
ScMergeFlagAttr::ScMergeFlagAttr():
    SfxInt16Item(ATTR_MERGE_FLAG, 0)
{
}

ScMergeFlagAttr::ScMergeFlagAttr(ScMF nFlags):
    SfxInt16Item(ATTR_MERGE_FLAG, static_cast<sal_Int16>(nFlags))
{
}

ScMergeFlagAttr::~ScMergeFlagAttr()
{
}

SfxPoolItem * ScMergeFlagAttr::Clone(SfxItemPool *) const
{
    return new ScMergeFlagAttr(*this);
}

bool ScMergeFlagAttr::HasPivotButton() const
{
    return bool(static_cast<ScMF>(GetValue()) & ScMF::Button);
}

bool ScMergeFlagAttr::HasPivotPopupButton() const
{
    return bool(static_cast<ScMF>(GetValue()) & ScMF::ButtonPopup);
}

/**
 * Protection
 */
ScProtectionAttr::ScProtectionAttr():
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(true),
    bHideFormula(false),
    bHideCell(false),
    bHidePrint(false)
{
}

ScProtectionAttr::ScProtectionAttr( bool bProtect, bool bHFormula,
                                    bool bHCell, bool bHPrint):
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(bProtect),
    bHideFormula(bHFormula),
    bHideCell(bHCell),
    bHidePrint(bHPrint)
{
}

ScProtectionAttr::ScProtectionAttr(const ScProtectionAttr& rItem):
    SfxPoolItem(ATTR_PROTECTION)
{
    bProtection  = rItem.bProtection;
    bHideFormula = rItem.bHideFormula;
    bHideCell    = rItem.bHideCell;
    bHidePrint   = rItem.bHidePrint;
}

ScProtectionAttr::~ScProtectionAttr()
{
}

bool ScProtectionAttr::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId  )
    {
        case 0 :
        {
            util::CellProtection aProtection;
            aProtection.IsLocked        = bProtection;
            aProtection.IsFormulaHidden = bHideFormula;
            aProtection.IsHidden        = bHideCell;
            aProtection.IsPrintHidden   = bHidePrint;
            rVal <<= aProtection;
            break;
        }
        case MID_1 :
            rVal <<= bProtection; break;
        case MID_2 :
            rVal <<= bHideFormula; break;
        case MID_3 :
            rVal <<= bHideCell; break;
        case MID_4 :
            rVal <<= bHidePrint; break;
        default:
            OSL_FAIL("Wrong MemberID!");
            return false;
    }

    return true;
}

bool ScProtectionAttr::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = false;
    bool bVal = false;
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case 0 :
        {
            util::CellProtection aProtection;
            if ( rVal >>= aProtection )
            {
                bProtection  = aProtection.IsLocked;
                bHideFormula = aProtection.IsFormulaHidden;
                bHideCell    = aProtection.IsHidden;
                bHidePrint   = aProtection.IsPrintHidden;
                bRet = true;
            }
            else
            {
                OSL_FAIL("exception - wrong argument");
            }
            break;
        }
        case MID_1 :
            bRet = (rVal >>= bVal); if (bRet) bProtection=bVal; break;
        case MID_2 :
            bRet = (rVal >>= bVal); if (bRet) bHideFormula=bVal; break;
        case MID_3 :
            bRet = (rVal >>= bVal); if (bRet) bHideCell=bVal; break;
        case MID_4 :
            bRet = (rVal >>= bVal); if (bRet) bHidePrint=bVal; break;
        default:
            OSL_FAIL("Wrong MemberID!");
    }

    return bRet;
}

OUString ScProtectionAttr::GetValueText() const
{
    const OUString aStrYes ( ScGlobal::GetRscString(STR_YES) );
    const OUString aStrNo  ( ScGlobal::GetRscString(STR_NO) );

    const OUString aValue  = "("
        + (bProtection ? aStrYes : aStrNo)
        + ","
        + (bHideFormula ? aStrYes : aStrNo)
        + ","
        + (bHideCell ? aStrYes : aStrNo)
        + ","
        + (bHidePrint ? aStrYes : aStrNo)
        + ")";

    return aValue;
}

bool ScProtectionAttr::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit /* eCoreMetric */,
        SfxMapUnit /* ePresMetric */,
        OUString& rText,
        const IntlWrapper* /* pIntl */
    ) const
{
    const OUString aStrYes ( ScGlobal::GetRscString(STR_YES) );
    const OUString aStrNo  ( ScGlobal::GetRscString(STR_NO) );

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = GetValueText();
            break;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText  = ScGlobal::GetRscString(STR_PROTECTION)
                + ": "
                + (bProtection ? aStrYes : aStrNo)
                + ", "
                + ScGlobal::GetRscString(STR_FORMULAS)
                + ": "
                + (!bHideFormula ? aStrYes : aStrNo)
                + ", "
                + ScGlobal::GetRscString(STR_HIDE)
                + ": "
                + (bHideCell ? aStrYes : aStrNo)
                + ", "
                + ScGlobal::GetRscString(STR_PRINT)
                + ": "
                + (!bHidePrint ? aStrYes : aStrNo);
            break;

        default: break;
    }

    return true;
}

bool ScProtectionAttr::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( Which() != rItem.Which() || typeid(*this) == typeid(rItem), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (bProtection == static_cast<const ScProtectionAttr&>(rItem).bProtection)
             && (bHideFormula == static_cast<const ScProtectionAttr&>(rItem).bHideFormula)
             && (bHideCell == static_cast<const ScProtectionAttr&>(rItem).bHideCell)
             && (bHidePrint == static_cast<const ScProtectionAttr&>(rItem).bHidePrint);
}

SfxPoolItem* ScProtectionAttr::Clone( SfxItemPool * ) const
{
    return new ScProtectionAttr(*this);
}

SfxPoolItem* ScProtectionAttr::Create( SvStream& rStream, sal_uInt16 /* n */ ) const
{
    bool bProtect;
    bool bHFormula;
    bool bHCell;
    bool bHPrint;

    rStream.ReadCharAsBool( bProtect );
    rStream.ReadCharAsBool( bHFormula );
    rStream.ReadCharAsBool( bHCell );
    rStream.ReadCharAsBool( bHPrint );

    return new ScProtectionAttr(bProtect,bHFormula,bHCell,bHPrint);
}

bool ScProtectionAttr::SetProtection( bool bProtect)
{
    bProtection =  bProtect;
    return true;
}

bool ScProtectionAttr::SetHideFormula( bool bHFormula)
{
    bHideFormula = bHFormula;
    return true;
}

bool ScProtectionAttr::SetHideCell( bool bHCell)
{
    bHideCell = bHCell;
    return true;
}

bool ScProtectionAttr::SetHidePrint( bool bHPrint)
{
    bHidePrint = bHPrint;
    return true;
}

/**
 * ScRangeItem - Table range
 */
bool ScRangeItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    return aRange == static_cast<const ScRangeItem&>(rAttr).aRange;
}

SfxPoolItem* ScRangeItem::Clone( SfxItemPool* ) const
{
    return new ScRangeItem( *this );
}

bool ScRangeItem::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit          /* eCoreUnit */,
        SfxMapUnit          /* ePresUnit */,
        OUString&           rText,
        const IntlWrapper*  /* pIntl */
    ) const
{
    rText.clear();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
        rText = ScGlobal::GetRscString(STR_AREA) + ": ";
        SAL_FALLTHROUGH;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        {
            /* Always use OOo:A1 format */
            rText += aRange.Format();
        }
        break;

        default:
        {
            // added to avoid warnings
        }
    }

    return true;
}

/**
 * ScTableListItem - List from Tables (-numbers)
 */
ScTableListItem::ScTableListItem( const ScTableListItem& rCpy )
    :   SfxPoolItem ( rCpy.Which() ),
        nCount      ( rCpy.nCount )
{
    if ( nCount > 0 )
    {
        pTabArr = new SCTAB [nCount];

        for ( sal_uInt16 i=0; i<nCount; i++ )
            pTabArr[i] = rCpy.pTabArr[i];
    }
    else
        pTabArr = nullptr;
}

ScTableListItem::~ScTableListItem()
{
    delete [] pTabArr;
}

ScTableListItem& ScTableListItem::operator=( const ScTableListItem& rCpy )
{
    delete [] pTabArr;

    if ( rCpy.nCount > 0 )
    {
        pTabArr = new SCTAB [rCpy.nCount];
        for ( sal_uInt16 i=0; i<rCpy.nCount; i++ )
            pTabArr[i] = rCpy.pTabArr[i];
    }
    else
        pTabArr = nullptr;

    nCount = rCpy.nCount;

    return *this;
}

bool ScTableListItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const ScTableListItem& rCmp   = static_cast<const ScTableListItem&>(rAttr);
    bool                   bEqual = (nCount == rCmp.nCount);

    if ( nCount > 0 )
    {
        sal_uInt16  i=0;

        bEqual = ( pTabArr && rCmp.pTabArr );

        while ( bEqual && i<nCount )
        {
            bEqual = ( pTabArr[i] == rCmp.pTabArr[i] );
            i++;
        }
    }
    return bEqual;
}

SfxPoolItem* ScTableListItem::Clone( SfxItemPool* ) const
{
    return new ScTableListItem( *this );
}

bool ScTableListItem::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit          /* eCoreUnit */,
        SfxMapUnit          /* ePresUnit */,
        OUString&           rText,
        const IntlWrapper* /* pIntl */
    ) const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            {
            rText  = "(";
            if ( nCount>0 && pTabArr )
                for ( sal_uInt16 i=0; i<nCount; i++ )
                {
                    rText += OUString::number( pTabArr[i] );
                    if ( i<(nCount-1) )
                        rText += ",";
                }
            rText += ")";
            }
            return true;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText.clear();
            return false;

        default:
        {
            // added to avoid warnings
        }
    }

    return false;
}

/**
 * ScPageHFItem - Dates from the Head and Foot lines
 */
ScPageHFItem::ScPageHFItem( sal_uInt16 nWhichP )
    :   SfxPoolItem ( nWhichP ),
        pLeftArea   ( nullptr ),
        pCenterArea ( nullptr ),
        pRightArea  ( nullptr )
{
}

ScPageHFItem::ScPageHFItem( const ScPageHFItem& rItem )
    :   SfxPoolItem ( rItem ),
        pLeftArea   ( nullptr ),
        pCenterArea ( nullptr ),
        pRightArea  ( nullptr )
{
    if ( rItem.pLeftArea )
        pLeftArea = rItem.pLeftArea->Clone();
    if ( rItem.pCenterArea )
        pCenterArea = rItem.pCenterArea->Clone();
    if ( rItem.pRightArea )
        pRightArea = rItem.pRightArea->Clone();
}

ScPageHFItem::~ScPageHFItem()
{
    delete pLeftArea;
    delete pCenterArea;
    delete pRightArea;
}

bool ScPageHFItem::QueryValue( uno::Any& rVal, sal_uInt8 /* nMemberId */ ) const
{
    uno::Reference<sheet::XHeaderFooterContent> xContent =
        new ScHeaderFooterContentObj( pLeftArea, pCenterArea, pRightArea );

    rVal <<= xContent;
    return true;
}

bool ScPageHFItem::PutValue( const uno::Any& rVal, sal_uInt8 /* nMemberId */ )
{
    bool bRet = false;
    uno::Reference<sheet::XHeaderFooterContent> xContent;
    if ( rVal >>= xContent )
    {
        if ( xContent.is() )
        {
            rtl::Reference<ScHeaderFooterContentObj> pImp =
                    ScHeaderFooterContentObj::getImplementation( xContent );
            if (pImp.is())
            {
                const EditTextObject* pImpLeft = pImp->GetLeftEditObject();
                delete pLeftArea;
                pLeftArea = pImpLeft ? pImpLeft->Clone() : nullptr;

                const EditTextObject* pImpCenter = pImp->GetCenterEditObject();
                delete pCenterArea;
                pCenterArea = pImpCenter ? pImpCenter->Clone() : nullptr;

                const EditTextObject* pImpRight = pImp->GetRightEditObject();
                delete pRightArea;
                pRightArea = pImpRight ? pImpRight->Clone() : nullptr;

                if ( !pLeftArea || !pCenterArea || !pRightArea )
                {
                    // no Text with Null are left
                    ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), true );
                    if (!pLeftArea)
                        pLeftArea = aEngine.CreateTextObject();
                    if (!pCenterArea)
                        pCenterArea = aEngine.CreateTextObject();
                    if (!pRightArea)
                        pRightArea = aEngine.CreateTextObject();
                }

                bRet = true;
            }
        }
    }

    if (!bRet)
    {
        OSL_FAIL("exception - wrong argument");
    }

    return true;
}

bool ScPageHFItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));

    const ScPageHFItem& r = static_cast<const ScPageHFItem&>(rItem);

    return    ScGlobal::EETextObjEqual(pLeftArea,   r.pLeftArea)
           && ScGlobal::EETextObjEqual(pCenterArea, r.pCenterArea)
           && ScGlobal::EETextObjEqual(pRightArea,  r.pRightArea);
}

SfxPoolItem* ScPageHFItem::Clone( SfxItemPool* ) const
{
    return new ScPageHFItem( *this );
}

static void lcl_SetSpace( OUString& rStr, const ESelection& rSel )
{
    // Text replaced by a space to ensure they are positions:
    sal_Int32 nLen = rSel.nEndPos-rSel.nStartPos;
    rStr = rStr.replaceAt( rSel.nStartPos, nLen, " " );
}

static bool lcl_ConvertFields(EditEngine& rEng, const OUString* pCommands)
{
    bool bChange = false;
    sal_Int32 nParCnt = rEng.GetParagraphCount();
    for (sal_Int32 nPar = 0; nPar<nParCnt; nPar++)
    {
        OUString aStr = rEng.GetText( nPar );
        sal_Int32 nPos;

        while ((nPos = aStr.indexOf(pCommands[0])) != -1)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[0].getLength() );
            rEng.QuickInsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.indexOf(pCommands[1])) != -1)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[1].getLength() );
            rEng.QuickInsertField( SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.indexOf(pCommands[2])) != -1)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[2].getLength() );
            rEng.QuickInsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SVXDATETYPE_VAR), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.indexOf(pCommands[3])) != -1)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[3].getLength() );
            rEng.QuickInsertField( SvxFieldItem(SvxTimeField(), EE_FEATURE_FIELD ), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.indexOf(pCommands[4])) != -1)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[4].getLength() );
            rEng.QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.indexOf(pCommands[5])) != -1)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[5].getLength() );
            rEng.QuickInsertField( SvxFieldItem(SvxTableField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
    }
    return bChange;
}

#define SC_FIELD_COUNT  6

SfxPoolItem* ScPageHFItem::Create( SvStream& rStream, sal_uInt16 nVer ) const
{
    EditTextObject* pLeft   = EditTextObject::Create(rStream);
    EditTextObject* pCenter = EditTextObject::Create(rStream);
    EditTextObject* pRight  = EditTextObject::Create(rStream);

    OSL_ENSURE( pLeft && pCenter && pRight, "Error reading ScPageHFItem" );

    if ( pLeft == nullptr   || pLeft->GetParagraphCount() == 0 ||
         pCenter == nullptr || pCenter->GetParagraphCount() == 0 ||
         pRight == nullptr  || pRight->GetParagraphCount() == 0 )
    {
        // If successfully loaded, each object contains at least one paragraph.
        // Excel import in 5.1 created broken TextObjects (#67442#) that are
        // corrected here to avoid saving wrong files again (#90487#).
        ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), true );
        if ( pLeft == nullptr || pLeft->GetParagraphCount() == 0 )
        {
            delete pLeft;
            pLeft = aEngine.CreateTextObject();
        }
        if ( pCenter == nullptr || pCenter->GetParagraphCount() == 0 )
        {
            delete pCenter;
            pCenter = aEngine.CreateTextObject();
        }
        if ( pRight == nullptr || pRight->GetParagraphCount() == 0 )
        {
            delete pRight;
            pRight = aEngine.CreateTextObject();
        }
    }

    if ( nVer < 1 ) // old field command conversions
    {
        sal_uInt16 i;
        const OUString& rDel = ScGlobal::GetRscString( STR_HFCMD_DELIMITER );
        OUString aCommands[SC_FIELD_COUNT];
        for (i=0; i<SC_FIELD_COUNT; i++)
            aCommands[i] = rDel;
        aCommands[0] += ScGlobal::GetRscString(STR_HFCMD_PAGE);
        aCommands[1] += ScGlobal::GetRscString(STR_HFCMD_PAGES);
        aCommands[2] += ScGlobal::GetRscString(STR_HFCMD_DATE);
        aCommands[3] += ScGlobal::GetRscString(STR_HFCMD_TIME);
        aCommands[4] += ScGlobal::GetRscString(STR_HFCMD_FILE);
        aCommands[5] += ScGlobal::GetRscString(STR_HFCMD_TABLE);
        for (i=0; i<SC_FIELD_COUNT; i++)
            aCommands[i] += rDel;

        ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), true );
        aEngine.SetText(*pLeft);
        if (lcl_ConvertFields(aEngine,aCommands))
        {
            delete pLeft;
            pLeft = aEngine.CreateTextObject();
        }
        aEngine.SetText(*pCenter);
        if (lcl_ConvertFields(aEngine,aCommands))
        {
            delete pCenter;
            pCenter = aEngine.CreateTextObject();
        }
        aEngine.SetText(*pRight);
        if (lcl_ConvertFields(aEngine,aCommands))
        {
            delete pRight;
            pRight = aEngine.CreateTextObject();
        }
    }
    else if ( nVer < 2 ) {} // nothing to do: SvxFileField is not exchanged for SvxExtFileField

    ScPageHFItem* pItem = new ScPageHFItem( Which() );
    pItem->SetArea( pLeft,    SC_HF_LEFTAREA   );
    pItem->SetArea( pCenter, SC_HF_CENTERAREA );
    pItem->SetArea( pRight,  SC_HF_RIGHTAREA  );

    return pItem;
}

void ScPageHFItem::SetLeftArea( const EditTextObject& rNew )
{
    delete pLeftArea;
    pLeftArea = rNew.Clone();
}

void ScPageHFItem::SetCenterArea( const EditTextObject& rNew )
{
    delete pCenterArea;
    pCenterArea = rNew.Clone();
}

void ScPageHFItem::SetRightArea( const EditTextObject& rNew )
{
    delete pRightArea;
    pRightArea = rNew.Clone();
}

void ScPageHFItem::SetArea( EditTextObject *pNew, int nArea )
{
    switch ( nArea )
    {
        case SC_HF_LEFTAREA:    delete pLeftArea;   pLeftArea   = pNew; break;
        case SC_HF_CENTERAREA:  delete pCenterArea; pCenterArea = pNew; break;
        case SC_HF_RIGHTAREA:   delete pRightArea;  pRightArea  = pNew; break;
        default:
            OSL_FAIL( "New Area?" );
    }
}

/**
 * ScViewObjectModeItem - Display Mode of View Objects
 */
ScViewObjectModeItem::ScViewObjectModeItem( sal_uInt16 nWhichP )
    : SfxEnumItem( nWhichP, VOBJ_MODE_SHOW )
{
}

ScViewObjectModeItem::ScViewObjectModeItem( sal_uInt16 nWhichP, ScVObjMode eMode )
    : SfxEnumItem( nWhichP, sal::static_int_cast<sal_uInt16>(eMode) )
{
}

ScViewObjectModeItem::~ScViewObjectModeItem()
{
}

bool ScViewObjectModeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /* eCoreUnit */,
    SfxMapUnit          /* ePresUnit */,
    OUString&           rText,
    const IntlWrapper* /* pIntl */
)   const
{
    OUString aDel(": ");
    rText.clear();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
        switch( Which() )
        {
            case SID_SCATTR_PAGE_CHARTS:
            rText = ScGlobal::GetRscString(STR_VOBJ_CHART) + aDel;
            break;

            case SID_SCATTR_PAGE_OBJECTS:
            rText = ScGlobal::GetRscString(STR_VOBJ_OBJECT) + aDel;
            break;

            case SID_SCATTR_PAGE_DRAWINGS:
            rText = ScGlobal::GetRscString(STR_VOBJ_DRAWINGS) + aDel;
            break;

            default:
            ePres = SFX_ITEM_PRESENTATION_NAMELESS; // Default setting!
            break;
        }
        SAL_FALLTHROUGH;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        rText += ScGlobal::GetRscString(STR_VOBJ_MODE_SHOW+GetValue());
        return true;
        break;

        default: break;
            // added to avoid warnings
    }

    return false;
}

sal_uInt16 ScViewObjectModeItem::GetValueCount() const
{
    return 2;
}

SfxPoolItem* ScViewObjectModeItem::Clone( SfxItemPool* ) const
{
    return new ScViewObjectModeItem( *this );
}

sal_uInt16 ScViewObjectModeItem::GetVersion( sal_uInt16 /* nFileVersion */ ) const
{
    return 1;
}

SfxPoolItem* ScViewObjectModeItem::Create(
                                    SvStream&   rStream,
                                    sal_uInt16      nVersion ) const
{
    if ( nVersion == 0 )
    {
        // Old Version with AllEnuItem -> produce with Mode "Show"
        return new ScViewObjectModeItem( Which() );
    }
    else
    {
        sal_uInt16 nVal;
        rStream.ReadUInt16( nVal );

        //#i80528# adapt to new range eventually
        if((sal_uInt16)VOBJ_MODE_HIDE < nVal) nVal = (sal_uInt16)VOBJ_MODE_SHOW;

        return new ScViewObjectModeItem( Which(), (ScVObjMode)nVal);
    }
}

/**
 * Double
 */
ScDoubleItem::ScDoubleItem( sal_uInt16 nWhichP, double nVal )
    :   SfxPoolItem ( nWhichP ),
        nValue  ( nVal )
{
}

ScDoubleItem::ScDoubleItem( const ScDoubleItem& rItem )
    :   SfxPoolItem ( rItem )
{
        nValue = rItem.nValue;
}

bool ScDoubleItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    const ScDoubleItem& _rItem = static_cast<const ScDoubleItem&>(rItem);
    return nValue == _rItem.nValue;
}

SfxPoolItem* ScDoubleItem::Clone( SfxItemPool* ) const
{
    return new ScDoubleItem( *this );
}

SfxPoolItem* ScDoubleItem::Create( SvStream& rStream, sal_uInt16 /* nVer */ ) const
{
    double nTmp=0;
    rStream.ReadDouble( nTmp );

    ScDoubleItem* pItem = new ScDoubleItem( Which(), nTmp );

    return pItem;
}

ScDoubleItem::~ScDoubleItem()
{
}

ScPageScaleToItem::ScPageScaleToItem() :
    SfxPoolItem( ATTR_PAGE_SCALETO ),
    mnWidth( 0 ),
    mnHeight( 0 )
{
}

ScPageScaleToItem::ScPageScaleToItem( sal_uInt16 nWidth, sal_uInt16 nHeight ) :
    SfxPoolItem( ATTR_PAGE_SCALETO ),
    mnWidth( nWidth ),
    mnHeight( nHeight )
{
}

ScPageScaleToItem::~ScPageScaleToItem()
{
}

ScPageScaleToItem* ScPageScaleToItem::Clone( SfxItemPool* ) const
{
    return new ScPageScaleToItem( *this );
}

bool ScPageScaleToItem::operator==( const SfxPoolItem& rCmp ) const
{
    assert(SfxPoolItem::operator==(rCmp));
    const ScPageScaleToItem& rPageCmp = static_cast< const ScPageScaleToItem& >( rCmp );
    return (mnWidth == rPageCmp.mnWidth) && (mnHeight == rPageCmp.mnHeight);
}

namespace {
void lclAppendScalePageCount( OUString& rText, sal_uInt16 nPages )
{
    rText += ": ";
    if( nPages )
    {
        OUString aPages( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_PAGES ) );
        rText += aPages.replaceFirst( "%1", OUString::number( nPages ) );
    }
    else
        rText += ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_AUTO );
}
} // namespace

bool ScPageScaleToItem::GetPresentation(
        SfxItemPresentation ePres, SfxMapUnit, SfxMapUnit, OUString& rText, const IntlWrapper* ) const
{
    rText.clear();
    if( !IsValid())
        return false;

    OUString aName( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALETO ) );
    OUString aValue( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_WIDTH ) );
    lclAppendScalePageCount( aValue, mnWidth );
    aValue = aValue + ", " + ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_HEIGHT );
    lclAppendScalePageCount( aValue, mnHeight );

    switch( ePres )
    {
        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = aValue;
            return true;
        break;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = aName + " (" + aValue + ")";
            return true;
        break;

        default:
            OSL_FAIL( "ScPageScaleToItem::GetPresentation - unknown presentation mode" );
    }
    return false;
}

bool ScPageScaleToItem::QueryValue( uno::Any& rAny, sal_uInt8 nMemberId ) const
{
    bool bRet = true;
    switch( nMemberId )
    {
        case SC_MID_PAGE_SCALETO_WIDTH:     rAny <<= mnWidth;   break;
        case SC_MID_PAGE_SCALETO_HEIGHT:    rAny <<= mnHeight;  break;
        default:
            OSL_FAIL( "ScPageScaleToItem::QueryValue - unknown member ID" );
            bRet = false;
    }
    return bRet;
}

bool ScPageScaleToItem::PutValue( const uno::Any& rAny, sal_uInt8 nMemberId )
{
    bool bRet = false;
    switch( nMemberId )
    {
        case SC_MID_PAGE_SCALETO_WIDTH:     bRet = rAny >>= mnWidth;    break;
        case SC_MID_PAGE_SCALETO_HEIGHT:    bRet = rAny >>= mnHeight;   break;
        default:
            OSL_FAIL( "ScPageScaleToItem::PutValue - unknown member ID" );
    }
    return bRet;
}

ScCondFormatItem::ScCondFormatItem():
    SfxPoolItem( ATTR_CONDITIONAL )
{
}

ScCondFormatItem::ScCondFormatItem( const std::vector<sal_uInt32>& rIndex ):
    SfxPoolItem( ATTR_CONDITIONAL ),
    maIndex( rIndex )
{
}

ScCondFormatItem::~ScCondFormatItem()
{
}

bool ScCondFormatItem::operator==( const SfxPoolItem& rCmp ) const
{
    return maIndex == static_cast<const ScCondFormatItem&>(rCmp).maIndex;
}

ScCondFormatItem* ScCondFormatItem::Clone(SfxItemPool*) const
{
    return new ScCondFormatItem(maIndex);
}

void ScCondFormatItem::AddCondFormatData( sal_uInt32 nIndex )
{
    maIndex.push_back(nIndex);
}

void ScCondFormatItem::SetCondFormatData( const std::vector<sal_uInt32>& rIndex )
{
    maIndex = rIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
