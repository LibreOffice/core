/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "AccessibleCellBase.hxx"
#include "attrib.hxx"
#include "scitems.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "docfunc.hxx"
#include "docsh.hxx"
#include "formulacell.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "unonames.hxx"
#include "detfunc.hxx"
#include "chgtrack.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <editeng/brushitem.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include <float.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;



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

    

sal_Bool SAL_CALL ScAccessibleCellBase::isVisible()
    throw (uno::RuntimeException, std::exception)
{
     SolarMutexGuard aGuard;
    IsObjectValid();
    
    sal_Bool bVisible(sal_True);
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
    throw (uno::RuntimeException)
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
                                uno::Any aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_CCOLOR));
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
    throw (uno::RuntimeException)
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
                                uno::Any aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_CELLBACK));
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

    

uno::Any SAL_CALL ScAccessibleCellBase::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
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

    

sal_Int32
    ScAccessibleCellBase::getAccessibleIndexInParent(void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    return mnIndex;
}

OUString SAL_CALL
    ScAccessibleCellBase::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    OUString sDescription = OUString(ScResId(STR_ACC_CELL_DESCR));

    return sDescription;
}

OUString SAL_CALL
    ScAccessibleCellBase::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    
    
    return maCellAddress.Format(SCA_VALID, NULL);
}

    

uno::Any SAL_CALL
    ScAccessibleCellBase::getCurrentValue()
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException)
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
            ScDocShell* pDocShell = (ScDocShell*) mpDoc->GetDocumentShell();
            bResult = pDocShell->GetDocFunc().SetValueCell(maCellAddress, fValue, false);
        }
    }
    return bResult;
}

uno::Any SAL_CALL
    ScAccessibleCellBase::getMaximumValue(  )
    throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= DBL_MAX;

    return aAny;
}

uno::Any SAL_CALL
    ScAccessibleCellBase::getMinimumValue(  )
    throw (uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= -DBL_MAX;

    return aAny;
}

    

OUString SAL_CALL ScAccessibleCellBase::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return OUString("ScAccessibleCellBase");
}

    

uno::Sequence< uno::Type > SAL_CALL ScAccessibleCellBase::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessibleCellBaseImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

namespace
{
    class theScAccessibleCellBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessibleCellBaseImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleCellBase::getImplementationId(void)
    throw (uno::RuntimeException)
{
    return theScAccessibleCellBaseImplementationId::get().getSeq();
}

bool ScAccessibleCellBase::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    bool bEditable(false);
    if (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::EDITABLE))
        bEditable = true;
    return bEditable;
}

OUString SAL_CALL ScAccessibleCellBase::GetNote(void)
                                throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    OUString msNote;
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
                                        msNote = xText->getString();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return msNote;
}

#include <com/sun/star/table/ShadowFormat.hpp>

OUString SAL_CALL ScAccessibleCellBase::getShadowAttrs(void)
                                        throw (::com::sun::star::uno::RuntimeException)
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
                                uno::Any aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_SHADOW));
                                aAny >>= aShadowFmt;
                            }
                        }
                    }
                }
            }
        }
    }
    
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
    
    if ( nLocationVal == 0 )
    {
        sShadowAttrs += sOuterSplit;
        return sShadowAttrs;
    }
    
    sShadowAttrs += "Location=";
    sShadowAttrs += OUString::number( (sal_Int32)nLocationVal );
    sShadowAttrs += sInnerSplit;
    sShadowAttrs += "ShadowWidth=";
    sShadowAttrs += OUString::number( (sal_Int32)aShadowFmt.ShadowWidth ) ;
    sShadowAttrs += sInnerSplit;
    sShadowAttrs += "IsTransparent=";
    sShadowAttrs += OUString::number( (sal_Bool)aShadowFmt.IsTransparent ) ;
    sShadowAttrs += sInnerSplit;
    sShadowAttrs += "Color=";
    sShadowAttrs += OUString::number( (sal_Int32)aShadowFmt.Color );
    sShadowAttrs += sOuterSplit;
    return sShadowAttrs;
}

