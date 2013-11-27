/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
#include <comphelper/processfactory.hxx>

#include "strmname.h"
#include "sdtreelb.hxx"
#include "DrawDocShell.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "navigatr.hxx"
#ifndef _SD_CFGID_HXX
#include "strings.hrc"
#endif
#include "res_bmp.hrc"
#include "DrawController.hxx"
#include "ViewShellBase.hxx"

#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <svtools/embedtransfer.hxx>
#include <tools/diagnose_ex.h>
#include <ViewShell.hxx>
#include <svx/svdlegacy.hxx>

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

    // High contrast icons.
    Image maImgPageH;
    Image maImgPageExclH;
    Image maImgPageObjsExclH;
    Image maImgPageObjsH;
    Image maImgObjectsH;
    Image maImgGroupH;
};


bool SD_DLLPRIVATE SdPageObjsTLB::bIsInDrag = false;

void SdPageObjsTLB::SetViewFrame( SfxViewFrame* pViewFrame )
{
    mpFrame = pViewFrame;
}

bool SdPageObjsTLB::IsLinkableSelected() const
{
    return mbLinkableSelected;
}

bool SdPageObjsTLB::IsInDrag()
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
    : SdTransferable(rDocShell.GetDoc(), NULL, true),
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
        ( 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = (sal_Int64)(sal_IntPtr)this;
    }
    else
        nRet = SdTransferable::getSomething(rId);

    return nRet;
}

// -----------------------------------------------------------------------------

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdPageObjsTLB::SdPageObjsTransferable::getUnoTunnelId()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;

    if( !aSeq.getLength() )
    {
        static osl::Mutex   aCreateMutex;
        osl::MutexGuard     aGuard( aCreateMutex );

        aSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
    }

    return aSeq;
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
        mnListBoxDropFormatId = SotExchange::RegisterFormatMimeType(
            ::rtl::OUString::createFromAscii(
                "application/x-openoffice-treelistbox-moveonly;"
                    "windows_formatname=\"SV_LBOX_DD_FORMAT_MOVE\""));
    return mnListBoxDropFormatId;
}

/*************************************************************************
|*
|* Ctor1 SdPageObjsTLB
|*
\************************************************************************/

SdPageObjsTLB::SdPageObjsTLB( Window* pParentWin, const SdResId& rSdResId )
:   SvTreeListBox       ( pParentWin, rSdResId )
,   bisInSdNavigatorWin  ( sal_False )
,   mpParent            ( pParentWin )
,   mpDoc               ( NULL )
,   mpBookmarkDoc       ( NULL )
,   mpMedium            ( NULL )
,   mpOwnMedium         ( NULL )
,   maImgOle             ( BitmapEx( SdResId( BMP_OLE ) ) )
,   maImgGraphic         ( BitmapEx( SdResId( BMP_GRAPHIC ) ) )
,   maImgOleH            ( BitmapEx( SdResId( BMP_OLE_H ) ) )
,   maImgGraphicH        ( BitmapEx( SdResId( BMP_GRAPHIC_H ) ) )
,   mbLinkableSelected  ( false )
,   mpDropNavWin        ( NULL )
,   mbShowAllShapes     ( false )
,   mbShowAllPages      ( false )
{
    // Tree-ListBox mit Linien versehen
    SetStyle( GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                           WB_HASBUTTONS | // WB_HASLINESATROOT |
                           WB_HSCROLL | // #31562#
                           WB_HASBUTTONSATROOT |
                           WB_QUICK_SEARCH /* i31275 */ );
    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND ) ),
                    Bitmap( SdResId( BMP_COLLAPSE ) ) );

    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND_H ) ),
                    Bitmap( SdResId( BMP_COLLAPSE_H ) ),
                    BMP_COLOR_HIGHCONTRAST );

    SetDragDropMode(
         SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY |
            SV_DRAGDROP_APP_MOVE  | SV_DRAGDROP_APP_COPY  | SV_DRAGDROP_APP_DROP );
}

/*************************************************************************
|*
|* Dtor SdPageObjsTLB
|*
\************************************************************************/

SdPageObjsTLB::~SdPageObjsTLB()
{
    if ( mpBookmarkDoc )
        CloseBookmarkDoc();
    else
        // no document was created from mpMedium, so this object is still the owner of it
        delete mpMedium;
}

