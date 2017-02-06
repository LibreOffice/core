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

#include <utility>

#include "AccessibleCell.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include "AccessibleText.hxx"
#include "AccessibleDocument.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "miscuno.hxx"
#include "editsrc.hxx"
#include "dociter.hxx"
#include "markdata.hxx"
#include "cellvalue.hxx"
#include "formulaiter.hxx"
#include "validat.hxx"

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <editeng/brushitem.hxx>
#include <comphelper/sequence.hxx>
#include <float.h>
#include <vcl/svapp.hxx>

#include "AccessibleSpreadsheet.hxx"
#include <o3tl/make_unique.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

rtl::Reference<ScAccessibleCell> ScAccessibleCell::create(
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos,
        ScAccessibleDocument* pAccDoc)
{
    rtl::Reference<ScAccessibleCell> x(new ScAccessibleCell(
        rxParent, pViewShell, rCellAddress, nIndex, eSplitPos, pAccDoc));
    x->Init();
    return x;
}

ScAccessibleCell::ScAccessibleCell(
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos,
        ScAccessibleDocument* pAccDoc)
    :
    ScAccessibleCellBase(rxParent, GetDocument(pViewShell), rCellAddress, nIndex),
        ::accessibility::AccessibleStaticTextBase(CreateEditSource(pViewShell, rCellAddress, eSplitPos)),
    mpViewShell(pViewShell),
    mpAccDoc(pAccDoc),
    meSplitPos(eSplitPos)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);
}

ScAccessibleCell::~ScAccessibleCell()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void ScAccessibleCell::Init()
{
    ScAccessibleCellBase::Init();

    SetEventSource(this);
}

void SAL_CALL ScAccessibleCell::disposing()
{
    SolarMutexGuard aGuard;
    // dispose in AccessibleStaticTextBase
    Dispose();

    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = nullptr;
    }
    mpAccDoc = nullptr;

    ScAccessibleCellBase::disposing();
}

    //=====  XInterface  =====================================================

IMPLEMENT_FORWARD_XINTERFACE3( ScAccessibleCell, ScAccessibleCellBase, AccessibleStaticTextBase, ScAccessibleCellAttributeImpl )

    //=====  XTypeProvider  ===================================================

IMPLEMENT_FORWARD_XTYPEPROVIDER3( ScAccessibleCell, ScAccessibleCellBase, AccessibleStaticTextBase, ScAccessibleCellAttributeImpl )

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleCell::getAccessibleAtPoint(
        const awt::Point& rPoint )
{
    return AccessibleStaticTextBase::getAccessibleAtPoint(rPoint);
}

void SAL_CALL ScAccessibleCell::grabFocus(  )
{
     SolarMutexGuard aGuard;
    IsObjectValid();
    if (getAccessibleParent().is() && mpViewShell)
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
        {
            xAccessibleComponent->grabFocus();
            mpViewShell->SetCursor(maCellAddress.Col(), maCellAddress.Row());
        }
    }
}

