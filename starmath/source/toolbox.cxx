/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"


#include <rtl/logfile.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <svl/intitem.hxx>
#include <svtools/imgdef.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imgmgr.hxx>
#include <vcl/wrkwin.hxx>
#include "toolbox.hxx"
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif
#ifndef _TOOLBOX_HRC_
#include "toolbox.hrc"
#endif
#include "view.hxx"


////////////////////////////////////////////////////////////

static sal_uInt16  GetImageListRID( sal_uInt16 nCategoryRID, sal_Bool bHighContrast )
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
            DBG_ERROR( "unkown category" );
    }
    if (nRes != 0xFFFF && bHighContrast)
        ++nRes;     //! the resource ID for the high contrast image list is just +1 compared to the regular ones
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
        case RID_IL_UNBINOPS        :
        case RID_ILH_UNBINOPS       : nRes = RID_UNBINOPS_CAT; break;
        case RID_IL_RELATIONS       :
        case RID_ILH_RELATIONS      : nRes = RID_RELATIONS_CAT; break;
        case RID_IL_SETOPERATIONS   :
        case RID_ILH_SETOPERATIONS  : nRes = RID_SETOPERATIONS_CAT; break;
        case RID_IL_FUNCTIONS       :
        case RID_ILH_FUNCTIONS      : nRes = RID_FUNCTIONS_CAT; break;
        case RID_IL_OPERATORS       :
        case RID_ILH_OPERATORS      : nRes = RID_OPERATORS_CAT; break;
        case RID_IL_ATTRIBUTES      :
        case RID_ILH_ATTRIBUTES     : nRes = RID_ATTRIBUTES_CAT; break;
        case RID_IL_BRACKETS        :
        case RID_ILH_BRACKETS       : nRes = RID_BRACKETS_CAT; break;
        case RID_IL_FORMAT          :
        case RID_ILH_FORMAT         : nRes = RID_FORMAT_CAT; break;
        case RID_IL_MISC            :
        case RID_ILH_MISC           : nRes = RID_MISC_CAT; break;
        default :
            if (nResId != RID_IL_CATALOG  &&  nResId != RID_ILH_CATALOG)
            {
#if OSL_DEBUG_LEVEL > 1
                DBG_ERROR( "unkown category" );
#endif
            }
    }
    return nRes;
}


////////////////////////////////////////////////////////////


SmToolBoxWindow::SmToolBoxWindow(SfxBindings *pTmpBindings,
                                 SfxChildWindow *pChildWindow,
                                 Window *pParent) :
    SfxFloatingWindow(pTmpBindings, pChildWindow, pParent, SmResId(RID_TOOLBOXWINDOW)),
    aToolBoxCat(this, SmResId(TOOLBOX_CATALOG)),
    aToolBoxCat_Delim(this, SmResId( FL_TOOLBOX_CAT_DELIM ))
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmToolBoxWindow::SmToolBoxWindow" );

    // allow for cursor travelling between toolbox and sub-categories
    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    nActiveCategoryRID = USHRT_MAX;

    aToolBoxCat.SetClickHdl(LINK(this, SmToolBoxWindow, CategoryClickHdl));

    sal_uInt16 i;
    for (i = 0;  i < NUM_TBX_CATEGORIES;  ++i)
    {
        ToolBox *pBox = new ToolBox(this, SmResId( TOOLBOX_CAT_A + i ));
        vToolBoxCategories[i] = pBox;
        pBox->SetSelectHdl(LINK(this, SmToolBoxWindow, CmdSelectHdl));
    }
    pToolBoxCmd = vToolBoxCategories[0];

    for (i = 0;  i <= NUM_TBX_CATEGORIES; ++i)
    {
        aImageLists [i] = 0;
        aImageListsH[i] = 0;
    }

    FreeResource();
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
    {
        delete aImageLists[i];
        delete aImageListsH[i];
    }
}


SmViewShell * SmToolBoxWindow::GetView()
{
    SfxViewShell *pView = GetBindings().GetDispatcher()->GetFrame()->GetViewShell();
    return PTR_CAST(SmViewShell, pView);
}


const ImageList * SmToolBoxWindow::GetImageList( sal_uInt16 nResId, sal_Bool bHighContrast )
{
    // creates the image list via its resource id and stores that
    // list for later use in the respective array.

    const ImageList *pIL = 0;

    // get index to use
    sal_uInt16 nCategoryRID = GetCategoryRID( nResId );
    sal_Int16 nIndex = GetToolBoxCategoriesIndex( nCategoryRID );
    if (nIndex == -1 && (nResId == RID_IL_CATALOG || nResId == RID_ILH_CATALOG))
        nIndex = NUM_TBX_CATEGORIES;

    if (nIndex >= 0)
    {
        ImageList **pImgList = bHighContrast ? aImageListsH : aImageLists;
        if (!pImgList[ nIndex ])
            pImgList[ nIndex ] = new ImageList( SmResId(nResId) );
        pIL = pImgList[ nIndex ];
    }

    DBG_ASSERT( pIL, "image list not found!" );
    return pIL;
}


