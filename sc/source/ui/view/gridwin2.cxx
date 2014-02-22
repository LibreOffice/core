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

#include "scitems.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

#include "gridwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "pivot.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "pagedata.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpoutput.hxx"
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "checklistmenu.hxx"
#include "dpcontrol.hxx"
#include "checklistmenu.hrc"
#include "strload.hxx"
#include "userlist.hxx"
#include "scabstdlg.hxx"
#include "spellcheckcontext.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>

#include <vector>
#include <boost/unordered_map.hpp>

using namespace com::sun::star;
using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::std::vector;
using ::std::auto_ptr;
using ::boost::unordered_map;





DataPilotFieldOrientation ScGridWindow::GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const
{
    using namespace ::com::sun::star::sheet;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(nCol, nRow, nTab);
    if (!pDPObj)
        return DataPilotFieldOrientation_HIDDEN;

    sal_uInt16 nOrient = DataPilotFieldOrientation_HIDDEN;

    
    if (nCol > 0)
    {
        
        long nField = pDPObj->GetHeaderDim( ScAddress( nCol-1, nRow, nTab ), nOrient );
        if ( nField >= 0 && nOrient == DataPilotFieldOrientation_PAGE )
        {
            bool bIsDataLayout = false;
            OUString aFieldName = pDPObj->GetDimName( nField, bIsDataLayout );
            if ( !aFieldName.isEmpty() && !bIsDataLayout )
                return DataPilotFieldOrientation_PAGE;
        }
    }

    nOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    
    long nField = pDPObj->GetHeaderDim(ScAddress(nCol, nRow, nTab), nOrient);
    if (nField >= 0 && (nOrient == DataPilotFieldOrientation_COLUMN || nOrient == DataPilotFieldOrientation_ROW) )
    {
        bool bIsDataLayout = false;
        OUString aFieldName = pDPObj->GetDimName(nField, bIsDataLayout);
        if (!aFieldName.isEmpty() && !bIsDataLayout)
            return static_cast<DataPilotFieldOrientation>(nOrient);
    }

    return DataPilotFieldOrientation_HIDDEN;
}


bool ScGridWindow::DoPageFieldSelection( SCCOL nCol, SCROW nRow )
{
    if (GetDPFieldOrientation( nCol, nRow ) == sheet::DataPilotFieldOrientation_PAGE)
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
    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    if ( bLayoutRTL )
        aDiffPix.X() = -aDiffPix.X();

    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    
    nSizeY = pViewData->ToPixel(pDoc->GetRowHeight(nRow, nTab), pViewData->GetPPTY());
    Size aScrSize(nSizeX-1, nSizeY-1);

    
    mpFilterButton.reset(new ScDPFieldButton(this, &GetSettings().GetStyleSettings(), &pViewData->GetZoomX(), &pViewData->GetZoomY(), pDoc));
    mpFilterButton->setBoundingBox(aScrPos, aScrSize, bLayoutRTL);
    mpFilterButton->setPopupLeft(bLayoutRTL);   
    Point aPopupPos;
    Size aPopupSize;
    mpFilterButton->getPopupBoundingBox(aPopupPos, aPopupSize);
    Rectangle aRect(aPopupPos, aPopupSize);
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
        sal_uInt16 nOrient = sheet::DataPilotFieldOrientation_HIDDEN;
        ScAddress aPos( nCol, nRow, nTab );
        ScAddress aDimPos = aPos;
        if (!bButton && bPopup && aDimPos.Col() > 0)
            
            aDimPos.IncCol(-1);

        long nField = pDPObj->GetHeaderDim(aDimPos, nOrient);
        if ( nField >= 0 )
        {
            bDPMouse   = false;
            nDPField   = nField;
            pDragDPObj = pDPObj;
            if (bPopup && DPTestFieldPopupArrow(rMEvt, aPos, aDimPos, pDPObj))
            {
                
                
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
            ReleaseMouse();         

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
                                        SCITEM_QUERYDATA, SCITEM_QUERYDATA );
            aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

            ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

            AbstractScPivotFilterDlg* pDlg = pFact->CreateScPivotFilterDlg( pViewData->GetViewShell()->GetDialogParent(),
                                                                            aArgSet, nSrcTab,
                                                                            RID_SCDLG_PIVOTFILTER);
            OSL_ENSURE(pDlg, "Dialog create fail!");
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
                pViewData->GetView()->CursorPosChanged();       
            }
            delete pDlg;
        }
    }
    else
    {
        OSL_FAIL("Da is ja garnix");
    }
}


