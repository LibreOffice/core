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
#include <sal/log.hxx>
#include <sot/formats.hxx>
#include <vcl/weld.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <cusshow.hxx>

#include <sfx2/viewfrm.hxx>

#include <sdtreelb.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <navigatr.hxx>
#include <strings.hrc>

#include <bitmaps.hlst>
#include <customshowlist.hxx>
#include <ViewShell.hxx>
#include <DrawController.hxx>
#include <ViewShellBase.hxx>

#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/embedtransfer.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/scopeguard.hxx>


using namespace com::sun::star;

class SdPageObjsTLB::IconProvider
{
public:
    IconProvider();

    // Regular icons.
    Image const maImgPage;
    Image const maImgPageExcl;
    Image const maImgPageObjsExcl;
    Image const maImgPageObjs;
    Image const maImgObjects;
    Image const maImgGroup;
};

bool SdPageObjsTLB::bIsInDrag = false;

bool SdPageObjsTLB::IsInDrag()
{
    return bIsInDrag;
}

SotClipboardFormatId SdPageObjsTLB::SdPageObjsTransferable::mnListBoxDropFormatId = static_cast<SotClipboardFormatId>(SAL_MAX_UINT32);


SdPageObjsTLB::SdPageObjsTransferable::SdPageObjsTransferable(
    SdPageObjsTLB& rParent,
        const INetBookmark& rBookmark,
    ::sd::DrawDocShell& rDocShell,
    NavigatorDragType eDragType)
    : SdTransferable(rDocShell.GetDoc(), nullptr, true),
      mrParent( rParent ),
      maBookmark( rBookmark ),
      mrDocShell( rDocShell ),
      meDragType( eDragType )
{
    rParent.SetupDragOrigin();
}

VCL_BUILDER_FACTORY_CONSTRUCTOR(SdPageObjsTLB, WB_TABSTOP)

SdPageObjsTLB::SdPageObjsTransferable::~SdPageObjsTransferable()
{
}

void SdPageObjsTLB::SdPageObjsTransferable::AddSupportedFormats()
{
    AddFormat(SotClipboardFormatId::NETSCAPE_BOOKMARK);
    AddFormat(SotClipboardFormatId::TREELISTBOX);
    AddFormat(GetListBoxDropFormatId());
}

bool SdPageObjsTLB::SdPageObjsTransferable::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    SotClipboardFormatId nFormatId = SotExchange::GetFormat( rFlavor );
    switch (nFormatId)
    {
        case SotClipboardFormatId::NETSCAPE_BOOKMARK:
            SetINetBookmark( maBookmark, rFlavor );
            return true;

        case SotClipboardFormatId::TREELISTBOX:
        {
            css::uno::Any aTreeListBoxData; // empty for now
            SetAny(aTreeListBoxData);
            return true;
        }

        default:
            return false;
    }
}

void SdPageObjsTLB::SdPageObjsTransferable::DragFinished( sal_Int8 nDropAction )
{
    mrParent.OnDragFinished();
    SdTransferable::DragFinished(nDropAction);
}

sal_Int64 SAL_CALL SdPageObjsTLB::SdPageObjsTransferable::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = static_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else
        nRet = SdTransferable::getSomething(rId);

    return nRet;
}

namespace
{
    class theSdPageObjsTLBUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdPageObjsTLBUnoTunnelId > {};
}

const css::uno::Sequence< sal_Int8 >& SdPageObjsTLB::SdPageObjsTransferable::getUnoTunnelId()
{
    return theSdPageObjsTLBUnoTunnelId::get().getSeq();
}

SdPageObjsTLB::SdPageObjsTransferable* SdPageObjsTLB::SdPageObjsTransferable::getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData )
    throw()
{
    try
    {
        css::uno::Reference< css::lang::XUnoTunnel > xUnoTunnel( rxData, css::uno::UNO_QUERY_THROW );

        return reinterpret_cast<SdPageObjsTLB::SdPageObjsTransferable*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xUnoTunnel->getSomething( SdPageObjsTLB::SdPageObjsTransferable::getUnoTunnelId()) ) );
    }
    catch( const css::uno::Exception& )
    {
    }
    return nullptr;
}

SotClipboardFormatId SdPageObjsTLB::SdPageObjsTransferable::GetListBoxDropFormatId()
{
    if (mnListBoxDropFormatId == static_cast<SotClipboardFormatId>(SAL_MAX_UINT32))
        mnListBoxDropFormatId = SotExchange::RegisterFormatMimeType("application/x-openoffice-treelistbox-moveonly;windows_formatname=\"SV_LBOX_DD_FORMAT_MOVE\"");
    return mnListBoxDropFormatId;
}

SdPageObjsTLB::SdPageObjsTLB( vcl::Window* pParentWin, WinBits nStyle )
:   SvTreeListBox       ( pParentWin, nStyle )
,   mpDoc               ( nullptr )
,   mpBookmarkDoc       ( nullptr )
,   mpMedium            ( nullptr )
,   mpOwnMedium         ( nullptr )
,   maImgOle            ( StockImage::Yes, BMP_OLE )
,   maImgGraphic        ( StockImage::Yes, BMP_GRAPHIC )
,   mbLinkableSelected  ( false )
,   mbSaveTreeItemState ( false )
,   mbShowAllShapes     ( false )
,   mbShowAllPages      ( false )
,   mbSelectionHandlerNavigates(false)
,   mbNavigationGrabsFocus(true)
{
    // add lines to Tree-ListBox
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                           WB_HASBUTTONS | // WB_HASLINESATROOT |
                           WB_HSCROLL |
                           WB_HASBUTTONSATROOT );
    SetQuickSearch(true); /* i31275 */;
    SetNodeBitmaps(Image(StockImage::Yes, BMP_EXPAND), Image(StockImage::Yes, BMP_COLLAPSE));

    SetDragDropMode(
         DragDropMode::CTRL_MOVE | DragDropMode::CTRL_COPY |
            DragDropMode::APP_MOVE  | DragDropMode::APP_COPY  | DragDropMode::APP_DROP );

    m_pAccel = ::svt::AcceleratorExecute::createAcceleratorHelper();
}

void SdPageObjsTLB::SetSdNavigator(SdNavigatorWin* pNavigator)
{
    mpNavigator = pNavigator;
}

