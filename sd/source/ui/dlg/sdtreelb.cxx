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


#include <sal/types.h>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <vcl/msgbox.hxx>
#include <svl/urihelper.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include "cusshow.hxx"
#include <sfx2/childwin.hxx>

#include <sfx2/viewfrm.hxx>

#include "strmname.h"
#include "sdtreelb.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "navigatr.hxx"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "customshowlist.hxx"
#include "ViewShell.hxx"
#include "DrawController.hxx"
#include "ViewShellBase.hxx"

#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <svtools/embedtransfer.hxx>
#include "svtools/treelistentry.hxx"
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>

using namespace com::sun::star;

class SdPageObjsTLB::IconProvider
{
public:
    IconProvider (void);

    // Regular icons.
    Image maImgPage;
    Image maImgPageExcl;
    Image maImgPageObjsExcl;
    Image maImgPageObjs;
    Image maImgObjects;
    Image maImgGroup;
};


sal_Bool SAL_DLLPRIVATE SdPageObjsTLB::bIsInDrag = sal_False;

sal_Bool SdPageObjsTLB::IsInDrag()
{
    return bIsInDrag;
}

sal_uInt32 SdPageObjsTLB::SdPageObjsTransferable::mnListBoxDropFormatId = SAL_MAX_UINT32;

// -----------------------------------------
// - SdPageObjsTLB::SdPageObjsTransferable -
// -----------------------------------------

SdPageObjsTLB::SdPageObjsTransferable::SdPageObjsTransferable(
    SdPageObjsTLB& rParent,
        const INetBookmark& rBookmark,
    ::sd::DrawDocShell& rDocShell,
    NavigatorDragType eDragType,
    const ::com::sun::star::uno::Any& rTreeListBoxData )
    : SdTransferable(rDocShell.GetDoc(), NULL, sal_True),
      mrParent( rParent ),
      maBookmark( rBookmark ),
      mrDocShell( rDocShell ),
      meDragType( eDragType ),
      maTreeListBoxData( rTreeListBoxData )
{
}




SdPageObjsTLB::SdPageObjsTransferable::~SdPageObjsTransferable()
{
}

// -----------------------------------------------------------------------------

void SdPageObjsTLB::SdPageObjsTransferable::AddSupportedFormats()
{
    AddFormat(SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK);
    AddFormat(SOT_FORMATSTR_ID_TREELISTBOX);
    AddFormat(GetListBoxDropFormatId());
}

// -----------------------------------------------------------------------------

sal_Bool SdPageObjsTLB::SdPageObjsTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_uLong nFormatId = SotExchange::GetFormat( rFlavor );
    switch (nFormatId)
    {
        case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
            SetINetBookmark( maBookmark, rFlavor );
            return sal_True;

        case SOT_FORMATSTR_ID_TREELISTBOX:
            SetAny(maTreeListBoxData, rFlavor);
            return sal_True;

        default:
            return sal_False;
    }
}

// -----------------------------------------------------------------------------

void SdPageObjsTLB::SdPageObjsTransferable::DragFinished( sal_Int8 nDropAction )
{
    mrParent.OnDragFinished( nDropAction );
    SdTransferable::DragFinished(nDropAction);
}

// -----------------------------------------------------------------------------

::sd::DrawDocShell& SdPageObjsTLB::SdPageObjsTransferable::GetDocShell() const
{
    return mrDocShell;
}

// -----------------------------------------------------------------------------

NavigatorDragType SdPageObjsTLB::SdPageObjsTransferable::GetDragType() const
{
    return meDragType;
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL SdPageObjsTLB::SdPageObjsTransferable::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = (sal_Int64)(sal_IntPtr)this;
    }
    else
        nRet = SdTransferable::getSomething(rId);

    return nRet;
}

namespace
{
    class theSdPageObjsTLBUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdPageObjsTLBUnoTunnelId > {};
}

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdPageObjsTLB::SdPageObjsTransferable::getUnoTunnelId()
{
    return theSdPageObjsTLBUnoTunnelId::get().getSeq();
}

// -----------------------------------------------------------------------------

SdPageObjsTLB::SdPageObjsTransferable* SdPageObjsTLB::SdPageObjsTransferable::getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData )
    throw()
{
    try
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( rxData, ::com::sun::star::uno::UNO_QUERY_THROW );

        return reinterpret_cast<SdPageObjsTLB::SdPageObjsTransferable*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xUnoTunnel->getSomething( SdPageObjsTLB::SdPageObjsTransferable::getUnoTunnelId()) ) );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
    return 0;
}


sal_uInt32 SdPageObjsTLB::SdPageObjsTransferable::GetListBoxDropFormatId (void)
{
    if (mnListBoxDropFormatId == SAL_MAX_UINT32)
        mnListBoxDropFormatId = SotExchange::RegisterFormatMimeType(OUString(
        "application/x-openoffice-treelistbox-moveonly;windows_formatname=\"SV_LBOX_DD_FORMAT_MOVE\""));
    return mnListBoxDropFormatId;
}