//IAccessibility2 Implementation 2009-----
// helper function for  GetEntryAltText and GetEntryLongDescription
String SdPageObjsTLB::getAltLongDescText( SvLBoxEntry* pEntry , sal_Bool isAltText) const
{
    sal_uInt16 maxPages = mpDoc->GetPageCount();
    sal_uInt16 pageNo;
    SdrObject*   pObj = NULL;
    SdPage* pPage = NULL;


    String ParentName = GetEntryText( GetRootLevelParent( pEntry ) );

    for( pageNo = 0;  pageNo < maxPages; pageNo++ )
    {
        pPage = (SdPage*) mpDoc->GetPage( pageNo );
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
    return String();

}

String SdPageObjsTLB::GetEntryAltText( SvLBoxEntry* pEntry ) const
{
    return getAltLongDescText( pEntry, sal_True );
}

String SdPageObjsTLB::GetEntryLongDescription( SvLBoxEntry* pEntry ) const
{
    return getAltLongDescText( pEntry, sal_False);
}

void  SdPageObjsTLB::MarkCurEntry( const String& rName )
{

    if( rName.Len() )
    {
        SvLBoxEntry* pCurEntry =GetCurEntry();
        SvLBoxEntry* pEntry =NULL;
        String aTmp1;
        String aTmp2;

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
                    // IA2 CWS. MT: Removed in SvLBoxEntry for now - only used in Sw/Sd/ScContentLBoxString, they should decide if they need this
                    pEntry->SetMarked(sal_False);
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
                    pEntry->SetMarked(sal_True);
                }
                else
                {
                        pEntry->SetMarked(sal_False);
                }
            }
              }
    }
    Invalidate();
}

void  SdPageObjsTLB:: FreshCurEntry()
{
       SvLBoxEntry* pEntry =NULL;
       for( pEntry = First(); pEntry ; pEntry = Next( pEntry ) )
    {
                pEntry->SetMarked(sal_False);
    }
    Invalidate();
}

class SdContentLBoxString : public SvLBoxString
{
public:
    SdContentLBoxString( SvLBoxEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags,
        SvLBoxEntry* pEntry);
};

void SdPageObjsTLB::InitEntry(SvLBoxEntry* pEntry,
        const XubString& rStr ,const Image& rImg1,const Image& rImg2,SvLBoxButtonKind eButtonKind)
{
    sal_uInt16 nColToHilite = 1; //0==Bitmap;1=="Spalte1";2=="Spalte2"
    SvTreeListBox::InitEntry( pEntry, rStr, rImg1, rImg2, eButtonKind );
    SvLBoxString* pCol = (SvLBoxString*)pEntry->GetItem( nColToHilite );
    SdContentLBoxString* pStr = new SdContentLBoxString( pEntry, 0, pCol->GetText() );
    pEntry->ReplaceItem( pStr, nColToHilite );
}

void SdContentLBoxString::Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags,
    SvLBoxEntry* pEntry )
{
    // IA2 CWS. MT: Removed for now (also in SvLBoxEntry) - only used in Sw/Sd/ScContentLBoxString, they should decide if they need this
    /*
    if (pEntry->IsMarked())
    {
            rDev.DrawText( rPos, GetText() );
            XubString str;
            str = XubString::CreateFromAscii("*");
            Point rPosStar(rPos.X()-6,rPos.Y());
            Font aOldFont( rDev.GetFont());
            Font aFont(aOldFont);
            Color aCol( aOldFont.GetColor() );
            aCol.DecreaseLuminance( 200 );
            aFont.SetColor( aCol );
            rDev.SetFont( aFont );
            rDev.DrawText( rPosStar, str);
            rDev.SetFont( aOldFont );
    }
    else
    */
        SvLBoxString::Paint( rPos, rDev, nFlags, pEntry);

}

void SdPageObjsTLB::SaveExpandedTreeItemState(SvLBoxEntry* pEntry, vector<String>& vectTreeItem)
{
    if (pEntry)
    {
        SvLBoxEntry* pListEntry = pEntry;
        while (pListEntry)
        {
            if (pListEntry->HasChilds())
            {
                if (IsExpanded(pListEntry))
                    vectTreeItem.push_back(GetEntryText(pListEntry));
                SvLBoxEntry* pChildEntry = FirstChild(pListEntry);
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
        maSelectionEntryText = String();
        maTreeItem.clear();
        if (GetCurEntry())
            maSelectionEntryText = GetSelectEntry();
        SvLBoxEntry* pEntry = FirstChild(NULL);
        SaveExpandedTreeItemState(pEntry, maTreeItem);
    }
    return SvTreeListBox::Clear();
}
//-----IAccessibility2 Implementation 2009
/*************************************************************************
|*
|* return name of object
|*
\************************************************************************/

String SdPageObjsTLB::GetObjectName(
    const SdrObject* pObject,
    const bool bCreate) const
{
    String aRet;

    if ( pObject )
    {
        aRet = pObject->GetName();

        if( !aRet.Len() )
        {
            const SdrOle2Obj* pSdrOle2Obj = dynamic_cast< const SdrOle2Obj* >(pObject);

            if(pSdrOle2Obj)
            {
                aRet = pSdrOle2Obj->GetPersistName();
            }
        }
    }

    if (bCreate
        && mbShowAllShapes
        && aRet.Len() == 0
        && pObject!=NULL)
    {
        aRet = SdResId(STR_NAVIGATOR_SHAPE_BASE_NAME);
        aRet.SearchAndReplaceAscii("%1", String::CreateFromInt32(pObject->GetNavigationPosition() + 1));
    }

    return aRet;
}

/*************************************************************************
|*
|* In TreeLB Eintrag selektieren
|*
\************************************************************************/

bool SdPageObjsTLB::SelectEntry( const String& rName )
{
    bool bFound = false;

    if( rName.Len() )
    {
        SvLBoxEntry* pEntry = NULL;
        String aTmp;

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
    return( bFound );
}

/*************************************************************************
|*
|* Gibt zurueck, ob Childs des uebergebenen Strings selektiert sind
|*
\************************************************************************/

bool SdPageObjsTLB::HasSelectedChilds( const String& rName )
{
    bool bFound  = false;
    bool bChilds = false;

    if( rName.Len() )
    {
        SvLBoxEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = true;
                bool bExpanded = IsExpanded( pEntry );
                long nCount = GetChildSelectionCount( pEntry );
                if( bExpanded && nCount > 0 )
                    bChilds = true;
            }
        }
    }
    return( bChilds );
}


/*************************************************************************
|*
|* TreeLB mit Seiten und Objekten fuellen
|*
\************************************************************************/

void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, bool bAllPages, const String& rDocName)
{
    String aSelection;
    if( GetSelectionCount() > 0 )
    {
        aSelection = GetSelectEntry();
        Clear();
    }

    mpDoc = pInDoc;
    maDocName = rDocName;
    mbShowAllPages = bAllPages;
    mpMedium = NULL;

    SdPage*      pPage = NULL;

    IconProvider aIconProvider;

    // first insert all pages including objects
    sal_uInt16 nPage = 0;
    const sal_uInt32 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) mpDoc->GetPage( nPage );
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

    // dann alle MasterPages incl. Objekte einfuegen
    if( mbShowAllPages )
    {
        nPage = 0;
        const sal_uInt32 nMaxMasterPages = mpDoc->GetMasterPageCount();

        while( nPage < nMaxMasterPages )
        {
            pPage = (SdPage*) mpDoc->GetMasterPage( nPage );
            AddShapeList(*pPage, NULL, pPage->GetName(), false, NULL, aIconProvider);
            nPage++;
        }
    }
    if( aSelection.Len() )
        SelectEntry( aSelection );
    else if (mbSaveTreeItemState && maSelectionEntryText.Len())
    {
        SelectEntry(maSelectionEntryText);
    }
}

