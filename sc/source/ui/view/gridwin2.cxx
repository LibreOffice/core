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

#include <scitems.hxx>
#include <vcl/settings.hxx>
#include <comphelper/lok.hxx>

#include <gridwin.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>
#include <pivot.hxx>
#include <uiitems.hxx>
#include <scresid.hxx>
#include <sc.hrc>
#include <globstr.hrc>
#include <strings.hrc>
#include <pagedata.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <dpoutput.hxx>
#include <dpshttab.hxx>
#include <dbdocfun.hxx>
#include <checklistmenu.hxx>
#include <dpcontrol.hxx>
#include <userlist.hxx>
#include <scabstdlg.hxx>
#include <spellcheckcontext.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#include <unordered_map>
#include <memory>
#include <vector>

using namespace css;
using namespace css::sheet;
using css::sheet::DataPilotFieldOrientation;
using std::vector;

DataPilotFieldOrientation ScGridWindow::GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(nCol, nRow, nTab);
    if (!pDPObj)
        return DataPilotFieldOrientation_HIDDEN;

    DataPilotFieldOrientation nOrient = DataPilotFieldOrientation_HIDDEN;

    // Check for page field first.
    if (nCol > 0)
    {
        // look for the dimension header left of the drop-down arrow
        long nField = pDPObj->GetHeaderDim( ScAddress( nCol-1, nRow, nTab ), nOrient );
        if ( nField >= 0 && nOrient == DataPilotFieldOrientation_PAGE )
        {
            bool bIsDataLayout = false;
            OUString aFieldName = pDPObj->GetDimName( nField, bIsDataLayout );
            if ( !aFieldName.isEmpty() && !bIsDataLayout )
                return DataPilotFieldOrientation_PAGE;
        }
    }

    nOrient = DataPilotFieldOrientation_HIDDEN;

    // Now, check for row/column field.
    long nField = pDPObj->GetHeaderDim(ScAddress(nCol, nRow, nTab), nOrient);
    if (nField >= 0 && (nOrient == DataPilotFieldOrientation_COLUMN || nOrient == DataPilotFieldOrientation_ROW) )
    {
        bool bIsDataLayout = false;
        OUString aFieldName = pDPObj->GetDimName(nField, bIsDataLayout);
        if (!aFieldName.isEmpty() && !bIsDataLayout)
            return nOrient;
    }

    return DataPilotFieldOrientation_HIDDEN;
}

// private method for mouse button handling
bool ScGridWindow::DoPageFieldSelection( SCCOL nCol, SCROW nRow )
{
    if (GetDPFieldOrientation( nCol, nRow ) == DataPilotFieldOrientation_PAGE)
    {
        LaunchPageFieldMenu( nCol, nRow );
        return true;
    }
    return false;
}

bool ScGridWindow::DoAutoFilterButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    Point aScrPos  = pViewData->GetScrPos(nCol, nRow, eWhich);
    Point aDiffPix = rMEvt.GetPosPixel();

    aDiffPix -= aScrPos;
    bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    if ( bLayoutRTL )
        aDiffPix.setX( -aDiffPix.X() );

    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    // The button height should not use the merged cell height, should still use single row height
    nSizeY = ScViewData::ToPixel(pDoc->GetRowHeight(nRow, nTab), pViewData->GetPPTY());
    Size aScrSize(nSizeX-1, nSizeY-1);

    // Check if the mouse cursor is clicking on the popup arrow box.
    mpFilterButton.reset(new ScDPFieldButton(this, &GetSettings().GetStyleSettings(), &pViewData->GetZoomY(), pDoc));
    mpFilterButton->setBoundingBox(aScrPos, aScrSize, bLayoutRTL);
    mpFilterButton->setPopupLeft(bLayoutRTL);   // #i114944# AutoFilter button is left-aligned in RTL
    Point aPopupPos;
    Size aPopupSize;
    mpFilterButton->getPopupBoundingBox(aPopupPos, aPopupSize);
    tools::Rectangle aRect(aPopupPos, aPopupSize);
    if (aRect.IsInside(rMEvt.GetPosPixel()))
    {
        if ( DoPageFieldSelection( nCol, nRow ) )
            return true;

        bool bFilterActive = IsAutoFilterActive(nCol, nRow, nTab);
        mpFilterButton->setHasHiddenMember(bFilterActive);
        mpFilterButton->setDrawBaseButton(false);
        mpFilterButton->setDrawPopupButton(true);
        mpFilterButton->setPopupPressed(true);
        mpFilterButton->draw();
        LaunchAutoFilterMenu(nCol, nRow);
        return true;
    }

    return false;
}