//

//

void ScGridWindow::DPTestMouse( const MouseEvent& rMEvt, bool bMove )
{
    OSL_ENSURE(pDragDPObj, "pDragDPObj missing");

    
    

    sal_Bool bTimer = false;
    Point aPixel = rMEvt.GetPosPixel();

    SCsCOL nDx = 0;
    SCsROW nDy = 0;
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
        UpdateDragRect( false, Rectangle() );

        if ( nDx  != 0)
            pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 )
            pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

        bTimer = sal_True;
    }

    

    SCsCOL  nPosX;
    SCsROW  nPosY;
    pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), eWhich, nPosX, nPosY );
    bool    bMouseLeft;
    bool    bMouseTop;
    pViewData->GetMouseQuadrant( aPixel, eWhich, nPosX, nPosY, bMouseLeft, bMouseTop );

    ScAddress aPos( nPosX, nPosY, pViewData->GetTabNo() );

    Rectangle aPosRect;
    sal_uInt16 nOrient;
    long nDimPos;
    sal_Bool bHasRange = pDragDPObj->GetHeaderDrag( aPos, bMouseLeft, bMouseTop, nDPField,
                                                aPosRect, nOrient, nDimPos );
    UpdateDragRect( bHasRange && bMove, aPosRect );

    bool bIsDataLayout;
    sal_Int32 nDimFlags = 0;
    OUString aDimName = pDragDPObj->GetDimName( nDPField, bIsDataLayout, &nDimFlags );
    bool bAllowed = !bHasRange || ScDPObject::IsOrientationAllowed( nOrient, nDimFlags );

    if (bMove)          
    {
        PointerStyle ePointer = POINTER_PIVOT_DELETE;
        if ( !bAllowed )
            ePointer = POINTER_NOTALLOWED;
        else if ( bHasRange )
            switch (nOrient)
            {
                case sheet::DataPilotFieldOrientation_COLUMN: ePointer = POINTER_PIVOT_COL; break;
                case sheet::DataPilotFieldOrientation_ROW:    ePointer = POINTER_PIVOT_ROW; break;
                case sheet::DataPilotFieldOrientation_PAGE:
                case sheet::DataPilotFieldOrientation_DATA:   ePointer = POINTER_PIVOT_FIELD;   break;
            }
        SetPointer( ePointer );
    }
    else                
    {
        if (!bHasRange)
            nOrient = sheet::DataPilotFieldOrientation_HIDDEN;

        if ( bIsDataLayout && ( nOrient != sheet::DataPilotFieldOrientation_COLUMN &&
                                nOrient != sheet::DataPilotFieldOrientation_ROW ) )
        {
            
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

            

            ScDPObject aNewObj( *pDragDPObj );
            aNewObj.SetSaveData( aSaveData );
            ScDBDocFunc aFunc( *pViewData->GetDocShell() );
            
            aFunc.DataPilotUpdate( pDragDPObj, &aNewObj, true, false, true );
            pViewData->GetView()->CursorPosChanged();       
        }
    }

    if (bTimer && bMove)
        pViewData->GetView()->SetTimer( this, rMEvt );          
    else
        pViewData->GetView()->ResetTimer();
}