SdPageObjsTLB::SdPageObjsTLB( Window* pParentWin, const SdResId& rSdResId )
:   SvTreeListBox       ( pParentWin, rSdResId )
,   mpParent            ( pParentWin )
,   mpDoc               ( NULL )
,   mpBookmarkDoc       ( NULL )
,   mpMedium            ( NULL )
,   mpOwnMedium         ( NULL )
,   maImgOle             ( BitmapEx( SdResId( BMP_OLE ) ) )
,   maImgGraphic         ( BitmapEx( SdResId( BMP_GRAPHIC ) ) )
,   mbLinkableSelected  ( sal_False )
,   mpDropNavWin        ( NULL )
,   mbShowAllShapes     ( false )
,   mbShowAllPages      ( false )

{
    // add lines to Tree-ListBox
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                           WB_HASBUTTONS | // WB_HASLINESATROOT |
                           WB_HSCROLL |
                           WB_HASBUTTONSATROOT |
                           WB_QUICK_SEARCH /* i31275 */ );
    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND ) ),
                    Bitmap( SdResId( BMP_COLLAPSE ) ) );

    SetDragDropMode(
         SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY |
            SV_DRAGDROP_APP_MOVE  | SV_DRAGDROP_APP_COPY  | SV_DRAGDROP_APP_DROP );
}


SdPageObjsTLB::~SdPageObjsTLB()
{
    if ( mpBookmarkDoc )
        CloseBookmarkDoc();
    else
        // no document was created from mpMedium, so this object is still the owner of it
        delete mpMedium;
}

String SdPageObjsTLB::GetObjectName(
    const SdrObject* pObject,
    const bool bCreate) const
{
    String aRet;

    if ( pObject )
    {
        aRet = pObject->GetName();

        if( !aRet.Len() && pObject->ISA( SdrOle2Obj ) )
            aRet = static_cast< const SdrOle2Obj* >( pObject )->GetPersistName();
    }

    if (bCreate
        && mbShowAllShapes
        && aRet.Len() == 0
        && pObject!=NULL)
    {
        aRet = SD_RESSTR(STR_NAVIGATOR_SHAPE_BASE_NAME);
        aRet.SearchAndReplaceAscii("%1", OUString::number(pObject->GetOrdNum() + 1));
    }

    return aRet;
}

/**
 * select a entry in TreeLB
 */
sal_Bool SdPageObjsTLB::SelectEntry( const String& rName )
{
    sal_Bool bFound = sal_False;

    if( rName.Len() )
    {
        SvTreeListEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = sal_True;
                SetCurEntry( pEntry );
            }
        }
    }
    return( bFound );
}

/**
 * @return true if children of the specified string are selected
 */
sal_Bool SdPageObjsTLB::HasSelectedChildren( const String& rName )
{
    sal_Bool bFound  = sal_False;
    sal_Bool bChildren = sal_False;

    if( rName.Len() )
    {
        SvTreeListEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = sal_True;
                sal_Bool bExpanded = IsExpanded( pEntry );
                long nCount = GetChildSelectionCount( pEntry );
                if( bExpanded && nCount > 0 )
                    bChildren = sal_True;
            }
        }
    }
    return( bChildren );
}


/**
 * Fill TreeLB with pages and objects
 */
void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, sal_Bool bAllPages,
                          const String& rDocName)
{
    String aSelection;
    if( GetSelectionCount() > 0 )
    {
        aSelection = GetSelectEntry();
        Clear();
    }

    mpDoc = pInDoc;
    maDocName = rDocName;
    mbShowAllPages = (bAllPages == sal_True);
    mpMedium = NULL;

    SdPage*      pPage = NULL;

    IconProvider aIconProvider;

    // first insert all pages including objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) mpDoc->GetPage( nPage );
        if(  (mbShowAllPages || pPage->GetPageKind() == PK_STANDARD)
             && !(pPage->GetPageKind()==PK_HANDOUT)   ) //#94954# never list the normal handout page ( handout-masterpage is used instead )
        {
            sal_Bool bPageExluded = pPage->IsExcluded();

            bool bPageBelongsToShow = PageBelongsToCurrentShow (pPage);
            bPageExluded |= !bPageBelongsToShow;

            AddShapeList(*pPage, NULL, pPage->GetName(), bPageExluded, NULL, aIconProvider);
        }
        nPage++;
    }

    // then insert all master pages including objects
    if( mbShowAllPages )
    {
        nPage = 0;
        const sal_uInt16 nMaxMasterPages = mpDoc->GetMasterPageCount();

        while( nPage < nMaxMasterPages )
        {
            pPage = (SdPage*) mpDoc->GetMasterPage( nPage );
            AddShapeList(*pPage, NULL, pPage->GetName(), false, NULL, aIconProvider);
            nPage++;
        }
    }
    if( aSelection.Len() )
        SelectEntry( aSelection );
}

/**
 * We insert only the first entry. Children are created on demand.
 */
