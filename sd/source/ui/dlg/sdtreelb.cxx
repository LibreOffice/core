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
#include <vcl/layout.hxx>
#include <svl/urihelper.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
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
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>

using namespace com::sun::star;

class SdPageObjsTLB::IconProvider
{
public:
    IconProvider();

    // Regular icons.
    Image maImgPage;
    Image maImgPageExcl;
    Image maImgPageObjsExcl;
    Image maImgPageObjs;
    Image maImgObjects;
    Image maImgGroup;
};

bool SdPageObjsTLB::bIsInDrag = false;

bool SdPageObjsTLB::IsInDrag()
{
    return bIsInDrag;
}

SotClipboardFormatId SdPageObjsTLB::SdPageObjsTransferable::mnListBoxDropFormatId = static_cast<SotClipboardFormatId>(SAL_MAX_UINT32);

// - SdPageObjsTLB::SdPageObjsTransferable -

SdPageObjsTLB::SdPageObjsTransferable::SdPageObjsTransferable(
    SdPageObjsTLB& rParent,
        const INetBookmark& rBookmark,
    ::sd::DrawDocShell& rDocShell,
    NavigatorDragType eDragType)
    : SdTransferable(rDocShell.GetDoc(), NULL, true),
      mrParent( rParent ),
      maBookmark( rBookmark ),
      mrDocShell( rDocShell ),
      meDragType( eDragType )
{
    rParent.SetupDragOrigin();
}

VCL_BUILDER_DECL_FACTORY(SdPageObjsTLB)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SdPageObjsTLB>::Create(pParent, nWinStyle);
}

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
            SetAny(aTreeListBoxData, rFlavor);
            return true;
        }

        default:
            return false;
    }
}

void SdPageObjsTLB::SdPageObjsTransferable::DragFinished( sal_Int8 nDropAction )
{
    mrParent.OnDragFinished( nDropAction );
    SdTransferable::DragFinished(nDropAction);
}

sal_Int64 SAL_CALL SdPageObjsTLB::SdPageObjsTransferable::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = (sal_Int64)reinterpret_cast<sal_IntPtr>(this);
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

SotClipboardFormatId SdPageObjsTLB::SdPageObjsTransferable::GetListBoxDropFormatId()
{
    if (mnListBoxDropFormatId == static_cast<SotClipboardFormatId>(SAL_MAX_UINT32))
        mnListBoxDropFormatId = SotExchange::RegisterFormatMimeType(OUString(
        "application/x-openoffice-treelistbox-moveonly;windows_formatname=\"SV_LBOX_DD_FORMAT_MOVE\""));
    return mnListBoxDropFormatId;
}

SdPageObjsTLB::SdPageObjsTLB( vcl::Window* pParentWin, const SdResId& rSdResId )
:   SvTreeListBox       ( pParentWin, rSdResId )
,   bisInSdNavigatorWin  ( false )
,   mpParent            ( pParentWin )
,   mpDoc               ( NULL )
,   mpBookmarkDoc       ( NULL )
,   mpMedium            ( NULL )
,   mpOwnMedium         ( NULL )
,   maImgOle             ( BitmapEx( SdResId( BMP_OLE ) ) )
,   maImgGraphic         ( BitmapEx( SdResId( BMP_GRAPHIC ) ) )
,   mbLinkableSelected  ( false )
,   mpDropNavWin        ( NULL )
,   mpFrame             ( NULL )
,   mbSaveTreeItemState ( false )
,   mbShowAllShapes     ( false )
,   mbShowAllPages      ( false )
{
    // add lines to Tree-ListBox
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                           WB_HASBUTTONS | // WB_HASLINESATROOT |
                           WB_HSCROLL |
                           WB_HASBUTTONSATROOT |
                           WB_QUICK_SEARCH /* i31275 */ );
    SetNodeBitmaps( Image(Bitmap( SdResId(BMP_EXPAND) )),
                    Image(Bitmap( SdResId(BMP_COLLAPSE) )));

    SetDragDropMode(
         DragDropMode::CTRL_MOVE | DragDropMode::CTRL_COPY |
            DragDropMode::APP_MOVE  | DragDropMode::APP_COPY  | DragDropMode::APP_DROP );
}