void SdPageObjsTLB::SetViewFrame( const SfxViewFrame* pViewFrame )
{
    sd::ViewShellBase* pBase = sd::ViewShellBase::GetViewShellBase(pViewFrame);
    const css::uno::Reference< css::frame::XFrame > xFrame = pBase->GetMainViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
    m_pAccel->init(::comphelper::getProcessComponentContext(), xFrame);
}


SdPageObjsTLB::~SdPageObjsTLB()
{
    disposeOnce();
}

void SdPageObjsTLB::dispose()
{
    if ( mpBookmarkDoc )
        CloseBookmarkDoc();
    else
        // no document was created from mpMedium, so this object is still the owner of it
        delete mpMedium;
    mpNavigator.clear();
    m_pAccel.reset();
    SvTreeListBox::dispose();
}

// helper function for  GetEntryAltText and GetEntryLongDescription
OUString SdPageObjsTLB::getAltLongDescText(SvTreeListEntry* pEntry , bool isAltText) const
{
    sal_uInt16 maxPages = mpDoc->GetPageCount();
    sal_uInt16 pageNo;
    SdrObject*   pObj = nullptr;

    OUString ParentName = GetEntryText( GetRootLevelParent( pEntry ) );

    for( pageNo = 0;  pageNo < maxPages; pageNo++ )
    {
        const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetPage( pageNo ) );
        if( pPage->GetPageKind() != PageKind::Standard ) continue;
        if( pPage->GetName() !=  ParentName ) continue;
        SdrObjListIter aIter( pPage, SdrIterMode::Flat );
        while( aIter.IsMore() )
        {
            pObj = aIter.Next();
            if( GetEntryText(pEntry) ==  GetObjectName( pObj )  )
            {
                if( isAltText )
                    return pObj->GetTitle();
                else
                    return pObj->GetDescription();
            }
        }
    }
    return OUString();

}

OUString SdPageObjsTLB::GetEntryAltText( SvTreeListEntry* pEntry ) const
{
    return getAltLongDescText( pEntry, true );
}

OUString SdPageObjsTLB::GetEntryLongDescription( SvTreeListEntry* pEntry ) const
{
    return getAltLongDescText( pEntry, false);
}

void SdPageObjsTLB::InitEntry(SvTreeListEntry* pEntry,
    const OUString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString& rCol = static_cast<SvLBoxString&>(pEntry->GetItem( nColToHilite ));
    pEntry->ReplaceItem(std::make_unique<SvLBoxString>(rCol.GetText()), nColToHilite );
}

void SdPageObjsTLB::SaveExpandedTreeItemState(SvTreeListEntry* pEntry, std::vector<OUString>& vectTreeItem)
{
    if (!pEntry)
        return;

    SvTreeListEntry* pListEntry = pEntry;
    while (pListEntry)
    {
        if (pListEntry->HasChildren())
        {
            if (IsExpanded(pListEntry))
                vectTreeItem.push_back(GetEntryText(pListEntry));
            SvTreeListEntry* pChildEntry = FirstChild(pListEntry);
            SaveExpandedTreeItemState(pChildEntry, vectTreeItem);
        }
        pListEntry = pListEntry->NextSibling();
    }
}
void SdPageObjsTLB::Clear()
{
    //Save the expanded tree item
    if (mbSaveTreeItemState)
    {
        maSelectionEntryText.clear();
        maTreeItem.clear();
        if (GetCurEntry())
            maSelectionEntryText = GetSelectedEntry();
        SvTreeListEntry* pEntry = FirstChild(nullptr);
        SaveExpandedTreeItemState(pEntry, maTreeItem);
    }
    return SvTreeListBox::Clear();
}

OUString SdPageObjsTLB::GetObjectName(
    const SdrObject* pObject,
    const bool bCreate) const
{
    OUString aRet;

    if ( pObject )
    {
        aRet = pObject->GetName();

        if (aRet.isEmpty() && dynamic_cast<const SdrOle2Obj* >(pObject) !=  nullptr)
            aRet = static_cast< const SdrOle2Obj* >( pObject )->GetPersistName();
    }

    if (bCreate
        && mbShowAllShapes
        && aRet.isEmpty()
        && pObject!=nullptr)
    {
        aRet = SdResId(STR_NAVIGATOR_SHAPE_BASE_NAME);
        aRet = aRet.replaceFirst("%1", OUString::number(pObject->GetOrdNum() + 1));
    }

    return aRet;
}

/**
 * select a entry in TreeLB
 */
bool SdPageObjsTLB::SelectEntry( const OUString& rName )
{
    bool bFound = false;

    if( !rName.isEmpty() )
    {
        SvTreeListEntry* pEntry = nullptr;
        OUString aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = true;
                SetCurEntry( pEntry );
            }
        }
    }
    return bFound;
}

/**
 * @return true if children of the specified string are selected
 */
bool SdPageObjsTLB::HasSelectedChildren( const OUString& rName )
{
    bool bChildren = false;

    if( !rName.isEmpty() )
    {
        bool bFound  = false;
        SvTreeListEntry* pEntry = nullptr;
        OUString aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = true;
                bool bExpanded = IsExpanded( pEntry );
                long nCount = GetChildSelectionCount( pEntry );
                if( bExpanded && nCount > 0 )
                    bChildren = true;
            }
        }
    }
    return bChildren;
}

/**
 * Fill TreeLB with pages and objects
 */
void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, bool bAllPages,
                          const OUString& rDocName)
{
    OUString aSelection;
    if( GetSelectionCount() > 0 )
    {
        aSelection = GetSelectedEntry();
        Clear();
    }

    mpDoc = pInDoc;
    maDocName = rDocName;
    mbShowAllPages = bAllPages;
    mpMedium = nullptr;

    IconProvider aIconProvider;

    // first insert all pages including objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetPage( nPage ) );
        if(  (mbShowAllPages || pPage->GetPageKind() == PageKind::Standard)
             && (pPage->GetPageKind() != PageKind::Handout)   ) //#94954# never list the normal handout page ( handout-masterpage is used instead )
        {
            bool bPageExluded = pPage->IsExcluded();

            bool bPageBelongsToShow = PageBelongsToCurrentShow (pPage);
            bPageExluded |= !bPageBelongsToShow;

            AddShapeList(*pPage, nullptr, pPage->GetName(), bPageExluded, nullptr, aIconProvider);
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
            const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetMasterPage( nPage ) );
            AddShapeList(*pPage, nullptr, pPage->GetName(), false, nullptr, aIconProvider);
            nPage++;
        }
    }
    if( !aSelection.isEmpty() )
        SelectEntry( aSelection );
    else if (mbSaveTreeItemState && !maSelectionEntryText.isEmpty())
    {
        SelectEntry(maSelectionEntryText);
    }
}