void ScGridWindow::DoPushPivotButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt, bool bButton, bool bPopup )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();

    ScDPObject* pDPObj  = pDoc->GetDPAtCursor(nCol, nRow, nTab);

    if (pDPObj)
    {
        DataPilotFieldOrientation nOrient = DataPilotFieldOrientation_HIDDEN;
        ScAddress aPos( nCol, nRow, nTab );
        ScAddress aDimPos = aPos;
        if (!bButton && bPopup && aDimPos.Col() > 0)
            // For page field selection cell, the real field position is to the left.
            aDimPos.IncCol(-1);

        long nField = pDPObj->GetHeaderDim(aDimPos, nOrient);
        if ( nField >= 0 )
        {
            bDPMouse   = false;
            nDPField   = nField;
            pDragDPObj = pDPObj;
            if (bPopup && DPTestFieldPopupArrow(rMEvt, aPos, aDimPos, pDPObj))
            {
                // field name pop up menu has been launched.  Don't activate
                // field move.
                return;
            }

            if (bButton)
            {
                bDPMouse = true;
                DPTestMouse( rMEvt, true );
                StartTracking();
            }
        }
        else if ( pDPObj->IsFilterButton(aPos) )
        {
            ReleaseMouse();         // may have been captured in ButtonDown

            ScQueryParam aQueryParam;
            SCTAB nSrcTab = 0;
            const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
            OSL_ENSURE(pDesc, "no sheet source for filter button");
            if (pDesc)
            {
                aQueryParam = pDesc->GetQueryParam();
                nSrcTab = pDesc->GetSourceRange().aStart.Tab();
            }

            SfxItemSet aArgSet( pViewData->GetViewShell()->GetPool(),
                                        svl::Items<SCITEM_QUERYDATA, SCITEM_QUERYDATA>{} );
            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

            ScopedVclPtr<AbstractScPivotFilterDlg> pDlg(
                pFact->CreateScPivotFilterDlg(
                    pViewData->GetViewShell()->GetDialogParent(), aArgSet, nSrcTab));
            if ( pDlg->Execute() == RET_OK )
            {
                ScSheetSourceDesc aNewDesc(pDoc);
                if (pDesc)
                    aNewDesc = *pDesc;

                const ScQueryItem& rQueryItem = pDlg->GetOutputItem();
                aNewDesc.SetQueryParam(rQueryItem.GetQueryData());

                ScDPObject aNewObj( *pDPObj );
                aNewObj.SetSheetDesc( aNewDesc );
                ScDBDocFunc aFunc( *pViewData->GetDocShell() );
                aFunc.DataPilotUpdate( pDPObj, &aNewObj, true, false );
                pViewData->GetView()->CursorPosChanged();       // shells may be switched
            }
        }
    }
    else
    {
        OSL_FAIL("Nothing here");
    }
}

//  Data Pilot interaction

void ScGridWindow::DPTestMouse( const MouseEvent& rMEvt, bool bMove )
{
    OSL_ENSURE(pDragDPObj, "pDragDPObj missing");

    //  scroll window if at edges
    //! move this to separate method

    bool bTimer = false;
    Point aPixel = rMEvt.GetPosPixel();

    SCCOL nDx = 0;
    SCROW nDy = 0;
    if ( aPixel.X() < 0 )
        nDx = -1;
    if ( aPixel.Y() < 0 )
        nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPixel.X() >= aSize.Width() )
        nDx = 1;
    if ( aPixel.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        UpdateDragRect( false, tools::Rectangle() );

        if ( nDx  != 0)
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        bTimer = true;
    }

    SCCOL  nPosX;
    SCROW  nPosY;
    pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), eWhich, nPosX, nPosY );
    bool    bMouseLeft;
    bool    bMouseTop;
    pViewData->GetMouseQuadrant( aPixel, eWhich, nPosX, nPosY, bMouseLeft, bMouseTop );

    ScAddress aPos( nPosX, nPosY, pViewData->GetTabNo() );

    tools::Rectangle aPosRect;
    DataPilotFieldOrientation nOrient;
    long nDimPos;
    bool bHasRange = pDragDPObj->GetHeaderDrag( aPos, bMouseLeft, bMouseTop, nDPField,
                                                aPosRect, nOrient, nDimPos );
    UpdateDragRect( bHasRange && bMove, aPosRect );

    bool bIsDataLayout;
    sal_Int32 nDimFlags = 0;
    OUString aDimName = pDragDPObj->GetDimName( nDPField, bIsDataLayout, &nDimFlags );
    bool bAllowed = !bHasRange || ScDPObject::IsOrientationAllowed( nOrient, nDimFlags );

    if (bMove)          // set mouse pointer
    {
        PointerStyle ePointer = PointerStyle::PivotDelete;
        if ( !bAllowed )
            ePointer = PointerStyle::NotAllowed;
        else if ( bHasRange )
            switch (nOrient)
            {
                case DataPilotFieldOrientation_COLUMN: ePointer = PointerStyle::PivotCol; break;
                case DataPilotFieldOrientation_ROW:    ePointer = PointerStyle::PivotRow; break;
                case DataPilotFieldOrientation_PAGE:
                case DataPilotFieldOrientation_DATA:   ePointer = PointerStyle::PivotField;   break;
                default: break;
            }
        SetPointer( ePointer );
    }
    else                // execute change
    {
        if (!bHasRange)
            nOrient = DataPilotFieldOrientation_HIDDEN;

        if ( bIsDataLayout && ( nOrient != DataPilotFieldOrientation_COLUMN &&
                                nOrient != DataPilotFieldOrientation_ROW ) )
        {
            //  removing data layout is not allowed
            pViewData->GetView()->ErrorMessage(STR_PIVOT_MOVENOTALLOWED);
        }
        else if ( bAllowed )
        {
            ScDPSaveData aSaveData( *pDragDPObj->GetSaveData() );

            ScDPSaveDimension* pDim;
            if ( bIsDataLayout )
                pDim = aSaveData.GetDataLayoutDimension();
            else
                pDim = aSaveData.GetDimensionByName(aDimName);
            pDim->SetOrientation( nOrient );
            aSaveData.SetPosition( pDim, nDimPos );

            //! docfunc method with ScDPSaveData as argument?

            ScDPObject aNewObj( *pDragDPObj );
            aNewObj.SetSaveData( aSaveData );
            ScDBDocFunc aFunc( *pViewData->GetDocShell() );
            // when dragging fields, allow re-positioning (bAllowMove)
            aFunc.DataPilotUpdate( pDragDPObj, &aNewObj, true, false, true );
            pViewData->GetView()->CursorPosChanged();       // shells may be switched
        }
    }

    if (bTimer && bMove)
        pViewData->GetView()->SetTimer( this, rMEvt );          // repeat event
    else
        pViewData->GetView()->ResetTimer();
}

