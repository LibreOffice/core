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

#include "textuno.hxx"  // ScHeaderFooterContentObj

using namespace com::sun::star;

//------------------------------------------------------------------------

TYPEINIT1(ScMergeAttr,          SfxPoolItem);
TYPEINIT1_AUTOFACTORY(ScProtectionAttr,     SfxPoolItem);
TYPEINIT1(ScRangeItem,          SfxPoolItem);
TYPEINIT1(ScTableListItem,      SfxPoolItem);
TYPEINIT1(ScPageHFItem,         SfxPoolItem);
TYPEINIT1(ScViewObjectModeItem, SfxEnumItem);
TYPEINIT1(ScDoubleItem,         SfxPoolItem);
TYPEINIT1(ScPageScaleToItem,    SfxPoolItem);
TYPEINIT1(ScCondFormatItem,    SfxPoolItem);

//------------------------------------------------------------------------

//
//      General Help Function
//

bool ScHasPriority( const ::editeng::SvxBorderLine* pThis, const ::editeng::SvxBorderLine* pOther )
{

    if (!pThis)
        return false;
    if (!pOther)
        return true;

    sal_uInt16 nThisSize = pThis->GetOutWidth() + pThis->GetDistance() + pThis->GetInWidth();
    sal_uInt16 nOtherSize = pOther->GetOutWidth() + pOther->GetDistance() + pOther->GetInWidth();

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
            return true;            //! ???
        }
    }
}


//
//      Item - Implementierungen
//

//------------------------------------------------------------------------
// Merge
//------------------------------------------------------------------------

ScMergeAttr::ScMergeAttr():
    SfxPoolItem(ATTR_MERGE),
    nColMerge(0),
    nRowMerge(0)
{}

//------------------------------------------------------------------------

ScMergeAttr::ScMergeAttr( SCsCOL nCol, SCsROW nRow):
    SfxPoolItem(ATTR_MERGE),
    nColMerge(nCol),
    nRowMerge(nRow)
{}

//------------------------------------------------------------------------

ScMergeAttr::ScMergeAttr(const ScMergeAttr& rItem):
    SfxPoolItem(ATTR_MERGE)
{
    nColMerge = rItem.nColMerge;
    nRowMerge = rItem.nRowMerge;
}

ScMergeAttr::~ScMergeAttr()
{
}

//------------------------------------------------------------------------

OUString ScMergeAttr::GetValueText() const
{
    OUString aRet = "("
        + OUString::number(static_cast<sal_Int32>(nColMerge))
        + ","
        + OUString::number(static_cast<sal_Int32>(nRowMerge))
        + ")";
    return aRet;
}

//------------------------------------------------------------------------

int ScMergeAttr::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( Which() != rItem.Which() || Type() == rItem.Type(), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (nColMerge == ((ScMergeAttr&)rItem).nColMerge)
             && (nRowMerge == ((ScMergeAttr&)rItem).nRowMerge);
}

//------------------------------------------------------------------------

SfxPoolItem* ScMergeAttr::Clone( SfxItemPool * ) const
{
    return new ScMergeAttr(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* ScMergeAttr::Create( SvStream& rStream, sal_uInt16 /* nVer */ ) const
{
    sal_Int16   nCol;
    sal_Int16   nRow;
    rStream >> nCol;
    rStream >> nRow;
    return new ScMergeAttr(static_cast<SCCOL>(nCol),static_cast<SCROW>(nRow));
}

//------------------------------------------------------------------------
// MergeFlag
//------------------------------------------------------------------------

ScMergeFlagAttr::ScMergeFlagAttr():
    SfxInt16Item(ATTR_MERGE_FLAG, 0)
{
}

//------------------------------------------------------------------------

ScMergeFlagAttr::ScMergeFlagAttr(sal_Int16 nFlags):
    SfxInt16Item(ATTR_MERGE_FLAG, nFlags)
{
}

ScMergeFlagAttr::~ScMergeFlagAttr()
{
}

bool ScMergeFlagAttr::HasPivotButton() const
{
    return (GetValue() & SC_MF_BUTTON) != 0;
}

bool ScMergeFlagAttr::HasPivotPopupButton() const
{
    return (GetValue() & SC_MF_BUTTON_POPUP) != 0;
}

//------------------------------------------------------------------------
// Protection
//------------------------------------------------------------------------

ScProtectionAttr::ScProtectionAttr():
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(true),
    bHideFormula(false),
    bHideCell(false),
    bHidePrint(false)
{
}

//------------------------------------------------------------------------

ScProtectionAttr::ScProtectionAttr( bool bProtect, bool bHFormula,
                                    bool bHCell, bool bHPrint):
    SfxPoolItem(ATTR_PROTECTION),
    bProtection(bProtect),
    bHideFormula(bHFormula),
    bHideCell(bHCell),
    bHidePrint(bHPrint)
{
}

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

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
            rVal <<= (sal_Bool ) bProtection; break;
        case MID_2 :
            rVal <<= (sal_Bool ) bHideFormula; break;
        case MID_3 :
            rVal <<= (sal_Bool ) bHideCell; break;
        case MID_4 :
            rVal <<= (sal_Bool ) bHidePrint; break;
        default:
            OSL_FAIL("Wrong MemberID!");
            return false;
    }

    return true;
}