/**
 * We insert only the first entry. Children are created on demand.
 */
void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, SfxMedium* pInMedium,
                          const OUString& rDocName )
{
    mpDoc = pInDoc;

    // this object now owns the Medium
    mpMedium = pInMedium;
    maDocName = rDocName;

    Image aImgDocOpen(StockImage::Yes, BMP_DOC_OPEN);
    Image aImgDocClosed(StockImage::Yes, BMP_DOC_CLOSED);

    // insert document name
    InsertEntry( maDocName, aImgDocOpen, aImgDocClosed, nullptr, true, TREELIST_APPEND,
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
    else if (pShape != nullptr)
        aIcon = rIconProvider.maImgGroup;

    void* pUserData (reinterpret_cast<void*>(1));
    if (pShape != nullptr)
        pUserData = pShape;

    SvTreeListEntry* pEntry = InsertEntry(
        rsName,
        aIcon,
        aIcon,
        pParentEntry,
        false,
        TREELIST_APPEND,
        pUserData);

    SdrObjListIter aIter(
        &rList,
        !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
        SdrIterMode::Flat);

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OSL_ASSERT(pObj!=nullptr);

        // Get the shape name.
        OUString aStr (GetObjectName( pObj ) );

        if( !aStr.isEmpty() )
        {
            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_OLE2 )
            {
                InsertEntry(
                    aStr,
                    maImgOle,
                    maImgOle,
                    pEntry,
                    false,
                    TREELIST_APPEND,
                    pObj
                );
            }
            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                InsertEntry(
                    aStr,
                    maImgGraphic,
                    maImgGraphic,
                    pEntry,
                    false,
                    TREELIST_APPEND,
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
                    false,
                    TREELIST_APPEND,
                    pObj
                );
            }
        }
    }

    if( !pEntry->HasChildren() )
        return;

    SetExpandedEntryBmp(
        pEntry,
        bIsExcluded ? rIconProvider.maImgPageObjsExcl : rIconProvider.maImgPageObjs);
    SetCollapsedEntryBmp(
        pEntry,
        bIsExcluded ? rIconProvider.maImgPageObjsExcl : rIconProvider.maImgPageObjs);
    if (mbSaveTreeItemState)
    {
        OUString strEntry = GetEntryText(pEntry);
        auto it = std::find(maTreeItem.begin(), maTreeItem.end(), strEntry);
        if (it != maTreeItem.end())
            Expand( pEntry );
    }
    else
        Expand( pEntry );
}

void SdPageObjsTLB::SetShowAllShapes (
    const bool bShowAllShapes,
    const bool bFillList)
{
    mbShowAllShapes = bShowAllShapes;
    if (bFillList)
    {
        if (mpMedium == nullptr)
            Fill(mpDoc, mbShowAllPages, maDocName);
        else
            Fill(mpDoc, mpMedium, maDocName);
    }
}

bool SdPageObjsTLB::IsEqualToShapeList(SvTreeListEntry*& pEntry, const SdrObjList& rList,
                                       const OUString& rListName)
{
    if (!pEntry)
        return false;
    OUString aName = GetEntryText(pEntry);

    if (rListName != aName)
        return false;

    pEntry = Next(pEntry);

    SdrObjListIter aIter(&rList,
                         !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
                         SdrIterMode::Flat);

    while (aIter.IsMore())
    {
        SdrObject* pObj = aIter.Next();

        const OUString aObjectName(GetObjectName(pObj));

        if (!aObjectName.isEmpty())
        {
            if (!pEntry)
                return false;

            aName = GetEntryText(pEntry);

            if (aObjectName != aName)
                return false;

            if (pObj->IsGroupObject())
            {
                bool bRet = IsEqualToShapeList(pEntry, *pObj->GetSubList(), aObjectName);
                if (!bRet)
                    return false;
            }
            else
                pEntry = Next(pEntry);
        }
    }

    return true;
}

/**
 * Checks if the pages (PageKind::Standard) of a doc and the objects on the pages
 * are identical to the TreeLB.
 * If a doc is provided, this will be the used doc (important by more than
 * one document).
 */
bool SdPageObjsTLB::IsEqualToDoc( const SdDrawDocument* pInDoc )
{
    if( pInDoc )
        mpDoc = pInDoc;

    if( !mpDoc )
        return false;

    SvTreeListEntry* pEntry = First();

    // compare all pages including the objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetPage( nPage ) );
        if( pPage->GetPageKind() == PageKind::Standard )
        {
            bool bRet = IsEqualToShapeList(pEntry, *pPage, pPage->GetName());
            if (!bRet)
                return false;
        }
        nPage++;
    }
    // If there are still entries in the listbox,
    // then objects (with names) or pages were deleted
    return !pEntry;
}

/**
 * @return selected string
 */
OUString SdPageObjsTLB::GetSelectedEntry()
{
    return GetEntryText( GetCurEntry() );
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
            SdrObject*   pObj = nullptr;
            SvTreeListEntry* pPageEntry = nullptr;

            Image aImgPage(StockImage::Yes, BMP_PAGE);
            Image aImgPageObjs(StockImage::Yes, BMP_PAGEOBJS);
            Image aImgObjects(StockImage::Yes, BMP_OBJECTS);

            // document name already inserted

            // only insert all "normal" ? slides with objects
            sal_uInt16 nPage = 0;
            const sal_uInt16 nMaxPages = mpBookmarkDoc->GetPageCount();

            while( nPage < nMaxPages )
            {
                SdPage* pPage = static_cast<SdPage*>( mpBookmarkDoc->GetPage( nPage ) );
                if( pPage->GetPageKind() == PageKind::Standard )
                {
                    pPageEntry = InsertEntry( pPage->GetName(),
                                              aImgPage,
                                              aImgPage,
                                              pFileEntry,
                                              false,
                                              TREELIST_APPEND,
                                              reinterpret_cast< void* >( 1 ) );

                    SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        OUString aStr( GetObjectName( pObj ) );
                        if( !aStr.isEmpty() )
                        {
                            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_OLE2 )
                            {
                                InsertEntry(aStr, maImgOle, maImgOle, pPageEntry);
                            }
                            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF )
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
            mpMedium = nullptr;

            // take over this Medium (currently used only be Navigator)
            mpOwnMedium = pMed;
        }

        DBG_ASSERT( mpMedium || pMed, "No SfxMedium provided!" );

        if( pMed )
        {
            // in this mode the document is also owned and controlled by this instance
            mxBookmarkDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD, true, DocumentType::Impress);
            if (mxBookmarkDocShRef->DoLoad(pMed))
                mpBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            else
                mpBookmarkDoc = nullptr;
        }
        else if ( mpMedium )
            // in this mode the document is owned and controlled by the SdDrawDocument
            // it can be released by calling the corresponding CloseBookmarkDoc method
            // successful creation of a document makes this the owner of the medium
            mpBookmarkDoc = const_cast<SdDrawDocument*>(mpDoc)->OpenBookmarkDoc(mpMedium);

        if ( !mpBookmarkDoc )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
            xErrorBox->run();
            mpMedium = nullptr; //On failure the SfxMedium is invalid
        }
    }

    return mpBookmarkDoc;
}

