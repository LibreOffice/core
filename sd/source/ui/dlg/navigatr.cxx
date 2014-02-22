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

#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>

#include <vcl/menu.hxx>
#include <vcl/settings.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/sfxresid.hxx>

#include "pres.hxx"
#include "navigatr.hxx"
#include "navigatr.hrc"
#include "pgjump.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "slideshow.hxx"
#include "FrameView.hxx"
#include "helpids.h"

namespace {
static const sal_uInt16 nShowNamedShapesFilter=1;
static const sal_uInt16 nShowAllShapesFilter=2;
}

/**
 * SdNavigatorWin - FloatingWindow
 */
SdNavigatorWin::SdNavigatorWin(
    ::Window* pParent,
    ::sd::NavigatorChildWindow* pChWinCtxt,
    const SdResId& rSdResId,
    SfxBindings* pInBindings,
    const UpdateRequestFunctor& rUpdateRequest)
    : ::Window( pParent, rSdResId )
    , maToolbox ( this, SdResId( 1 ) )
    , maTlbObjects( this, SdResId( TLB_OBJECTS ) )
    , maLbDocs ( this, SdResId( LB_DOCS ) )
    , mpChildWinContext( pChWinCtxt )
    , mbDocImported ( sal_False )
      
    , meDragType ( NAVIGATOR_DRAGTYPE_EMBEDDED )
    , mpBindings ( pInBindings )
    , maImageList ( SdResId( IL_NAVIGATR ) )
{
    maTlbObjects.SetViewFrame( mpBindings->GetDispatcher()->GetFrame() );

    FreeResource();

    maTlbObjects.SetAccessibleName(SD_RESSTR(STR_OBJECTS_TREE));

    mpNavigatorCtrlItem = new SdNavigatorControllerItem( SID_NAVIGATOR_STATE, this, mpBindings, rUpdateRequest);
    mpPageNameCtrlItem = new SdPageNameControllerItem( SID_NAVIGATOR_PAGENAME, this, mpBindings, rUpdateRequest);

    ApplyImageList(); 

    Size aTbxSize( maToolbox.CalcWindowSizePixel() );
    maToolbox.SetOutputSizePixel( aTbxSize );
    maToolbox.SetSelectHdl( LINK( this, SdNavigatorWin, SelectToolboxHdl ) );
    maToolbox.SetClickHdl( LINK( this, SdNavigatorWin, ClickToolboxHdl ) );
    maToolbox.SetDropdownClickHdl( LINK(this, SdNavigatorWin, DropdownClickToolBoxHdl) );
    maToolbox.SetItemBits( TBI_DRAGTYPE, maToolbox.GetItemBits( TBI_DRAGTYPE ) | TIB_DROPDOWNONLY );

    
    maToolbox.SetItemBits(
        TBI_SHAPE_FILTER,
        maToolbox.GetItemBits(TBI_SHAPE_FILTER) | TIB_DROPDOWNONLY);

    
    
    long nListboxYPos = maToolbox.GetPosPixel().Y() + maToolbox.GetSizePixel().Height() + 4;
    maTlbObjects.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );
    maTlbObjects.SetDoubleClickHdl( LINK( this, SdNavigatorWin, ClickObjectHdl ) );
    maTlbObjects.SetSelectionMode( SINGLE_SELECTION );
    
    
    maTlbObjects.GrabFocus();
    maTlbObjects.SetSdNavigatorWinFlag(sal_True);

    
    maLbDocs.SetSelectHdl( LINK( this, SdNavigatorWin, SelectDocumentHdl ) );
    
    nListboxYPos = maTlbObjects.GetPosPixel().Y() + maTlbObjects.GetSizePixel().Height() + 4;
    maLbDocs.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );

    
    {
        const Size aTlbSize( maTlbObjects.GetOutputSizePixel() );
        if ( aTlbSize.Width() > aTbxSize.Width() )
        {
            maToolbox.setPosSizePixel( 0, 0, aTlbSize.Width(), 0, WINDOW_POSSIZE_WIDTH );
            aTbxSize = maToolbox.GetOutputSizePixel();
        }
    }

    
    const long nFullHeight = nListboxYPos + maLbDocs.GetSizePixel().Height() + 4;
    maSize = GetOutputSizePixel();
    if( maSize.Height() < nFullHeight )
    {
        maSize.Height() = nFullHeight;
        SetOutputSizePixel( maSize );
    }
    maMinSize = maSize;
    const long nMinWidth = 2*maToolbox.GetPosPixel().X() + aTbxSize.Width(); 
    if( nMinWidth > maMinSize.Width() )
        maMinSize.Width() = nMinWidth;
    maMinSize.Height() -= 40;
    SfxDockingWindow* pDockingParent = dynamic_cast<SfxDockingWindow*>(GetParent());
    if (pDockingParent != NULL)
        pDockingParent->SetMinOutputSizePixel( maMinSize );

    
    if (rUpdateRequest)
        rUpdateRequest();
}



