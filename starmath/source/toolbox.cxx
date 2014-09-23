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

#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <svl/intitem.hxx>
#include <svtools/imgdef.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imgmgr.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>

#include "toolbox.hxx"
#include "starmath.hrc"
#include "toolbox.hrc"
#include "view.hxx"




static sal_uInt16  GetImageListRID( sal_uInt16 nCategoryRID )
{
    sal_uInt16 nRes = 0xFFFF;
    switch (nCategoryRID)
    {
        case RID_UNBINOPS_CAT       : nRes = RID_IL_UNBINOPS; break;
        case RID_RELATIONS_CAT      : nRes = RID_IL_RELATIONS; break;
        case RID_SETOPERATIONS_CAT  : nRes = RID_IL_SETOPERATIONS; break;
        case RID_FUNCTIONS_CAT      : nRes = RID_IL_FUNCTIONS; break;
        case RID_OPERATORS_CAT      : nRes = RID_IL_OPERATORS; break;
        case RID_ATTRIBUTES_CAT     : nRes = RID_IL_ATTRIBUTES; break;
        case RID_BRACKETS_CAT       : nRes = RID_IL_BRACKETS; break;
        case RID_FORMAT_CAT         : nRes = RID_IL_FORMAT; break;
        case RID_MISC_CAT           : nRes = RID_IL_MISC; break;
        default :
            SAL_WARN( "starmath", "unknown category" );
    }
    return nRes;
}


static sal_Int16  GetToolBoxCategoriesIndex( sal_uInt16 nCategoryRID )
{
    sal_Int16 nIdx = -1;
    switch (nCategoryRID)
    {
        case RID_UNBINOPS_CAT       : nIdx = 0; break;
        case RID_RELATIONS_CAT      : nIdx = 1; break;
        case RID_SETOPERATIONS_CAT  : nIdx = 2; break;
        case RID_FUNCTIONS_CAT      : nIdx = 3; break;
        case RID_OPERATORS_CAT      : nIdx = 4; break;
        case RID_ATTRIBUTES_CAT     : nIdx = 5; break;
        case RID_BRACKETS_CAT       : nIdx = 6; break;
        case RID_FORMAT_CAT         : nIdx = 7; break;
        case RID_MISC_CAT           : nIdx = 8; break;
        default:
            ;
    }
    return nIdx;
}


static sal_uInt16  GetCategoryRID( sal_uInt16 nResId )
{
    sal_uInt16 nRes = 0xFFFF;
    switch (nResId)
    {
        case RID_IL_UNBINOPS       : nRes = RID_UNBINOPS_CAT; break;
        case RID_IL_RELATIONS      : nRes = RID_RELATIONS_CAT; break;
        case RID_IL_SETOPERATIONS  : nRes = RID_SETOPERATIONS_CAT; break;
        case RID_IL_FUNCTIONS      : nRes = RID_FUNCTIONS_CAT; break;
        case RID_IL_OPERATORS      : nRes = RID_OPERATORS_CAT; break;
        case RID_IL_ATTRIBUTES     : nRes = RID_ATTRIBUTES_CAT; break;
        case RID_IL_BRACKETS       : nRes = RID_BRACKETS_CAT; break;
        case RID_IL_FORMAT         : nRes = RID_FORMAT_CAT; break;
        case RID_IL_MISC           : nRes = RID_MISC_CAT; break;
        default :
            SAL_WARN( "starmath", "unknown category" );
            break;
    }
    return nRes;
}

SmToolBoxWindow::SmToolBoxWindow(SfxBindings *pTmpBindings,
                                 SfxChildWindow *pChildWindow,
                                 vcl::Window *pParent)
    : SfxFloatingWindow(pTmpBindings, pChildWindow, pParent, "FloatingElements",
        "modules/smath/ui/floatingelements.ui")
{
    get(m_pToolBoxCat, "catalog");
    m_nUnbinopsId = m_pToolBoxCat->GetItemId("RID_UNBINOPS_CAT");
    m_nRelationsId = m_pToolBoxCat->GetItemId("RID_RELATIONS_CAT");
    m_nSetoperationsId = m_pToolBoxCat->GetItemId("RID_SETOPERATIONS_CAT");
    m_nFunctionsId = m_pToolBoxCat->GetItemId("RID_FUNCTIONS_CAT");
    m_nOperatorsId = m_pToolBoxCat->GetItemId("RID_OPERATORS_CAT");
    m_nAttributesId = m_pToolBoxCat->GetItemId("RID_ATTRIBUTES_CAT");
    m_nBracketsId = m_pToolBoxCat->GetItemId("RID_BRACKETS_CAT");
    m_nFormatId = m_pToolBoxCat->GetItemId("RID_FORMAT_CAT");
    m_nMiscId = m_pToolBoxCat->GetItemId("RID_MISC_CAT");
    m_pToolBoxCat->InsertSpace(7);
    m_pToolBoxCat->InsertBreak(5);
    m_pToolBoxCat->SetLineCount(2);

    // allow for cursor travelling between toolbox and sub-categories
    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    nActiveCategoryRID = USHRT_MAX;

    m_pToolBoxCat->SetClickHdl(LINK(this, SmToolBoxWindow, CategoryClickHdl));

    sal_uInt16 i;
    for (i = 0;  i < NUM_TBX_CATEGORIES;  ++i)
    {
        ToolBox *pBox = new ToolBox(get<vcl::Window>("box"), SmResId( TOOLBOX_CAT_A + i ));
        vToolBoxCategories[i] = pBox;
        pBox->SetSelectHdl(LINK(this, SmToolBoxWindow, CmdSelectHdl));
    }
    pToolBoxCmd = vToolBoxCategories[0];

    for (i = 0;  i <= NUM_TBX_CATEGORIES; ++i)
        aImageLists [i] = 0;
}