/**
 * Close and delete bookmark document
 */
void SdPageObjsTLB::CloseBookmarkDoc()
{
    if (mxBookmarkDocShRef.is())
    {
        mxBookmarkDocShRef->DoClose();
        mxBookmarkDocShRef.clear();

        // Medium is owned by document, so it's destroyed already
        mpOwnMedium = nullptr;
    }
    else if ( mpBookmarkDoc )
    {
        DBG_ASSERT( !mpOwnMedium, "SfxMedium confusion!" );
        if ( mpDoc )
        {
            // The document owns the Medium, so the Medium will be invalid after closing the document
            const_cast<SdDrawDocument*>(mpDoc)->CloseBookmarkDoc();
            mpMedium = nullptr;
        }
    }
    else
    {
        // perhaps mpOwnMedium provided, but no successful creation of BookmarkDoc
        delete mpOwnMedium;
        mpOwnMedium = nullptr;
    }

    mpBookmarkDoc = nullptr;
}

void SdPageObjsTLB::SelectHdl()
{
    SvTreeListEntry* pEntry = FirstSelected();

    mbLinkableSelected = true;

    while( pEntry && mbLinkableSelected )
    {
        if( nullptr == pEntry->GetUserData() )
            mbLinkableSelected = false;

        pEntry = NextSelected( pEntry );
    }

    SvTreeListBox::SelectHdl();

    if (mbSelectionHandlerNavigates)
        DoubleClickHdl();
}

/**
 * Overloads RETURN with the functionality of DoubleClick
 */
void SdPageObjsTLB::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& aKeyCode = rKEvt.GetKeyCode();
    if ( m_pAccel->execute( aKeyCode ) )
        // the accelerator consumed the event
        return;
    if( rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // commented code from svtools/source/contnr/svimpbox.cxx
        SvTreeListEntry* pCursor = GetCurEntry();
        if (!pCursor)
            return;
        if( pCursor->HasChildren() || pCursor->HasChildrenOnDemand() )
        {
            if( IsExpanded( pCursor ) )
                Collapse( pCursor );
            else
                Expand( pCursor );
        }

        DoubleClickHdl();
    }
    else if (rKEvt.GetKeyCode().GetCode() == KEY_SPACE)
    {
        if (mpNavigator)
        {
            SvTreeListEntry* pNewEntry = GetCurEntry();
            if (!pNewEntry)
                return;
            SvTreeListEntry* pParentEntry = GetParent(pNewEntry);
            if (!pParentEntry)
                return;
            Invalidate();
        }
    }
    else
        SvTreeListBox::KeyInput( rKEvt );
}

void SdPageObjsTLB::MouseButtonDown(const MouseEvent& rMEvt)
{
    mbSelectionHandlerNavigates = rMEvt.GetClicks() == 1;
    comphelper::ScopeGuard aNavigationGuard([this]() { this->mbSelectionHandlerNavigates = false; });
    mbNavigationGrabsFocus = rMEvt.GetClicks() != 1;
    comphelper::ScopeGuard aGrabGuard([this]() { this->mbNavigationGrabsFocus = true; });

    SvTreeListBox::MouseButtonDown(rMEvt);
}

/**
 * StartDrag-Request
 */
void SdPageObjsTLB::StartDrag( sal_Int8, const Point& rPosPixel)
{
    SvTreeListEntry* pEntry = GetEntry(rPosPixel);

    if (!(pEntry && mpNavigator && mpNavigator->GetNavigatorDragType() != NAVIGATOR_DRAGTYPE_NONE))
        return;

    // Mark only the children of the page under the mouse as drop
    // targets.  This prevents moving shapes from one page to another.

    // Select all entries and disable them as drop targets.
    SetSelectionMode(SelectionMode::Multiple);
    SetCursor(static_cast<SvTreeListEntry*>(nullptr));
    SelectAll(true, false);
    EnableSelectionAsDropTarget(false);

    // Enable only the entries as drop targets that are children of the
    // page under the mouse.
    SvTreeListEntry* pParent = GetRootLevelParent(pEntry);
    if (pParent != nullptr)
    {
        SelectAll(false, false);
        Select(pParent);
        //            for (SvTreeListEntry*pChild=FirstChild(pParent); pChild!=NULL; pChild=NextSibling(pChild))
        //                Select(pChild, sal_True);
        EnableSelectionAsDropTarget();//sal_False);
    }

    // Set selection back to the entry under the mouse.
    SelectAll(false, false);
    SetSelectionMode(SelectionMode::Single);
    Select(pEntry);

    // We can delete the Navigator from ExecuteDrag (when switching to
    // another document type), but that would kill the StarView MouseMove
    // Handler which is calling Command().
    // For this reason, Drag&Drop is asynchronous.
    Application::PostUserEvent( LINK( this, SdPageObjsTLB, ExecDragHdl ), nullptr, true );
}

/**
 * Begin drag
 */