void SmToolBoxWindow::ApplyImageLists( sal_uInt16 nCategoryRID )
{
    sal_Bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();

    // set image list for toolbox 'catalog'
    const ImageList *pImageList = GetImageList( bHighContrast ? RID_ILH_CATALOG : RID_IL_CATALOG, bHighContrast );
    DBG_ASSERT( pImageList, "image list missing" );
    if (pImageList)
        aToolBoxCat.SetImageList( *pImageList );

    // set image list for active (visible) category of 'catalog'
    sal_Int16 nIdx = GetToolBoxCategoriesIndex( nCategoryRID );
    sal_uInt16 nResId = GetImageListRID( nCategoryRID, bHighContrast );
    pImageList = GetImageList( nResId, bHighContrast );
    DBG_ASSERT( pImageList && nIdx >= 0, "image list or index missing" );
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
    static sal_Bool bSetPosition = sal_True;
    if (STATE_CHANGE_INITSHOW == nStateChange)
    {
        SetCategory( nActiveCategoryRID == USHRT_MAX ? RID_UNBINOPS_CAT : nActiveCategoryRID );

        // calculate initial position to be used after creation of the window...
        AdjustPosSize( bSetPosition );
        bSetPosition = sal_False;
    }
    //... otherwise the base class will remember the last position of the window
    SfxFloatingWindow::StateChanged( nStateChange );
}


void SmToolBoxWindow::AdjustPosSize( sal_Bool bSetPos )
{
    Size aCatSize( aToolBoxCat.CalcWindowSizePixel( 2 ) );
    Size aCmdSize( pToolBoxCmd->CalcWindowSizePixel( 4 /* see nLines in SetCategory*/ ) );
    DBG_ASSERT( aCatSize.Width() == aCmdSize.Width(), "width mismatch" );

    // catalog settings
    aToolBoxCat.SetPosPixel( Point(0, 3) );
    aToolBoxCat.SetSizePixel( aCatSize );
    // settings for catalog / category delimiter
    Point aP( aToolBoxCat_Delim.GetPosPixel() );
    aP.X() = 0;
    aToolBoxCat_Delim.SetPosPixel( aP );
    aToolBoxCat_Delim.SetSizePixel( Size( aCatSize.Width(), aToolBoxCat_Delim.GetSizePixel().Height() ) );
    // category settings
    aP.Y() += aToolBoxCat_Delim.GetSizePixel().Height();
    for (int i = 0;  i < NUM_TBX_CATEGORIES;  ++i)
    {
        vToolBoxCategories[i]->SetPosPixel( aP );
        vToolBoxCategories[i]->SetSizePixel( aCmdSize );
    }
    // main window settings
    Size    aWndSize ( aCatSize.Width(), pToolBoxCmd->GetPosPixel().Y() + pToolBoxCmd->GetSizePixel().Height() + 3);
    SetOutputSizePixel( aWndSize );

    if (bSetPos)
    {
        SmViewShell *pView = GetView();
        DBG_ASSERT( pView, "view shell missing" );
        Point aPos( 50, 75 );
        if (pView)
        {
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
}


sal_Bool SmToolBoxWindow::Close()
{
    SmViewShell *pViewSh = GetView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_TOOLBOX, SFX_CALLMODE_STANDARD,
                new SfxBoolItem(SID_TOOLBOX, sal_False), 0L);
    return sal_True;
}

void SmToolBoxWindow::GetFocus()
{
    // give focus to category toolbox
    // (allow for cursor travelling when a category is selected with the mouse)
    aToolBoxCat.GrabFocus();
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
        case RID_RELATIONS_CAT:     nLines = 4; break;
        case RID_SETOPERATIONS_CAT: nLines = 4; break;
        case RID_FUNCTIONS_CAT:     nLines = 4; break;
        case RID_OPERATORS_CAT:     nLines = 3; break;
        case RID_ATTRIBUTES_CAT:    nLines = 4; break;
        case RID_MISC_CAT:          nLines = 4; break;
        case RID_BRACKETS_CAT:      nLines = 4; break;
        case RID_FORMAT_CAT:        nLines = 3; break;
        default:
            // nothing to be done
            return;
    }

    pToolBoxCmd->Hide();

    sal_Int16 nIdx = GetToolBoxCategoriesIndex( nCategoryRID );
    DBG_ASSERT( nIdx >= 0, "unkown category" );
    if (nIdx >= 0)
        pToolBoxCmd = vToolBoxCategories[nIdx];

    // calculate actual size of window to use
    Size aCatSize( aToolBoxCat.CalcWindowSizePixel( 2 ) );
    Size aCmdSize( pToolBoxCmd->CalcWindowSizePixel( nLines ) );
    DBG_ASSERT( aCatSize.Width() == aCmdSize.Width(), "width mismatch" );
    // main window settings
    Size  aWndSize ( aCatSize.Width(), pToolBoxCmd->GetPosPixel().Y() + aCmdSize.Height() + 3);
    SetOutputSizePixel( aWndSize );

    if (nActiveCategoryRID)
        aToolBoxCat.CheckItem(nActiveCategoryRID, sal_False);
    nActiveCategoryRID = nCategoryRID;
    aToolBoxCat.CheckItem(nActiveCategoryRID, sal_True);

    pToolBoxCmd->Show();
}


IMPL_LINK( SmToolBoxWindow, CategoryClickHdl, ToolBox*, pToolBox)
{
    int nItemId = pToolBox->GetCurItemId();
    if (nItemId != 0)
        SetCategory( sal::static_int_cast< sal_uInt16 >(nItemId) );
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

SFX_IMPL_FLOATINGWINDOW(SmToolBoxWrapper, SID_TOOLBOXWINDOW);

SmToolBoxWrapper::SmToolBoxWrapper(Window *pParentWindow,
                                   sal_uInt16 nId, SfxBindings* pBindings,
                                   SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWindow = new SmToolBoxWindow(pBindings, this, pParentWindow);
    ((SfxFloatingWindow *)pWindow)->Initialize(pInfo);
}