SdPageObjsTLB::SdPageObjsTLB( vcl::Window* pParentWin, WinBits nStyle )
:   SvTreeListBox       ( pParentWin, nStyle )
,   bisInSdNavigatorWin ( false )
,   mpParent            ( pParentWin )
,   mpDoc               ( NULL )
,   mpBookmarkDoc       ( NULL )
,   mpMedium            ( NULL )
,   mpOwnMedium         ( NULL )
,   maImgOle             ( BitmapEx( SdResId( BMP_OLE ) ) )
,   maImgGraphic         ( BitmapEx( SdResId( BMP_GRAPHIC ) ) )
,   mbLinkableSelected  ( false )
,   mpDropNavWin        ( NULL )
,   mpFrame             ( NULL )
,   mbSaveTreeItemState ( false )
,   mbShowAllShapes     ( false )
,   mbShowAllPages      ( false )
{
    // add lines to Tree-ListBox
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                           WB_HASBUTTONS | // WB_HASLINESATROOT |
                           WB_HSCROLL |
                           WB_HASBUTTONSATROOT |
                           WB_QUICK_SEARCH /* i31275 */ );
    SetNodeBitmaps( Image(Bitmap( SdResId(BMP_EXPAND) )),
                    Image(Bitmap( SdResId(BMP_COLLAPSE) )));

    SetDragDropMode(
         DragDropMode::CTRL_MOVE | DragDropMode::CTRL_COPY |
            DragDropMode::APP_MOVE  | DragDropMode::APP_COPY  | DragDropMode::APP_DROP );
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
    mpParent.clear();
    mpDropNavWin.clear();
    SvTreeListBox::dispose();
}

// helper function for  GetEntryAltText and GetEntryLongDescription
OUString SdPageObjsTLB::getAltLongDescText(SvTreeListEntry* pEntry , bool isAltText) const
{
    sal_uInt16 maxPages = mpDoc->GetPageCount();
    sal_uInt16 pageNo;
    SdrObject*   pObj = NULL;

    OUString ParentName = GetEntryText( GetRootLevelParent( pEntry ) );

    for( pageNo = 0;  pageNo < maxPages; pageNo++ )
    {
        const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetPage( pageNo ) );
        if( pPage->GetPageKind() != PK_STANDARD ) continue;
        if( pPage->GetName() !=  ParentName ) continue;
        SdrObjListIter aIter( *pPage, IM_FLAT );
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

void  SdPageObjsTLB::MarkCurEntry( const OUString& rName )
{

    if (!rName.isEmpty())
    {
        SvTreeListEntry* pCurEntry =GetCurEntry();
        SvTreeListEntry* pEntry =NULL;
        OUString aTmp1;
        OUString aTmp2;

        if( GetParent(pCurEntry)==NULL )
        {
            aTmp1 = GetEntryText( pCurEntry );
            for( pEntry = First(); pEntry ; pEntry = Next( pEntry ) )
            {
               if(GetParent( pEntry )==NULL)
                   continue;
                aTmp2 = GetEntryText( GetParent( pEntry ));
                if( aTmp1 != aTmp2)
                {
                    // IA2 CWS. MT: Removed in SvTreeListEntry for now - only used in Sw/Sd/ScContentLBoxString, they should decide if they need this
                    pEntry->SetMarked(false);
                }
            }
        }
        else
        {
            for( pEntry = First(); pEntry ; pEntry = Next( pEntry ) )
            {
                aTmp2 = GetEntryText( pEntry );
                if( aTmp2 == rName)
                {
                    pEntry->SetMarked(true);
                }
                else
                {
                    pEntry->SetMarked(false);
                }
            }
        }
    }
    Invalidate();
}

void  SdPageObjsTLB:: FreshCurEntry()
{
    SvTreeListEntry* pEntry =NULL;
    for( pEntry = First(); pEntry ; pEntry = Next( pEntry ) )
    {
                pEntry->SetMarked(false);
    }
    Invalidate();
}

void SdPageObjsTLB::InitEntry(SvTreeListEntry* pEntry,
    const OUString& rStr, const Image& rImg1, const Image& rImg2, SvLBoxButtonKind eButtonKind)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString& rCol = static_cast<SvLBoxString&>(pEntry->GetItem( nColToHilite ));
    std::unique_ptr<SvLBoxString> pStr(new SvLBoxString(pEntry, 0, rCol.GetText()));
    pEntry->ReplaceItem(std::move(pStr), nColToHilite );
}