/*************************************************************************
|*
|* Es wird nur der erste Eintrag eingefuegt. Childs werden OnDemand erzeugt
|*
\************************************************************************/

void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, SfxMedium* pInMedium,
                          const String& rDocName )
{
    mpDoc = pInDoc;

    // this object now owns the Medium
    mpMedium = pInMedium;
    maDocName = rDocName;

    Image aImgDocOpen=Image( BitmapEx( SdResId( BMP_DOC_OPEN ) ) );
    Image aImgDocClosed=Image( BitmapEx( SdResId( BMP_DOC_CLOSED ) ) );
    Image aImgDocOpenH=Image( BitmapEx( SdResId( BMP_DOC_OPEN_H ) ) );
    Image aImgDocClosedH=Image( BitmapEx( SdResId( BMP_DOC_CLOSED_H ) ) );

    // Dokumentnamen einfuegen
    SvLBoxEntry* pFileEntry = InsertEntry( maDocName,
                              aImgDocOpen,
                              aImgDocClosed,
                              NULL,
                              true,
                              LIST_APPEND,
                              reinterpret_cast< void* >( 1 ) );

    SetExpandedEntryBmp( pFileEntry, aImgDocOpenH, BMP_COLOR_HIGHCONTRAST );
    SetCollapsedEntryBmp( pFileEntry, aImgDocClosedH, BMP_COLOR_HIGHCONTRAST );
}




