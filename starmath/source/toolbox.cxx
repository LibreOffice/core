/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toolbox.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:13:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXIMGMGR_HXX //autogen
#include <sfx2/imgmgr.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef TOOLBOX_HXX
#include "toolbox.hxx"
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif
#ifndef _TOOLBOX_HRC_
#include "toolbox.hrc"
#endif
#include "view.hxx"


////////////////////////////////////////////////////////////

static USHORT  GetImageListRID( USHORT nCategoryRID, BOOL bHighContrast )
{
    USHORT nRes = 0xFFFF;
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


static sal_Int16  GetToolBoxCategoriesIndex( USHORT nCategoryRID )
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


static USHORT  GetCategoryRID( USHORT nResId )
{
    USHORT nRes = 0xFFFF;
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
                DBG_ERROR( "unkown category" );
    }
    return nRes;
}


////////////////////////////////////////////////////////////


SmToolBoxWindow::SmToolBoxWindow(SfxBindings *pBindings,
                                 SfxChildWindow *pChildWindow,
                                 Window *pParent) :
    SfxFloatingWindow(pBindings, pChildWindow, pParent, SmResId(RID_TOOLBOXWINDOW)),
    aToolBoxCat(this, ResId(NUM_TBX_CATEGORIES + 1)),
    aToolBoxCat_Delim(this, ResId( FL_TOOLBOX_CAT_DELIM ))
{
    RTL_LOGFILE_CONTEXT( aLog, "starmath: SmToolBoxWindow::SmToolBoxWindow" );

    // allow for cursor travelling between toolbox and sub-categories
    SetStyle( GetStyle() | WB_DIALOGCONTROL );

    nActiveCategoryRID = -1;

    aToolBoxCat.SetClickHdl(LINK(this, SmToolBoxWindow, CategoryClickHdl));

    int i;
    for (i = 0;  i < NUM_TBX_CATEGORIES;  i++)
    {
        ToolBox *pBox = new ToolBox(this, ResId (i+1));
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

    ApplyImageLists( RID_UNBINOPS_CAT );
    SetCategory( RID_UNBINOPS_CAT );
}

SmToolBoxWindow::~SmToolBoxWindow()
{
    int i;
    for (i = 0;  i < NUM_TBX_CATEGORIES;  i++)
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


const ImageList * SmToolBoxWindow::GetImageList( USHORT nResId, BOOL bHighContrast )
{
    // creates the image list via its resource id and stores that
    // list for later use in the respective array.

    const ImageList *pIL = 0;

    // get index to use
    USHORT nCategoryRID = GetCategoryRID( nResId );
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


void SmToolBoxWindow::ApplyImageLists( USHORT nCategoryRID )
{
    BOOL bHighContrast = GetDisplayBackground().GetColor().IsDark() != 0;

    // set image list for toolbox 'catalog'
    const ImageList *pImageList = GetImageList( bHighContrast ? RID_ILH_CATALOG : RID_IL_CATALOG, bHighContrast );
    DBG_ASSERT( pImageList, "image list missing" );
    if (pImageList)
        aToolBoxCat.SetImageList( *pImageList );

    // set image list for active (visible) category of 'catalog'
    sal_Int16 nIdx = GetToolBoxCategoriesIndex( nCategoryRID );
    USHORT nResId = GetImageListRID( nCategoryRID, bHighContrast );
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
    static BOOL bSetPosition = TRUE;
    if (STATE_CHANGE_INITSHOW == nStateChange)
    {
        // calculate initial position to be used after creation of the window...
        AdjustPosSize( bSetPosition );
        bSetPosition = FALSE;

        SetCategory(RID_UNBINOPS_CAT);
    }
    //... otherwise the base class will remember the last position of the window
    SfxFloatingWindow::StateChanged( nStateChange );
}


void SmToolBoxWindow::AdjustPosSize( BOOL bSetPos )
{
    Size aCatSize( aToolBoxCat.CalcWindowSizePixel( 2 ) );
    Size aCmdSize( pToolBoxCmd->CalcWindowSizePixel( 5 ) );
    DBG_ASSERT( aCatSize.Width() == aCmdSize.Width(), "width mismatch" );

    // catalog settings
    aToolBoxCat.SetPosPixel( Point(0, 3) );
    aToolBoxCat.SetSizePixel( aCatSize );
    // settings for catalog / category delimiter
    Point aP( aToolBoxCat_Delim.GetPosPixel() );
    aP.X() += 5;
    aToolBoxCat_Delim.SetPosPixel( aP );
    Size  aS( aCatSize.Width() - 10, 10 );
    aToolBoxCat_Delim.SetSizePixel( aS );
    // category settings
    aP.X() = 0;
    aP.Y() += aToolBoxCat_Delim.GetSizePixel().Height();
    for (int i = 0;  i < NUM_TBX_CATEGORIES;  i++)
    {
        vToolBoxCategories[i]->SetPosPixel( aP );
        vToolBoxCategories[i]->SetSizePixel( aCmdSize );
    }
    // main window settings
    Size    aWndSize ( aCatSize.Width(), pToolBoxCmd->GetPosPixel().Y() + pToolBoxCmd->GetSizePixel().Height() + 3);
    SetOutputSizePixel( aWndSize );

    if (bSetPos)
    {
        SmViewShell *pView = SmGetActiveView();
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


BOOL SmToolBoxWindow::Close()
{
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_TOOLBOX, SFX_CALLMODE_STANDARD,
                new SfxBoolItem(SID_TOOLBOX, FALSE), 0L);
    return TRUE;
}

void SmToolBoxWindow::GetFocus()
{
    // give focus to category toolbox
    // (allow for cursor travelling when a category is selected with the mouse)
    aToolBoxCat.GrabFocus();
}

void SmToolBoxWindow::SetCategory(USHORT nCategoryRID)
{
    if (nCategoryRID != nActiveCategoryRID)
    {
        ApplyImageLists( nCategoryRID );

        USHORT nLines;
        // check for valid resource id
        switch (nCategoryRID)
        {
            case RID_UNBINOPS_CAT :     nLines = 4; break;
            case RID_RELATIONS_CAT:     nLines = 5; break;
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
            aToolBoxCat.CheckItem(nActiveCategoryRID, FALSE);
        nActiveCategoryRID = nCategoryRID;
        aToolBoxCat.CheckItem(nActiveCategoryRID, TRUE);

        pToolBoxCmd->Show();
    }
}


IMPL_LINK_INLINE_START( SmToolBoxWindow, CategoryClickHdl, ToolBox*, pToolBox)
{
    int nItemId = pToolBox->GetCurItemId();
    if (nItemId != 0)
        SetCategory( nItemId );
    return 0;
}
IMPL_LINK_INLINE_END( SmToolBoxWindow, CategoryClickHdl, ToolBox*, pToolBox)


IMPL_LINK_INLINE_START( SmToolBoxWindow, CmdSelectHdl, ToolBox*, pToolBox)
{
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                SID_INSERTCOMMAND, SFX_CALLMODE_STANDARD,
                new SfxInt16Item(SID_INSERTCOMMAND, pToolBox->GetCurItemId()), 0L);
    return 0;
}
IMPL_LINK_INLINE_END( SmToolBoxWindow, CmdSelectHdl, ToolBox*, pToolBox)


/**************************************************************************/

SFX_IMPL_FLOATINGWINDOW(SmToolBoxWrapper, SID_TOOLBOXWINDOW);

SmToolBoxWrapper::SmToolBoxWrapper(Window *pParentWindow,
                                   USHORT nId, SfxBindings* pBindings,
                                   SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pWindow = new SmToolBoxWindow(pBindings, this, pParentWindow);
    ((SfxFloatingWindow *)pWindow)->Initialize(pInfo);
}