SdNavigatorWin::~SdNavigatorWin()
{
    delete mpNavigatorCtrlItem;
    delete mpPageNameCtrlItem;
}




void SdNavigatorWin::FreshTree( const SdDrawDocument* pDoc )
{
    SdDrawDocument* pNonConstDoc = (SdDrawDocument*) pDoc; 
    sd::DrawDocShell* pDocShell = pNonConstDoc->GetDocSh();
    OUString aDocShName( pDocShell->GetName() );
    OUString aDocName = pDocShell->GetMedium()->GetName();
    maTlbObjects.SetSaveTreeItemStateFlag(sal_True); 
    maTlbObjects.Clear();
    maTlbObjects.Fill( pDoc, sal_False, aDocName ); 
    maTlbObjects.SetSaveTreeItemStateFlag(sal_False); 
    RefreshDocumentLB();
    maLbDocs.SelectEntry( aDocShName );
}

void SdNavigatorWin::FreshEntry( )
{
    maTlbObjects.FreshCurEntry();
}

void SdNavigatorWin::InitTreeLB( const SdDrawDocument* pDoc )
{
    SdDrawDocument* pNonConstDoc = (SdDrawDocument*) pDoc; 
    ::sd::DrawDocShell* pDocShell = pNonConstDoc->GetDocSh();
    OUString aDocShName( pDocShell->GetName() );
    ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

    
    
    if (pViewShell != NULL)
    {
        ::sd::FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != NULL)
            maTlbObjects.SetShowAllShapes(pFrameView->IsNavigatorShowingAllShapes(), false);
    }

    
    
    if (pViewShell!=NULL && sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ))
        maToolbox.EnableItem(TBI_SHAPE_FILTER, false);
    else
        maToolbox.EnableItem(TBI_SHAPE_FILTER);

    if( !maTlbObjects.IsEqualToDoc( pDoc ) )
    {
        OUString aDocName = pDocShell->GetMedium()->GetName();
        maTlbObjects.Clear();
        maTlbObjects.Fill( pDoc, (sal_Bool) sal_False, aDocName ); 

        RefreshDocumentLB();
        maLbDocs.SelectEntry( aDocShName );
    }
    else
    {
        maLbDocs.SetNoSelection();
        maLbDocs.SelectEntry( aDocShName );



        {
            RefreshDocumentLB();
            maLbDocs.SelectEntry( aDocShName );
        }
    }

    SfxViewFrame* pViewFrame = ( ( pViewShell && pViewShell->GetViewFrame() ) ? pViewShell->GetViewFrame() : SfxViewFrame::Current() );
    if( pViewFrame )
        pViewFrame->GetBindings().Invalidate(SID_NAVIGATOR_PAGENAME, sal_True, sal_True);
}

/**
 * DragType is set on dependance if a Drag is even possible. For example,
 * under certain circumstances, it is not allowed to drag graphics (#31038#).
 */
NavigatorDragType SdNavigatorWin::GetNavigatorDragType()
{
    NavigatorDragType   eDT = meDragType;
    NavDocInfo*         pInfo = GetDocInfo();

    if( ( eDT == NAVIGATOR_DRAGTYPE_LINK ) && ( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() ) )
        eDT = NAVIGATOR_DRAGTYPE_NONE;

    return( eDT );
}


sd::DrawDocShell* SdNavigatorWin::GetDrawDocShell( const SdDrawDocument* pDoc )
{
    if( !pDoc )
        return NULL; 
    sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
    return pDocShell;
}

