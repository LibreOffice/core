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

#include <AccessibleCellBase.hxx>
#include <document.hxx>
#include <docfunc.hxx>
#include <docsh.hxx>
#include <strings.hxx>
#include <unonames.hxx>
#include <detfunc.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <comphelper/sequence.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include <float.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

#define DEFAULT_LINE_WIDTH 2

//=====  internal  ============================================================

ScAccessibleCellBase::ScAccessibleCellBase(
        const uno::Reference<XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScAddress& rCellAddress,
        sal_Int32 nIndex)
    :
    ScAccessibleContextBase(rxParent, AccessibleRole::TABLE_CELL),
    maCellAddress(rCellAddress),
    mpDoc(pDoc),
    mnIndex(nIndex)
{
}

ScAccessibleCellBase::~ScAccessibleCellBase()
{
}

    //=====  XAccessibleComponent  ============================================

bool ScAccessibleCellBase::isVisible()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    // test whether the cell is hidden (column/row - hidden/filtered)
    bool bVisible(true);
    if (mpDoc)
    {
        bool bColHidden = mpDoc->ColHidden(maCellAddress.Col(), maCellAddress.Tab());
        bool bRowHidden = mpDoc->RowHidden(maCellAddress.Row(), maCellAddress.Tab());
        bool bColFiltered = mpDoc->ColFiltered(maCellAddress.Col(), maCellAddress.Tab());
        bool bRowFiltered = mpDoc->RowFiltered(maCellAddress.Row(), maCellAddress.Tab());

        if (bColHidden || bColFiltered || bRowHidden || bRowFiltered)
            bVisible = false;
    }
    return bVisible;
}

sal_Int32 SAL_CALL ScAccessibleCellBase::getForeground()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);
    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(SC_UNONAME_CCOLOR);
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }
    return nColor;
}

sal_Int32 SAL_CALL ScAccessibleCellBase::getBackground()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    sal_Int32 nColor(0);

    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(SC_UNONAME_CELLBACK);
                                aAny >>= nColor;
                            }
                        }
                    }
                }
            }
        }
    }

    return nColor;
}

    //=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleCellBase::queryInterface( uno::Type const & rType )
{
    uno::Any aAny (ScAccessibleCellBaseImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessibleCellBase::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleCellBase::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

    //=====  XAccessibleContext  ==============================================

sal_Int32
    ScAccessibleCellBase::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return mnIndex;
}

OUString
    ScAccessibleCellBase::createAccessibleDescription()
{
    OUString sDescription = STR_ACC_CELL_DESCR;

    return sDescription;
}

OUString
    ScAccessibleCellBase::createAccessibleName()
{
    // Document not needed, because only the cell address, but not the tablename is needed
    // always us OOO notation
    return maCellAddress.Format(ScRefFlags::VALID);
}

    //=====  XAccessibleValue  ================================================

uno::Any SAL_CALL
    ScAccessibleCellBase::getCurrentValue()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    uno::Any aAny;
    if (mpDoc)
    {
        aAny <<= mpDoc->GetValue(maCellAddress);
    }
    return aAny;
}

sal_Bool SAL_CALL
    ScAccessibleCellBase::setCurrentValue( const uno::Any& aNumber )
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    double fValue = 0;
    bool bResult = false;
    if((aNumber >>= fValue) && mpDoc && mpDoc->GetDocumentShell())
    {
        uno::Reference<XAccessibleStateSet> xParentStates;
        if (getAccessibleParent().is())
        {
            uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
            xParentStates = xParentContext->getAccessibleStateSet();
        }
        if (IsEditable(xParentStates))
        {
            ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDoc->GetDocumentShell());
            bResult = pDocShell->GetDocFunc().SetValueCell(maCellAddress, fValue, false);
        }
    }
    return bResult;
}

uno::Any SAL_CALL
    ScAccessibleCellBase::getMaximumValue(  )
{
    return uno::Any(DBL_MAX);
}