void SdPageObjsTLB::AddShapeList (
    const SdrObjList& rList,
    SdrObject* pShape,
    const ::rtl::OUString& rsName,
    const bool bIsExcluded,
    SvLBoxEntry* pParentEntry,
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

    SvLBoxEntry* pEntry = InsertEntry(
        rsName,
        aIcon,
        aIcon,
        pParentEntry,
        false,
        LIST_APPEND,
        pUserData);

    SetExpandedEntryBmp(
        pEntry,
        bIsExcluded ? rIconProvider.maImgPageExclH : rIconProvider.maImgPageH,
        BMP_COLOR_HIGHCONTRAST );
    SetCollapsedEntryBmp(
        pEntry,
        bIsExcluded ? rIconProvider.maImgPageExclH : rIconProvider.maImgPageH,
        BMP_COLOR_HIGHCONTRAST );

    SdrObjListIter aIter(
        rList,
        IM_FLAT,
        false /*not reverse*/);

    //IAccessibility2 Implementation 2009-----
    sal_Bool  bMarked=sal_False;
    if(bisInSdNavigatorWin)
    {
        Window* pWindow=NULL;
        SdNavigatorWin* pSdNavigatorWin=NULL;
        sd::DrawDocShell* pSdDrawDocShell = NULL;
        if(pEntry)
            pWindow=(Window*)GetParent(pEntry);
        if(pWindow)
            pSdNavigatorWin = (SdNavigatorWin*)pWindow;
        if( pSdNavigatorWin )
            pSdDrawDocShell = pSdNavigatorWin->GetDrawDocShell(mpDoc);
        if(pSdDrawDocShell)
            bMarked=pSdDrawDocShell->IsMarked(pShape);
        if(pEntry)
        {
            if(bMarked)
                pEntry->SetMarked(sal_True);
            else
                pEntry->SetMarked( sal_False );
        }
    }
    //-----IAccessibility2 Implementation 2009
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
                SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgOle, maImgOle, pEntry,
                    false, LIST_APPEND, pObj);
                //IAccessibility2 Implementation 2009-----
                if(bisInSdNavigatorWin)
                {
                    Window* pWindow=NULL;
                    SdNavigatorWin* pSdNavigatorWin=NULL;
                    sd::DrawDocShell* pSdDrawDocShell = NULL;
                    if(pNewEntry)
                        pWindow=(Window*)GetParent(pNewEntry);
                    if(pWindow)
                        pSdNavigatorWin = (SdNavigatorWin*)pWindow;
                    if( pSdNavigatorWin )
                        pSdDrawDocShell = pSdNavigatorWin->GetDrawDocShell(mpDoc);
                    if(pSdDrawDocShell)
                        bMarked=pSdDrawDocShell->IsMarked((SdrObject*)pObj);
                    if(pNewEntry)
                    {
                        if(bMarked)
                            pNewEntry->SetMarked(sal_True);
                        else
                            pNewEntry->SetMarked( sal_False );
                    }
                }
                //-----IAccessibility2 Implementation 2009
                SetExpandedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                SetCollapsedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
            }
            else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgGraphic, maImgGraphic, pEntry,
                    false, LIST_APPEND, pObj );
                //IAccessibility2 Implementation 2009-----
                if(bisInSdNavigatorWin)
                {
                    Window* pWindow=NULL;
                    SdNavigatorWin* pSdNavigatorWin=NULL;
                    sd::DrawDocShell* pSdDrawDocShell = NULL;
                    if(pNewEntry)
                        pWindow=(Window*)GetParent(pNewEntry);
                    if(pWindow)
                        pSdNavigatorWin = (SdNavigatorWin*)pWindow;
                    if( pSdNavigatorWin )
                        pSdDrawDocShell = pSdNavigatorWin->GetDrawDocShell(mpDoc);
                    if(pSdDrawDocShell)
                        bMarked=pSdDrawDocShell->IsMarked((SdrObject*)pObj);
                    if(pNewEntry)
                    {
                        if(bMarked)
                        {
                            pNewEntry->SetMarked(sal_True);
                        }
                        else
                        {
                            pNewEntry->SetMarked( sal_False );
                        }
                    }
                }
                //-----IAccessibility2 Implementation 2009
                SetExpandedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                SetCollapsedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
            }
            else if (pObj->getChildrenOfSdrObject())
            {
                AddShapeList(
                    *pObj->getChildrenOfSdrObject(),
                    pObj,
                    aStr,
                    false,
                    pEntry,
                    rIconProvider);
            }
            else
            {
                SvLBoxEntry* pNewEntry = InsertEntry( aStr, rIconProvider.maImgObjects, rIconProvider.maImgObjects, pEntry,
                    false, LIST_APPEND, pObj );
                //IAccessibility2 Implementation 2009-----
                if(bisInSdNavigatorWin)
                {
                    Window* pWindow=NULL;
                    SdNavigatorWin* pSdNavigatorWin=NULL;
                    sd::DrawDocShell* pSdDrawDocShell = NULL;
                    if(pNewEntry)
                        pWindow=(Window*)GetParent(pNewEntry);
                    if(pWindow)
                        pSdNavigatorWin = (SdNavigatorWin*)pWindow;
                    if( pSdNavigatorWin )
                        pSdDrawDocShell = pSdNavigatorWin->GetDrawDocShell(mpDoc);
                    if(pSdDrawDocShell)
                        bMarked=pSdDrawDocShell->IsMarked((SdrObject*)pObj);
                    if(pNewEntry)
                    {
                        if(bMarked)
                        {
                            pNewEntry->SetMarked(sal_True);
                        }
                        else
                        {
                            pNewEntry->SetMarked( sal_False );
                        }
                    }
                }
                //-----IAccessibility2 Implementation 2009
                SetExpandedEntryBmp( pNewEntry, rIconProvider.maImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                SetCollapsedEntryBmp( pNewEntry, rIconProvider.maImgObjectsH, BMP_COLOR_HIGHCONTRAST );
            }
        }
    }

    if( pEntry->HasChilds() )
    {
        SetExpandedEntryBmp(
            pEntry,
            bIsExcluded ? rIconProvider.maImgPageObjsExcl : rIconProvider.maImgPageObjs);
        SetCollapsedEntryBmp(
            pEntry,
            bIsExcluded ? rIconProvider.maImgPageObjsExcl : rIconProvider.maImgPageObjs);
        SetExpandedEntryBmp(
            pEntry,
            bIsExcluded ? rIconProvider.maImgPageObjsExclH : rIconProvider.maImgPageObjsH,
            BMP_COLOR_HIGHCONTRAST);
        SetCollapsedEntryBmp(
            pEntry,
            bIsExcluded ? rIconProvider.maImgPageObjsExclH : rIconProvider.maImgPageObjsH,
            BMP_COLOR_HIGHCONTRAST);
        //IAccessibility2 Implementation 2009-----
        if (mbSaveTreeItemState)
        {
            vector<String>:: iterator iteStart = maTreeItem.begin();
            while (iteStart != maTreeItem.end())
            {
                String strEntry = GetEntryText(pEntry);
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
        //-----IAccessibility2 Implementation 2009
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




/*************************************************************************
|*
|* Prueft, ob die Seiten (PK_STANDARD) und die darauf befindlichen Objekte
|* des Docs und der TreeLB identisch sind.
|* Wird ein Doc uebergeben, wird dieses zum aktuellem Doc (Wichtig bei
|* mehreren Documenten).
|*
\************************************************************************/

bool SdPageObjsTLB::IsEqualToDoc( const SdDrawDocument* pInDoc )
{
    if( pInDoc )
        mpDoc = pInDoc;

    if( !mpDoc )
        return( false );

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvLBoxEntry* pEntry = First();
    String       aName;

    // Alle Pages incl. Objekte vergleichen
    sal_uInt16 nPage = 0;
    const sal_uInt32 nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) mpDoc->GetPage( nPage );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            if( !pEntry )
                return( false );
            aName = GetEntryText( pEntry );

            if( pPage->GetName() != aName )
                return( false );

            pEntry = Next( pEntry );

            SdrObjListIter aIter(
                *pPage,
                IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();

                const String aObjectName( GetObjectName( pObj ) );

                if( aObjectName.Len() )
                {
                    if( !pEntry )
                        return( false );

                    aName = GetEntryText( pEntry );

                    if( aObjectName != aName )
                        return( false );

                    pEntry = Next( pEntry );
                }
            }
        }
        nPage++;
    }
    // Wenn noch Eintraege in der Listbox vorhanden sind, wurden
    // Objekte (mit Namen) oder Seiten geloescht
    return( !pEntry );
}