bool ScGridWindow::DPTestFieldPopupArrow(
    const MouseEvent& rMEvt, const ScAddress& rPos, const ScAddress& rDimPos, ScDPObject* pDPObj)
{
    sal_Bool bLayoutRTL = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );

    
    Point aScrPos = pViewData->GetScrPos(rPos.Col(), rPos.Row(), eWhich);
    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel(rPos.Col(), rPos.Row(), nSizeX, nSizeY);
    Size aScrSize(nSizeX-1, nSizeY-1);

    
    ScDPFieldButton aBtn(this, &GetSettings().GetStyleSettings());
    aBtn.setBoundingBox(aScrPos, aScrSize, bLayoutRTL);
    aBtn.setPopupLeft(false);   
    Point aPopupPos;
    Size aPopupSize;
    aBtn.getPopupBoundingBox(aPopupPos, aPopupSize);
    Rectangle aRect(aPopupPos, aPopupSize);
    if (aRect.IsInside(rMEvt.GetPosPixel()))
    {
        
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

    virtual void execute()
    {
        mpGridWindow->UpdateDPFromFieldPopupMenu();
    }
private:
    ScGridWindow* mpGridWindow;
};

class PopupSortAction : public ScMenuFloatingWindow::Action
{
public:
    enum SortType { ASCENDING, DESCENDING, CUSTOM };

    explicit PopupSortAction(const ScAddress& rPos, SortType eType, sal_uInt16 nUserListIndex, ScTabViewShell* pViewShell) :
        maPos(rPos), meType(eType), mnUserListIndex(nUserListIndex), mpViewShell(pViewShell) {}

    virtual void execute()
    {
        switch (meType)
        {
            case ASCENDING:
                mpViewShell->DataPilotSort(maPos, true);
            break;
            case DESCENDING:
                mpViewShell->DataPilotSort(maPos, false);
            break;
            case CUSTOM:
                mpViewShell->DataPilotSort(maPos, true, &mnUserListIndex);
            break;
            default:
                ;
        }
    }

private:
    ScAddress       maPos;
    SortType        meType;
    sal_uInt16      mnUserListIndex;
    ScTabViewShell* mpViewShell;
};

}