void SdPageObjsTLB::DoDrag()
{
    if (!mpNavigator)
        return;

    ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
    OUString aURL = INetURLObject( pDocShell->GetMedium()->GetPhysicalName(), INetProtocol::File ).GetMainURL( INetURLObject::DecodeMechanism::NONE );
    NavigatorDragType   eDragType = mpNavigator->GetNavigatorDragType();

    aURL += "#" + GetSelectedEntry();

    INetBookmark    aBookmark( aURL, GetSelectedEntry() );
    sal_Int8        nDNDActions = DND_ACTION_COPYMOVE;

    if( eDragType == NAVIGATOR_DRAGTYPE_LINK )
        nDNDActions = DND_ACTION_LINK;  // Either COPY *or* LINK, never both!
    else if (mpDoc->GetSdPageCount(PageKind::Standard) == 1)
    {
        // Can not move away the last slide in a document.
        nDNDActions = DND_ACTION_COPY;
    }

    SvTreeListBox::ReleaseMouse();

    bIsInDrag = true;

    // Get the view.
    ::sd::ViewShell* pViewShell = GetViewShellForDocShell(*pDocShell);
    if (pViewShell == nullptr)
    {
        OSL_ASSERT(pViewShell!=nullptr);
        return;
    }
    sd::View* pView = pViewShell->GetView();
    if (pView == nullptr)
    {
        OSL_ASSERT(pView!=nullptr);
        return;
    }

    // object is destroyed by internal reference mechanism
    SdTransferable* pTransferable =
            new SdPageObjsTLB::SdPageObjsTransferable(
                        *this, aBookmark, *pDocShell, eDragType);

    SdrObject* pObject = nullptr;
    void* pUserData = GetCurEntry()->GetUserData();
    if (pUserData != nullptr && pUserData != reinterpret_cast<void*>(1))
        pObject = static_cast<SdrObject*>(pUserData);
    if (pObject != nullptr)
    {
        // For shapes without a user supplied name (the automatically
        // created name does not count), a different drag and drop technique
        // is used.
        if (GetObjectName(pObject, false).isEmpty())
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

void SdPageObjsTLB::OnDragFinished()
{
    if (mpNavigator)
    {
        MouseEvent aMEvt(mpNavigator->GetPointerPosPixel());
        SvTreeListBox::MouseButtonUp(aMEvt);
    }

    bIsInDrag = false;
}

/**
 * AcceptDrop-Event
 */
sal_Int8 SdPageObjsTLB::AcceptDrop (const AcceptDropEvent& rEvent)
{
    sal_Int8 nResult (DND_ACTION_NONE);

    if ( !bIsInDrag && IsDropFormatSupported( SotClipboardFormatId::SIMPLE_FILE ) )
    {
        nResult = rEvent.mnAction;
    }
    else
    {
        SvTreeListEntry* pEntry = GetDropTarget(rEvent.maPosPixel);
        if (rEvent.mbLeaving || !CheckDragAndDropMode( this, rEvent.mnAction ))
        {
            ImplShowTargetEmphasis( pTargetEntry, false );
        }
        else if( GetDragDropMode() == DragDropMode::NONE )
        {
            SAL_WARN( "sc.ui", "SdPageObjsTLB::AcceptDrop(): no target" );
        }
        else if (IsDropAllowed(pEntry))
        {
            nResult = DND_ACTION_MOVE;

            // Draw emphasis.
            if (pEntry != pTargetEntry || !(nImpFlags & SvTreeListBoxFlags::TARGEMPH_VIS))
            {
                ImplShowTargetEmphasis( pTargetEntry, false );
                pTargetEntry = pEntry;
                ImplShowTargetEmphasis( pTargetEntry, true );
            }
        }
    }

    // Hide emphasis when there is no valid drop action.
    if (nResult == DND_ACTION_NONE)
        ImplShowTargetEmphasis(pTargetEntry, false);

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
            if (mpNavigator)
            {
                TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );
                OUString                aFile;

                if( aDataHelper.GetString( SotClipboardFormatId::SIMPLE_FILE, aFile ) &&
                    mpNavigator->InsertFile( aFile ) )
                {
                    nRet = rEvt.mnAction;
                }
            }
        }
    }
    catch (css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }

    if (nRet == DND_ACTION_NONE)
        SvTreeListBox::ExecuteDrop(rEvt, this);

    return nRet;
}

/**
 * Handler for Dragging
 */
IMPL_LINK_NOARG(SdPageObjsTLB, ExecDragHdl, void*, void)
{
    // as link, then it is allowed to asynchronous, without ImpMouseMoveMsg on
    // the stack, delete the Navigator
    DoDrag();
}

bool SdPageObjsTLB::PageBelongsToCurrentShow (const SdPage* pPage) const
{
    // Return <TRUE/> as default when there is no custom show or when none
    // is used.  The page does then belong to the standard show.
    bool bBelongsToShow = true;

    if (mpDoc->getPresentationSettings().mbCustomShow)
    {
        // Get the current custom show.
        SdCustomShow* pCustomShow = nullptr;
        SdCustomShowList* pShowList = const_cast<SdDrawDocument*>(mpDoc)->GetCustomShowList();
        if (pShowList != nullptr)
        {
            sal_uLong nCurrentShowIndex = pShowList->GetCurPos();
            pCustomShow = (*pShowList)[nCurrentShowIndex].get();
        }

        // Check whether the given page is part of that custom show.
        if (pCustomShow != nullptr)
        {
            bBelongsToShow = false;
            size_t nPageCount = pCustomShow->PagesVector().size();
            for (size_t i=0; i<nPageCount && !bBelongsToShow; i++)
                if (pPage == pCustomShow->PagesVector()[i])
                    bBelongsToShow = true;
        }
    }

    return bBelongsToShow;
}