/*************************************************************************
|*
|* Selectierten String zurueckgeben
|*
\************************************************************************/

String SdPageObjsTLB::GetSelectEntry()
{
    return( GetEntryText( GetCurEntry() ) );
}

/*************************************************************************
|*
|* Selektierte Eintrage zurueckgeben
|* nDepth == 0 -> Seiten
|* nDepth == 1 -> Objekte
|*
\************************************************************************/

List* SdPageObjsTLB::GetSelectEntryList( sal_uInt16 nDepth )
{
    List*        pList  = NULL;
    SvLBoxEntry* pEntry = FirstSelected();

    while( pEntry )
    {
        sal_uInt16 nListDepth = GetModel()->GetDepth( pEntry );
        if( nListDepth == nDepth )
        {
            if( !pList )
                pList = new List();

            const String aEntryText( GetEntryText( pEntry ) );
            pList->Insert( new String( aEntryText ), LIST_APPEND );
        }
        pEntry = NextSelected( pEntry );
    }

    return( pList );
}

/*************************************************************************
|*
|* Eintraege werden erst auf Anforderung (Doppelklick) eingefuegt
|*
\************************************************************************/

void SdPageObjsTLB::RequestingChilds( SvLBoxEntry* pFileEntry )
{
    if( !pFileEntry->HasChilds() )
    {
        if( GetBookmarkDoc() )
        {
            SdrObject*   pObj = NULL;
            SdPage*      pPage = NULL;
            SvLBoxEntry* pPageEntry = NULL;

            Image aImgPage=Image( BitmapEx( SdResId( BMP_PAGE ) ) );
            Image aImgPageObjs=Image( BitmapEx( SdResId( BMP_PAGEOBJS ) ) );
            Image aImgObjects=Image( BitmapEx( SdResId( BMP_OBJECTS ) ) );
            Image aImgPageH=Image( BitmapEx( SdResId( BMP_PAGE_H ) ) );
            Image aImgPageObjsH=Image( BitmapEx( SdResId( BMP_PAGEOBJS_H ) ) );
            Image aImgObjectsH=Image( BitmapEx( SdResId( BMP_OBJECTS_H ) ) );

            // document name already inserted

            // only insert all "normal" ? slides with objects
            sal_uInt16 nPage = 0;
            const sal_uInt32 nMaxPages = mpBookmarkDoc->GetPageCount();

            while( nPage < nMaxPages )
            {
                pPage = (SdPage*) mpBookmarkDoc->GetPage( nPage );
                if( pPage->GetPageKind() == PK_STANDARD )
                {
                    pPageEntry = InsertEntry( pPage->GetName(),
                                              aImgPage,
                                              aImgPage,
                                              pFileEntry,
                                              false,
                                              LIST_APPEND,
                                              reinterpret_cast< void* >( 1 ) );

                    SetExpandedEntryBmp( pPageEntry, aImgPageH, BMP_COLOR_HIGHCONTRAST );
                    SetCollapsedEntryBmp( pPageEntry, aImgPageH, BMP_COLOR_HIGHCONTRAST );

                    SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        String aStr( GetObjectName( pObj ) );
                        if( aStr.Len() )
                        {
                            if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
                            {
                                SvLBoxEntry* pNewEntry = InsertEntry(aStr, maImgOle, maImgOle, pPageEntry);


                                SetExpandedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                                SetCollapsedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                            }
                            else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
                            {
                                SvLBoxEntry* pNewEntry = InsertEntry(aStr, maImgGraphic, maImgGraphic, pPageEntry);

                                SetExpandedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                                SetCollapsedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                            }
                            else
                            {
                                SvLBoxEntry* pNewEntry = InsertEntry(aStr, aImgObjects, aImgObjects, pPageEntry);

                                SetExpandedEntryBmp( pNewEntry, aImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                                SetCollapsedEntryBmp( pNewEntry, aImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                            }
                        }
                    }
                    if( pPageEntry->HasChilds() )
                    {
                        SetExpandedEntryBmp( pPageEntry, aImgPageObjs );
                        SetCollapsedEntryBmp( pPageEntry, aImgPageObjs );
                        SetExpandedEntryBmp( pPageEntry, aImgPageObjsH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pPageEntry, aImgPageObjsH, BMP_COLOR_HIGHCONTRAST );
                    }
                }
                nPage++;
            }
        }
    }
    else
        SvTreeListBox::RequestingChilds( pFileEntry );
}

/*************************************************************************
|*
|*  Prueft, ob es sich um eine Draw-Datei handelt und oeffnet anhand des
|*  uebergebenen Docs das BookmarkDoc
|*
\************************************************************************/

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
            // it looks that it is undefined if a Medium was set by Fill() allready
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
            mxBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, true);
            if (mxBookmarkDocShRef->DoLoad(pMed))
                mpBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            else
                mpBookmarkDoc = NULL;
        }
        else if ( mpMedium )
            // in this mode the document is owned and controlled by the SdDrawDocument
            // it can be released by calling the corresponding CloseBookmarkDoc method
            // successfull creation of a document makes this the owner of the medium
            mpBookmarkDoc = ((SdDrawDocument*) mpDoc)->OpenBookmarkDoc(*mpMedium);

        if ( !mpBookmarkDoc )
        {
            ErrorBox aErrorBox( this, WB_OK, String( SdResId( STR_READ_DATA_ERROR ) ) );
            aErrorBox.Execute();
            mpMedium = 0; //On failure the SfxMedium is invalid
        }
    }

    return( mpBookmarkDoc );
}