bool ScGridWindow::DPTestFieldPopupArrow(
    const MouseEvent& rMEvt, const ScAddress& rPos, const ScAddress& rDimPos, ScDPObject* pDPObj)
{
    bool bLayoutRTL = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );

    // Get the geometry of the cell.
    Point aScrPos = pViewData->GetScrPos(rPos.Col(), rPos.Row(), eWhich);
    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel(rPos.Col(), rPos.Row(), nSizeX, nSizeY);
    Size aScrSize(nSizeX-1, nSizeY-1);

    // Check if the mouse cursor is clicking on the popup arrow box.
    ScDPFieldButton aBtn(this, &GetSettings().GetStyleSettings());
    aBtn.setBoundingBox(aScrPos, aScrSize, bLayoutRTL);
    aBtn.setPopupLeft(false);   // DataPilot popup is always right-aligned for now
    Point aPopupPos;
    Size aPopupSize;
    aBtn.getPopupBoundingBox(aPopupPos, aPopupSize);
    tools::Rectangle aRect(aPopupPos, aPopupSize);
    if (aRect.IsInside(rMEvt.GetPosPixel()))
    {
        // Mouse cursor inside the popup arrow box.  Launch the field menu.
        DPLaunchFieldPopupMenu(OutputToScreenPixel(aScrPos), aScrSize, rDimPos, pDPObj);
        return true;
    }

    return false;
}

namespace {

struct DPFieldPopupData : public ScCheckListMenuWindow::ExtendedData
{
    ScDPLabelData   maLabels;
    ScDPObject*     mpDPObj;
    long            mnDim;
};

class DPFieldPopupOKAction : public ScMenuFloatingWindow::Action
{
public:
    explicit DPFieldPopupOKAction(ScGridWindow* p) :
        mpGridWindow(p) {}

    virtual void execute() override
    {
        mpGridWindow->UpdateDPFromFieldPopupMenu();
    }
private:
    VclPtr<ScGridWindow> mpGridWindow;
};

class PopupSortAction : public ScMenuFloatingWindow::Action
{
public:
    enum SortType { ASCENDING, DESCENDING, CUSTOM };

    explicit PopupSortAction(ScDPObject* pDPObject, long nDimIndex, SortType eType,
                             sal_uInt16 nUserListIndex, ScTabViewShell* pViewShell)
        : mpDPObject(pDPObject)
        , mnDimIndex(nDimIndex)
        , meType(eType)
        , mnUserListIndex(nUserListIndex)
        , mpViewShell(pViewShell)
    {}

    virtual void execute() override
    {
        switch (meType)
        {
            case ASCENDING:
                mpViewShell->DataPilotSort(mpDPObject, mnDimIndex, true);
            break;
            case DESCENDING:
                mpViewShell->DataPilotSort(mpDPObject, mnDimIndex, false);
            break;
            case CUSTOM:
                mpViewShell->DataPilotSort(mpDPObject, mnDimIndex, true, &mnUserListIndex);
            break;
            default:
                ;
        }
    }

private:
    ScDPObject* const     mpDPObject;
    long const            mnDimIndex;
    SortType const        meType;
    sal_uInt16      mnUserListIndex;
    ScTabViewShell* mpViewShell;
};

}