TriState SdPageObjsTLB::NotifyMoving(
    SvTreeListEntry* pTarget,
    SvTreeListEntry* pEntry,
    SvTreeListEntry*& rpNewParent,
    sal_uLong& rNewChildPos)
{
    SvTreeListEntry* pDestination = pTarget;
    while (GetParent(pDestination) != nullptr && GetParent(GetParent(pDestination)) != nullptr)
        pDestination = GetParent(pDestination);

    SdrObject* pTargetObject = static_cast<SdrObject*>(pDestination->GetUserData());
    SdrObject* pSourceObject = static_cast<SdrObject*>(pEntry->GetUserData());
    if (pSourceObject == reinterpret_cast<SdrObject*>(1))
        pSourceObject = nullptr;

    if (pTargetObject != nullptr && pSourceObject != nullptr)
    {
        SdrPage* pObjectList = pSourceObject->getSdrPageFromSdrObject();
        if (pObjectList != nullptr)
        {
            sal_uInt32 nNewPosition;
            if (pTargetObject == reinterpret_cast<SdrObject*>(1))
                nNewPosition = 0;
            else
                nNewPosition = pTargetObject->GetNavigationPosition() + 1;
            pObjectList->SetObjectNavigationPosition(*pSourceObject, nNewPosition);
        }

        // Update the tree list.
        if (GetParent(pDestination) == nullptr)
        {
            rpNewParent = pDestination;
            rNewChildPos = 0;
        }
        else
        {
            rpNewParent = GetParent(pDestination);
            rNewChildPos = SvTreeList::GetRelPos(pDestination) + 1;
            rNewChildPos += nCurEntrySelPos;
            nCurEntrySelPos++;
        }
        return TRISTATE_TRUE;
    }
    else
        return TRISTATE_FALSE;
}

SvTreeListEntry* SdPageObjsTLB::GetDropTarget (const Point& rLocation)
{
    SvTreeListEntry* pEntry = SvTreeListBox::GetDropTarget(rLocation);
    if (pEntry == nullptr)
        return nullptr;

    if (GetParent(pEntry) == nullptr)
    {
        // Use page entry as insertion position.
    }
    else
    {
        // Go to second hierarchy level, i.e. top level shapes,
        // i.e. children of pages.
        while (GetParent(pEntry) != nullptr && GetParent(GetParent(pEntry)) != nullptr)
            pEntry = GetParent(pEntry);

        // Advance to next sibling.
        SvTreeListEntry* pNext;
        sal_uInt16 nDepth (0);
        do
        {
            pNext = NextVisible(pEntry, &nDepth);
            if (pNext != nullptr && nDepth > 0 && nDepth!=0xffff)
                pEntry = pNext;
            else
                break;
        }
        while (pEntry != nullptr);
    }

    return pEntry;
}

bool SdPageObjsTLB::IsDropAllowed (SvTreeListEntry const * pEntry)
{
    if (pEntry == nullptr)
        return false;

    if ( ! IsDropFormatSupported(SdPageObjsTransferable::GetListBoxDropFormatId()))
        return false;

    if (pEntry->GetFlags() & SvTLEntryFlags::DISABLE_DROP)
        return false;

    return true;
}