void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, SfxMedium* pInMedium,
                          const String& rDocName )
{
    mpDoc = pInDoc;

    // this object now owns the Medium
    mpMedium = pInMedium;
    maDocName = rDocName;

    Image aImgDocOpen=Image( BitmapEx( SdResId( BMP_DOC_OPEN ) ) );
    Image aImgDocClosed=Image( BitmapEx( SdResId( BMP_DOC_CLOSED ) ) );

    // insert document name
    InsertEntry( maDocName, aImgDocOpen, aImgDocClosed, NULL, sal_True, LIST_APPEND,
                 reinterpret_cast< void* >( 1 )
    );
}




void SdPageObjsTLB::AddShapeList (
    const SdrObjList& rList,
    SdrObject* pShape,
    const OUString& rsName,
    const bool bIsExcluded,
    SvTreeListEntry* pParentEntry,
    const IconProvider& rIconProvider)
{
    Image aIcon (rIconProvider.maImgPage);
    if (bIsExcluded)
        aIcon = rIconProvider.maImgPageExcl;
    else if (pShape != NULL)
        aIcon = rIconProvider.maImgGroup;

    void* pUserData (reinterpret_cast<void*>(1));
    if (pShape != NULL)
        pUserData = pShape;

    SvTreeListEntry* pEntry = InsertEntry(
        rsName,
        aIcon,
        aIcon,
        pParentEntry,
        sal_False,
        LIST_APPEND,
        pUserData);

    SdrObjListIter aIter(
        rList,
        !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
        IM_FLAT,
        sal_False /*not reverse*/);

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OSL_ASSERT(pObj!=NULL);

        // Get the shape name.
        String aStr (GetObjectName( pObj ) );

        if( aStr.Len() )
        {
            if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
            {
                InsertEntry(
                    aStr,
                    maImgOle,
                    maImgOle,
                    pEntry,
                    sal_False,
                    LIST_APPEND,
                    pObj
                );
            }
            else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                InsertEntry(
                    aStr,
                    maImgGraphic,
                    maImgGraphic,
                    pEntry,
                    sal_False,
                    LIST_APPEND,
                    pObj
                );
            }
            else if (pObj->IsGroupObject())
            {
                AddShapeList(
                    *pObj->GetSubList(),
                    pObj,
                    aStr,
                    false,
                    pEntry,
                    rIconProvider
                );
            }
            else
            {
                InsertEntry(
                    aStr,
                    rIconProvider.maImgObjects,
                    rIconProvider.maImgObjects,
                    pEntry,
                    sal_False,
                    LIST_APPEND,
                    pObj
                );
            }
        }
    }

    if( pEntry->HasChildren() )
    {
        SetExpandedEntryBmp(
            pEntry,
            bIsExcluded ? rIconProvider.maImgPageObjsExcl : rIconProvider.maImgPageObjs);
        SetCollapsedEntryBmp(
            pEntry,
            bIsExcluded ? rIconProvider.maImgPageObjsExcl : rIconProvider.maImgPageObjs);
    }
}




void SdPageObjsTLB::SetShowAllShapes (
    const bool bShowAllShapes,
    const bool bFillList)
{
    mbShowAllShapes = bShowAllShapes;
    if (bFillList)
    {
        if (mpMedium == NULL)
            Fill(mpDoc, mbShowAllPages, maDocName);
        else
            Fill(mpDoc, mpMedium, maDocName);
    }
}




bool SdPageObjsTLB::GetShowAllShapes (void) const
{
    return mbShowAllShapes;
}




/**
 * Checks if the pages (PK_STANDARD) of a doc and the objects on the pages
 * are identical to the TreeLB.
 * If a doc is provided, this will be the used doc (important by more than
 * one document).
 */
sal_Bool SdPageObjsTLB::IsEqualToDoc( const SdDrawDocument* pInDoc )
{
    if( pInDoc )
        mpDoc = pInDoc;

    if( !mpDoc )
        return( sal_False );

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvTreeListEntry* pEntry = First();
    String       aName;

    // compare all pages including the objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) mpDoc->GetPage( nPage );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            if( !pEntry )
                return( sal_False );
            aName = GetEntryText( pEntry );

            if( pPage->GetName() != aName )
                return( sal_False );

            pEntry = Next( pEntry );

            SdrObjListIter aIter(
                *pPage,
                !pPage->HasObjectNavigationOrder() /* use navigation order, if available */,
                IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();

                const String aObjectName( GetObjectName( pObj ) );

                if( aObjectName.Len() )
                {
                    if( !pEntry )
                        return( sal_False );

                    aName = GetEntryText( pEntry );

                    if( aObjectName != aName )
                        return( sal_False );

                    pEntry = Next( pEntry );
                }
            }
        }
        nPage++;
    }
    // If there are still entries in the listbox,
    // then objects (with names) or pages were deleted
    return( !pEntry );
}

/**
 * @return selected string
 */