void ScGridWindow::DPLaunchFieldPopupMenu(const Point& rScreenPosition, const Size& rScreenSize,
                                          const ScAddress& rAddress, ScDPObject* pDPObject)
{
    DataPilotFieldOrientation nOrient;
    long nDimIndex = pDPObject->GetHeaderDim(rAddress, nOrient);

    DPLaunchFieldPopupMenu(rScreenPosition, rScreenSize, nDimIndex, pDPObject);
}

void ScGridWindow::DPLaunchFieldPopupMenu(const Point& rScrPos, const Size& rScrSize,
                                          long nDimIndex, ScDPObject* pDPObj)
{
    std::unique_ptr<DPFieldPopupData> pDPData(new DPFieldPopupData);
    pDPData->mnDim = nDimIndex;
    pDPObj->GetSource();

    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName(pDPData->mnDim, bIsDataLayout);
    pDPObj->BuildAllDimensionMembers();
    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    const ScDPSaveDimension* pDim = pSaveData->GetExistingDimensionByName(aDimName);
    if (!pDim)
        // This should never happen.
        return;

    // We need to get the list of field members.
    pDPObj->FillLabelData(pDPData->mnDim, pDPData->maLabels);
    pDPData->mpDPObj = pDPObj;

    const ScDPLabelData& rLabelData = pDPData->maLabels;

    mpDPFieldPopup.disposeAndClear();
    mpDPFieldPopup.reset(VclPtr<ScCheckListMenuWindow>::Create(this, pViewData->GetDocument()));
    mpDPFieldPopup->setName("DataPilot field member popup");
    mpDPFieldPopup->setExtendedData(std::move(pDPData));
    mpDPFieldPopup->setOKAction(new DPFieldPopupOKAction(this));
    {
        // Populate field members.
        size_t n = rLabelData.maMembers.size();
        mpDPFieldPopup->setMemberSize(n);
        for (size_t i = 0; i < n; ++i)
        {
            const ScDPLabelData::Member& rMem = rLabelData.maMembers[i];
            OUString aName = rMem.getDisplayName();
            if (aName.isEmpty())
                // Use special string for an empty name.
                mpDPFieldPopup->addMember(ScResId(STR_EMPTYDATA), rMem.mbVisible);
            else
                mpDPFieldPopup->addMember(rMem.getDisplayName(), rMem.mbVisible);
        }
        mpDPFieldPopup->initMembers();
    }

    if (pDim->GetOrientation() != DataPilotFieldOrientation_PAGE)
    {
        vector<OUString> aUserSortNames;
        ScUserList* pUserList = ScGlobal::GetUserList();
        if (pUserList)
        {
            size_t n = pUserList->size();
            aUserSortNames.reserve(n);
            for (size_t i = 0; i < n; ++i)
            {
                const ScUserListData& rData = (*pUserList)[i];
                aUserSortNames.push_back(rData.GetString());
            }
        }

        // Populate the menus.
        ScTabViewShell* pViewShell = pViewData->GetViewShell();
        mpDPFieldPopup->addMenuItem(
            ScResId(STR_MENU_SORT_ASC),
            new PopupSortAction(pDPObj, nDimIndex, PopupSortAction::ASCENDING, 0, pViewShell));
        mpDPFieldPopup->addMenuItem(
            ScResId(STR_MENU_SORT_DESC),
            new PopupSortAction(pDPObj, nDimIndex, PopupSortAction::DESCENDING, 0, pViewShell));
        ScMenuFloatingWindow* pSubMenu = mpDPFieldPopup->addSubMenuItem(
            ScResId(STR_MENU_SORT_CUSTOM), !aUserSortNames.empty());

        if (pSubMenu)
        {
            size_t n = aUserSortNames.size();
            for (size_t i = 0; i < n; ++i)
            {
                pSubMenu->addMenuItem(
                    aUserSortNames[i],
                    new PopupSortAction(pDPObj, nDimIndex, PopupSortAction::CUSTOM, sal_uInt16(i), pViewShell));
            }
        }
    }

    tools::Rectangle aCellRect(rScrPos, rScrSize);

    mpDPFieldPopup->SetPopupModeEndHdl( LINK(this, ScGridWindow, PopupModeEndHdl) );
    ScCheckListMenuWindow::Config aConfig;
    aConfig.mbAllowEmptySet = false;
    aConfig.mbRTL = pViewData->GetDocument()->IsLayoutRTL(pViewData->GetTabNo());
    mpDPFieldPopup->setConfig(aConfig);
    mpDPFieldPopup->launch(aCellRect);
}