/*************************************************************************
|*
|* Bookmark-Dokument schlieáen und loeschen
|*
\************************************************************************/

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
        // perhaps mpOwnMedium provided, but no successfull creation of BookmarkDoc
        delete mpOwnMedium;
        mpOwnMedium = NULL;
    }

    mpBookmarkDoc = NULL;
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SdPageObjsTLB::SelectHdl()
{
    SvLBoxEntry* pEntry = FirstSelected();

    mbLinkableSelected = true;

    while( pEntry && mbLinkableSelected )
    {
        if( NULL == pEntry->GetUserData() )
            mbLinkableSelected = false;

        pEntry = NextSelected( pEntry );
    }

    SvTreeListBox::SelectHdl();
}

/*************************************************************************
|*
|* Ueberlaedt RETURN mit der Funktionsweise von DoubleClick
|*
\************************************************************************/

void SdPageObjsTLB::KeyInput( const KeyEvent& rKEvt )
{
    if( rKEvt.GetKeyCode().GetCode() == KEY_RETURN )
    {
        // Auskommentierter Code aus svtools/source/contnr/svimpbox.cxx
        SvLBoxEntry* pCursor = GetCurEntry();
        if( pCursor->HasChilds() || pCursor->HasChildsOnDemand() )
        {
            if( IsExpanded( pCursor ) )
                Collapse( pCursor );
            else
                Expand( pCursor );
        }

        DoubleClickHdl();
    }
//IAccessibility2 Implementation 2009-----
    else if (rKEvt.GetKeyCode().GetCode() == KEY_SPACE)
    {
       if(bisInSdNavigatorWin)
       {
           sal_Bool bMarked=sal_False;
           SvLBoxEntry* pNewEntry = GetCurEntry();
           if( GetParent(pNewEntry) == NULL )
               return;
           String  aStr=GetSelectEntry();
           Window* pWindow=NULL;
           SdNavigatorWin* pSdNavigatorWin=NULL;
           sd::DrawDocShell* pSdDrawDocShell = NULL;
           if(pNewEntry)
               pWindow=(Window*)GetParent(pNewEntry);
           if(pWindow)
               pSdNavigatorWin = (SdNavigatorWin*)pWindow;
           if( pSdNavigatorWin )
               pSdDrawDocShell = pSdNavigatorWin->GetDrawDocShell(mpDoc);
           if(pSdDrawDocShell)
           {
               pSdDrawDocShell->GotoTreeBookmark(aStr);
               bMarked=pSdDrawDocShell->GetObjectIsmarked(aStr);
           }
           //Removed by yanjun for sym2_6385
           //The symphony2.0 can support morn than one level tree list, also support to select tow or more items in different level.
           /*
           SvLBoxEntry* pBeginEntry = First();
           if( pBeginEntry )
           {
               if( GetParent(pBeginEntry) !=  GetParent(pNewEntry) )
                   pBeginEntry->SetMarked( sal_False );
               SvLBoxEntry* pNextEntry = Next( pBeginEntry );
               while( pNextEntry )
               {
                   if( GetParent(pNextEntry) !=  GetParent(pNewEntry) )
                       pNextEntry->SetMarked( sal_False );
                   pNextEntry =  Next( pNextEntry );
               }
           }
           End*/
           if(pNewEntry)
           {
               if(bMarked)
               {
                   pNewEntry->SetMarked(sal_True);
               }
               else
               {
                   pNewEntry->SetMarked( sal_False );
               }
           }
           Invalidate();
       }
    }
//-----IAccessibility2 Implementation 2009
    else
        SvTreeListBox::KeyInput( rKEvt );
}