void ScGridWindow::DPLaunchFieldPopupMenu(
    const Point& rScrPos, const Size& rScrSize, const ScAddress& rPos, ScDPObject* pDPObj)
{
    auto_ptr<DPFieldPopupData> pDPData(new DPFieldPopupData);
    sal_uInt16 nOrient;
    pDPData->mnDim = pDPObj->GetHeaderDim(rPos, nOrient);

    bool bIsDataLayout;
    OUString aDimName = pDPObj->GetDimName(pDPData->mnDim, bIsDataLayout);
    pDPObj->BuildAllDimensionMembers();
    const ScDPSaveData* pSaveData = pDPObj->GetSaveData();
    const ScDPSaveDimension* pDim = pSaveData->GetExistingDimensionByName(aDimName);
    if (!pDim)
        
        return;

    
    pDPObj->FillLabelData(pDPData->mnDim, pDPData->maLabels);
    pDPData->mpDPObj = pDPObj;

    const ScDPLabelData& rLabelData = pDPData->maLabels;

    mpDPFieldPopup.reset(new ScCheckListMenuWindow(this, pViewData->GetDocument()));
    mpDPFieldPopup->setName("DataPilot field member popup");
    mpDPFieldPopup->setExtendedData(pDPData.release());
    mpDPFieldPopup->setOKAction(new DPFieldPopupOKAction(this));
    {
        
        size_t n = rLabelData.maMembers.size();
        mpDPFieldPopup->setMemberSize(n);
        for (size_t i = 0; i < n; ++i)
        {
            const ScDPLabelData::Member& rMem = rLabelData.maMembers[i];
            OUString aName = rMem.getDisplayName();
            if (aName.isEmpty())
                
                mpDPFieldPopup->addMember(ScGlobal::GetRscString(STR_EMPTYDATA), rMem.mbVisible);
            else
                mpDPFieldPopup->addMember(rMem.getDisplayName(), rMem.mbVisible);
        }
        mpDPFieldPopup->initMembers();
    }

    if (pDim->GetOrientation() != sheet::DataPilotFieldOrientation_PAGE)
    {
        vector<OUString> aUserSortNames;
        ScUserList* pUserList = ScGlobal::GetUserList();
        if (pUserList)
        {
            size_t n = pUserList->size();
            aUserSortNames.reserve(n);
            for (size_t i = 0; i < n; ++i)
            {
                const ScUserListData* pData = (*pUserList)[i];
                aUserSortNames.push_back(pData->GetString());
            }
        }

        
        ScTabViewShell* pViewShell = pViewData->GetViewShell();
        mpDPFieldPopup->addMenuItem(
            SC_STRLOAD(RID_POPUP_FILTER, STR_MENU_SORT_ASC), true,
            new PopupSortAction(rPos, PopupSortAction::ASCENDING, 0, pViewShell));
        mpDPFieldPopup->addMenuItem(
            SC_STRLOAD(RID_POPUP_FILTER, STR_MENU_SORT_DESC), true,
            new PopupSortAction(rPos, PopupSortAction::DESCENDING, 0, pViewShell));
        ScMenuFloatingWindow* pSubMenu = mpDPFieldPopup->addSubMenuItem(
            SC_STRLOAD(RID_POPUP_FILTER, STR_MENU_SORT_CUSTOM), !aUserSortNames.empty());

        if (pSubMenu && !aUserSortNames.empty())
        {
            size_t n = aUserSortNames.size();
            for (size_t i = 0; i < n; ++i)
            {
                pSubMenu->addMenuItem(
                    aUserSortNames[i], true,
                    new PopupSortAction(rPos, PopupSortAction::CUSTOM, static_cast<sal_uInt16>(i), pViewShell));
            }
        }
    }

    Rectangle aCellRect(rScrPos, rScrSize);

    mpDPFieldPopup->SetPopupModeEndHdl( LINK(this, ScGridWindow, PopupModeEndHdl) );
    ScCheckListMenuWindow::Config aConfig;
    aConfig.mbAllowEmptySet = false;
    aConfig.mbRTL = pViewData->GetDocument()->IsLayoutRTL(pViewData->GetTabNo());
    mpDPFieldPopup->setConfig(aConfig);
    mpDPFieldPopup->launch(aCellRect);
}

void ScGridWindow::UpdateDPFromFieldPopupMenu()
{
    typedef boost::unordered_map<OUString, OUString, OUStringHash> MemNameMapType;

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

    
    const ScDPLabelData& rLabelData = pDPData->maLabels;
    MemNameMapType aMemNameMap;
    for (vector<ScDPLabelData::Member>::const_iterator itr = rLabelData.maMembers.begin(), itrEnd = rLabelData.maMembers.end();
           itr != itrEnd; ++itr)
        aMemNameMap.insert(MemNameMapType::value_type(itr->maLayoutName, itr->maName));

    
    ScCheckListMenuWindow::ResultType aRawResult;
    mpDPFieldPopup->getResult(aRawResult);

    ScCheckListMenuWindow::ResultType aResult;
    ScCheckListMenuWindow::ResultType::const_iterator itr = aRawResult.begin(), itrEnd = aRawResult.end();
    for (; itr != itrEnd; ++itr)
    {
        MemNameMapType::const_iterator itrNameMap = aMemNameMap.find(itr->first);
        if (itrNameMap == aMemNameMap.end())
        {
            
            OUString aName = itr->first;
            if (aName.equals(ScGlobal::GetRscString(STR_EMPTYDATA)))
                
                aName = OUString();

            aResult.insert(
                ScCheckListMenuWindow::ResultType::value_type(
                    aName, itr->second));
        }
        else
        {
            
            aResult.insert(
                ScCheckListMenuWindow::ResultType::value_type(
                    itrNameMap->second, itr->second));
        }
    }
    pDim->UpdateMemberVisibility(aResult);

    ScDBDocFunc aFunc(*pViewData->GetDocShell());
    aFunc.UpdatePivotTable(*pDPObj, true, false);
}