void ScGridWindow::UpdateDPFromFieldPopupMenu()
{
    typedef std::unordered_map<OUString, OUString> MemNameMapType;

    if (!mpDPFieldPopup)
        return;

    DPFieldPopupData* pDPData = static_cast<DPFieldPopupData*>(mpDPFieldPopup->getExtendedData());
    if (!pDPData)
        return;

    ScDPObject* pDPObj = pDPData->mpDPObj;
    ScDPSaveData* pSaveData = pDPObj->GetSaveData();

    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName(pDPData->mnDim, bIsDataLayout);
    ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aDimName);
    if (!pDim)
        return;

    // Build a map of layout names to original names.
    const ScDPLabelData& rLabelData = pDPData->maLabels;
    MemNameMapType aMemNameMap;
    for (const auto& rMember : rLabelData.maMembers)
        aMemNameMap.emplace(rMember.maLayoutName, rMember.maName);

    // The raw result may contain a mixture of layout names and original names.
    ScCheckListMenuWindow::ResultType aRawResult;
    mpDPFieldPopup->getResult(aRawResult);

    std::unordered_map<OUString, bool> aResult;
    for (const auto& rItem : aRawResult)
    {
        MemNameMapType::const_iterator itrNameMap = aMemNameMap.find(rItem.aName);
        if (itrNameMap == aMemNameMap.end())
        {
            // This is an original member name.  Use it as-is.
            OUString aName = rItem.aName;
            if (aName == ScResId(STR_EMPTYDATA))
                // Translate the special empty name into an empty string.
                aName.clear();

            aResult.emplace(aName, rItem.bValid);
        }
        else
        {
            // This is a layout name.  Get the original member name and use it.
            aResult.emplace(itrNameMap->second, rItem.bValid);
        }
    }
    pDim->UpdateMemberVisibility(aResult);

    ScDBDocFunc aFunc(*pViewData->GetDocShell());
    aFunc.UpdatePivotTable(*pDPObj, true, false);
}

bool ScGridWindow::UpdateVisibleRange()
{
    SCCOL nPosX = 0;
    SCROW nPosY = 0;
    SCCOL nXRight = MAXCOL;
    SCROW nYBottom = MAXROW;

    if (comphelper::LibreOfficeKit::isActive())
    {
        // entire table in the tiled rendering case
        SCTAB nTab = pViewData->GetTabNo();
        ScDocument const& rDoc = *pViewData->GetDocument();
        SCCOL nEndCol = 0;
        SCROW nEndRow = 0;

        if (rDoc.GetPrintArea(nTab, nEndCol, nEndRow, false))
        {
            nXRight = nEndCol;
            nYBottom = nEndRow;
        }
    }
    else
    {
        nPosX = pViewData->GetPosX(eHWhich);
        nPosY = pViewData->GetPosY(eVWhich);
        nXRight = nPosX + pViewData->VisibleCellsX(eHWhich);
        if (nXRight > MAXCOL)
            nXRight = MAXCOL;
        nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
        if (nYBottom > MAXROW)
            nYBottom = MAXROW;
    }

    // Store the current visible range.
    bool bChanged = maVisibleRange.set(nPosX, nPosY, nXRight, nYBottom);
    if (bChanged)
        ResetAutoSpell();

    return bChanged;
}

void ScGridWindow::DPMouseMove( const MouseEvent& rMEvt )
{
    DPTestMouse( rMEvt, true );
}

void ScGridWindow::DPMouseButtonUp( const MouseEvent& rMEvt )
{
    bDPMouse = false;
    ReleaseMouse();

    DPTestMouse( rMEvt, false );
    SetPointer( PointerStyle::Arrow );
}

void ScGridWindow::UpdateDragRect( bool bShowRange, const tools::Rectangle& rPosRect )
{
    SCCOL nStartX = ( rPosRect.Left()   >= 0 ) ? static_cast<SCCOL>(rPosRect.Left())   : SCCOL_MAX;
    SCROW nStartY = ( rPosRect.Top()    >= 0 ) ? static_cast<SCROW>(rPosRect.Top())    : SCROW_MAX;
    SCCOL nEndX   = ( rPosRect.Right()  >= 0 ) ? static_cast<SCCOL>(rPosRect.Right())  : SCCOL_MAX;
    SCROW nEndY   = ( rPosRect.Bottom() >= 0 ) ? static_cast<SCROW>(rPosRect.Bottom()) : SCROW_MAX;

    if ( bShowRange == bDragRect && nDragStartX == nStartX && nDragEndX == nEndX &&
                                    nDragStartY == nStartY && nDragEndY == nEndY )
    {
        return;         // everything unchanged
    }

    if ( bShowRange )
    {
        nDragStartX = nStartX;
        nDragStartY = nStartY;
        nDragEndX = nEndX;
        nDragEndY = nEndY;
        bDragRect = true;
    }
    else
        bDragRect = false;

    UpdateDragRectOverlay();
}