String SdPageObjsTLB::GetSelectEntry()
{
    return( GetEntryText( GetCurEntry() ) );
}

std::vector<OUString> SdPageObjsTLB::GetSelectEntryList( const sal_uInt16 nDepth ) const
{
    std::vector<OUString> aEntries;
    SvTreeListEntry* pEntry = FirstSelected();

    while( pEntry )
    {
        sal_uInt16 nListDepth = GetModel()->GetDepth( pEntry );

        if( nListDepth == nDepth )
            aEntries.push_back(GetEntryText(pEntry));

        pEntry = NextSelected( pEntry );
    }

    return aEntries;
}

/**
 * Entries are inserted only by request (double click)
 */
void SdPageObjsTLB::RequestingChildren( SvTreeListEntry* pFileEntry )
{
    if( !pFileEntry->HasChildren() )
    {
        if( GetBookmarkDoc() )
        {
            SdrObject*   pObj = NULL;
            SdPage*      pPage = NULL;
            SvTreeListEntry* pPageEntry = NULL;

            Image aImgPage     = Image( BitmapEx( SdResId( BMP_PAGE     ) ) );
            Image aImgPageObjs = Image( BitmapEx( SdResId( BMP_PAGEOBJS ) ) );
            Image aImgObjects  = Image( BitmapEx( SdResId( BMP_OBJECTS  ) ) );

            // document name already inserted

            // only insert all "normal" ? slides with objects
            sal_uInt16 nPage = 0;
            const sal_uInt16 nMaxPages = mpBookmarkDoc->GetPageCount();

            while( nPage < nMaxPages )
            {
                pPage = (SdPage*) mpBookmarkDoc->GetPage( nPage );
                if( pPage->GetPageKind() == PK_STANDARD )
                {
                    pPageEntry = InsertEntry( pPage->GetName(),
                                              aImgPage,
                                              aImgPage,
                                              pFileEntry,
                                              sal_False,
                                              LIST_APPEND,
                                              reinterpret_cast< void* >( 1 ) );

                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        String aStr( GetObjectName( pObj ) );
                        if( aStr.Len() )
                        {
                            if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
                            {
                                InsertEntry(aStr, maImgOle, maImgOle, pPageEntry);
                            }
                            else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
                            {
                                InsertEntry(aStr, maImgGraphic, maImgGraphic, pPageEntry);
                            }
                            else
                            {
                                InsertEntry(aStr, aImgObjects, aImgObjects, pPageEntry);
                            }
                        }
                    }
                    if( pPageEntry->HasChildren() )
                    {
                        SetExpandedEntryBmp(  pPageEntry, aImgPageObjs );
                        SetCollapsedEntryBmp( pPageEntry, aImgPageObjs );
                    }
                }
                nPage++;
            }
        }
    }
    else
        SvTreeListBox::RequestingChildren( pFileEntry );
}

/**
 * Checks if it is a draw file and opens the BookmarkDoc depending of
 * the provided Docs
 */
SdDrawDocument* SdPageObjsTLB::GetBookmarkDoc(SfxMedium* pMed)
{
    if (
       !mpBookmarkDoc ||
         (pMed && (!mpOwnMedium || mpOwnMedium->GetName() != pMed->GetName()))
      )
    {
        // create a new BookmarkDoc if now one exists or if a new Medium is provided
        if (mpOwnMedium != pMed)
        {
            CloseBookmarkDoc();
        }

        if (pMed)
        {
            // it looks that it is undefined if a Medium was set by Fill() already
            DBG_ASSERT( !mpMedium, "SfxMedium confusion!" );
            delete mpMedium;
            mpMedium = NULL;

            // take over this Medium (currently used only be Navigator)
            mpOwnMedium = pMed;
        }

        DBG_ASSERT( mpMedium || pMed, "No SfxMedium provided!" );

        if( pMed )
        {
            // in this mode the document is also owned and controlled by this instance
            mxBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, sal_True);
            if (mxBookmarkDocShRef->DoLoad(pMed))
                mpBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            else
                mpBookmarkDoc = NULL;
        }
        else if ( mpMedium )
            // in this mode the document is owned and controlled by the SdDrawDocument
            // it can be released by calling the corresponding CloseBookmarkDoc method
            // successful creation of a document makes this the owner of the medium
            mpBookmarkDoc = ((SdDrawDocument*) mpDoc)->OpenBookmarkDoc(*mpMedium);

        if ( !mpBookmarkDoc )
        {
            ErrorBox aErrorBox( this, WB_OK, SD_RESSTR( STR_READ_DATA_ERROR ) );
            aErrorBox.Execute();
            mpMedium = 0; //On failure the SfxMedium is invalid
        }
    }

    return( mpBookmarkDoc );
}

/**
 * Close and delete bookmark document
 */