SmToolBoxWindow::~SmToolBoxWindow()
{
    int i;
    for (i = 0;  i < NUM_TBX_CATEGORIES;  ++i)
    {
        ToolBox *pBox = vToolBoxCategories[i];
        delete pBox;
    }
    for (i = 0;  i < NUM_TBX_CATEGORIES + 1;  ++i)
        delete aImageLists[i];
}

SmViewShell * SmToolBoxWindow::GetView()
{
    SfxViewShell *pView = GetBindings().GetDispatcher()->GetFrame()->GetViewShell();
    return PTR_CAST(SmViewShell, pView);
}

const ImageList * SmToolBoxWindow::GetImageList( sal_uInt16 nResId )
{
    // creates the image list via its resource id and stores that
    // list for later use in the respective array.

    const ImageList *pIL = 0;

    // get index to use
    sal_uInt16 nCategoryRID = GetCategoryRID( nResId );
    sal_Int16 nIndex = GetToolBoxCategoriesIndex( nCategoryRID );

    if (nIndex >= 0)
    {
        ImageList **pImgList = aImageLists;
        if (!pImgList[ nIndex ])
            pImgList[ nIndex ] = new ImageList( SmResId(nResId) );
        pIL = pImgList[ nIndex ];
    }

    OSL_ENSURE( pIL, "image list not found!" );
    return pIL;
}


void SmToolBoxWindow::ApplyImageLists( sal_uInt16 nCategoryRID )
{
    // set image list for active (visible) category of 'catalog'
    sal_Int16 nIdx = GetToolBoxCategoriesIndex( nCategoryRID );
    sal_uInt16 nResId = GetImageListRID( nCategoryRID );
    const ImageList *pImageList = GetImageList( nResId );
    OSL_ENSURE( pImageList && nIdx >= 0, "image list or index missing" );
    if (pImageList && nIdx >= 0)
        vToolBoxCategories[ nIdx ]->SetImageList( *pImageList );
}

void SmToolBoxWindow::DataChanged( const DataChangedEvent &rEvt )
{
    if ( (rEvt.GetType() == DATACHANGED_SETTINGS) && (rEvt.GetFlags() & SETTINGS_STYLE) )
        ApplyImageLists( nActiveCategoryRID );

    SfxFloatingWindow::DataChanged( rEvt );
}

void SmToolBoxWindow::StateChanged( StateChangedType nStateChange )
{
    static bool bSetPosition = true;
    if (STATE_CHANGE_INITSHOW == nStateChange)
    {
        SetCategory( nActiveCategoryRID == USHRT_MAX ? RID_UNBINOPS_CAT : nActiveCategoryRID );

        // calculate initial position to be used after creation of the window...
        if (bSetPosition)
        {
            AdjustPos();
            bSetPosition = false;
        }
    }
    //... otherwise the base class will remember the last position of the window
    SfxFloatingWindow::StateChanged( nStateChange );
}

void SmToolBoxWindow::AdjustPos()
{
    SmViewShell *pView = GetView();
    OSL_ENSURE( pView, "view shell missing" );
    Point aPos( 50, 75 );
    if (pView)
    {
        Size aWndSize(GetOutputSizePixel());
        SmGraphicWindow &rWin = pView->GetGraphicWindow();
        aPos = Point( rWin.OutputToScreenPixel(
                        Point( rWin.GetSizePixel().Width() - aWndSize.Width(), 0) ) );
    }
    if (aPos.X() < 0)
        aPos.X() = 0;
    if (aPos.Y() < 0)
        aPos.Y() = 0;
    SetPosPixel( aPos );
}