Rectangle ScAccessibleCell::GetBoundingBoxOnScreen() const
{
    Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative(nullptr);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

Rectangle ScAccessibleCell::GetBoundingBox() const
{
    Rectangle aCellRect;
    if (mpViewShell)
    {
        long nSizeX, nSizeY;
        mpViewShell->GetViewData().GetMergeSizePixel(
            maCellAddress.Col(), maCellAddress.Row(), nSizeX, nSizeY);
        aCellRect.SetSize(Size(nSizeX, nSizeY));
        aCellRect.SetPos(mpViewShell->GetViewData().GetScrPos(maCellAddress.Col(), maCellAddress.Row(), meSplitPos, true));

        vcl::Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            Rectangle aRect(pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow()));
            aRect.Move(-aRect.Left(), -aRect.Top());
            aCellRect = aRect.Intersection(aCellRect);
        }

        /*  #i19430# Gnopernicus reads text partly if it sticks out of the cell
            boundaries. This leads to wrong results in cases where the cell
            text is rotated, because rotation is not taken into account when
            calculating the visible part of the text. In these cases we will
            simply expand the cell size to the width of the unrotated text. */
        if (mpDoc)
        {
            const SfxInt32Item* pItem = static_cast< const SfxInt32Item* >(
                mpDoc->GetAttr( maCellAddress.Col(), maCellAddress.Row(), maCellAddress.Tab(), ATTR_ROTATE_VALUE ) );
            if( pItem && (pItem->GetValue() != 0) )
            {
                Rectangle aParaRect = GetParagraphBoundingBox();
                if( !aParaRect.IsEmpty() && (aCellRect.GetWidth() < aParaRect.GetWidth()) )
                    aCellRect.SetSize( Size( aParaRect.GetWidth(), aCellRect.GetHeight() ) );
            }
        }
    }
    if (aCellRect.IsEmpty())
        aCellRect.SetPos(Point(-1, -1));
    return aCellRect;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleCell::getAccessibleChildCount()
{
    return AccessibleStaticTextBase::getAccessibleChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleCell::getAccessibleChild(sal_Int32 nIndex)
{
    return AccessibleStaticTextBase::getAccessibleChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleCell::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        if (IsFormulaMode())
        {
            pStateSet->AddState(AccessibleStateType::ENABLED);
            pStateSet->AddState(AccessibleStateType::MULTI_LINE);
            pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
            if (IsOpaque(xParentStates))
                pStateSet->AddState(AccessibleStateType::OPAQUE);
            pStateSet->AddState(AccessibleStateType::SELECTABLE);
            if (IsSelected())
                pStateSet->AddState(AccessibleStateType::SELECTED);
            if (isShowing())
                pStateSet->AddState(AccessibleStateType::SHOWING);
            pStateSet->AddState(AccessibleStateType::TRANSIENT);
            if (isVisible())
                pStateSet->AddState(AccessibleStateType::VISIBLE);
            return pStateSet;
        }
        if (IsEditable(xParentStates))
        {
            pStateSet->AddState(AccessibleStateType::EDITABLE);
            pStateSet->AddState(AccessibleStateType::RESIZABLE);
        }
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
        pStateSet->AddState(AccessibleStateType::FOCUSABLE);
        if (IsOpaque(xParentStates))
            pStateSet->AddState(AccessibleStateType::OPAQUE);
        pStateSet->AddState(AccessibleStateType::SELECTABLE);
        if (IsSelected())
            pStateSet->AddState(AccessibleStateType::SELECTED);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::TRANSIENT);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleCell::getAccessibleRelationSet()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    utl::AccessibleRelationSetHelper* pRelationSet = nullptr;
    if (mpAccDoc)
        pRelationSet = mpAccDoc->GetRelationSet(&maCellAddress);
    if (!pRelationSet)
        pRelationSet = new utl::AccessibleRelationSetHelper();
    FillDependents(pRelationSet);
    FillPrecedents(pRelationSet);
    return pRelationSet;
}

    //=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleCell::getImplementationName()
{
    return OUString("ScAccessibleCell");
}

uno::Sequence< OUString> SAL_CALL
    ScAccessibleCell::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);

    aSequence[nOldSize] = "com.sun.star.sheet.AccessibleCell";

    return aSequence;
}

    //====  internal  =========================================================

bool ScAccessibleCell::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || (mpDoc == nullptr) || (mpViewShell == nullptr) || !getAccessibleParent().is() ||
         (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

bool ScAccessibleCell::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    bool bEditable(true);
    if (rxParentStates.is() && !rxParentStates->contains(AccessibleStateType::EDITABLE) &&
        mpDoc)
    {
        // here I have to test whether the protection of the table should influence this cell.
        const ScProtectionAttr* pItem = static_cast<const ScProtectionAttr*>(mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(),
            maCellAddress.Tab(), ATTR_PROTECTION));
        if (pItem)
            bEditable = !pItem->GetProtection();
    }
    return bEditable;
}

bool ScAccessibleCell::IsOpaque(
    const uno::Reference<XAccessibleStateSet>& /* rxParentStates */)
{
    // test whether there is a background color
    bool bOpaque(true);
    if (mpDoc)
    {
        const SvxBrushItem* pItem = static_cast<const SvxBrushItem*>(mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(),
            maCellAddress.Tab(), ATTR_BACKGROUND));
        if (pItem)
            bOpaque = pItem->GetColor() != COL_TRANSPARENT;
    }
    return bOpaque;
}

bool ScAccessibleCell::IsSelected()
{
    if (IsFormulaMode())
    {
        const ScAccessibleSpreadsheet *pSheet =static_cast<const ScAccessibleSpreadsheet*>(mxParent.get());
        if (pSheet)
        {
            return pSheet->IsScAddrFormulaSel(maCellAddress);
        }
        return false;
    }

    bool bResult(false);
    if (mpViewShell)
    {
        const ScMarkData& rMarkdata = mpViewShell->GetViewData().GetMarkData();
        bResult = rMarkdata.IsCellMarked(maCellAddress.Col(), maCellAddress.Row());
    }
    return bResult;
}

ScDocument* ScAccessibleCell::GetDocument(ScTabViewShell* pViewShell)
{
    ScDocument* pDoc = nullptr;
    if (pViewShell)
        pDoc = pViewShell->GetViewData().GetDocument();
    return pDoc;
}