void SdPageObjsTLB::CloseBookmarkDoc()
{
    if (mxBookmarkDocShRef.Is())
    {
        mxBookmarkDocShRef->DoClose();
        mxBookmarkDocShRef.Clear();

        // Medium is owned by document, so it's destroyed already
        mpOwnMedium = 0;
    }
    else if ( mpBookmarkDoc )
    {
        DBG_ASSERT( !mpOwnMedium, "SfxMedium confusion!" );
        if ( mpDoc )
        {
            // The document owns the Medium, so the Medium will be invalid after closing the document
            ((SdDrawDocument*) mpDoc)->CloseBookmarkDoc();
            mpMedium = 0;
        }
    }
    else
    {
        // perhaps mpOwnMedium provided, but no successful creation of BookmarkDoc
        delete mpOwnMedium;
        mpOwnMedium = NULL;
    }

    mpBookmarkDoc = NULL;
}

void SdPageObjsTLB::SelectHdl()
{
    SvTreeListEntry* pEntry = FirstSelected();

    mbLinkableSelected = sal_True;

    while( pEntry && mbLinkableSelected )
    {
        if( NULL == pEntry->GetUserData() )
            mbLinkableSelected = sal_False;

        pEntry = NextSelected( pEntry );
    }

    SvTreeListBox::SelectHdl();
}

/**
 * Overloads RETURN with the functionality of DoubleClick
 */
void SdPageObjsTLB::KeyInput( const KeyEvent& rKEvt )
{
    if( rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // commented code from svtools/source/contnr/svimpbox.cxx
        SvTreeListEntry* pCursor = GetCurEntry();
        if( pCursor->HasChildren() || pCursor->HasChildrenOnDemand() )
        {
            if( IsExpanded( pCursor ) )
                Collapse( pCursor );
            else
                Expand( pCursor );
        }

        DoubleClickHdl();
    }
    else
        SvTreeListBox::KeyInput( rKEvt );
}

/**
 * StartDrag-Request
 */
void SdPageObjsTLB::StartDrag( sal_Int8 nAction, const Point& rPosPixel)
{
    (void)nAction;
    (void)rPosPixel;

    SdNavigatorWin* pNavWin = NULL;
    SvTreeListEntry* pEntry = GetEntry(rPosPixel);

    if( mpFrame->HasChildWindow( SID_NAVIGATOR ) )
        pNavWin = (SdNavigatorWin*) ( mpFrame->GetChildWindow( SID_NAVIGATOR )->GetContextWindow( SD_MOD() ) );

    if (pEntry != NULL
        && pNavWin !=NULL
        && pNavWin == mpParent
        && pNavWin->GetNavigatorDragType() != NAVIGATOR_DRAGTYPE_NONE )
    {
        // Mark only the children of the page under the mouse as drop
        // targets.  This prevents moving shapes from one page to another.

        // Select all entries and disable them as drop targets.
        SetSelectionMode(MULTIPLE_SELECTION);
        SetCursor(NULL, sal_False);
        SelectAll(sal_True, sal_False);
        EnableSelectionAsDropTarget(sal_False, sal_True);

        // Enable only the entries as drop targets that are children of the
        // page under the mouse.
        SvTreeListEntry* pParent = GetRootLevelParent(pEntry);
        if (pParent != NULL)
        {
            SelectAll(sal_False, sal_False);
            Select(pParent, sal_True);
            //            for (SvTreeListEntry*pChild=FirstChild(pParent); pChild!=NULL; pChild=NextSibling(pChild))
            //                Select(pChild, sal_True);
            EnableSelectionAsDropTarget(sal_True, sal_True);//sal_False);
        }

        // Set selection back to the entry under the mouse.
        SelectAll(sal_False,sal_False);
        SetSelectionMode(SINGLE_SELECTION);
        Select(pEntry, sal_True);

        // We can delete the Navigator from ExecuteDrag (when switching to
        // another document type), but that would kill the StarView MouseMove
        // Handler which is calling Command().
        // For this reason, Drag&Drop is asynchronous.
        Application::PostUserEvent( STATIC_LINK( this, SdPageObjsTLB, ExecDragHdl ) );
    }
}

/**
 * Begin drag
 */