void SdPageObjsTLB::AddShapeToTransferable (
    SdTransferable& rTransferable,
    SdrObject& rObject) const
{
    std::unique_ptr<TransferableObjectDescriptor> pObjectDescriptor(new TransferableObjectDescriptor);
    bool bIsDescriptorFillingPending (true);

    const SdrOle2Obj* pOleObject = dynamic_cast<const SdrOle2Obj*>(&rObject);
    if (pOleObject != nullptr && pOleObject->GetObjRef().is())
    {
        // If object has no persistence it must be copied as part of the document
        try
        {
            uno::Reference< embed::XEmbedPersist > xPersObj (pOleObject->GetObjRef(), uno::UNO_QUERY );
            if (xPersObj.is() && xPersObj->hasEntry())
            {
                SvEmbedTransferHelper::FillTransferableObjectDescriptor(
                    *pObjectDescriptor,
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
    if (bIsDescriptorFillingPending && pDocShell!=nullptr)
    {
        pDocShell->FillTransferableObjectDescriptor(*pObjectDescriptor);
    }

    Point aDragPos (rObject.GetCurrentBoundRect().Center());
    //Point aDragPos (0,0);
    pObjectDescriptor->maDragStartPos = aDragPos;
    //  aObjectDescriptor.maSize = GetAllMarkedRect().GetSize();
    if (pDocShell != nullptr)
        pObjectDescriptor->maDisplayName = pDocShell->GetMedium()->GetURLObject().GetURLNoPass();
    else
        pObjectDescriptor->maDisplayName.clear();

    rTransferable.SetStartPos(aDragPos);
    rTransferable.SetObjectDescriptor( std::move(pObjectDescriptor) );
}

::sd::ViewShell* SdPageObjsTLB::GetViewShellForDocShell (::sd::DrawDocShell& rDocShell)
{
    {
        ::sd::ViewShell* pViewShell = rDocShell.GetViewShell();
        if (pViewShell != nullptr)
            return pViewShell;
    }

    try
    {
        // Get a component enumeration from the desktop and search it for documents.
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext());

        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);

        uno::Reference<frame::XFramesSupplier> xFrameSupplier (xDesktop, uno::UNO_QUERY);
        if ( ! xFrameSupplier.is())
            return nullptr;

        uno::Reference<container::XIndexAccess> xFrameAccess (xFrameSupplier->getFrames(), uno::UNO_QUERY);
        if ( ! xFrameAccess.is())
            return nullptr;

        for (sal_Int32 nIndex=0,nCount=xFrameAccess->getCount(); nIndex<nCount; ++nIndex)
        {
            uno::Reference<frame::XFrame> xFrame;
            if ( ! (xFrameAccess->getByIndex(nIndex) >>= xFrame))
                continue;

            ::sd::DrawController* pController = dynamic_cast<sd::DrawController*>(xFrame->getController().get());
            if (pController == nullptr)
                continue;
            ::sd::ViewShellBase* pBase = pController->GetViewShellBase();
            if (pBase == nullptr)
                continue;
            if (pBase->GetDocShell() != &rDocShell)
                continue;

            const std::shared_ptr<sd::ViewShell> pViewShell (pBase->GetMainViewShell());
            if (pViewShell)
                return pViewShell.get();
        }
    }
    catch (uno::Exception &)
    {
        // When there is an exception then simply use the default value of
        // bIsEnabled and disable the controls.
    }
    return nullptr;
}

//===== IconProvider ==========================================================

SdPageObjsTLB::IconProvider::IconProvider()
    : maImgPage(StockImage::Yes, BMP_PAGE),
      maImgPageExcl(StockImage::Yes, BMP_PAGE_EXCLUDED),
      maImgPageObjsExcl(StockImage::Yes, BMP_PAGEOBJS_EXCLUDED),
      maImgPageObjs(StockImage::Yes, BMP_PAGEOBJS),
      maImgObjects(StockImage::Yes, BMP_OBJECTS),
      maImgGroup(StockImage::Yes, BMP_GROUP)
{
}

SdPageObjsTLV::SdPageObjsTLV(std::unique_ptr<weld::TreeView> xTreeView)
    : m_xTreeView(std::move(xTreeView))
    , m_xAccel(::svt::AcceleratorExecute::createAcceleratorHelper())
    , m_pDoc(nullptr)
    , m_pBookmarkDoc(nullptr)
    , m_pMedium(nullptr)
    , m_pOwnMedium(nullptr)
    , m_bLinkableSelected(false)
    , m_bShowAllPages(false)
{
    m_xTreeView->connect_expanding(LINK(this, SdPageObjsTLV, RequestingChildrenHdl));
    m_xTreeView->connect_changed(LINK(this, SdPageObjsTLV, SelectHdl));
}

IMPL_LINK_NOARG(SdPageObjsTLV, SelectHdl, weld::TreeView&, void)
{
    m_bLinkableSelected = true;

    m_xTreeView->selected_foreach([this](weld::TreeIter& rEntry){
        if (m_xTreeView->get_id(rEntry).toInt64() == 0)
            m_bLinkableSelected = false;
        return false;
    });

    m_aChangeHdl.Call(*m_xTreeView);
}

OUString SdPageObjsTLV::GetObjectName(const SdrObject* pObject)
{
    if ( !pObject )
        return OUString();

    OUString aRet = pObject->GetName();
    if (!aRet.isEmpty())
        return aRet;

    if (auto pOle = dynamic_cast<const SdrOle2Obj* >(pObject))
        aRet = pOle->GetPersistName();

    return aRet;
}

std::vector<OUString> SdPageObjsTLV::GetSelectEntryList(const int nDepth) const
{
    std::vector<OUString> aEntries;

    m_xTreeView->selected_foreach([this, nDepth, &aEntries](weld::TreeIter& rEntry){
        int nListDepth = m_xTreeView->get_iter_depth(rEntry);
        if (nListDepth == nDepth)
            aEntries.push_back(m_xTreeView->get_text(rEntry));
        return false;
    });

    return aEntries;
}

/**
 * Checks if it is a draw file and opens the BookmarkDoc depending of
 * the provided Docs
 */
SdDrawDocument* SdPageObjsTLV::GetBookmarkDoc()
{
    if (!m_pBookmarkDoc)
    {
        // create a new BookmarkDoc if now one exists or if a new Medium is provided
        if (m_pOwnMedium != nullptr)
        {
            CloseBookmarkDoc();
        }

        DBG_ASSERT( m_pMedium, "No SfxMedium provided!" );

        if ( m_pMedium )
            // in this mode the document is owned and controlled by the SdDrawDocument
            // it can be released by calling the corresponding CloseBookmarkDoc method
            // successful creation of a document makes this the owner of the medium
            m_pBookmarkDoc = const_cast<SdDrawDocument*>(m_pDoc)->OpenBookmarkDoc(m_pMedium);

        if ( !m_pBookmarkDoc )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xTreeView.get(),
                                                           VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
            xErrorBox->run();
            m_pMedium = nullptr; //On failure the SfxMedium is invalid
        }
    }

    return m_pBookmarkDoc;
}

/**
 * Entries are inserted only by request (double click)
 */
IMPL_LINK(SdPageObjsTLV, RequestingChildrenHdl, const weld::TreeIter&, rFileEntry, bool)
{
    if (!m_xTreeView->iter_has_child(rFileEntry))
    {
        if (GetBookmarkDoc())
        {
            SdrObject*   pObj = nullptr;

            OUString sImgPage(BMP_PAGE);
            OUString sImgPageObjs(BMP_PAGEOBJS);
            OUString sImgObjects(BMP_OBJECTS);
            OUString sImgOle(BMP_OLE);
            OUString sImgGraphic(BMP_GRAPHIC);

            // document name already inserted

            // only insert all "normal" ? slides with objects
            sal_uInt16 nPage = 0;
            const sal_uInt16 nMaxPages = m_pBookmarkDoc->GetPageCount();

            std::unique_ptr<weld::TreeIter> xPageEntry;
            while (nPage < nMaxPages)
            {
                SdPage* pPage = static_cast<SdPage*>(m_pBookmarkDoc->GetPage(nPage));
                if (pPage->GetPageKind() == PageKind::Standard)
                {
                    OUString sId(OUString::number(1));
                    m_xTreeView->insert(&rFileEntry, -1, &pPage->GetName(), &sId,
                                        nullptr, nullptr, &sImgPage, false, nullptr);

                    if (!xPageEntry)
                    {
                        xPageEntry = m_xTreeView->make_iterator(&rFileEntry);
                        m_xTreeView->iter_children(*xPageEntry);
                    }
                    else
                        m_xTreeView->iter_next_sibling(*xPageEntry);

                    SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        OUString aStr( GetObjectName( pObj ) );
                        if( !aStr.isEmpty() )
                        {
                            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_OLE2 )
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, &sImgOle, false, nullptr);
                            }
                            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF )
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, &sImgGraphic, false, nullptr);
                            }
                            else
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, &sImgObjects, false, nullptr);
                            }
                        }
                    }
                    if (m_xTreeView->iter_has_child(*xPageEntry))
                    {
                        m_xTreeView->set_image(*xPageEntry, sImgPageObjs);
                    }
                }
                nPage++;
            }
        }
    }
    return true;
}

void SdPageObjsTLV::SetViewFrame(const SfxViewFrame* pViewFrame)
{
    sd::ViewShellBase* pBase = sd::ViewShellBase::GetViewShellBase(pViewFrame);
    const css::uno::Reference< css::frame::XFrame > xFrame = pBase->GetMainViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
    m_xAccel->init(::comphelper::getProcessComponentContext(), xFrame);
}

/**
 * Close and delete bookmark document
 */
void SdPageObjsTLV::CloseBookmarkDoc()
{
    if (m_xBookmarkDocShRef.is())
    {
        m_xBookmarkDocShRef->DoClose();
        m_xBookmarkDocShRef.clear();

        // Medium is owned by document, so it's destroyed already
        m_pOwnMedium = nullptr;
    }
    else if (m_pBookmarkDoc)
    {
        DBG_ASSERT(!m_pOwnMedium, "SfxMedium confusion!");
        if (m_pDoc)
        {
            // The document owns the Medium, so the Medium will be invalid after closing the document
            const_cast<SdDrawDocument*>(m_pDoc)->CloseBookmarkDoc();
            m_pMedium = nullptr;
        }
    }
    else
    {
        // perhaps mpOwnMedium provided, but no successful creation of BookmarkDoc
        delete m_pOwnMedium;
        m_pOwnMedium = nullptr;
    }

    m_pBookmarkDoc = nullptr;
}