/*************************************************************************
|*
|* StartDrag-Request
|*
\************************************************************************/

void SdPageObjsTLB::StartDrag( sal_Int8 nAction, const Point& rPosPixel)
{
    (void)nAction;
    (void)rPosPixel;

    SdNavigatorWin* pNavWin = NULL;
    SvLBoxEntry* pEntry = GetEntry(rPosPixel);

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
        SetCursor(NULL, false);
        SelectAll(true, false);
        EnableSelectionAsDropTarget(false, true);

        // Enable only the entries as drop targets that are children of the
        // page under the mouse.
        SvLBoxEntry* pParent = GetRootLevelParent(pEntry);
        if (pParent != NULL)
        {
            SelectAll(false, false);
            Select(pParent, true);
            EnableSelectionAsDropTarget(true, true);//false);
        }

        // Set selection back to the entry under the mouse.
        SelectAll(false,false);
        SetSelectionMode(SINGLE_SELECTION);
        Select(pEntry, true);

        //  Aus dem ExecuteDrag heraus kann der Navigator geloescht werden
        //  (beim Umschalten auf einen anderen Dokument-Typ), das wuerde aber
        //  den StarView MouseMove-Handler, der Command() aufruft, umbringen.
        //  Deshalb Drag&Drop asynchron:
        Application::PostUserEvent( STATIC_LINK( this, SdPageObjsTLB, ExecDragHdl ) );
    }
}

/*************************************************************************
|*
|* Begin drag
|*
\************************************************************************/

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
            nDNDActions = DND_ACTION_LINK;  // #93240# Either COPY *or* LINK, never both!

        SvTreeListBox::ReleaseMouse();

        bIsInDrag = true;

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
        OSL_TRACE("created new SdPageObjsTransferable at %x", pTransferable);

        // Get the view.
        sd::View* pView = NULL;
        if (pDocShell != NULL)
        {
            ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
            if (pViewShell != NULL)
                pView = pViewShell->GetView();
        }
        if (pView == NULL)
        {
            OSL_ASSERT(pView!=NULL);
            return;
        }

        SdrObject* pObject = NULL;
        void* pUserData = GetCurEntry()->GetUserData();
        if (pUserData != NULL && pUserData != (void*)1)
            pObject = reinterpret_cast<SdrObject*>(pUserData);
        if (pObject == NULL)
            return;

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
        pView->UnmarkAllObj();
        pView->MarkObj(*pObject);

        pTransferable->StartDrag( this, nDNDActions );
    }
}

/*************************************************************************
|*
|* Drag finished
|*
\************************************************************************/

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
    bIsInDrag = false;
}

/*************************************************************************
|*
|* AcceptDrop-Event
|*
\************************************************************************/