void SdPageObjsTLB::DoDrag()
{
    mpDropNavWin = ( mpFrame->HasChildWindow( SID_NAVIGATOR ) ) ?
                  (SdNavigatorWin*)( mpFrame->GetChildWindow( SID_NAVIGATOR )->GetContextWindow( SD_MOD() ) ) :
                  NULL;

    if( mpDropNavWin )
    {
        ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
        String aURL = INetURLObject( pDocShell->GetMedium()->GetPhysicalName(), INET_PROT_FILE ).GetMainURL( INetURLObject::NO_DECODE );
        NavigatorDragType   eDragType = mpDropNavWin->GetNavigatorDragType();

        aURL.Append( '#' );
        aURL.Append( GetSelectEntry() );

        INetBookmark    aBookmark( aURL, GetSelectEntry() );
        sal_Int8        nDNDActions = DND_ACTION_COPYMOVE;

        if( eDragType == NAVIGATOR_DRAGTYPE_LINK )
            nDNDActions = DND_ACTION_LINK;  // Either COPY *or* LINK, never both!
        else if (mpDoc->GetSdPageCount(PK_STANDARD) == 1)
        {
            // Can not move away the last slide in a document.
            nDNDActions = DND_ACTION_COPY;
        }

        SvTreeListBox::ReleaseMouse();

        bIsInDrag = sal_True;

        SvLBoxDDInfo aDDInfo;
        memset(&aDDInfo,0,sizeof(SvLBoxDDInfo));
        aDDInfo.pApp = GetpApp();
        aDDInfo.pSource = this;
        //            aDDInfo.pDDStartEntry = pEntry;
        ::com::sun::star::uno::Sequence<sal_Int8> aSequence (sizeof(SvLBoxDDInfo));
        memcpy(aSequence.getArray(), (sal_Char*)&aDDInfo, sizeof(SvLBoxDDInfo));
        ::com::sun::star::uno::Any aTreeListBoxData (aSequence);

        // object is destroyed by internal reference mechanism
        SdTransferable* pTransferable = new SdPageObjsTLB::SdPageObjsTransferable(
            *this, aBookmark, *pDocShell, eDragType, aTreeListBoxData);

        // Get the view.
        ::sd::ViewShell* pViewShell = GetViewShellForDocShell(*pDocShell);
        if (pViewShell == NULL)
        {
            OSL_ASSERT(pViewShell!=NULL);
            return;
        }
        sd::View* pView = pViewShell->GetView();
        if (pView == NULL)
        {
            OSL_ASSERT(pView!=NULL);
            return;
        }

        SdrObject* pObject = NULL;
        void* pUserData = GetCurEntry()->GetUserData();
        if (pUserData != NULL && pUserData != (void*)1)
            pObject = reinterpret_cast<SdrObject*>(pUserData);
        if (pObject != NULL)
        {
            // For shapes without a user supplied name (the automatically
            // created name does not count), a different drag and drop technique
            // is used.
            if (GetObjectName(pObject, false).Len() == 0)
            {
                AddShapeToTransferable(*pTransferable, *pObject);
                pTransferable->SetView(pView);
                SD_MOD()->pTransferDrag = pTransferable;
            }

            // Unnamed shapes have to be selected to be recognized by the
            // current drop implementation.  In order to have a consistent
            // behaviour for all shapes, every shape that is to be dragged is
            // selected first.
            SdrPageView* pPageView = pView->GetSdrPageView();
            pView->UnmarkAllObj(pPageView);
            pView->MarkObj(pObject, pPageView);
        }
        else
        {
            pTransferable->SetView(pView);
            SD_MOD()->pTransferDrag = pTransferable;
        }

        pTransferable->StartDrag( this, nDNDActions );
    }
}

void SdPageObjsTLB::OnDragFinished( sal_uInt8 )
{
    if( mpFrame->HasChildWindow( SID_NAVIGATOR ) )
    {
        SdNavigatorWin* pNewNavWin = (SdNavigatorWin*) ( mpFrame->GetChildWindow( SID_NAVIGATOR )->GetContextWindow( SD_MOD() ) );

        if( mpDropNavWin == pNewNavWin)
        {
            MouseEvent aMEvt( mpDropNavWin->GetPointerPosPixel() );
            SvTreeListBox::MouseButtonUp( aMEvt );
        }
    }

    mpDropNavWin = NULL;
    bIsInDrag = sal_False;
}

/**
 * AcceptDrop-Event
 */
sal_Int8 SdPageObjsTLB::AcceptDrop (const AcceptDropEvent& rEvent)
{
    sal_Int8 nResult (DND_ACTION_NONE);

    if ( !bIsInDrag && IsDropFormatSupported( FORMAT_FILE ) )
    {
        nResult = rEvent.mnAction;
    }
    else
    {
        SvTreeListEntry* pEntry = GetDropTarget(rEvent.maPosPixel);
        if (rEvent.mbLeaving || !CheckDragAndDropMode( this, rEvent.mnAction ))
        {
            ImplShowTargetEmphasis( pTargetEntry, sal_False );
        }
        else if( !GetDragDropMode() )
        {
            SAL_WARN( "sc.ui", "SdPageObjsTLB::AcceptDrop(): no target" );
        }
        else if (IsDropAllowed(pEntry))
        {
            nResult = DND_ACTION_MOVE;

            // Draw emphasis.
            if (pEntry != pTargetEntry || !(nImpFlags & SVLBOX_TARGEMPH_VIS))
            {
                ImplShowTargetEmphasis( pTargetEntry, sal_False );
                pTargetEntry = pEntry;
                ImplShowTargetEmphasis( pTargetEntry, sal_True );
            }
        }
    }

    // Hide emphasis when there is no valid drop action.
    if (nResult == DND_ACTION_NONE)
        ImplShowTargetEmphasis(pTargetEntry, sal_False);

    return nResult;
}

/**
 * ExecuteDrop-Event
 */