void SdPageObjsTLB::SaveExpandedTreeItemState(SvTreeListEntry* pEntry, std::vector<OUString>& vectTreeItem)
{
    if (pEntry)
    {
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
            pListEntry = NextSibling(pListEntry);
        }
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
            maSelectionEntryText = GetSelectEntry();
        SvTreeListEntry* pEntry = FirstChild(NULL);
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
        && pObject!=NULL)
    {
        aRet = SD_RESSTR(STR_NAVIGATOR_SHAPE_BASE_NAME);
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
        SvTreeListEntry* pEntry = NULL;
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
        SvTreeListEntry* pEntry = NULL;
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
        aSelection = GetSelectEntry();
        Clear();
    }

    mpDoc = pInDoc;
    maDocName = rDocName;
    mbShowAllPages = bAllPages;
    mpMedium = NULL;

    IconProvider aIconProvider;

    // first insert all pages including objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetPage( nPage ) );
        if(  (mbShowAllPages || pPage->GetPageKind() == PK_STANDARD)
             && !(pPage->GetPageKind()==PK_HANDOUT)   ) //#94954# never list the normal handout page ( handout-masterpage is used instead )
        {
            bool bPageExluded = pPage->IsExcluded();

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
            const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetMasterPage( nPage ) );
            AddShapeList(*pPage, NULL, pPage->GetName(), false, NULL, aIconProvider);
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

    Image aImgDocOpen=Image( BitmapEx( SdResId( BMP_DOC_OPEN ) ) );
    Image aImgDocClosed=Image( BitmapEx( SdResId( BMP_DOC_CLOSED ) ) );

    // insert document name
    InsertEntry( maDocName, aImgDocOpen, aImgDocClosed, NULL, true, TREELIST_APPEND,
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
        false,
        TREELIST_APPEND,
        pUserData);

    SdrObjListIter aIter(
        rList,
        !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
        IM_FLAT,
        false /*not reverse*/);

    bool  bMarked=false;
    if(bisInSdNavigatorWin)
    {
        vcl::Window* pWindow=NULL;
        SdNavigatorWin* pSdNavigatorWin=NULL;
        sd::DrawDocShell* pSdDrawDocShell = NULL;
        if(pEntry)
            pWindow=reinterpret_cast<vcl::Window*>(GetParent(pEntry));
        if(pWindow)
            pSdNavigatorWin = static_cast<SdNavigatorWin*>(pWindow);
        if( pSdNavigatorWin )
            pSdDrawDocShell = SdNavigatorWin::GetDrawDocShell(mpDoc);
        if(pSdDrawDocShell)
            bMarked = pSdDrawDocShell->IsMarked(pShape);
        if(pEntry)
        {
            if(bMarked)
                pEntry->SetMarked(true);
            else
                pEntry->SetMarked(false);
        }
    }
    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OSL_ASSERT(pObj!=NULL);

        // Get the shape name.
        OUString aStr (GetObjectName( pObj ) );

        if( !aStr.isEmpty() )
        {
            if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
            {
                SvTreeListEntry *pNewEntry = InsertEntry(
                    aStr,
                    maImgOle,
                    maImgOle,
                    pEntry,
                    false,
                    TREELIST_APPEND,
                    pObj
                );

                if(bisInSdNavigatorWin)
                {
                    vcl::Window* pWindow=NULL;
                    SdNavigatorWin* pSdNavigatorWin=NULL;
                    sd::DrawDocShell* pSdDrawDocShell = NULL;
                    if(pNewEntry)
                        pWindow=reinterpret_cast<vcl::Window*>(GetParent(pNewEntry));
                    if(pWindow)
                        pSdNavigatorWin = static_cast<SdNavigatorWin*>(pWindow);
                    if( pSdNavigatorWin )
                        pSdDrawDocShell = SdNavigatorWin::GetDrawDocShell(mpDoc);
                    if(pSdDrawDocShell)
                        bMarked = pSdDrawDocShell->IsMarked(pObj);
                    if(pNewEntry)
                    {
                        if(bMarked)
                            pNewEntry->SetMarked(true);
                        else
                            pNewEntry->SetMarked(false);
                    }
                }
            }
            else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                SvTreeListEntry *pNewEntry = InsertEntry(
                    aStr,
                    maImgGraphic,
                    maImgGraphic,
                    pEntry,
                    false,
                    TREELIST_APPEND,
                    pObj
                );

                if(bisInSdNavigatorWin)
                {
                    vcl::Window* pWindow=NULL;
                    SdNavigatorWin* pSdNavigatorWin=NULL;
                    sd::DrawDocShell* pSdDrawDocShell = NULL;
                    if(pNewEntry)
                        pWindow=reinterpret_cast<vcl::Window*>(GetParent(pNewEntry));
                    if(pWindow)
                        pSdNavigatorWin = static_cast<SdNavigatorWin*>(pWindow);
                    if( pSdNavigatorWin )
                        pSdDrawDocShell = SdNavigatorWin::GetDrawDocShell(mpDoc);
                    if(pSdDrawDocShell)
                        bMarked = pSdDrawDocShell->IsMarked(pObj);
                    if(pNewEntry)
                    {
                        if(bMarked)
                        {
                            pNewEntry->SetMarked(true);
                        }
                        else
                        {
                            pNewEntry->SetMarked(false);
                        }
                    }
                }
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
                SvTreeListEntry *pNewEntry = InsertEntry(
                    aStr,
                    rIconProvider.maImgObjects,
                    rIconProvider.maImgObjects,
                    pEntry,
                    false,
                    TREELIST_APPEND,
                    pObj
                );

                if(bisInSdNavigatorWin)
                {
                    vcl::Window* pWindow=NULL;
                    SdNavigatorWin* pSdNavigatorWin=NULL;
                    sd::DrawDocShell* pSdDrawDocShell = NULL;
                    if(pNewEntry)
                        pWindow=reinterpret_cast<vcl::Window*>(GetParent(pNewEntry));
                    if(pWindow)
                        pSdNavigatorWin = static_cast<SdNavigatorWin*>(pWindow);
                    if( pSdNavigatorWin )
                        pSdDrawDocShell = SdNavigatorWin::GetDrawDocShell(mpDoc);
                    if(pSdDrawDocShell)
                        bMarked = pSdDrawDocShell->IsMarked(pObj);
                    if(pNewEntry)
                    {
                        if(bMarked)
                        {
                            pNewEntry->SetMarked(true);
                        }
                        else
                        {
                            pNewEntry->SetMarked(false);
                        }
                    }
                }
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
        if (mbSaveTreeItemState)
        {
            std::vector<OUString>::iterator iteStart = maTreeItem.begin();
            while (iteStart != maTreeItem.end())
            {
                OUString strEntry = GetEntryText(pEntry);
                if (*iteStart == strEntry)
                {
                    Expand( pEntry );
                    break;
                }
                ++iteStart;
            }
        }
        else
            Expand( pEntry );
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

/**
 * Checks if the pages (PK_STANDARD) of a doc and the objects on the pages
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

    SdrObject*   pObj = NULL;
    SvTreeListEntry* pEntry = First();
    OUString     aName;

    // compare all pages including the objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( mpDoc->GetPage( nPage ) );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            if( !pEntry )
                return false;
            aName = GetEntryText( pEntry );

            if( pPage->GetName() != aName )
                return false;

            pEntry = Next( pEntry );

            SdrObjListIter aIter(
                *pPage,
                !pPage->HasObjectNavigationOrder() /* use navigation order, if available */,
                IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();

                const OUString aObjectName( GetObjectName( pObj ) );

                if( !aObjectName.isEmpty() )
                {
                    if( !pEntry )
                        return false;

                    aName = GetEntryText( pEntry );

                    if( aObjectName != aName )
                        return false;

                    pEntry = Next( pEntry );
                }
            }
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
OUString SdPageObjsTLB::GetSelectEntry()
{
    return GetEntryText( GetCurEntry() );
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
                SdPage* pPage = static_cast<SdPage*>( mpBookmarkDoc->GetPage( nPage ) );
                if( pPage->GetPageKind() == PK_STANDARD )
                {
                    pPageEntry = InsertEntry( pPage->GetName(),
                                              aImgPage,
                                              aImgPage,
                                              pFileEntry,
                                              false,
                                              TREELIST_APPEND,
                                              reinterpret_cast< void* >( 1 ) );

                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        OUString aStr( GetObjectName( pObj ) );
                        if( !aStr.isEmpty() )
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
            mxBookmarkDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD, true);
            if (mxBookmarkDocShRef->DoLoad(pMed))
                mpBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            else
                mpBookmarkDoc = NULL;
        }
        else if ( mpMedium )
            // in this mode the document is owned and controlled by the SdDrawDocument
            // it can be released by calling the corresponding CloseBookmarkDoc method
            // successful creation of a document makes this the owner of the medium
            mpBookmarkDoc = const_cast<SdDrawDocument*>(mpDoc)->OpenBookmarkDoc(*mpMedium);

        if ( !mpBookmarkDoc )
        {
            ScopedVclPtrInstance< MessageDialog > aErrorBox(this, SD_RESSTR(STR_READ_DATA_ERROR));
            aErrorBox->Execute();
            mpMedium = 0; //On failure the SfxMedium is invalid
        }
    }

    return mpBookmarkDoc;
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
            const_cast<SdDrawDocument*>(mpDoc)->CloseBookmarkDoc();
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

    mbLinkableSelected = true;

    while( pEntry && mbLinkableSelected )
    {
        if( NULL == pEntry->GetUserData() )
            mbLinkableSelected = false;

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
        if(bisInSdNavigatorWin)
        {
            bool bMarked=false;
            SvTreeListEntry* pNewEntry = GetCurEntry();
            if (!pNewEntry)
                return;
            SvTreeListEntry* pParentEntry = GetParent(pNewEntry);
            if (!pParentEntry)
                return;
            OUString  aStr(GetSelectEntry());
            sd::DrawDocShell* pSdDrawDocShell = SdNavigatorWin::GetDrawDocShell(mpDoc);
            if (pSdDrawDocShell)
            {
                pSdDrawDocShell->GotoTreeBookmark(aStr);
                bMarked = pSdDrawDocShell->GetObjectIsmarked(aStr);
            }
            pNewEntry->SetMarked(bMarked);
            Invalidate();
        }
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

    if (mpFrame->HasChildWindow(SID_NAVIGATOR))
    {
        SfxChildWindow* pWnd = mpFrame->GetChildWindow(SID_NAVIGATOR);
        pNavWin = pWnd ? static_cast<SdNavigatorWin*>(pWnd->GetContextWindow(SD_MOD())) : NULL;
    }

    if (pEntry != NULL
        && pNavWin !=NULL
        && pNavWin == mpParent
        && pNavWin->GetNavigatorDragType() != NAVIGATOR_DRAGTYPE_NONE )
    {
        // Mark only the children of the page under the mouse as drop
        // targets.  This prevents moving shapes from one page to another.

        // Select all entries and disable them as drop targets.
        SetSelectionMode(MULTIPLE_SELECTION);
        SetCursor(static_cast<SvTreeListEntry*>(NULL));
        SelectAll(true, false);
        EnableSelectionAsDropTarget(false);

        // Enable only the entries as drop targets that are children of the
        // page under the mouse.
        SvTreeListEntry* pParent = GetRootLevelParent(pEntry);
        if (pParent != NULL)
        {
            SelectAll(false, false);
            Select(pParent);
            //            for (SvTreeListEntry*pChild=FirstChild(pParent); pChild!=NULL; pChild=NextSibling(pChild))
            //                Select(pChild, sal_True);
            EnableSelectionAsDropTarget();//sal_False);
        }

        // Set selection back to the entry under the mouse.
        SelectAll(false, false);
        SetSelectionMode(SINGLE_SELECTION);
        Select(pEntry);

        // We can delete the Navigator from ExecuteDrag (when switching to
        // another document type), but that would kill the StarView MouseMove
        // Handler which is calling Command().
        // For this reason, Drag&Drop is asynchronous.
        Application::PostUserEvent( LINK( this, SdPageObjsTLB, ExecDragHdl ), NULL, true );
    }
}