bool SmToolBoxWindow::Close()
{
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_TOOLBOX, SFX_CALLMODE_STANDARD,
                new SfxBoolItem(SID_TOOLBOX, false), 0L);
    return true;
}

void SmToolBoxWindow::GetFocus()
{
    // give focus to category toolbox
    // (allow for cursor travelling when a category is selected with the mouse)
    m_pToolBoxCat->GrabFocus();
}

void SmToolBoxWindow::SetCategory(sal_uInt16 nCategoryRID)
{
    if (nCategoryRID != nActiveCategoryRID)
        ApplyImageLists( nCategoryRID );

    sal_uInt16 nLines;
    // check for valid resource id
    switch (nCategoryRID)
    {
        case RID_UNBINOPS_CAT :     nLines = 4; break;
        case RID_RELATIONS_CAT:     nLines = 7; break;
        case RID_SETOPERATIONS_CAT: nLines = 5; break;
        case RID_FUNCTIONS_CAT:     nLines = 5; break;
        case RID_OPERATORS_CAT:     nLines = 3; break;
        case RID_ATTRIBUTES_CAT:    nLines = 5; break;
        case RID_MISC_CAT:          nLines = 4; break;
        case RID_BRACKETS_CAT:      nLines = 5; break;
        case RID_FORMAT_CAT:        nLines = 3; break;
        default:
            // nothing to be done
            return;
    }

    pToolBoxCmd->Hide();

    sal_Int16 nIdx = GetToolBoxCategoriesIndex( nCategoryRID );
        OSL_ENSURE( nIdx >= 0, "unknown category" );
    if (nIdx >= 0)
        pToolBoxCmd = vToolBoxCategories[nIdx];

    // calculate actual size of window to use
    Size aCatSize( m_pToolBoxCat->CalcWindowSizePixel( 2 ) );
    Size aCmdSize( pToolBoxCmd->CalcWindowSizePixel( nLines ) );
        OSL_ENSURE( aCatSize.Width() == aCmdSize.Width(), "width mismatch" );
    // main window settings
    Size  aWndSize ( aCatSize.Width(), pToolBoxCmd->GetPosPixel().Y() + aCmdSize.Height() + 3);
    SetOutputSizePixel( aWndSize );

    if (nActiveCategoryRID)
            m_pToolBoxCat->CheckItem(nActiveCategoryRID, false);
    nActiveCategoryRID = nCategoryRID;
        m_pToolBoxCat->CheckItem(nActiveCategoryRID, true);

    pToolBoxCmd->Show();
}

sal_uInt16 SmToolBoxWindow::MapToolbarIdToCategory(sal_uInt16 nId) const
{
    if (nId == m_nUnbinopsId)
        return RID_UNBINOPS_CAT;
    if (nId == m_nRelationsId)
        return RID_RELATIONS_CAT;
    if (nId == m_nSetoperationsId)
        return RID_SETOPERATIONS_CAT;
    if (nId == m_nFunctionsId)
        return RID_FUNCTIONS_CAT;
    if (nId == m_nOperatorsId)
        return RID_OPERATORS_CAT;
    if (nId == m_nAttributesId)
        return RID_ATTRIBUTES_CAT;
    if (nId == m_nBracketsId)
        return RID_BRACKETS_CAT;
    if (nId == m_nFormatId)
        return RID_FORMAT_CAT;
    return RID_MISC_CAT;
}

IMPL_LINK( SmToolBoxWindow, CategoryClickHdl, ToolBox*, pToolBox)
{
    sal_uInt16 nItemId = pToolBox->GetCurItemId();
    if (nItemId != 0)
    {
        SetCategory(MapToolbarIdToCategory(nItemId));
    }
    return 0;
}

IMPL_LINK( SmToolBoxWindow, CmdSelectHdl, ToolBox*, pToolBox)
{
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERTCOMMAND, SFX_CALLMODE_STANDARD,
                new SfxInt16Item(SID_INSERTCOMMAND, pToolBox->GetCurItemId()), 0L);
    return 0;
}


/**************************************************************************/

SFX_IMPL_FLOATINGWINDOW_WITHID(SmToolBoxWrapper, SID_TOOLBOXWINDOW);

SmToolBoxWrapper::SmToolBoxWrapper(vcl::Window *pParentWindow,
                                   sal_uInt16 nId, SfxBindings* pBindings,
                                   SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWindow = new SmToolBoxWindow(pBindings, this, pParentWindow);
    ((SfxFloatingWindow *)pWindow)->Initialize(pInfo);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