sal_Int8 SdPageObjsTLB::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    try
    {
        if( !bIsInDrag )
        {
            SdNavigatorWin* pNavWin = NULL;
            sal_uInt16          nId = SID_NAVIGATOR;

            if( mpFrame->HasChildWindow( nId ) )
                pNavWin = (SdNavigatorWin*)( mpFrame->GetChildWindow( nId )->GetContextWindow( SD_MOD() ) );

            if( pNavWin && ( pNavWin == mpParent ) )
            {
                TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );
                OUString                aFile;

                if( aDataHelper.GetString( FORMAT_FILE, aFile ) &&
                    ( (SdNavigatorWin*) mpParent)->InsertFile( aFile ) )
                {
                    nRet = rEvt.mnAction;
                }
            }
        }
    }
    catch (com::sun::star::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if (nRet == DND_ACTION_NONE)
        SvTreeListBox::ExecuteDrop(rEvt, this);


    return nRet;
}

/**
 * Handler for Dragging
 */
IMPL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*, EMPTYARG)
{
    // as link, then it is allowed to asynchronous, without ImpMouseMoveMsg on
    // the stack, delete the Navigator
    pThis->DoDrag();
    return 0;
}


bool SdPageObjsTLB::PageBelongsToCurrentShow (const SdPage* pPage) const
{
    // Return <TRUE/> as default when there is no custom show or when none
    // is used.  The page does then belong to the standard show.
    bool bBelongsToShow = true;

    if (mpDoc->getPresentationSettings().mbCustomShow)
    {
        // Get the current custom show.
        SdCustomShow* pCustomShow = NULL;
        SdCustomShowList* pShowList = const_cast<SdDrawDocument*>(mpDoc)->GetCustomShowList();
        if (pShowList != NULL)
        {
            sal_uLong nCurrentShowIndex = pShowList->GetCurPos();
            pCustomShow = (*pShowList)[nCurrentShowIndex];
        }

        // Check whether the given page is part of that custom show.
        if (pCustomShow != NULL)
        {
            bBelongsToShow = false;
            sal_uLong nPageCount = pCustomShow->PagesVector().size();
            for (sal_uInt16 i=0; i<nPageCount && !bBelongsToShow; i++)
                if (pPage == pCustomShow->PagesVector()[i])
                    bBelongsToShow = true;
        }
    }

    return bBelongsToShow;
}




sal_Bool SdPageObjsTLB::NotifyMoving(
    SvTreeListEntry* pTarget,
    SvTreeListEntry* pEntry,
    SvTreeListEntry*& rpNewParent,
    sal_uLong& rNewChildPos)
{
    SvTreeListEntry* pDestination = pTarget;
    while (GetParent(pDestination) != NULL && GetParent(GetParent(pDestination)) != NULL)
        pDestination = GetParent(pDestination);

    SdrObject* pTargetObject = reinterpret_cast<SdrObject*>(pDestination->GetUserData());
    SdrObject* pSourceObject = reinterpret_cast<SdrObject*>(pEntry->GetUserData());
    if (pSourceObject == reinterpret_cast<SdrObject*>(1))
        pSourceObject = NULL;

    if (pTargetObject != NULL && pSourceObject != NULL)
    {
        SdrPage* pObjectList = pSourceObject->GetPage();
        if (pObjectList != NULL)
        {
            sal_uInt32 nNewPosition;
            if (pTargetObject == reinterpret_cast<SdrObject*>(1))
                nNewPosition = 0;
            else
                nNewPosition = pTargetObject->GetNavigationPosition() + 1;
            pObjectList->SetObjectNavigationPosition(*pSourceObject, nNewPosition);
        }

        // Update the tree list.
        if (pTarget == NULL)
        {
            rpNewParent = 0;
            rNewChildPos = 0;
            return sal_True;
        }
        else if (GetParent(pDestination) == NULL)
        {
            rpNewParent = pDestination;
            rNewChildPos = 0;
        }
        else
        {
            rpNewParent = GetParent(pDestination);
            rNewChildPos = pModel->GetRelPos(pDestination) + 1;
            rNewChildPos += nCurEntrySelPos;
            nCurEntrySelPos++;
        }
        return sal_True;
    }
    else
        return sal_False;
}




SvTreeListEntry* SdPageObjsTLB::GetDropTarget (const Point& rLocation)
{
    SvTreeListEntry* pEntry = SvTreeListBox::GetDropTarget(rLocation);
    if (pEntry == NULL)
        return NULL;

    if (GetParent(pEntry) == NULL)
    {
        // Use page entry as insertion position.
    }
    else
    {
        // Go to second hierarchy level, i.e. top level shapes,
        // i.e. children of pages.
        while (GetParent(pEntry) != NULL && GetParent(GetParent(pEntry)) != NULL)
            pEntry = GetParent(pEntry);

        // Advance to next sibling.
        SvTreeListEntry* pNext;
        sal_uInt16 nDepth (0);
        while (pEntry != NULL)
        {
            pNext = dynamic_cast<SvTreeListEntry*>(NextVisible(pEntry, &nDepth));
            if (pNext != NULL && nDepth > 0 && nDepth!=0xffff)
                pEntry = pNext;
            else
                break;
        }
    }

    return pEntry;
}