/**
 * Begin drag
 */
void SdPageObjsTLB::DoDrag()
{
    SfxChildWindow* pWnd = mpFrame->HasChildWindow(SID_NAVIGATOR) ? mpFrame->GetChildWindow(SID_NAVIGATOR) : NULL;
    mpDropNavWin = pWnd ? static_cast<SdNavigatorWin*>(pWnd->GetContextWindow(SD_MOD())) : NULL;

    if( mpDropNavWin )
    {
        ::sd::DrawDocShell* pDocShell = mpDoc->GetDocSh();
        OUString aURL = INetURLObject( pDocShell->GetMedium()->GetPhysicalName(), INetProtocol::File ).GetMainURL( INetURLObject::NO_DECODE );
        NavigatorDragType   eDragType = mpDropNavWin->GetNavigatorDragType();

        aURL += "#" + GetSelectEntry();

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

        bIsInDrag = true;

        // object is destroyed by internal reference mechanism
        SdTransferable* pTransferable =
                new SdPageObjsTLB::SdPageObjsTransferable(
                            *this, aBookmark, *pDocShell, eDragType);

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
        if (pUserData != NULL && pUserData != reinterpret_cast<void*>(1))
            pObject = static_cast<SdrObject*>(pUserData);
        if (pObject != NULL)
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
}

void SdPageObjsTLB::OnDragFinished( sal_uInt8 )
{
    if( mpFrame->HasChildWindow( SID_NAVIGATOR ) )
    {
        SfxChildWindow* pWnd = mpFrame->GetChildWindow(SID_NAVIGATOR);
        SdNavigatorWin* pNewNavWin = pWnd ? static_cast<SdNavigatorWin*>(pWnd->GetContextWindow(SD_MOD())) : NULL;
        if (mpDropNavWin == pNewNavWin)
        {
            MouseEvent aMEvt( mpDropNavWin->GetPointerPosPixel() );
            SvTreeListBox::MouseButtonUp( aMEvt );
        }
    }

    mpDropNavWin = NULL;
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
            SdNavigatorWin* pNavWin = NULL;
            sal_uInt16          nId = SID_NAVIGATOR;

            if (mpFrame->HasChildWindow(nId))
            {
                SfxChildWindow* pWnd = mpFrame->GetChildWindow(nId);
                pNavWin = pWnd ? static_cast<SdNavigatorWin*>(pWnd->GetContextWindow(SD_MOD())) : NULL;
            }

            if( pNavWin && ( pNavWin == mpParent ) )
            {
                TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );
                OUString                aFile;

                if( aDataHelper.GetString( SotClipboardFormatId::SIMPLE_FILE, aFile ) &&
                    static_cast<SdNavigatorWin*>(mpParent.get())->InsertFile( aFile ) )
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
IMPL_LINK_NOARG_TYPED(SdPageObjsTLB, ExecDragHdl, void*, void)
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
    while (GetParent(pDestination) != NULL && GetParent(GetParent(pDestination)) != NULL)
        pDestination = GetParent(pDestination);

    SdrObject* pTargetObject = static_cast<SdrObject*>(pDestination->GetUserData());
    SdrObject* pSourceObject = static_cast<SdrObject*>(pEntry->GetUserData());
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
        if (GetParent(pDestination) == NULL)
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
        do
        {
            pNext = dynamic_cast<SvTreeListEntry*>(NextVisible(pEntry, &nDepth));
            if (pNext != NULL && nDepth > 0 && nDepth!=0xffff)
                pEntry = pNext;
            else
                break;
        }
        while (pEntry != NULL);
    }

    return pEntry;
}

bool SdPageObjsTLB::IsDropAllowed (SvTreeListEntry* pEntry)
{
    if (pEntry == NULL)
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
        aObjectDescriptor.maDisplayName.clear();
    aObjectDescriptor.mbCanLink = false;

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
    return NULL;
}

//===== IconProvider ==========================================================

SdPageObjsTLB::IconProvider::IconProvider()
    : maImgPage( BitmapEx( SdResId( BMP_PAGE ) ) ),
      maImgPageExcl( BitmapEx( SdResId( BMP_PAGE_EXCLUDED ) ) ),
      maImgPageObjsExcl( BitmapEx( SdResId( BMP_PAGEOBJS_EXCLUDED ) ) ),
      maImgPageObjs( BitmapEx( SdResId( BMP_PAGEOBJS ) ) ),
      maImgObjects( BitmapEx( SdResId( BMP_OBJECTS ) ) ),
      maImgGroup( BitmapEx( SdResId( BMP_GROUP ) ) )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