IMPL_LINK_NOARG(SdNavigatorWin, SelectToolboxHdl)
{
    sal_uInt16 nId = maToolbox.GetCurItemId();
    sal_uInt16 nSId = 0;
    PageJump ePage = PAGE_NONE;

    switch( nId )
    {
        case TBI_PEN:
        {
            if( nId == TBI_PEN )
            {
                nSId = SID_NAVIGATOR_PEN;
            }

            if( nSId > 0 )
            {
                SfxBoolItem aItem( nSId, true );
                mpBindings->GetDispatcher()->Execute(
                    nSId, SFX_CALLMODE_SLOT |SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
        break;

        case TBI_FIRST:
        case TBI_PREVIOUS:
        case TBI_NEXT:
        case TBI_LAST:
        {
            if( nId == TBI_FIRST )
                ePage = PAGE_FIRST;
            else if( nId == TBI_PREVIOUS )
                ePage = PAGE_PREVIOUS;
            else if( nId == TBI_NEXT )
                ePage = PAGE_NEXT;
            else if( nId == TBI_LAST )
                ePage = PAGE_LAST;

            if( ePage != PAGE_NONE )
            {
                SfxUInt16Item aItem( SID_NAVIGATOR_PAGE, (sal_uInt16)ePage );
                mpBindings->GetDispatcher()->Execute(
                    SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
        break;
    }
    return 0;
}




IMPL_LINK_NOARG(SdNavigatorWin, ClickToolboxHdl)
{
    return 0;
}



IMPL_LINK( SdNavigatorWin, DropdownClickToolBoxHdl, ToolBox*, pBox )
{
    sal_uInt16 nId = maToolbox.GetCurItemId();

    switch( nId )
    {
        case TBI_DRAGTYPE:
        {
            
            PopupMenu *pMenu = new PopupMenu;

            static const char* aHIDs[] =
            {
                 HID_SD_NAVIGATOR_MENU1,
                 HID_SD_NAVIGATOR_MENU2,
                 HID_SD_NAVIGATOR_MENU3,
                 0
            };

            for( sal_uInt16 nID = NAVIGATOR_DRAGTYPE_URL;
                 nID < NAVIGATOR_DRAGTYPE_COUNT;
                 nID++ )
            {
                sal_uInt16 nRId = GetDragTypeSdResId( (NavigatorDragType)nID );
                if( nRId > 0 )
                {
                    DBG_ASSERT(aHIDs[nID-NAVIGATOR_DRAGTYPE_URL],"HelpId not added!");
                    pMenu->InsertItem( nID, SD_RESSTR( nRId ) );
                    pMenu->SetHelpId( nID, aHIDs[nID - NAVIGATOR_DRAGTYPE_URL] );
                }

            }
            NavDocInfo* pInfo = GetDocInfo();

            if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() )
            {
                pMenu->EnableItem( NAVIGATOR_DRAGTYPE_LINK, false );
                pMenu->EnableItem( NAVIGATOR_DRAGTYPE_URL, false );
                meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
            }

            pMenu->CheckItem( (sal_uInt16)meDragType );
            pMenu->SetSelectHdl( LINK( this, SdNavigatorWin, MenuSelectHdl ) );

            pMenu->Execute( this, maToolbox.GetItemRect( nId ), POPUPMENU_EXECUTE_DOWN );
            pBox->EndSelection();
            delete pMenu;
        }
        break;

        case TBI_SHAPE_FILTER:
        {
            PopupMenu *pMenu = new PopupMenu;

            pMenu->InsertItem(
                nShowNamedShapesFilter,
                SD_RESSTR(STR_NAVIGATOR_SHOW_NAMED_SHAPES));
            pMenu->InsertItem(
                nShowAllShapesFilter,
                SD_RESSTR(STR_NAVIGATOR_SHOW_ALL_SHAPES));

            if (maTlbObjects.GetShowAllShapes())
                pMenu->CheckItem(nShowAllShapesFilter);
            else
                pMenu->CheckItem(nShowNamedShapesFilter);
            pMenu->SetSelectHdl( LINK( this, SdNavigatorWin, ShapeFilterCallback ) );

            pMenu->Execute( this, maToolbox.GetItemRect( nId ), POPUPMENU_EXECUTE_DOWN );
            pBox->EndSelection();
            delete pMenu;
        }
        break;
    }
    return 0;
}



IMPL_LINK_NOARG(SdNavigatorWin, ClickObjectHdl)
{
    if( !mbDocImported || maLbDocs.GetSelectEntryPos() != 0 )
    {
        NavDocInfo* pInfo = GetDocInfo();

        
        if( pInfo && pInfo->IsActive() )
        {
            OUString aStr( maTlbObjects.GetSelectEntry() );

            if( !aStr.isEmpty() )
            {
                SfxStringItem aItem( SID_NAVIGATOR_OBJECT, aStr );
                mpBindings->GetDispatcher()->Execute(
                    SID_NAVIGATOR_OBJECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );
                
                maTlbObjects.MarkCurEntry(aStr);

                
                
                
                SfxViewShell* pCurSh = SfxViewShell::Current();

                if ( pCurSh )
                {
                    Window* pShellWnd = pCurSh->GetWindow();
                    if ( pShellWnd )
                        pShellWnd->GrabFocus();
                }
            }
        }
    }
    return( 0L );
}



IMPL_LINK_NOARG(SdNavigatorWin, SelectDocumentHdl)
{
    OUString aStrLb = maLbDocs.GetSelectEntry();
    long   nPos = maLbDocs.GetSelectEntryPos();
    sal_Bool   bFound = sal_False;
    ::sd::DrawDocShell* pDocShell = NULL;
    NavDocInfo* pInfo = GetDocInfo();

    
    if( mbDocImported && nPos == 0 )
    {
        
        InsertFile( aStrLb );
    }
    else if (pInfo)
    {
        pDocShell = pInfo->mpDocShell;

        bFound = sal_True;
    }

    if( bFound )
    {
        SdDrawDocument* pDoc = pDocShell->GetDoc();
        if( !maTlbObjects.IsEqualToDoc( pDoc ) )
        {
            SdDrawDocument* pNonConstDoc = (SdDrawDocument*) pDoc; 
            ::sd::DrawDocShell* pNCDocShell = pNonConstDoc->GetDocSh();
            OUString aDocName = pNCDocShell->GetMedium()->GetName();
            maTlbObjects.Clear();
            maTlbObjects.Fill( pDoc, (sal_Bool) sal_False, aDocName ); 
        }
    }

    
    if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() || ( meDragType != NAVIGATOR_DRAGTYPE_EMBEDDED ) )
    {
        meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
        SetDragImage();
    }

    return( 0L );
}

/**
 * Set DrageType and set image accordingly to it.
 * If the handler is called with NULL, the default (URL) is set.
 */
IMPL_LINK( SdNavigatorWin, MenuSelectHdl, Menu *, pMenu )
{
    sal_uInt16 nMenuId;
    if( pMenu )
        nMenuId = pMenu->GetCurItemId();
    else
        nMenuId = NAVIGATOR_DRAGTYPE_URL;

    if( nMenuId != USHRT_MAX ) 
    {
        NavigatorDragType eDT = (NavigatorDragType) nMenuId;
        if( meDragType != eDT )
        {
            meDragType = eDT;
            SetDragImage();

            if( meDragType == NAVIGATOR_DRAGTYPE_URL )
            {
                
                if( maTlbObjects.GetSelectionCount() > 1 )
                    maTlbObjects.SelectAll( sal_False );

                maTlbObjects.SetSelectionMode( SINGLE_SELECTION );
            }
            else
                maTlbObjects.SetSelectionMode( MULTIPLE_SELECTION );
        }
    }
    return( 0 );
}




IMPL_LINK( SdNavigatorWin, ShapeFilterCallback, Menu *, pMenu )
{
    if (pMenu != NULL)
    {
        bool bShowAllShapes (maTlbObjects.GetShowAllShapes());
        sal_uInt16 nMenuId (pMenu->GetCurItemId());
        switch (nMenuId)
        {
            case nShowNamedShapesFilter:
                bShowAllShapes = false;
                break;

            case nShowAllShapesFilter:
                bShowAllShapes = true;
                break;

            default:
                OSL_FAIL(
                    "SdNavigatorWin::ShapeFilterCallback called for unknown menu entry");
                break;
        }

        maTlbObjects.SetShowAllShapes(bShowAllShapes, true);

        
        NavDocInfo* pInfo = GetDocInfo();
        if (pInfo != NULL)
        {
            ::sd::DrawDocShell* pDocShell = pInfo->mpDocShell;
            if (pDocShell != NULL)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell != NULL)
                {
                    ::sd::FrameView* pFrameView = pViewShell->GetFrameView();
                    if (pFrameView != NULL)
                    {
                        pFrameView->SetIsNavigatorShowingAllShapes(bShowAllShapes);
                    }
                }
            }
        }
    }

    return 0;
}



void SdNavigatorWin::Resize()
{
    Size aWinSize( GetOutputSizePixel() );
    if( aWinSize.Height() >= maMinSize.Height() )
        
    {
        Size aDiffSize;
        aDiffSize.Width() = aWinSize.Width() - maSize.Width();
        aDiffSize.Height() = aWinSize.Height() - maSize.Height();

        
        Size aObjSize( maToolbox.GetOutputSizePixel() );
        aObjSize.Width() += aDiffSize.Width();
        maToolbox.SetOutputSizePixel( aObjSize );

        
        aObjSize = maTlbObjects.GetSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        aObjSize.Height() = maLbDocs.GetPosPixel().Y() + aDiffSize.Height() -
            maTlbObjects.GetPosPixel().Y() - 4;
        maTlbObjects.SetSizePixel( aObjSize );

        Point aPt( 0, aDiffSize.Height() );

        
        maLbDocs.Hide();
        aObjSize = maLbDocs.GetOutputSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        maLbDocs.SetPosPixel( maLbDocs.GetPosPixel() + aPt );
        maLbDocs.SetOutputSizePixel( aObjSize );
        maLbDocs.Show();

        maSize = aWinSize;
    }
    Window::Resize();
}



bool SdNavigatorWin::InsertFile(const OUString& rFileName)
{
    INetURLObject   aURL( rFileName );

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        OUString aURLStr;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aURLStr );
        aURL = INetURLObject( aURLStr );
    }

    
    OUString aFileName( aURL.GetMainURL( INetURLObject::NO_DECODE ) );

    if (aFileName.isEmpty())
    {
        
        maDropFileName = aFileName;
    }
    else
    {
        
        const SfxFilter* pFilter = NULL;
        ErrCode nErr = 0;

        if (aFileName != maDropFileName)
        {
            SfxMedium aMed(aFileName, (STREAM_READ | STREAM_SHARE_DENYNONE));
            SfxFilterMatcher aMatch( OUString("simpress") );
            aMed.UseInteractionHandler( sal_True );
            nErr = aMatch.GuessFilter(aMed, &pFilter);
        }

        if ((pFilter && !nErr) || aFileName == maDropFileName)
        {
            
            
            SfxMedium* pMedium = new SfxMedium( aFileName,
                                                STREAM_READ | STREAM_NOCREATE);

            if (pMedium->IsStorage())
            {
                
                
                
                SdDrawDocument* pDropDoc = maTlbObjects.GetBookmarkDoc(pMedium);

                if (pDropDoc)
                {
                    maTlbObjects.Clear();
                    maDropFileName = aFileName;

                    if( !maTlbObjects.IsEqualToDoc( pDropDoc ) )
                    {
                        
                        maTlbObjects.Fill(pDropDoc, (sal_Bool) sal_False, maDropFileName);
                        RefreshDocumentLB( &maDropFileName );
                    }
                }
            }
            else
            {
                delete pMedium;
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}



void SdNavigatorWin::RefreshDocumentLB( const OUString* pDocName )
{
    sal_uInt16 nPos = 0;

    if( pDocName )
    {
        if( mbDocImported )
            maLbDocs.RemoveEntry( 0 );

        maLbDocs.InsertEntry( *pDocName, 0 );
        mbDocImported = sal_True;
    }
    else
    {
        nPos = maLbDocs.GetSelectEntryPos();
        if( nPos == LISTBOX_ENTRY_NOTFOUND )
            nPos = 0;

        OUString aStr;
        if( mbDocImported )
            aStr = maLbDocs.GetEntry( 0 );

        maLbDocs.Clear();

        
         maDocList.clear();

        if( mbDocImported )
            maLbDocs.InsertEntry( aStr, 0 );

        ::sd::DrawDocShell* pCurrentDocShell =
              PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current() );
        SfxObjectShell* pSfxDocShell = SfxObjectShell::GetFirst(0, sal_False);
        while( pSfxDocShell )
        {
            ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, pSfxDocShell );
            if( pDocShell  && !pDocShell->IsInDestruction() && ( pDocShell->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED ) )
            {
                NavDocInfo aInfo ;
                aInfo.mpDocShell = pDocShell;

                SfxMedium *pMedium = pDocShell->GetMedium();
                aStr = pMedium ? pMedium->GetName() : OUString();
                if( !aStr.isEmpty() )
                    aInfo.SetName();
                else
                    aInfo.SetName( sal_False );
                
                
                
                aStr = pDocShell->GetName();

                maLbDocs.InsertEntry( aStr, LISTBOX_APPEND );

                if( pDocShell == pCurrentDocShell )
                    aInfo.SetActive();
                else
                    aInfo.SetActive( sal_False );

                maDocList.push_back( aInfo );
            }
            pSfxDocShell = SfxObjectShell::GetNext( *pSfxDocShell, 0 , sal_False );
        }
    }
    maLbDocs.SelectEntryPos( nPos );
}



sal_uInt16 SdNavigatorWin::GetDragTypeSdResId( NavigatorDragType eDT, sal_Bool bImage )
{
    switch( eDT )
    {
        case NAVIGATOR_DRAGTYPE_NONE:
                return( bImage ? 0 : STR_NONE );
        case NAVIGATOR_DRAGTYPE_URL:
                return( bImage ? TBI_HYPERLINK : STR_DRAGTYPE_URL );
        case NAVIGATOR_DRAGTYPE_EMBEDDED:
                return( bImage ? TBI_EMBEDDED : STR_DRAGTYPE_EMBEDDED );
        case NAVIGATOR_DRAGTYPE_LINK:
                return( bImage ? TBI_LINK : STR_DRAGTYPE_LINK );
        default: OSL_FAIL( "No resource for DragType available!" );
    }
    return( 0 );
}



NavDocInfo* SdNavigatorWin::GetDocInfo()
{
    sal_uInt32 nPos = maLbDocs.GetSelectEntryPos();

    if( mbDocImported )
    {
        if( nPos == 0 )
        {
            return( NULL );
        }
        nPos--;
    }

    return nPos < maDocList.size() ? &(maDocList[ nPos ]) : NULL;
}

/**
 * PreNotify
 */
bool SdNavigatorWin::Notify(NotifyEvent& rNEvt)
{
    const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
    bool            nOK = false;

    if( pKEvt )
    {
        if( KEY_ESCAPE == pKEvt->GetKeyCode().GetCode() )
        {
            if( SdPageObjsTLB::IsInDrag() )
            {
                
                nOK = true;
            }
            else
            {
                ::sd::ViewShellBase* pBase = ::sd::ViewShellBase::GetViewShellBase( mpBindings->GetDispatcher()->GetFrame());
                if( pBase )
                {
                    sd::SlideShow::Stop( *pBase );
                    
                    
                    
                    
                    return true;
                }
            }
        }
    }

    if( !nOK )
        nOK = Window::Notify( rNEvt );

    return( nOK );
}


/**
 * catch ESCAPE in order to end show
 */
void SdNavigatorWin::KeyInput( const KeyEvent& rKEvt )
{
    long nOK = sal_False;

    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        if( SdPageObjsTLB::IsInDrag() )
        {
            
            nOK = sal_True;
        }
        else
        {
            ::sd::ViewShellBase* pBase = ::sd::ViewShellBase::GetViewShellBase( mpBindings->GetDispatcher()->GetFrame());
            if(pBase)
            {
                ::sd::SlideShow::Stop( *pBase );
            }
        }
    }

    if (!nOK)
    {
        Window::KeyInput(rKEvt);
    }
}

void SdNavigatorWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}

void SdNavigatorWin::SetDragImage()
{
    maToolbox.SetItemImage( TBI_DRAGTYPE, maToolbox.GetImageList().GetImage( GetDragTypeSdResId( meDragType, sal_True ) ) );
}

void SdNavigatorWin::ApplyImageList()
{

    maToolbox.SetImageList( maImageList );
    maToolbox.SetItemImage(TBI_SHAPE_FILTER, Image(BitmapEx(SdResId(BMP_GRAPHIC))));

    SetDragImage();
}



/**
 * ControllerItem for Navigator
 */
SdNavigatorControllerItem::SdNavigatorControllerItem(
    sal_uInt16 _nId,
    SdNavigatorWin* pNavWin,
    SfxBindings*    _pBindings,
    const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest)
    : SfxControllerItem( _nId, *_pBindings ),
      pNavigatorWin( pNavWin ),
      maUpdateRequest(rUpdateRequest)
{
}



void SdNavigatorControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_NAVIGATOR_STATE )
    {
        const SfxUInt32Item* pStateItem = PTR_CAST( SfxUInt32Item, pItem );
        DBG_ASSERT( pStateItem, "SfxUInt16Item expected");
        sal_uInt32 nState = pStateItem->GetValue();

        
        if( nState & NAVBTN_PEN_ENABLED &&
            !pNavigatorWin->maToolbox.IsItemEnabled( TBI_PEN ) )
            pNavigatorWin->maToolbox.EnableItem( TBI_PEN );
        if( nState & NAVBTN_PEN_DISABLED &&
            pNavigatorWin->maToolbox.IsItemEnabled( TBI_PEN ) )
            pNavigatorWin->maToolbox.EnableItem( TBI_PEN, false );
        if( nState & NAVBTN_PEN_CHECKED &&
            !pNavigatorWin->maToolbox.IsItemChecked( TBI_PEN ) )
            pNavigatorWin->maToolbox.CheckItem( TBI_PEN );
        if( nState & NAVBTN_PEN_UNCHECKED &&
            pNavigatorWin->maToolbox.IsItemChecked( TBI_PEN ) )
            pNavigatorWin->maToolbox.CheckItem( TBI_PEN, false );

        
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            
            if( nState & NAVBTN_FIRST_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_FIRST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_FIRST );
            if( nState & NAVBTN_FIRST_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_FIRST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_FIRST, false );

            
            if( nState & NAVBTN_PREV_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_PREVIOUS ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_PREVIOUS );
            if( nState & NAVBTN_PREV_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_PREVIOUS ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_PREVIOUS, false );

            
            if( nState & NAVBTN_LAST_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_LAST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_LAST );
            if( nState & NAVBTN_LAST_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_LAST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_LAST, false );

            
            if( nState & NAVBTN_NEXT_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_NEXT ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_NEXT );
            if( nState & NAVBTN_NEXT_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_NEXT ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_NEXT, false );

            if( nState & NAVTLB_UPDATE )
            {
                
                if (maUpdateRequest)
                    maUpdateRequest();
            }
        }
    }
}

/**
 * ControllerItem for Navigator to show page in TreeLB
 */
SdPageNameControllerItem::SdPageNameControllerItem(
    sal_uInt16 _nId,
    SdNavigatorWin* pNavWin,
    SfxBindings*    _pBindings,
    const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest)
    : SfxControllerItem( _nId, *_pBindings ),
      pNavigatorWin( pNavWin ),
      maUpdateRequest(rUpdateRequest)
{
}



void SdPageNameControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_NAVIGATOR_PAGENAME )
    {
        
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            const SfxStringItem* pStateItem = PTR_CAST( SfxStringItem, pItem );
            DBG_ASSERT( pStateItem, "SfxStringItem expected");
            OUString aPageName = pStateItem->GetValue();

            if( !pNavigatorWin->maTlbObjects.HasSelectedChildren( aPageName ) )
            {
                if( pNavigatorWin->maTlbObjects.GetSelectionMode() == MULTIPLE_SELECTION )
                {
                    
                    pNavigatorWin->maTlbObjects.SelectAll( sal_False );
                }
                pNavigatorWin->maTlbObjects.SelectEntry( aPageName );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