bool SdPageObjsTLB::IsDropAllowed (SvTreeListEntry* pEntry)
{
    if (pEntry == NULL)
        return false;

    if ( ! IsDropFormatSupported(SdPageObjsTransferable::GetListBoxDropFormatId()))
        return false;

    if ((pEntry->GetFlags() & SV_ENTRYFLAG_DISABLE_DROP) != 0)
        return false;

    return true;
}




void SdPageObjsTLB::AddShapeToTransferable (
    SdTransferable& rTransferable,
    SdrObject& rObject) const
{
    TransferableObjectDescriptor aObjectDescriptor;
    bool bIsDescriptorFillingPending (true);

    const SdrOle2Obj* pOleObject = dynamic_cast<const SdrOle2Obj*>(&rObject);
    if (pOleObject != NULL && pOleObject->GetObjRef().is())
    {
        // If object has no persistence it must be copied as part of the document
        try
        {
            uno::Reference< embed::XEmbedPersist > xPersObj (pOleObject->GetObjRef(), uno::UNO_QUERY );
            if (xPersObj.is() && xPersObj->hasEntry())
            {
                SvEmbedTransferHelper::FillTransferableObjectDescriptor(
                    aObjectDescriptor,
                    pOleObject->GetObjRef(),
                    pOleObject->GetGraphic(),
                    pOleObject->GetAspect());
                bIsDescriptorFillingPending = false;
            }
        }
        catch( uno::Exception& )
        {
        }
    }

    ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
    if (bIsDescriptorFillingPending && pDocShell!=NULL)
    {
        pDocShell->FillTransferableObjectDescriptor(aObjectDescriptor);
    }

    Point aDragPos (rObject.GetCurrentBoundRect().Center());
    //Point aDragPos (0,0);
    aObjectDescriptor.maDragStartPos = aDragPos;
    //  aObjectDescriptor.maSize = GetAllMarkedRect().GetSize();
    if (pDocShell != NULL)
        aObjectDescriptor.maDisplayName = pDocShell->GetMedium()->GetURLObject().GetURLNoPass();
    else
        aObjectDescriptor.maDisplayName = String();
    aObjectDescriptor.mbCanLink = sal_False;

    rTransferable.SetStartPos(aDragPos);
    rTransferable.SetObjectDescriptor( aObjectDescriptor );
}




::sd::ViewShell* SdPageObjsTLB::GetViewShellForDocShell (::sd::DrawDocShell& rDocShell)
{
    {
        ::sd::ViewShell* pViewShell = rDocShell.GetViewShell();
        if (pViewShell != NULL)
            return pViewShell;
    }

    try
    {
        // Get a component enumeration from the desktop and search it for documents.
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext());

        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);

        uno::Reference<frame::XFramesSupplier> xFrameSupplier (xDesktop, uno::UNO_QUERY);
        if ( ! xFrameSupplier.is())
            return NULL;

        uno::Reference<container::XIndexAccess> xFrameAccess (xFrameSupplier->getFrames(), uno::UNO_QUERY);
        if ( ! xFrameAccess.is())
            return NULL;

        for (sal_Int32 nIndex=0,nCount=xFrameAccess->getCount(); nIndex<nCount; ++nIndex)
        {
            uno::Reference<frame::XFrame> xFrame;
            if ( ! (xFrameAccess->getByIndex(nIndex) >>= xFrame))
                continue;

            ::sd::DrawController* pController = dynamic_cast<sd::DrawController*>(xFrame->getController().get());
            if (pController == NULL)
                continue;
            ::sd::ViewShellBase* pBase = pController->GetViewShellBase();
            if (pBase == NULL)
                continue;
            if (pBase->GetDocShell() != &rDocShell)
                continue;

            const ::boost::shared_ptr<sd::ViewShell> pViewShell (pBase->GetMainViewShell());
            if (pViewShell)
                return pViewShell.get();
        }
    }
    catch (uno::Exception &)
    {
        // When there is an exception then simply use the default value of
        // bIsEnabled and disable the controls.
    }
    return NULL;
}




//===== IconProvider ==========================================================

SdPageObjsTLB::IconProvider::IconProvider (void)
    : maImgPage( BitmapEx( SdResId( BMP_PAGE ) ) ),
      maImgPageExcl( BitmapEx( SdResId( BMP_PAGE_EXCLUDED ) ) ),
      maImgPageObjsExcl( BitmapEx( SdResId( BMP_PAGEOBJS_EXCLUDED ) ) ),
      maImgPageObjs( BitmapEx( SdResId( BMP_PAGEOBJS ) ) ),
      maImgObjects( BitmapEx( SdResId( BMP_OBJECTS ) ) ),
      maImgGroup( BitmapEx( SdResId( BMP_GROUP ) ) )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