//  Page-Break Mode

sal_uInt16 ScGridWindow::HitPageBreak( const Point& rMouse, ScRange* pSource,
                                    SCCOLROW* pBreak, SCCOLROW* pPrev )
{
    sal_uInt16 nFound = SC_PD_NONE;     // 0
    ScRange aSource;
    SCCOLROW nBreak = 0;
    SCCOLROW nPrev = 0;

    ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
    if ( pPageData )
    {
        bool bHori = false;
        bool bVert = false;
        SCCOL nHitX = 0;
        SCROW nHitY = 0;

        long nMouseX = rMouse.X();
        long nMouseY = rMouse.Y();
        SCCOL nPosX;
        SCROW nPosY;
        pViewData->GetPosFromPixel( nMouseX, nMouseY, eWhich, nPosX, nPosY );
        Point aTL = pViewData->GetScrPos( nPosX, nPosY, eWhich );
        Point aBR = pViewData->GetScrPos( nPosX+1, nPosY+1, eWhich );

        //  Horizontal more tolerances as for vertical, because there is more space
        if ( nMouseX <= aTL.X() + 4 )
        {
            bHori = true;
            nHitX = nPosX;
        }
        else if ( nMouseX >= aBR.X() - 6 )
        {
            bHori = true;
            nHitX = nPosX+1;                    // left edge of the next cell
        }
        if ( nMouseY <= aTL.Y() + 2 )
        {
            bVert = true;
            nHitY = nPosY;
        }
        else if ( nMouseY >= aBR.Y() - 4 )
        {
            bVert = true;
            nHitY = nPosY+1;                    // upper edge of the next cell
        }

        if ( bHori || bVert )
        {
            sal_uInt16 nCount = sal::static_int_cast<sal_uInt16>( pPageData->GetCount() );
            for (sal_uInt16 nPos=0; nPos<nCount && !nFound; nPos++)
            {
                ScPrintRangeData& rData = pPageData->GetData(nPos);
                ScRange aRange = rData.GetPrintRange();
                bool bLHit = ( bHori && nHitX == aRange.aStart.Col() );
                bool bRHit = ( bHori && nHitX == aRange.aEnd.Col() + 1 );
                bool bTHit = ( bVert && nHitY == aRange.aStart.Row() );
                bool bBHit = ( bVert && nHitY == aRange.aEnd.Row() + 1 );
                bool bInsideH = ( nPosX >= aRange.aStart.Col() && nPosX <= aRange.aEnd.Col() );
                bool bInsideV = ( nPosY >= aRange.aStart.Row() && nPosY <= aRange.aEnd.Row() );

                if ( bLHit )
                {
                    if ( bTHit )
                        nFound = SC_PD_RANGE_TL;
                    else if ( bBHit )
                        nFound = SC_PD_RANGE_BL;
                    else if ( bInsideV )
                        nFound = SC_PD_RANGE_L;
                }
                else if ( bRHit )
                {
                    if ( bTHit )
                        nFound = SC_PD_RANGE_TR;
                    else if ( bBHit )
                        nFound = SC_PD_RANGE_BR;
                    else if ( bInsideV )
                        nFound = SC_PD_RANGE_R;
                }
                else if ( bTHit && bInsideH )
                    nFound = SC_PD_RANGE_T;
                else if ( bBHit && bInsideH )
                    nFound = SC_PD_RANGE_B;
                if (nFound)
                    aSource = aRange;

                //  breaks

                if ( bVert && bInsideH && !nFound )
                {
                    size_t nRowCount = rData.GetPagesY();
                    const SCROW* pRowEnd = rData.GetPageEndY();
                    for (size_t nRowPos=0; nRowPos+1<nRowCount; nRowPos++)
                        if ( pRowEnd[nRowPos]+1 == nHitY )
                        {
                            nFound = SC_PD_BREAK_V;
                            aSource = aRange;
                            nBreak = nHitY;
                            if ( nRowPos )
                                nPrev = pRowEnd[nRowPos-1]+1;
                            else
                                nPrev = aRange.aStart.Row();
                        }
                }
                if ( bHori && bInsideV && !nFound )
                {
                    size_t nColCount = rData.GetPagesX();
                    const SCCOL* pColEnd = rData.GetPageEndX();
                    for (size_t nColPos=0; nColPos+1<nColCount; nColPos++)
                        if ( pColEnd[nColPos]+1 == nHitX )
                        {
                            nFound = SC_PD_BREAK_H;
                            aSource = aRange;
                            nBreak = nHitX;
                            if ( nColPos )
                                nPrev = pColEnd[nColPos-1]+1;
                            else
                                nPrev = aRange.aStart.Col();
                        }
                }
            }
        }
    }

    if (pSource)
        *pSource = aSource;     // print break
    if (pBreak)
        *pBreak = nBreak;       // X/Y position of the moved page break
    if (pPrev)
        *pPrev = nPrev;         // X/Y beginning of the page, which is above the break
    return nFound;
}