#include <com/sun/star/table/BorderLine.hpp>

OUString SAL_CALL ScAccessibleCellBase::getBorderAttrs(void)
                                        throw (::com::sun::star::uno::RuntimeException)
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
                                uno::Any aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_TOPBORDER));
                                aAny >>= aTopBorder;
                                aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_BOTTBORDER));
                                aAny >>= aBottomBorder;
                                aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_LEFTBORDER));
                                aAny >>= aLeftBorder;
                                aAny = xCellProps->getPropertyValue(OUString(SC_UNONAME_RIGHTBORDER));
                                aAny >>= aRightBorder;
                            }
                        }
                    }
                }
            }
        }
    }

    Color aColor;
    sal_Bool bIn = mpDoc ? mpDoc->IsCellInChangeTrack(maCellAddress,&aColor) : sal_False;
    if (bIn)
    {
        aTopBorder.Color = aColor.GetColor();
        aBottomBorder.Color = aColor.GetColor();
        aLeftBorder.Color = aColor.GetColor();
        aRightBorder.Color = aColor.GetColor();
        aTopBorder.OuterLineWidth =2;
        aBottomBorder.OuterLineWidth =2;
        aLeftBorder.OuterLineWidth =2;
        aRightBorder.OuterLineWidth =2;
    }

    
    OUString sBorderAttrs;
    OUString sInnerSplit(",");
    OUString sOuterSplit(";");
    
    
    if ( aTopBorder.InnerLineWidth == 0 && aTopBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "TopBorder:;";
    }
    else
    {
        sBorderAttrs += "TopBorder:Color=";
        sBorderAttrs += OUString::number( (sal_Int32)aTopBorder.Color );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aTopBorder.InnerLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aTopBorder.OuterLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( (sal_Int32)aTopBorder.LineDistance );
        sBorderAttrs += sOuterSplit;
    }
    
    if ( aBottomBorder.InnerLineWidth == 0 && aBottomBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "BottomBorde:;";
    }
    else
    {
        sBorderAttrs += "BottomBorder:Color=";
        sBorderAttrs += OUString::number( (sal_Int32)aBottomBorder.Color );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aBottomBorder.InnerLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aBottomBorder.OuterLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( (sal_Int32)aBottomBorder.LineDistance );
        sBorderAttrs += sOuterSplit;
    }
    
    if ( aLeftBorder.InnerLineWidth == 0 && aLeftBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "LeftBorder:;";
    }
    else
    {
        sBorderAttrs += "LeftBorder:Color=";
        sBorderAttrs += OUString::number( (sal_Int32)aLeftBorder.Color );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aLeftBorder.InnerLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aLeftBorder.OuterLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( (sal_Int32)aLeftBorder.LineDistance );
        sBorderAttrs += sOuterSplit;
    }
    
    if ( aRightBorder.InnerLineWidth == 0 && aRightBorder.OuterLineWidth == 0 )
    {
        sBorderAttrs += "RightBorder:;";
    }
    else
    {
        sBorderAttrs += "RightBorder:Color=";
        sBorderAttrs += OUString::number( (sal_Int32)aRightBorder.Color );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "InnerLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aRightBorder.InnerLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "OuterLineWidth=";
        sBorderAttrs += OUString::number( (sal_Int32)aRightBorder.OuterLineWidth );
        sBorderAttrs += sInnerSplit;
        sBorderAttrs += "LineDistance=";
        sBorderAttrs += OUString::number( (sal_Int32)aRightBorder.LineDistance );
        sBorderAttrs += sOuterSplit;
    }
    return sBorderAttrs;
}


OUString SAL_CALL ScAccessibleCellBase::GetAllDisplayNote(void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString strNote;
    OUString strTrackText;
    if (mpDoc)
    {
        sal_Bool bLeftedge=sal_False;
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