::std::unique_ptr< SvxEditSource > ScAccessibleCell::CreateEditSource(ScTabViewShell* pViewShell, ScAddress aCell, ScSplitPos eSplitPos)
{
    if (IsFormulaMode())
    {
        return ::std::unique_ptr< SvxEditSource >();
    }
    ::std::unique_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(o3tl::make_unique<ScAccessibleCellTextData>(pViewShell, aCell, eSplitPos, this)));

    return pEditSource;
}

void ScAccessibleCell::FillDependents(utl::AccessibleRelationSetHelper* pRelationSet)
{
    if (mpDoc)
    {
        ScRange aRange(0, 0, maCellAddress.Tab(), MAXCOL, MAXROW, maCellAddress.Tab());
        ScCellIterator aCellIter(mpDoc, aRange);

        for (bool bHasCell = aCellIter.first(); bHasCell; bHasCell = aCellIter.next())
        {
            if (aCellIter.getType() == CELLTYPE_FORMULA)
            {
                bool bFound = false;
                ScDetectiveRefIter aIter(aCellIter.getFormulaCell());
                ScRange aRef;
                while ( !bFound && aIter.GetNextRef( aRef ) )
                {
                    if (aRef.In(maCellAddress))
                        bFound = true;
                }
                if (bFound)
                    AddRelation(aCellIter.GetPos(), AccessibleRelationType::CONTROLLER_FOR, pRelationSet);
            }
        }
    }
}

void ScAccessibleCell::FillPrecedents(utl::AccessibleRelationSetHelper* pRelationSet)
{
    if (mpDoc)
    {
        ScRefCellValue aCell(*mpDoc, maCellAddress);
        if (aCell.meType == CELLTYPE_FORMULA)
        {
            ScFormulaCell* pCell = aCell.mpFormula;
            ScDetectiveRefIter aIter(pCell);
            ScRange aRef;
            while ( aIter.GetNextRef( aRef ) )
            {
                AddRelation( aRef, AccessibleRelationType::CONTROLLED_BY, pRelationSet);
            }
        }
    }
}

void ScAccessibleCell::AddRelation(const ScAddress& rCell,
    const sal_uInt16 aRelationType,
    utl::AccessibleRelationSetHelper* pRelationSet)
{
    AddRelation(ScRange(rCell, rCell), aRelationType, pRelationSet);
}

void ScAccessibleCell::AddRelation(const ScRange& rRange,
    const sal_uInt16 aRelationType,
    utl::AccessibleRelationSetHelper* pRelationSet)
{
    uno::Reference < XAccessibleTable > xTable ( getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY );
    if (xTable.is())
    {
        sal_uInt32 nCount(static_cast<sal_uInt32>(rRange.aEnd.Col() -
                    rRange.aStart.Col() + 1) * (rRange.aEnd.Row() -
                    rRange.aStart.Row() + 1));
        uno::Sequence < uno::Reference < uno::XInterface > > aTargetSet( nCount );
        uno::Reference < uno::XInterface >* pTargetSet = aTargetSet.getArray();
        if (pTargetSet)
        {
            sal_uInt32 nPos(0);
            for (sal_uInt32 nRow = rRange.aStart.Row(); nRow <= sal::static_int_cast<sal_uInt32>(rRange.aEnd.Row()); ++nRow)
            {
                for (sal_uInt32 nCol = rRange.aStart.Col(); nCol <= sal::static_int_cast<sal_uInt32>(rRange.aEnd.Col()); ++nCol)
                {
                    pTargetSet[nPos] = xTable->getAccessibleCellAt(nRow, nCol);
                    ++nPos;
                }
            }
            OSL_ENSURE(nCount == nPos, "something wents wrong");
        }
        AccessibleRelation aRelation;
        aRelation.RelationType = aRelationType;
        aRelation.TargetSet = aTargetSet;
        pRelationSet->AddRelation(aRelation);
    }
}

static OUString ReplaceOneChar(const OUString& oldOUString, const OUString& replacedChar, const OUString& replaceStr)
{
    int iReplace = oldOUString.lastIndexOf(replacedChar);
    OUString aRet = oldOUString;
    while(iReplace > -1)
    {
        aRet = aRet.replaceAt(iReplace, 1, replaceStr);
        iReplace = aRet.lastIndexOf(replacedChar, iReplace);
    }
    return aRet;
}

static OUString ReplaceFourChar(const OUString& oldOUString)
{
    OUString aRet = ReplaceOneChar(oldOUString, "\\", "\\\\");
    aRet = ReplaceOneChar(aRet, ";", "\\;");
    aRet = ReplaceOneChar(aRet, "=", "\\=");
    aRet = ReplaceOneChar(aRet, ",", "\\,");
    aRet = ReplaceOneChar(aRet, ":", "\\:");
    return aRet;
}