uno::Any SAL_CALL
    ScAccessibleCellBase::getMinimumValue(  )
{
    return uno::Any(-DBL_MAX);
}

    //=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleCellBase::getImplementationName()
{
    return OUString("ScAccessibleCellBase");
}

    //=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleCellBase::getTypes()
{
    return comphelper::concatSequences(ScAccessibleCellBaseImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleCellBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

bool ScAccessibleCellBase::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    bool bEditable(false);
    if (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::EDITABLE))
        bEditable = true;
    return bEditable;
}

OUString ScAccessibleCellBase::GetNote()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    OUString sNote;
    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference <sheet::XSheetAnnotationAnchor> xAnnotationAnchor ( xCell, uno::UNO_QUERY);
                            if(xAnnotationAnchor.is())
                            {
                                uno::Reference <sheet::XSheetAnnotation> xSheetAnnotation = xAnnotationAnchor->getAnnotation();
                                if (xSheetAnnotation.is())
                                {
                                    uno::Reference <text::XSimpleText> xText (xSheetAnnotation, uno::UNO_QUERY);
                                    if (xText.is())
                                    {
                                        sNote = xText->getString();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return sNote;
}

OUString ScAccessibleCellBase::getShadowAttrs()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    table::ShadowFormat aShadowFmt;
    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(SC_UNONAME_SHADOW);
                                aAny >>= aShadowFmt;
                            }
                        }
                    }
                }
            }
        }
    }
    //construct shadow attributes string
    OUString sShadowAttrs("Shadow:");
    OUString sInnerSplit(",");
    OUString sOuterSplit(";");
    sal_Int32 nLocationVal = 0;
    switch( aShadowFmt.Location )
    {
    case table::ShadowLocation_TOP_LEFT:
        nLocationVal = 1;
        break;
    case table::ShadowLocation_TOP_RIGHT:
        nLocationVal = 2;
        break;
    case table::ShadowLocation_BOTTOM_LEFT:
        nLocationVal = 3;
        break;
    case table::ShadowLocation_BOTTOM_RIGHT:
        nLocationVal = 4;
        break;
    default:
        break;
    }
    //if there is no shadow property for the cell
    if ( nLocationVal == 0 )
    {
        sShadowAttrs += sOuterSplit;
        return sShadowAttrs;
    }
    //else return all the shadow properties
    sShadowAttrs += "Location=";
    sShadowAttrs += OUString::number( nLocationVal );
    sShadowAttrs += sInnerSplit;
    sShadowAttrs += "ShadowWidth=";
    sShadowAttrs += OUString::number( static_cast<sal_Int32>(aShadowFmt.ShadowWidth) ) ;
    sShadowAttrs += sInnerSplit;
    sShadowAttrs += "IsTransparent=";
    sShadowAttrs += OUString::number( static_cast<int>(aShadowFmt.IsTransparent) ) ;
    sShadowAttrs += sInnerSplit;
    sShadowAttrs += "Color=";
    sShadowAttrs += OUString::number( static_cast<sal_Int32>(aShadowFmt.Color) );
    sShadowAttrs += sOuterSplit;
    return sShadowAttrs;
}