bool ScGridWindow::UpdateVisibleRange()
{
    SCCOL nPosX = pViewData->GetPosX(eHWhich);
    SCROW nPosY = pViewData->GetPosY(eVWhich);
    SCCOL nXRight = nPosX + pViewData->VisibleCellsX(eHWhich);
    if (nXRight > MAXCOL) nXRight = MAXCOL;
    SCROW nYBottom = nPosY + pViewData->VisibleCellsY(eVWhich);
    if (nYBottom > MAXROW) nYBottom = MAXROW;

    
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
    SetPointer( Pointer( POINTER_ARROW ) );
}



void ScGridWindow::UpdateDragRect( bool bShowRange, const Rectangle& rPosRect )
{
    SCCOL nStartX = ( rPosRect.Left()   >= 0 ) ? static_cast<SCCOL>(rPosRect.Left())   : SCCOL_MAX;
    SCROW nStartY = ( rPosRect.Top()    >= 0 ) ? static_cast<SCROW>(rPosRect.Top())    : SCROW_MAX;
    SCCOL nEndX   = ( rPosRect.Right()  >= 0 ) ? static_cast<SCCOL>(rPosRect.Right())  : SCCOL_MAX;
    SCROW nEndY   = ( rPosRect.Bottom() >= 0 ) ? static_cast<SCROW>(rPosRect.Bottom()) : SCROW_MAX;

    if ( bShowRange == bDragRect && nDragStartX == nStartX && nDragEndX == nEndX &&
                                    nDragStartY == nStartY && nDragEndY == nEndY )
    {
        return;         
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





sal_uInt16 ScGridWindow::HitPageBreak( const Point& rMouse, ScRange* pSource,
                                    SCCOLROW* pBreak, SCCOLROW* pPrev )
{
    sal_uInt16 nFound = SC_PD_NONE;     
    ScRange aSource;
    SCCOLROW nBreak = 0;
    SCCOLROW nPrev = 0;

    ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
    if ( pPageData )
    {
        sal_Bool bHori = false;
        sal_Bool bVert = false;
        SCCOL nHitX = 0;
        SCROW nHitY = 0;

        long nMouseX = rMouse.X();
        long nMouseY = rMouse.Y();
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( nMouseX, nMouseY, eWhich, nPosX, nPosY );
        Point aTL = pViewData->GetScrPos( nPosX, nPosY, eWhich );
        Point aBR = pViewData->GetScrPos( nPosX+1, nPosY+1, eWhich );

        
        if ( nMouseX <= aTL.X() + 4 )
        {
            bHori = sal_True;
            nHitX = nPosX;
        }
        else if ( nMouseX >= aBR.X() - 6 )
        {
            bHori = sal_True;
            nHitX = nPosX+1;                    
        }
        if ( nMouseY <= aTL.Y() + 2 )
        {
            bVert = sal_True;
            nHitY = nPosY;
        }
        else if ( nMouseY >= aBR.Y() - 4 )
        {
            bVert = sal_True;
            nHitY = nPosY+1;                    
        }

        if ( bHori || bVert )
        {
            sal_uInt16 nCount = sal::static_int_cast<sal_uInt16>( pPageData->GetCount() );
            for (sal_uInt16 nPos=0; nPos<nCount && !nFound; nPos++)
            {
                ScPrintRangeData& rData = pPageData->GetData(nPos);
                ScRange aRange = rData.GetPrintRange();
                sal_Bool bLHit = ( bHori && nHitX == aRange.aStart.Col() );
                sal_Bool bRHit = ( bHori && nHitX == aRange.aEnd.Col() + 1 );
                sal_Bool bTHit = ( bVert && nHitY == aRange.aStart.Row() );
                sal_Bool bBHit = ( bVert && nHitY == aRange.aEnd.Row() + 1 );
                sal_Bool bInsideH = ( nPosX >= aRange.aStart.Col() && nPosX <= aRange.aEnd.Col() );
                sal_Bool bInsideV = ( nPosY >= aRange.aStart.Row() && nPosY <= aRange.aEnd.Row() );

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
        *pSource = aSource;     
    if (pBreak)
        *pBreak = nBreak;       
    if (pPrev)
        *pPrev = nPrev;         
    return nFound;
}

void ScGridWindow::PagebreakMove( const MouseEvent& rMEvt, bool bUp )
{
    
    

    

    sal_Bool bTimer = false;
    Point aPos = rMEvt.GetPosPixel();
    SCsCOL nDx = 0;
    SCsROW nDy = 0;
    if ( aPos.X() < 0 ) nDx = -1;
    if ( aPos.Y() < 0 ) nDy = -1;
    Size aSize = GetOutputSizePixel();
    if ( aPos.X() >= aSize.Width() )
        nDx = 1;
    if ( aPos.Y() >= aSize.Height() )
        nDy = 1;
    if ( nDx != 0 || nDy != 0 )
    {
        if ( bPagebreakDrawn )          
        {
            bPagebreakDrawn = false;
            UpdateDragRectOverlay();
        }

        if ( nDx != 0 ) pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
        if ( nDy != 0 ) pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );
        bTimer = sal_True;
    }

    

    if ( eWhich == pViewData->GetActivePart() )     
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

    

    
    SCsCOL nPosX;
    SCsROW nPosY;
    pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
    bool bLeft, bTop;
    pViewData->GetMouseQuadrant( aPos, eWhich, nPosX, nPosY, bLeft, bTop );
    if ( !bLeft ) ++nPosX;
    if ( !bTop )  ++nPosY;

    sal_Bool bBreak = ( nPagebreakMouse == SC_PD_BREAK_H || nPagebreakMouse == SC_PD_BREAK_V );
    sal_Bool bHide = false;
    sal_Bool bToEnd = false;
    ScRange aDrawRange = aPagebreakSource;
    if ( bBreak )
    {
        if ( nPagebreakMouse == SC_PD_BREAK_H )
        {
            if ( nPosX > aPagebreakSource.aStart.Col() &&
                 nPosX <= aPagebreakSource.aEnd.Col() + 1 )     
            {
                bToEnd = ( nPosX == aPagebreakSource.aEnd.Col() + 1 );
                aDrawRange.aStart.SetCol( nPosX );
                aDrawRange.aEnd.SetCol( nPosX - 1 );
            }
            else
                bHide = sal_True;
        }
        else
        {
            if ( nPosY > aPagebreakSource.aStart.Row() &&
                 nPosY <= aPagebreakSource.aEnd.Row() + 1 )     
            {
                bToEnd = ( nPosY == aPagebreakSource.aEnd.Row() + 1 );
                aDrawRange.aStart.SetRow( nPosY );
                aDrawRange.aEnd.SetRow( nPosY - 1 );
            }
            else
                bHide = sal_True;
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
                bHide = sal_True;
        }
        if ( nPagebreakMouse & SC_PD_RANGE_B )
        {
            if ( nPosY > 0 )
                aDrawRange.aEnd.SetRow( nPosY-1 );
            else
                bHide = sal_True;
        }
        if ( aDrawRange.aStart.Col() > aDrawRange.aEnd.Col() ||
             aDrawRange.aStart.Row() > aDrawRange.aEnd.Row() )
            bHide = sal_True;
    }

    if ( !bPagebreakDrawn || bUp || aDrawRange != aPagebreakDrag )
    {
        

        if ( bPagebreakDrawn )
        {
            
            bPagebreakDrawn = false;
        }
        aPagebreakDrag = aDrawRange;
        if ( !bUp && !bHide )
        {
            
            bPagebreakDrawn = true;
        }
        UpdateDragRectOverlay();
    }

    

    if ( bUp )
    {
        ScViewFunc* pViewFunc = pViewData->GetView();
        ScDocShell* pDocSh = pViewData->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        SCTAB nTab = pViewData->GetTabNo();
        sal_Bool bUndo (pDoc->IsUndoEnabled());

        if ( bBreak )
        {
            sal_Bool bColumn = ( nPagebreakMouse == SC_PD_BREAK_H );
            SCCOLROW nNew = bColumn ? static_cast<SCCOLROW>(nPosX) : static_cast<SCCOLROW>(nPosY);
            if ( nNew != nPagebreakBreak )
            {
                if (bUndo)
                {
                    OUString aUndo = ScGlobal::GetRscString( STR_UNDO_DRAG_BREAK );
                    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
                }

                sal_Bool bGrow = !bHide && nNew > nPagebreakBreak;
                if ( bColumn )
                {
                    if (pDoc->HasColBreak(static_cast<SCCOL>(nPagebreakBreak), nTab) & BREAK_MANUAL)
                    {
                        ScAddress aOldAddr( static_cast<SCCOL>(nPagebreakBreak), nPosY, nTab );
                        pViewFunc->DeletePageBreak( true, true, &aOldAddr, false );
                    }
                    if ( !bHide && !bToEnd )    
                    {
                        ScAddress aNewAddr( static_cast<SCCOL>(nNew), nPosY, nTab );
                        pViewFunc->InsertPageBreak( true, true, &aNewAddr, false );
                    }
                    if ( bGrow )
                    {
                        
                        bool bManualBreak = (pDoc->HasColBreak(static_cast<SCCOL>(nPagebreakPrev), nTab) & BREAK_MANUAL);
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
                    if (pDoc->HasRowBreak(nPagebreakBreak, nTab) & BREAK_MANUAL)
                    {
                        ScAddress aOldAddr( nPosX, nPagebreakBreak, nTab );
                        pViewFunc->DeletePageBreak( false, true, &aOldAddr, false );
                    }
                    if ( !bHide && !bToEnd )    
                    {
                        ScAddress aNewAddr( nPosX, nNew, nTab );
                        pViewFunc->InsertPageBreak( false, true, &aNewAddr, false );
                    }
                    if ( bGrow )
                    {
                        
                        bool bManualBreak = (pDoc->HasRowBreak(nPagebreakPrev, nTab) & BREAK_MANUAL);
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

                if (!bGrow)     
                {
                    pViewFunc->UpdatePageBreakData( true );
                    pDocSh->SetDocumentModified();
                }
            }
        }
        else if ( bHide || aPagebreakDrag != aPagebreakSource )
        {
            

            OUString aNewRanges;
            sal_uInt16 nOldCount = pDoc->GetPrintRangeCount( nTab );
            if ( nOldCount )
            {
                for (sal_uInt16 nPos=0; nPos<nOldCount; nPos++)
                {
                    const ScRange* pOld = pDoc->GetPrintRange( nTab, nPos );
                    if ( pOld )
                    {
                        OUString aTemp;
                        if ( *pOld != aPagebreakSource )
                            aTemp = pOld->Format(SCA_VALID);
                        else if ( !bHide )
                            aTemp = aPagebreakDrag.Format(SCA_VALID);
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
                aNewRanges = aPagebreakDrag.Format(SCA_VALID);

            pViewFunc->SetPrintRanges( pDoc->IsPrintEntireSheet( nTab ), &aNewRanges, NULL, NULL, false );
        }
    }

    

    if (bTimer && !bUp)
        pViewData->GetView()->SetTimer( this, rMEvt );          
    else
        pViewData->GetView()->ResetTimer();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