void ScGridWindow::PagebreakMove( const MouseEvent& rMEvt, bool bUp )
{
    //! Combine scrolling and switching with RFMouseMove !
    //! (Inverting before scrolling is different)

    //  Scrolling

    bool bTimer = false;
    Point aPos = rMEvt.GetPosPixel();
    SCCOL nDx = 0;
    SCROW nDy = 0;
    if ( aPos.X() < 0 ) nDx = -1;
    if ( aPos.Y() < 0 ) nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        if ( bPagebreakDrawn )          // invert
        {
            bPagebreakDrawn = false;
            UpdateDragRectOverlay();
        }

        if ( nDx != 0 ) pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 ) pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );
        bTimer = true;
    }

    // Switching when fixating (so Scrolling works)

    if ( eWhich == pViewData->GetActivePart() )     //??
    {
        if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
            if ( nDx > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
                else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }

        if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
            if ( nDy > 0 )
            {
                if ( eWhich == SC_SPLIT_TOPLEFT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
                else if ( eWhich == SC_SPLIT_TOPRIGHT )
                    pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
            }
    }

    // from here new

    // Searching for a position between the cells (before nPosX / nPosY)
    SCCOL nPosX;
    SCROW nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    bool bLeft, bTop;
    pViewData->GetMouseQuadrant( aPos, eWhich, nPosX, nPosY, bLeft, bTop );
    if ( !bLeft ) ++nPosX;
    if ( !bTop )  ++nPosY;

    bool bBreak = ( nPagebreakMouse == SC_PD_BREAK_H || nPagebreakMouse == SC_PD_BREAK_V );
    bool bHide = false;
    bool bToEnd = false;
    ScRange aDrawRange = aPagebreakSource;
    if ( bBreak )
    {
        if ( nPagebreakMouse == SC_PD_BREAK_H )
        {
            if ( nPosX > aPagebreakSource.aStart.Col() &&
                 nPosX <= aPagebreakSource.aEnd.Col() + 1 )     // to the end is also allowed
            {
                bToEnd = ( nPosX == aPagebreakSource.aEnd.Col() + 1 );
                aDrawRange.aStart.SetCol( nPosX );
                aDrawRange.aEnd.SetCol( nPosX - 1 );
            }
            else
                bHide = true;
        }
        else
        {
            if ( nPosY > aPagebreakSource.aStart.Row() &&
                 nPosY <= aPagebreakSource.aEnd.Row() + 1 )     //  to the end is also allowed
            {
                bToEnd = ( nPosY == aPagebreakSource.aEnd.Row() + 1 );
                aDrawRange.aStart.SetRow( nPosY );
                aDrawRange.aEnd.SetRow( nPosY - 1 );
            }
            else
                bHide = true;
        }
    }
    else
    {
        if ( nPagebreakMouse & SC_PD_RANGE_L )
            aDrawRange.aStart.SetCol( nPosX );
        if ( nPagebreakMouse & SC_PD_RANGE_T )
            aDrawRange.aStart.SetRow( nPosY );
        if ( nPagebreakMouse & SC_PD_RANGE_R )
        {
            if ( nPosX > 0 )
                aDrawRange.aEnd.SetCol( nPosX-1 );
            else
                bHide = true;
        }
        if ( nPagebreakMouse & SC_PD_RANGE_B )
        {
            if ( nPosY > 0 )
                aDrawRange.aEnd.SetRow( nPosY-1 );
            else
                bHide = true;
        }
        if ( aDrawRange.aStart.Col() > aDrawRange.aEnd.Col() ||
             aDrawRange.aStart.Row() > aDrawRange.aEnd.Row() )
            bHide = true;
    }

    if ( !bPagebreakDrawn || bUp || aDrawRange != aPagebreakDrag )
    {
        // draw...

        if ( bPagebreakDrawn )
        {
            // invert
            bPagebreakDrawn = false;
        }
        aPagebreakDrag = aDrawRange;
        if ( !bUp && !bHide )
        {
            // revert
            bPagebreakDrawn = true;
        }
        UpdateDragRectOverlay();
    }

    // when ButtonUp execute the changes

    if ( bUp )
    {
        ScViewFunc* pViewFunc = pViewData->GetView();
        ScDocShell* pDocSh = pViewData->GetDocShell();
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = pViewData->GetTabNo();
        bool bUndo (rDoc.IsUndoEnabled());

        if ( bBreak )
        {
            bool bColumn = ( nPagebreakMouse == SC_PD_BREAK_H );
            SCCOLROW nNew = bColumn ? static_cast<SCCOLROW>(nPosX) : static_cast<SCCOLROW>(nPosY);
            if ( nNew != nPagebreakBreak )
            {
                if (bUndo)
                {
                    OUString aUndo = ScResId( STR_UNDO_DRAG_BREAK );
                    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo, 0, pViewData->GetViewShell()->GetViewShellId() );
                }

                bool bGrow = !bHide && nNew > nPagebreakBreak;
                if ( bColumn )
                {
                    if (rDoc.HasColBreak(static_cast<SCCOL>(nPagebreakBreak), nTab) & ScBreakType::Manual)
                    {
                        ScAddress aOldAddr( static_cast<SCCOL>(nPagebreakBreak), nPosY, nTab );
                        pViewFunc->DeletePageBreak( true, true, &aOldAddr, false );
                    }
                    if ( !bHide && !bToEnd )    // not at the end
                    {
                        ScAddress aNewAddr( static_cast<SCCOL>(nNew), nPosY, nTab );
                        pViewFunc->InsertPageBreak( true, true, &aNewAddr, false );
                    }
                    if ( bGrow )
                    {
                        // change last break to hard, and change scaleing
                        bool bManualBreak(rDoc.HasColBreak(static_cast<SCCOL>(nPagebreakPrev), nTab) & ScBreakType::Manual);
                        if ( static_cast<SCCOL>(nPagebreakPrev) > aPagebreakSource.aStart.Col() && !bManualBreak )
                        {
                            ScAddress aPrev( static_cast<SCCOL>(nPagebreakPrev), nPosY, nTab );
                            pViewFunc->InsertPageBreak( true, true, &aPrev, false );
                        }

                        if (!pDocSh->AdjustPrintZoom( ScRange(
                                      static_cast<SCCOL>(nPagebreakPrev),0,nTab, static_cast<SCCOL>(nNew-1),0,nTab ) ))
                            bGrow = false;
                    }
                }
                else
                {
                    if (rDoc.HasRowBreak(nPagebreakBreak, nTab) & ScBreakType::Manual)
                    {
                        ScAddress aOldAddr( nPosX, nPagebreakBreak, nTab );
                        pViewFunc->DeletePageBreak( false, true, &aOldAddr, false );
                    }
                    if ( !bHide && !bToEnd )    // not at the end
                    {
                        ScAddress aNewAddr( nPosX, nNew, nTab );
                        pViewFunc->InsertPageBreak( false, true, &aNewAddr, false );
                    }
                    if ( bGrow )
                    {
                        // change last break to hard, and change scaleing
                        bool bManualBreak(rDoc.HasRowBreak(nPagebreakPrev, nTab) & ScBreakType::Manual);
                        if ( nPagebreakPrev > aPagebreakSource.aStart.Row() && !bManualBreak )
                        {
                            ScAddress aPrev( nPosX, nPagebreakPrev, nTab );
                            pViewFunc->InsertPageBreak( false, true, &aPrev, false );
                        }

                        if (!pDocSh->AdjustPrintZoom( ScRange(
                                      0,nPagebreakPrev,nTab, 0,nNew-1,nTab ) ))
                            bGrow = false;
                    }
                }

                if (bUndo)
                {
                    pDocSh->GetUndoManager()->LeaveListAction();
                }

                if (!bGrow)     // otherwise has already happened in AdjustPrintZoom
                {
                    pViewFunc->UpdatePageBreakData( true );
                    pDocSh->SetDocumentModified();
                }
            }
        }
        else if ( bHide || aPagebreakDrag != aPagebreakSource )
        {
            // set print range

            OUString aNewRanges;
            sal_uInt16 nOldCount = rDoc.GetPrintRangeCount( nTab );
            if ( nOldCount )
            {
                for (sal_uInt16 nPos=0; nPos<nOldCount; nPos++)
                {
                    const ScRange* pOld = rDoc.GetPrintRange( nTab, nPos );
                    if ( pOld )
                    {
                        OUString aTemp;
                        if ( *pOld != aPagebreakSource )
                            aTemp = pOld->Format(ScRefFlags::VALID);
                        else if ( !bHide )
                            aTemp = aPagebreakDrag.Format(ScRefFlags::VALID);
                        if (!aTemp.isEmpty())
                        {
                            if ( !aNewRanges.isEmpty() )
                                aNewRanges += ";";
                            aNewRanges += aTemp;
                        }
                    }
                }
            }
            else if (!bHide)
                aNewRanges = aPagebreakDrag.Format(ScRefFlags::VALID);

            pViewFunc->SetPrintRanges( rDoc.IsPrintEntireSheet( nTab ), &aNewRanges, nullptr, nullptr, false );
        }
    }

    //  Timer for Scrolling

    if (bTimer && !bUp)
        pViewData->GetView()->SetTimer( this, rMEvt );          // repeat event
    else
        pViewData->GetView()->ResetTimer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