OUString ScAccessibleCellBase::getBorderAttrs()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    table::BorderLine aTopBorder;
    table::BorderLine aBottomBorder;
    table::BorderLine aLeftBorder;
    table::BorderLine aRightBorder;
    if (mpDoc)
    {
        SfxObjectShell* pObjSh = mpDoc->GetDocumentShell();
        if ( pObjSh )
        {
            uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( pObjSh->GetModel(), uno::UNO_QUERY );
            if ( xSpreadDoc.is() )
            {
                uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
                uno::Reference<container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
                if ( xIndex.is() )
                {
                    uno::Any aTable = xIndex->getByIndex(maCellAddress.Tab());
                    uno::Reference<sheet::XSpreadsheet> xTable;
                    if (aTable>>=xTable)
                    {
                        uno::Reference<table::XCell> xCell = xTable->getCellByPosition(maCellAddress.Col(), maCellAddress.Row());
                        if (xCell.is())
                        {
                            uno::Reference<beans::XPropertySet> xCellProps(xCell, uno::UNO_QUERY);
                            if (xCellProps.is())
                            {
                                uno::Any aAny = xCellProps->getPropertyValue(SC_UNONAME_TOPBORDER);
                                aAny >>= aTopBorder;
                                aAny = xCellProps->getPropertyValue(SC_UNONAME_BOTTBORDER);
                                aAny >>= aBottomBorder;
                                aAny = xCellProps->getPropertyValue(SC_UNONAME_LEFTBORDER);
                                aAny >>= aLeftBorder;
                                aAny = xCellProps->getPropertyValue(SC_UNONAME_RIGHTBORDER);
                                aAny >>= aRightBorder;
                            }
                        }
                    }
                }
            }
        }
    }

    Color aColor;
    bool bIn = mpDoc && mpDoc->IsCellInChangeTrack(maCellAddress,&aColor);
    if (bIn)
    {
        aTopBorder.Color = sal_Int32(aColor);
        aBottomBorder.Color = sal_Int32(aColor);
        aLeftBorder.Color = sal_Int32(aColor);
        aRightBorder.Color = sal_Int32(aColor);
        aTopBorder.OuterLineWidth = DEFAULT_LINE_WIDTH;
        aBottomBorder.OuterLineWidth = DEFAULT_LINE_WIDTH;
        aLeftBorder.OuterLineWidth = DEFAULT_LINE_WIDTH;
        aRightBorder.OuterLineWidth = DEFAULT_LINE_WIDTH;
    }

    //construct border attributes string
    OUString sBorderAttrs;
    OUString sInnerSplit(",");
    OUString sOuterSplit(";");
    //top border
    //if top of the cell has no border
    if ( aTopBorder.InnerLineWidth == 0 && aTopBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "TopBorder:;";
    }
    else//add all the border properties to the return string.
    {
        sBorderAttrs += "TopBorder:Color=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aTopBorder.Color) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aTopBorder.InnerLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aTopBorder.OuterLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aTopBorder.LineDistance) );
        sBorderAttrs += sOuterSplit;
    }
    //bottom border
    if ( aBottomBorder.InnerLineWidth == 0 && aBottomBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "BottomBorder:;";
    }
    else
    {
        sBorderAttrs += "BottomBorder:Color=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aBottomBorder.Color) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aBottomBorder.InnerLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aBottomBorder.OuterLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aBottomBorder.LineDistance) );
        sBorderAttrs += sOuterSplit;
    }
    //left border
    if ( aLeftBorder.InnerLineWidth == 0 && aLeftBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "LeftBorder:;";
    }
    else
    {
        sBorderAttrs += "LeftBorder:Color=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aLeftBorder.Color) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aLeftBorder.InnerLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aLeftBorder.OuterLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aLeftBorder.LineDistance) );
        sBorderAttrs += sOuterSplit;
    }
    //right border
    if ( aRightBorder.InnerLineWidth == 0 && aRightBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "RightBorder:;";
    }
    else
    {
        sBorderAttrs += "RightBorder:Color=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aRightBorder.Color) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aRightBorder.InnerLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aRightBorder.OuterLineWidth) );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( static_cast<sal_Int32>(aRightBorder.LineDistance) );
        sBorderAttrs += sOuterSplit;
    }
    return sBorderAttrs;
}
//end of cell attributes

OUString ScAccessibleCellBase::GetAllDisplayNote()
{
    OUString strNote;
    OUString strTrackText;
    if (mpDoc)
    {
        bool bLeftedge = false;
        mpDoc->GetCellChangeTrackNote(maCellAddress,strTrackText,bLeftedge);
    }
    if (!strTrackText.isEmpty())
    {
        ScDetectiveFunc::AppendChangTrackNoteSeparator(strTrackText);
        strNote = strTrackText;
    }
    strNote += GetNote();
    return strNote;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