sal_Int8 SdPageObjsTLB::AcceptDrop (const AcceptDropEvent& rEvent)
{
    sal_Int8 nResult (DND_ACTION_NONE);

    if ( !bIsInDrag && IsDropFormatSupported( FORMAT_FILE ) )
    {
        nResult = rEvent.mnAction;
    }
    else
    {
        SvLBoxEntry* pEntry = GetDropTarget(rEvent.maPosPixel);
        if (rEvent.mbLeaving || !CheckDragAndDropMode( this, rEvent.mnAction ))
        {
            ImplShowTargetEmphasis( pTargetEntry, false );
        }
        else if( !nDragDropMode )
        {
            DBG_ERRORFILE( "SdPageObjsTLB::AcceptDrop(): no target" );
        }
        else if (IsDropAllowed(pEntry))
        {
            nResult = DND_ACTION_MOVE;

            // Draw emphasis.
            if (pEntry != pTargetEntry || !(nImpFlags & SVLBOX_TARGEMPH_VIS))
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

/*************************************************************************
|*
|* ExecuteDrop-Event
|*
\************************************************************************/

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
                String                  aFile;

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

/*************************************************************************
|*
|* Handler fuers Dragging
|*
\************************************************************************/

IMPL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*, EMPTYARG)
{
    //  als Link, damit asynchron ohne ImpMouseMoveMsg auf dem Stack auch der
    //  Navigator geloescht werden darf
    pThis->DoDrag();
    return 0;
}


bool SdPageObjsTLB::PageBelongsToCurrentShow (const SdPage* pPage) const
{
    // Return <true/> as default when there is no custom show or when none
    // is used.  The page does then belong to the standard show.
    bool bBelongsToShow = true;

    if (mpDoc->getPresentationSettings().mbCustomShow)
    {
        // Get the current custom show.
        SdCustomShow* pCustomShow = NULL;
        List* pShowList = const_cast<SdDrawDocument*>(mpDoc)->GetCustomShowList();
        if (pShowList != NULL)
        {
            sal_uLong nCurrentShowIndex = pShowList->GetCurPos();
            void* pObject = pShowList->GetObject(nCurrentShowIndex);
            pCustomShow = static_cast<SdCustomShow*>(pObject);
        }

        // Check whether the given page is part of that custom show.
        if (pCustomShow != NULL)
        {
            bBelongsToShow = false;
            sal_uLong nPageCount = pCustomShow->Count();
            for (sal_uInt16 i=0; i<nPageCount && !bBelongsToShow; i++)
                if (pPage == static_cast<SdPage*>(pCustomShow->GetObject (i)))
                    bBelongsToShow = true;
        }
    }

    return bBelongsToShow;
}




sal_Bool SdPageObjsTLB::NotifyMoving(
    SvLBoxEntry* pTarget,
    SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewParent,
    sal_uLong& rNewChildPos)
{
    SvLBoxEntry* pDestination = pTarget;
    while (GetParent(pDestination) != NULL && GetParent(GetParent(pDestination)) != NULL)
        pDestination = GetParent(pDestination);

    SdrObject* pTargetObject = reinterpret_cast<SdrObject*>(pDestination->GetUserData());
    SdrObject* pSourceObject = reinterpret_cast<SdrObject*>(pEntry->GetUserData());
    if (pSourceObject == reinterpret_cast<SdrObject*>(1))
        pSourceObject = NULL;

    if (pTargetObject && pSourceObject )
    {
        SdrPage* pObjectList = pSourceObject->getSdrPageFromSdrObject();
        if (pObjectList )
        {
            sal_uInt32 nNewPosition;
            if (pTargetObject == reinterpret_cast<SdrObject*>(1))
                nNewPosition = 0;
            else
                nNewPosition = pTargetObject->GetNavigationPosition() + 1;
            pObjectList->SetUserNavigationPosition(*pSourceObject, nNewPosition);
        }

        // Update the tree list.
        if (!pTarget)
        {
            rpNewParent = 0;
            rNewChildPos = 0;
            return true;
        }
        else if ( !GetParent(pDestination) )
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
        return true;
    }
    else
        return false;
}




SvLBoxEntry* SdPageObjsTLB::GetDropTarget (const Point& rLocation)
{
    SvLBoxEntry* pEntry = SvTreeListBox::GetDropTarget(rLocation);
    if (pEntry == NULL)
        return NULL;

        OSL_TRACE("entry is %s",
            ::rtl::OUStringToOString(GetEntryText(pEntry), RTL_TEXTENCODING_UTF8).getStr());
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
        SvLBoxEntry* pNext;
        sal_uInt16 nDepth (0);
        while (pEntry != NULL)
        {
            pNext = dynamic_cast<SvLBoxEntry*>(NextVisible(pEntry, &nDepth));
            if (pNext != NULL && nDepth > 0 && nDepth!=0xffff)
                pEntry = pNext;
            else
                break;
        }
        OSL_TRACE("returning %s",
            ::rtl::OUStringToOString(GetEntryText(pEntry), RTL_TEXTENCODING_UTF8).getStr());
    }

    return pEntry;
}




bool SdPageObjsTLB::IsDropAllowed (SvLBoxEntry* pEntry)
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
        bIsDescriptorFillingPending = false;
        pDocShell->FillTransferableObjectDescriptor(aObjectDescriptor);
    }

    const basegfx::B2DPoint aDragPos(rObject.getObjectRange(0).getCenter());
    aObjectDescriptor.maDragStartPos = Point(basegfx::fround(aDragPos.getX()), basegfx::fround(aDragPos.getY()));

    if (pDocShell != NULL)
        aObjectDescriptor.maDisplayName = pDocShell->GetMedium()->GetURLObject().GetURLNoPass();
    else
        aObjectDescriptor.maDisplayName = String();
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
        uno::Reference<lang::XMultiServiceFactory> xFactory (
            ::comphelper::getProcessServiceFactory ());
        if ( ! xFactory.is())
            return NULL;

        uno::Reference<frame::XDesktop> xDesktop (xFactory->createInstance (
                ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), uno::UNO_QUERY);
        if ( ! xDesktop.is())
            return NULL;

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
    catch (uno::Exception e)
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
      maImgGroup( BitmapEx( SdResId( BMP_GROUP ) ) ),

      maImgPageH( BitmapEx( SdResId( BMP_PAGE_H ) ) ),
      maImgPageExclH( BitmapEx( SdResId( BMP_PAGE_EXCLUDED_H ) ) ),
      maImgPageObjsExclH( BitmapEx( SdResId( BMP_PAGEOBJS_EXCLUDED_H ) ) ),
      maImgPageObjsH( BitmapEx( SdResId( BMP_PAGEOBJS_H ) ) ),
      maImgObjectsH( BitmapEx( SdResId( BMP_OBJECTS_H ) ) ),
      maImgGroupH( BitmapEx( SdResId( BMP_GROUP_H ) ) )
{
}