bool ScProtectionAttr::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
{
    bool bRet = false;
    sal_Bool bVal = sal_False;
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

//------------------------------------------------------------------------

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

//------------------------------------------------------------------------

SfxItemPresentation ScProtectionAttr::GetPresentation
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
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            break;

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

        default:
            ePres = SFX_ITEM_PRESENTATION_NONE;
    }

    return ePres;
}

//------------------------------------------------------------------------

int ScProtectionAttr::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( Which() != rItem.Which() || Type() == rItem.Type(), "which ==, type !=" );
    return (Which() == rItem.Which())
             && (bProtection == ((ScProtectionAttr&)rItem).bProtection)
             && (bHideFormula == ((ScProtectionAttr&)rItem).bHideFormula)
             && (bHideCell == ((ScProtectionAttr&)rItem).bHideCell)
             && (bHidePrint == ((ScProtectionAttr&)rItem).bHidePrint);
}

//------------------------------------------------------------------------

SfxPoolItem* ScProtectionAttr::Clone( SfxItemPool * ) const
{
    return new ScProtectionAttr(*this);
}

//------------------------------------------------------------------------

SfxPoolItem* ScProtectionAttr::Create( SvStream& rStream, sal_uInt16 /* n */ ) const
{
    sal_Bool bProtect;
    sal_Bool bHFormula;
    sal_Bool bHCell;
    sal_Bool bHPrint;

    rStream >> bProtect;
    rStream >> bHFormula;
    rStream >> bHCell;
    rStream >> bHPrint;

    return new ScProtectionAttr(bProtect,bHFormula,bHCell,bHPrint);
}

//------------------------------------------------------------------------

bool ScProtectionAttr::SetProtection( bool bProtect)
{
    bProtection =  bProtect;
    return true;
}

//------------------------------------------------------------------------

bool ScProtectionAttr::SetHideFormula( bool bHFormula)
{
    bHideFormula = bHFormula;
    return true;
}

//------------------------------------------------------------------------

bool ScProtectionAttr::SetHideCell( bool bHCell)
{
    bHideCell = bHCell;
    return true;
}

//------------------------------------------------------------------------

bool ScProtectionAttr::SetHidePrint( bool bHPrint)
{
    bHidePrint = bHPrint;
    return true;
}

// -----------------------------------------------------------------------
//      ScRangeItem - Tabellenbereich
// -----------------------------------------------------------------------

int ScRangeItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "unequal types" );

    return ( aRange == ( (ScRangeItem&)rAttr ).aRange );
}

// -----------------------------------------------------------------------

SfxPoolItem* ScRangeItem::Clone( SfxItemPool* ) const
{
    return new ScRangeItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation ScRangeItem::GetPresentation
    (
        SfxItemPresentation ePres,
        SfxMapUnit          /* eCoreUnit */,
        SfxMapUnit          /* ePresUnit */,
        OUString&           rText,
        const IntlWrapper*  /* pIntl */
    ) const
{
    rText = OUString();

    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_COMPLETE:
        rText = ScGlobal::GetRscString(STR_AREA) + ": ";
        /* !!! fall-through !!! */

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

    return ePres;
}

// -----------------------------------------------------------------------
//      ScTableListItem - List from Tables (-numbers)
// -----------------------------------------------------------------------

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
        pTabArr = NULL;
}

// -----------------------------------------------------------------------


ScTableListItem::~ScTableListItem()
{
    delete [] pTabArr;
}

// -----------------------------------------------------------------------

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
        pTabArr = NULL;

    nCount = rCpy.nCount;

    return *this;
}

// -----------------------------------------------------------------------

int ScTableListItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "unequal types" );

    ScTableListItem&    rCmp   = (ScTableListItem&)rAttr;
    bool                bEqual = (nCount == rCmp.nCount);

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

// -----------------------------------------------------------------------

SfxPoolItem* ScTableListItem::Clone( SfxItemPool* ) const
{
    return new ScTableListItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation ScTableListItem::GetPresentation
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
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
            return ePres;

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
            return ePres;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = OUString();
            return SFX_ITEM_PRESENTATION_NONE;

        default:
        {
            // added to avoid warnings
        }
    }

    return SFX_ITEM_PRESENTATION_NONE;
}


// -----------------------------------------------------------------------
//      ScPageHFItem - Dates from the Head and Foot lines
// -----------------------------------------------------------------------

ScPageHFItem::ScPageHFItem( sal_uInt16 nWhichP )
    :   SfxPoolItem ( nWhichP ),
        pLeftArea   ( NULL ),
        pCenterArea ( NULL ),
        pRightArea  ( NULL )
{
}

//------------------------------------------------------------------------

ScPageHFItem::ScPageHFItem( const ScPageHFItem& rItem )
    :   SfxPoolItem ( rItem ),
        pLeftArea   ( NULL ),
        pCenterArea ( NULL ),
        pRightArea  ( NULL )
{
    if ( rItem.pLeftArea )
        pLeftArea = rItem.pLeftArea->Clone();
    if ( rItem.pCenterArea )
        pCenterArea = rItem.pCenterArea->Clone();
    if ( rItem.pRightArea )
        pRightArea = rItem.pRightArea->Clone();
}

//------------------------------------------------------------------------

ScPageHFItem::~ScPageHFItem()
{
    delete pLeftArea;
    delete pCenterArea;
    delete pRightArea;
}

//------------------------------------------------------------------------

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
            ScHeaderFooterContentObj* pImp =
                    ScHeaderFooterContentObj::getImplementation( xContent );
            if (pImp)
            {
                const EditTextObject* pImpLeft = pImp->GetLeftEditObject();
                delete pLeftArea;
                pLeftArea = pImpLeft ? pImpLeft->Clone() : NULL;

                const EditTextObject* pImpCenter = pImp->GetCenterEditObject();
                delete pCenterArea;
                pCenterArea = pImpCenter ? pImpCenter->Clone() : NULL;

                const EditTextObject* pImpRight = pImp->GetRightEditObject();
                delete pRightArea;
                pRightArea = pImpRight ? pImpRight->Clone() : NULL;

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

//------------------------------------------------------------------------

OUString ScPageHFItem::GetValueText() const
{
    return OUString("ScPageHFItem");
}

//------------------------------------------------------------------------

int ScPageHFItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );

    const ScPageHFItem& r = (const ScPageHFItem&)rItem;

    return    ScGlobal::EETextObjEqual(pLeftArea,   r.pLeftArea)
           && ScGlobal::EETextObjEqual(pCenterArea, r.pCenterArea)
           && ScGlobal::EETextObjEqual(pRightArea,  r.pRightArea);
}

//------------------------------------------------------------------------

SfxPoolItem* ScPageHFItem::Clone( SfxItemPool* ) const
{
    return new ScPageHFItem( *this );
}

//------------------------------------------------------------------------

static void lcl_SetSpace( String& rStr, const ESelection& rSel )
{
    // Text replaced by a space to ensure they are positions:

    xub_StrLen nLen = rSel.nEndPos-rSel.nStartPos;
    rStr.Erase( rSel.nStartPos, nLen-1 );
    rStr.SetChar( rSel.nStartPos, ' ' );
}