uno::Any SAL_CALL ScAccessibleCell::getExtendedAttributes()
{
    SolarMutexGuard aGuard;

    uno::Any strRet;
    if (mpViewShell)
    {
        OUString strFor = mpViewShell->GetFormula(maCellAddress) ;
        strFor = strFor.replaceAt(0,1,"");
        strFor = ReplaceFourChar(strFor);
        strFor = "Formula:" + strFor;
        strFor += ";Note:";
        strFor += ReplaceFourChar(GetAllDisplayNote());
        strFor += ";";
        strFor += getShadowAttrs();//the string returned contains the spliter ";"
        strFor += getBorderAttrs();//the string returned contains the spliter ";"
        //end of cell attributes
        if( mpDoc )
        {
            strFor += "isdropdown:";
            if( IsDropdown() )
                strFor += "true";
            else
                strFor += "false";
            strFor += ";";
        }
        strRet <<= strFor ;
    }
    return strRet;
}

// cell has its own ParaIndent property, so when calling character attributes on cell, the ParaIndent should replace the ParaLeftMargin if its value is not zero.
uno::Sequence< beans::PropertyValue > SAL_CALL ScAccessibleCell::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes )
{
    SolarMutexGuard aGuard;

    uno::Sequence< beans::PropertyValue > aAttribs = AccessibleStaticTextBase::getCharacterAttributes( nIndex, aRequestedAttributes );
    beans::PropertyValue *pAttribs = aAttribs.getArray();

    sal_uInt16 nParaIndent = static_cast< const SfxUInt16Item* >( mpDoc->GetAttr( maCellAddress.Col(), maCellAddress.Row(), maCellAddress.Tab(), ATTR_INDENT ) )->GetValue();
    if (nParaIndent > 0)
    {
        OUString sLeftMarginName ("ParaLeftMargin");
        for (int i = 0; i < aAttribs.getLength(); ++i)
        {
            if (sLeftMarginName == pAttribs[i].Name)
            {
                pAttribs[i].Value = uno::makeAny( nParaIndent );
                break;
            }
        }
    }
    return aAttribs;
}

bool ScAccessibleCell::IsFormulaMode()
{
    ScAccessibleSpreadsheet* pSheet = static_cast<ScAccessibleSpreadsheet*>(mxParent.get());
    if (pSheet)
    {
        return pSheet->IsFormulaMode();
    }
    return false;
}

bool ScAccessibleCell::IsDropdown()
{
    sal_uInt16 nPosX = maCellAddress.Col();
    sal_uInt16 nPosY = sal_uInt16(maCellAddress.Row());
    sal_uInt16 nTab = maCellAddress.Tab();
    sal_uInt32 nValidation = static_cast< const SfxUInt32Item* >( mpDoc->GetAttr( nPosX, nPosY, nTab, ATTR_VALIDDATA ) )->GetValue();
    if( nValidation )
    {
        const ScValidationData* pData = mpDoc->GetValidationEntry( nValidation );
        if( pData && pData->HasSelectionList() )
            return true;
    }
    const ScMergeFlagAttr* pAttr;
    pAttr = static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr( nPosX, nPosY, nTab, ATTR_MERGE_FLAG ));
    if( pAttr->HasAutoFilter() )
    {
        return true;
    }
    else
    {
        sal_uInt16 nTabCount = mpDoc->GetTableCount();
        if ( nTab+1<nTabCount && mpDoc->IsScenario(nTab+1) && !mpDoc->IsScenario(nTab) )
        {
            SCTAB i;
            ScMarkData aMarks;
            for (i=nTab+1; i<nTabCount && mpDoc->IsScenario(i); i++)
                mpDoc->MarkScenario( i, nTab, aMarks, false, ScScenarioFlags::ShowFrame );
            ScRangeList aRanges;
            aMarks.FillRangeListWithMarks( &aRanges, false );
            bool bHasScenario;
            SCTAB nRangeCount = aRanges.size();
            for (i=0; i<nRangeCount; i++)
            {
                ScRange aRange = *aRanges[i];
                mpDoc->ExtendTotalMerge( aRange );
                bool bTextBelow = ( aRange.aStart.Row() == 0 );
                // MT IA2: Not used: sal_Bool bIsInScen = sal_False;
                if ( bTextBelow )
                {
                    bHasScenario = (aRange.aStart.Col() == nPosX && aRange.aEnd.Row() == nPosY-1);
                }
                else
                {
                    bHasScenario = (aRange.aStart.Col() == nPosX && aRange.aStart.Row() == nPosY+1);
                }
                if( bHasScenario ) return true;
            }
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