bool SdPageObjsTLV::PageBelongsToCurrentShow(const SdPage* pPage) const
{
    // Return <TRUE/> as default when there is no custom show or when none
    // is used.  The page does then belong to the standard show.
    bool bBelongsToShow = true;

    if (m_pDoc->getPresentationSettings().mbCustomShow)
    {
        // Get the current custom show.
        SdCustomShow* pCustomShow = nullptr;
        SdCustomShowList* pShowList = const_cast<SdDrawDocument*>(m_pDoc)->GetCustomShowList();
        if (pShowList != nullptr)
        {
            sal_uLong nCurrentShowIndex = pShowList->GetCurPos();
            pCustomShow = (*pShowList)[nCurrentShowIndex].get();
        }

        // Check whether the given page is part of that custom show.
        if (pCustomShow != nullptr)
        {
            bBelongsToShow = false;
            size_t nPageCount = pCustomShow->PagesVector().size();
            for (size_t i=0; i<nPageCount && !bBelongsToShow; i++)
                if (pPage == pCustomShow->PagesVector()[i])
                    bBelongsToShow = true;
        }
    }

    return bBelongsToShow;
}

void SdPageObjsTLV::AddShapeList (
    const SdrObjList& rList,
    SdrObject* pShape,
    const OUString& rsName,
    const bool bIsExcluded,
    weld::TreeIter* pParentEntry)
{
    OUString aIcon(BMP_PAGE);
    if (bIsExcluded)
        aIcon = BMP_PAGE_EXCLUDED;
    else if (pShape != nullptr)
        aIcon = BMP_GROUP;

    OUString aUserData("1");
    if (pShape != nullptr)
        aUserData = OUString::number(reinterpret_cast<sal_Int64>(pShape));

    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    InsertEntry(pParentEntry, aUserData, rsName, aIcon, xEntry.get());

    SdrObjListIter aIter(
        &rList,
        !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
        SdrIterMode::Flat);

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OSL_ASSERT(pObj!=nullptr);

        // Get the shape name.
        OUString aStr (GetObjectName( pObj ) );
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pObj)));

        if( !aStr.isEmpty() )
        {
            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_OLE2 )
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_OLE);
            }
            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_GRAPHIC);
            }
            else if (pObj->IsGroupObject())
            {
                AddShapeList(
                    *pObj->GetSubList(),
                    pObj,
                    aStr,
                    false,
                    xEntry.get());
            }
            else
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_OBJECTS);
            }
        }
    }

    if (!m_xTreeView->iter_has_child(*xEntry))
        return;

    if (bIsExcluded)
        m_xTreeView->set_image(*xEntry, BMP_PAGEOBJS_EXCLUDED);
    else
        m_xTreeView->set_image(*xEntry, BMP_PAGEOBJS);
    m_xTreeView->expand_row(*xEntry);
}

/**
 * Fill TreeLB with pages and objects
 */
void SdPageObjsTLV::Fill(const SdDrawDocument* pInDoc, bool bAllPages, const OUString& rDocName)
{
    OUString aSelection = m_xTreeView->get_selected_text();
    clear();

    m_pDoc = pInDoc;
    m_aDocName = rDocName;
    m_bShowAllPages = bAllPages;
    m_pMedium = nullptr;

    // first insert all pages including objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = m_pDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( m_pDoc->GetPage( nPage ) );
        if(  (m_bShowAllPages || pPage->GetPageKind() == PageKind::Standard)
             && (pPage->GetPageKind() != PageKind::Handout)   ) //#94954# never list the normal handout page ( handout-masterpage is used instead )
        {
            bool bPageExluded = pPage->IsExcluded();

            bool bPageBelongsToShow = PageBelongsToCurrentShow (pPage);
            bPageExluded |= !bPageBelongsToShow;

            AddShapeList(*pPage, nullptr, pPage->GetName(), bPageExluded, nullptr);
        }
        nPage++;
    }

    // then insert all master pages including objects
    if( m_bShowAllPages )
    {
        nPage = 0;
        const sal_uInt16 nMaxMasterPages = m_pDoc->GetMasterPageCount();

        while( nPage < nMaxMasterPages )
        {
            const SdPage* pPage = static_cast<const SdPage*>( m_pDoc->GetMasterPage( nPage ) );
            AddShapeList(*pPage, nullptr, pPage->GetName(), false, nullptr);
            nPage++;
        }
    }
    if (!aSelection.isEmpty())
        m_xTreeView->select_text(aSelection);
}

/**
 * We insert only the first entry. Children are created on demand.
 */
void SdPageObjsTLV::Fill( const SdDrawDocument* pInDoc, SfxMedium* pInMedium,
                          const OUString& rDocName )
{
    m_pDoc = pInDoc;

    // this object now owns the Medium
    m_pMedium = pInMedium;
    m_aDocName = rDocName;

    OUString sImgDoc(BMP_DOC_OPEN);

    OUString sId(OUString::number(1));
    // insert document name
    m_xTreeView->insert(nullptr, -1, &m_aDocName, &sId, nullptr, nullptr, &sImgDoc, true, nullptr);
}

/**
 * select a entry in TreeLB
 */
bool SdPageObjsTLV::SelectEntry( const OUString& rName )
{
    bool bFound = false;

    if (!rName.isEmpty())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        OUString aTmp;

        if (m_xTreeView->get_iter_first(*xEntry))
        {
            do
            {
                aTmp = m_xTreeView->get_text(*xEntry);
                if (aTmp == rName)
                {
                    m_xTreeView->set_cursor(*xEntry);
                    m_xTreeView->select(*xEntry);
                    bFound = true;
                    break;
                }
            }
            while (m_xTreeView->iter_next(*xEntry));
        }
    }

    return bFound;
}

SdPageObjsTLV::~SdPageObjsTLV()
{
    if (m_pBookmarkDoc)
        CloseBookmarkDoc();
    else
    {
        // no document was created from m_pMedium, so this object is still the owner of it
        delete m_pMedium;
    }
    m_xAccel.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