static bool lcl_ConvertFields(EditEngine& rEng, const String* pCommands)
{
    bool bChange = false;
    sal_Int32 nParCnt = rEng.GetParagraphCount();
    for (sal_Int32 nPar = 0; nPar<nParCnt; nPar++)
    {
        String aStr = rEng.GetText( nPar );
        xub_StrLen nPos;

        while ((nPos = aStr.Search(pCommands[0])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[0].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxPageField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.Search(pCommands[1])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[1].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxPagesField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.Search(pCommands[2])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[2].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxDateField(Date( Date::SYSTEM ),SVXDATETYPE_VAR), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.Search(pCommands[3])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[3].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxTimeField(), EE_FEATURE_FIELD ), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.Search(pCommands[4])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[4].Len() );
            rEng.QuickInsertField( SvxFieldItem(SvxFileField(), EE_FEATURE_FIELD), aSel );
            lcl_SetSpace(aStr, aSel ); bChange = true;
        }
        while ((nPos = aStr.Search(pCommands[5])) != STRING_NOTFOUND)
        {
            ESelection aSel( nPar,nPos, nPar,nPos+pCommands[5].Len() );
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

    if ( pLeft == NULL   || pLeft->GetParagraphCount() == 0 ||
         pCenter == NULL || pCenter->GetParagraphCount() == 0 ||
         pRight == NULL  || pRight->GetParagraphCount() == 0 )
    {
        //  If successfully loaded, each object contains at least one paragraph.
        //  Excel import in 5.1 created broken TextObjects (#67442#) that are
        //  corrected here to avoid saving wrong files again (#90487#).

        ScEditEngineDefaulter aEngine( EditEngine::CreatePool(), true );
        if ( pLeft == NULL || pLeft->GetParagraphCount() == 0 )
        {
            delete pLeft;
            pLeft = aEngine.CreateTextObject();
        }
        if ( pCenter == NULL || pCenter->GetParagraphCount() == 0 )
        {
            delete pCenter;
            pCenter = aEngine.CreateTextObject();
        }
        if ( pRight == NULL || pRight->GetParagraphCount() == 0 )
        {
            delete pRight;
            pRight = aEngine.CreateTextObject();
        }
    }

    if ( nVer < 1 )             //old field command conversions
    {
        sal_uInt16 i;
        const String& rDel = ScGlobal::GetRscString( STR_HFCMD_DELIMITER );
        String aCommands[SC_FIELD_COUNT];
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
    else if ( nVer < 2 )
    {   // not to do, SvxFileField is not exchanged for SvxExtFileField
    }

    ScPageHFItem* pItem = new ScPageHFItem( Which() );
    pItem->SetArea( pLeft,    SC_HF_LEFTAREA   );
    pItem->SetArea( pCenter, SC_HF_CENTERAREA );
    pItem->SetArea( pRight,  SC_HF_RIGHTAREA  );

    return pItem;
}

//------------------------------------------------------------------------


void ScPageHFItem::SetLeftArea( const EditTextObject& rNew )
{
    delete pLeftArea;
    pLeftArea = rNew.Clone();
}

//------------------------------------------------------------------------

void ScPageHFItem::SetCenterArea( const EditTextObject& rNew )
{
    delete pCenterArea;
    pCenterArea = rNew.Clone();
}

//------------------------------------------------------------------------

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

//-----------------------------------------------------------------------
//  ScViewObjectModeItem - Display Mode of View Objects
//-----------------------------------------------------------------------

ScViewObjectModeItem::ScViewObjectModeItem( sal_uInt16 nWhichP )
    : SfxEnumItem( nWhichP, VOBJ_MODE_SHOW )
{
}

//------------------------------------------------------------------------

ScViewObjectModeItem::ScViewObjectModeItem( sal_uInt16 nWhichP, ScVObjMode eMode )
    : SfxEnumItem( nWhichP, sal::static_int_cast<sal_uInt16>(eMode) )
{
}

//------------------------------------------------------------------------

ScViewObjectModeItem::~ScViewObjectModeItem()
{
}

//------------------------------------------------------------------------

SfxItemPresentation ScViewObjectModeItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /* eCoreUnit */,
    SfxMapUnit          /* ePresUnit */,
    OUString&           rText,
    const IntlWrapper* /* pIntl */
)   const
{
    OUString aDel(": ");
    rText = OUString();

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
            ePres = SFX_ITEM_PRESENTATION_NAMELESS;//this always goes!
            break;
        }
        /* !!! fall-through !!! */

        case SFX_ITEM_PRESENTATION_NAMELESS:
        rText += ScGlobal::GetRscString(STR_VOBJ_MODE_SHOW+GetValue());
        break;

        default:
        {
            // added to avoid warnings
        }
    }

    return ePres;
}

//------------------------------------------------------------------------

OUString ScViewObjectModeItem::GetValueText( sal_uInt16 nVal ) const
{
    OSL_ENSURE( nVal <= VOBJ_MODE_HIDE, "enum overflow!" );

    return ScGlobal::GetRscString( STR_VOBJ_MODE_SHOW + (nVal % 2));
}

//------------------------------------------------------------------------

sal_uInt16 ScViewObjectModeItem::GetValueCount() const
{
    return 2;
}

//------------------------------------------------------------------------

SfxPoolItem* ScViewObjectModeItem::Clone( SfxItemPool* ) const
{
    return new ScViewObjectModeItem( *this );
}

//------------------------------------------------------------------------

sal_uInt16 ScViewObjectModeItem::GetVersion( sal_uInt16 /* nFileVersion */ ) const
{
    return 1;
}

//------------------------------------------------------------------------

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
        rStream >> nVal;

        //#i80528# adapt to new range eventually
        if((sal_uInt16)VOBJ_MODE_HIDE < nVal) nVal = (sal_uInt16)VOBJ_MODE_SHOW;

        return new ScViewObjectModeItem( Which(), (ScVObjMode)nVal);
    }
}

// -----------------------------------------------------------------------
//      double
// -----------------------------------------------------------------------

ScDoubleItem::ScDoubleItem( sal_uInt16 nWhichP, double nVal )
    :   SfxPoolItem ( nWhichP ),
        nValue  ( nVal )
{
}

//------------------------------------------------------------------------

ScDoubleItem::ScDoubleItem( const ScDoubleItem& rItem )
    :   SfxPoolItem ( rItem )
{
        nValue = rItem.nValue;
}

//------------------------------------------------------------------------

OUString ScDoubleItem::GetValueText() const
{
    return OUString("ScDoubleItem");
}

//------------------------------------------------------------------------

int ScDoubleItem::operator==( const SfxPoolItem& rItem ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rItem ), "unequal Which or Type" );
    const ScDoubleItem& _rItem = (const ScDoubleItem&)rItem;
    return int(nValue == _rItem.nValue);
}

//------------------------------------------------------------------------

SfxPoolItem* ScDoubleItem::Clone( SfxItemPool* ) const
{
    return new ScDoubleItem( *this );
}

//------------------------------------------------------------------------

SfxPoolItem* ScDoubleItem::Create( SvStream& rStream, sal_uInt16 /* nVer */ ) const
{
    double nTmp=0;
    rStream >> nTmp;

    ScDoubleItem* pItem = new ScDoubleItem( Which(), nTmp );

    return pItem;
}

//------------------------------------------------------------------------

ScDoubleItem::~ScDoubleItem()
{
}


// ============================================================================

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

int ScPageScaleToItem::operator==( const SfxPoolItem& rCmp ) const
{
    OSL_ENSURE( SfxPoolItem::operator==( rCmp ), "ScPageScaleToItem::operator== - unequal wid or type" );
    const ScPageScaleToItem& rPageCmp = static_cast< const ScPageScaleToItem& >( rCmp );
    return ((mnWidth == rPageCmp.mnWidth) && (mnHeight == rPageCmp.mnHeight)) ? 1 : 0;
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

SfxItemPresentation ScPageScaleToItem::GetPresentation(
        SfxItemPresentation ePres, SfxMapUnit, SfxMapUnit, OUString& rText, const IntlWrapper* ) const
{
    rText = OUString();
    if( !IsValid() || (ePres == SFX_ITEM_PRESENTATION_NONE) )
        return SFX_ITEM_PRESENTATION_NONE;

    OUString aName( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALETO ) );
    OUString aValue( ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_WIDTH ) );
    lclAppendScalePageCount( aValue, mnWidth );
    aValue = aValue + ", " + ScGlobal::GetRscString( STR_SCATTR_PAGE_SCALE_HEIGHT );
    lclAppendScalePageCount( aValue, mnHeight );

    switch( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
        break;

        case SFX_ITEM_PRESENTATION_NAMEONLY:
            rText = aName;
        break;

        case SFX_ITEM_PRESENTATION_NAMELESS:
            rText = aValue;
        break;

        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = aName + " (" + aValue + ")";
        break;

        default:
            OSL_FAIL( "ScPageScaleToItem::GetPresentation - unknown presentation mode" );
            ePres = SFX_ITEM_PRESENTATION_NONE;
    }
    return ePres;
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

// ============================================================================

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

int ScCondFormatItem::operator==( const SfxPoolItem& rCmp ) const
{
    return maIndex == static_cast<const ScCondFormatItem&>(rCmp).maIndex;
}

ScCondFormatItem* ScCondFormatItem::Clone(SfxItemPool*) const
{
    return new ScCondFormatItem(maIndex);
}

const std::vector<sal_uInt32>& ScCondFormatItem::GetCondFormatData() const
{
    return maIndex;
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
