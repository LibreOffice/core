/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdtreelb.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:09:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <sal/types.h>

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SD_CUSSHOW_HXX
#include "cusshow.hxx"
#endif
#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif

#include <sfx2/viewfrm.hxx>

#include "strmname.h"
#ifndef _SDTREELB_HXX
#include "sdtreelb.hxx"
#endif
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef _SDPAGE_HXX
#include "sdpage.hxx"
#endif
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#ifndef SD_NAVIGATOR_HXX
#include "navigatr.hxx"
#endif
#ifndef _SD_CFGID_HXX
#include "strings.hrc"
#endif
#include "res_bmp.hrc"

using namespace com::sun::star;

BOOL SdPageObjsTLB::bIsInDrag = FALSE;

// -----------------------------------------
// - SdPageObjsTLB::SdPageObjsTransferable -
// -----------------------------------------

SdPageObjsTLB::SdPageObjsTransferable::~SdPageObjsTransferable()
{
}

// -----------------------------------------------------------------------------

void SdPageObjsTLB::SdPageObjsTransferable::AddSupportedFormats()
{
    AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
}

// -----------------------------------------------------------------------------

sal_Bool SdPageObjsTLB::SdPageObjsTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    SetINetBookmark( maBookmark, rFlavor );
    return sal_True;
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
        nRet = 0;

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

/*************************************************************************
|*
|* Ctor1 SdPageObjsTLB
|*
\************************************************************************/

SdPageObjsTLB::SdPageObjsTLB( Window* pParentWin, const SdResId& rSdResId )
:   SvTreeListBox       ( pParentWin, rSdResId )
,   mpParent            ( pParentWin )
,   mpDoc               ( NULL )
,   mpBookmarkDoc       ( NULL )
,   mpMedium            ( NULL )
,   mpOwnMedium         ( NULL )
,   maTransparencyColor ( RGB_COLORDATA (0xff,0,0xff) )
,   maImgOle            ( Bitmap( SdResId( BMP_OLE ) ), maTransparencyColor )
,   maImgGraphic        ( Bitmap( SdResId( BMP_GRAPHIC ) ), maTransparencyColor )
,   maImgOleH           ( Bitmap( SdResId( BMP_OLE_H ) ), maTransparencyColor)
,   maImgGraphicH       ( Bitmap( SdResId( BMP_GRAPHIC_H ) ), maTransparencyColor)
,   mbLinkableSelected  ( FALSE )
,   mpDropNavWin        ( NULL )
{
    // Tree-ListBox mit Linien versehen
    SetWindowBits( WinBits( WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS | // WB_HASLINESATROOT |
                            WB_HSCROLL | // #31562#
                            WB_HASBUTTONSATROOT ) );
    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND ) ),
                    Bitmap( SdResId( BMP_COLLAPSE ) ) );

    SetNodeBitmaps( Bitmap( SdResId( BMP_EXPAND_H ) ),
                    Bitmap( SdResId( BMP_COLLAPSE_H ) ),
                    BMP_COLOR_HIGHCONTRAST );
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

/*************************************************************************
|*
|* return name of object
|*
\************************************************************************/

String SdPageObjsTLB::GetObjectName( const SdrObject* pObj ) const
{
    String aRet;

    if( pObj )
    {
        aRet = pObj->GetName();

        if( !aRet.Len() && pObj->ISA( SdrOle2Obj ) )
            aRet = static_cast< const SdrOle2Obj* >( pObj )->GetPersistName();
    }

    return aRet;
}

/*************************************************************************
|*
|* In TreeLB Eintrag selektieren
|*
\************************************************************************/

BOOL SdPageObjsTLB::SelectEntry( const String& rName )
{
    BOOL bFound = FALSE;

    if( rName.Len() )
    {
        SvLBoxEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = TRUE;
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

BOOL SdPageObjsTLB::HasSelectedChilds( const String& rName )
{
    BOOL bFound  = FALSE;
    BOOL bChilds = FALSE;

    if( rName.Len() )
    {
        SvLBoxEntry* pEntry = NULL;
        String aTmp;

        for( pEntry = First(); pEntry && !bFound; pEntry = Next( pEntry ) )
        {
            aTmp = GetEntryText( pEntry );
            if( aTmp == rName )
            {
                bFound = TRUE;
                BOOL bExpanded = IsExpanded( pEntry );
                long nCount = GetChildSelectionCount( pEntry );
#if SUPD < 335
                nCount++;
#endif
                if( bExpanded && nCount > 0 )
                    bChilds = TRUE;
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

void SdPageObjsTLB::Fill( const SdDrawDocument* pInDoc, BOOL bAllPages,
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

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvLBoxEntry* pEntry = NULL;

    Bitmap aBmpPage( SdResId( BMP_PAGE ) );
    Image aImgPage( aBmpPage, maTransparencyColor );
    Bitmap aBmpPageExcl( SdResId( BMP_PAGE_EXCLUDED ) );
    Image aImgPageExcl( aBmpPageExcl, maTransparencyColor );
    Bitmap aBmpPageObjsExcl( SdResId( BMP_PAGEOBJS_EXCLUDED ) );
    Image aImgPageObjsExcl( aBmpPageObjsExcl, maTransparencyColor );
    Bitmap aBmpPageObjs( SdResId( BMP_PAGEOBJS ) );
    Image aImgPageObjs( aBmpPageObjs, maTransparencyColor );
    Bitmap aBmpObjects( SdResId( BMP_OBJECTS ) );
    Image aImgObjects( aBmpObjects, maTransparencyColor );

    Bitmap aBmpPageH( SdResId( BMP_PAGE_H ) );
    Image aImgPageH( aBmpPageH , maTransparencyColor );
    Bitmap aBmpPageExclH( SdResId( BMP_PAGE_EXCLUDED_H ) );
    Image aImgPageExclH( aBmpPageExclH, maTransparencyColor );
    Bitmap aBmpPageObjExclH( SdResId( BMP_PAGEOBJS_EXCLUDED_H ) );
    Image aImgPageObjsExclH( aBmpPageObjExclH, maTransparencyColor );
    Bitmap aBmpPageObjsH( SdResId( BMP_PAGEOBJS_H ) );
    Image aImgPageObjsH( aBmpPageObjsH, maTransparencyColor );
    Bitmap aBmpObjectsH( SdResId( BMP_OBJECTS_H ) );
    Image aImgObjectsH( aBmpObjectsH, maTransparencyColor );

    // Zuerst alle Pages incl. Objekte einfuegen
    USHORT nPage = 0;
    const USHORT nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) mpDoc->GetPage( nPage );
        if(  (bAllPages || pPage->GetPageKind() == PK_STANDARD)
             && !(pPage->GetPageKind()==PK_HANDOUT)   ) //#94954# never list the normal handout page ( handout-masterpage is used instead )
        {
            BOOL bPageExluded = pPage->IsExcluded();

            bool bPageBelongsToShow = PageBelongsToCurrentShow (pPage);
            bPageExluded |= !bPageBelongsToShow;

            pEntry = InsertEntry( pPage->GetName(),
                                  bPageExluded ? aImgPageExcl : aImgPage,
                                  bPageExluded ? aImgPageExcl : aImgPage,
                                  0,
                                  FALSE,
                                  LIST_APPEND,
                                  reinterpret_cast< void* >( 1 ) );

            SetExpandedEntryBmp( pEntry, bPageExluded ? aImgPageExclH : aImgPageH, BMP_COLOR_HIGHCONTRAST );
            SetCollapsedEntryBmp( pEntry, bPageExluded ? aImgPageExclH : aImgPageH, BMP_COLOR_HIGHCONTRAST );

            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();
                String aStr( GetObjectName( pObj ) );
                if( aStr.Len() )
                {
                    if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
                    {
                        SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgOle, maImgOle, pEntry );

                        SetExpandedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                    }
                    else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
                    {
                        SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgGraphic, maImgGraphic, pEntry );

                        SetExpandedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                    }
                    else
                    {
                        SvLBoxEntry* pNewEntry = InsertEntry( aStr, aImgObjects, aImgObjects, pEntry );

                        SetExpandedEntryBmp( pNewEntry, aImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pNewEntry, aImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                    }
                }
            }
            if( pEntry->HasChilds() )
            {
                SetExpandedEntryBmp( pEntry, bPageExluded ? aImgPageObjsExcl : aImgPageObjs );
                SetCollapsedEntryBmp( pEntry, bPageExluded ? aImgPageObjsExcl : aImgPageObjs );
                SetExpandedEntryBmp( pEntry, bPageExluded ? aImgPageObjsExclH : aImgPageObjsH, BMP_COLOR_HIGHCONTRAST );
                SetCollapsedEntryBmp( pEntry, bPageExluded ? aImgPageObjsExclH : aImgPageObjsH, BMP_COLOR_HIGHCONTRAST );
            }
        }
        nPage++;
    }

    // dann alle MasterPages incl. Objekte einfuegen
    if( bAllPages )
    {
        nPage = 0;
        const USHORT nMaxMasterPages = mpDoc->GetMasterPageCount();

        while( nPage < nMaxMasterPages )
        {
            pPage = (SdPage*) mpDoc->GetMasterPage( nPage );
            pEntry = InsertEntry( pPage->GetName(),
                                  aImgPage,
                                  aImgPage,
                                  0,
                                  FALSE,
                                  LIST_APPEND,
                                  reinterpret_cast< void* >( 1 ) );

            SetExpandedEntryBmp( pEntry, aImgPageH, BMP_COLOR_HIGHCONTRAST );
            SetCollapsedEntryBmp( pEntry, aImgPageH, BMP_COLOR_HIGHCONTRAST );

            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();
                String aStr( GetObjectName( pObj ) );
                if( aStr.Len() )
                {
                    if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_OLE2 )
                    {
                        SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgOle, maImgOle, pEntry ); // pEntry entspr. Parent

                        SetExpandedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                    }
                    else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
                    {
                        SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgGraphic, maImgGraphic, pEntry ); // pEntry entspr. Parent

                        SetExpandedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                    }
                    else
                    {
                        SvLBoxEntry* pNewEntry = InsertEntry( aStr, aImgObjects, aImgObjects, pEntry );

                        SetExpandedEntryBmp( pNewEntry, aImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                        SetCollapsedEntryBmp( pNewEntry, aImgObjectsH, BMP_COLOR_HIGHCONTRAST );
                    }
                }
            }
            if( pEntry->HasChilds() )
            {
                SetExpandedEntryBmp( pEntry, aImgPageObjs );
                SetCollapsedEntryBmp( pEntry, aImgPageObjs );
                SetExpandedEntryBmp( pEntry, aImgPageObjsH, BMP_COLOR_HIGHCONTRAST );
                SetCollapsedEntryBmp( pEntry, aImgPageObjsH, BMP_COLOR_HIGHCONTRAST );
            }
            nPage++;
        }
    }
    if( aSelection.Len() )
        SelectEntry( aSelection );
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

    Bitmap aBmpDocOpen( SdResId( BMP_DOC_OPEN ) );
    Image aImgDocOpen( aBmpDocOpen, maTransparencyColor );
    Bitmap aBmpDocClosed( SdResId( BMP_DOC_CLOSED ) );
    Image aImgDocClosed( aBmpDocClosed, maTransparencyColor );
    Bitmap aBmpDocOpenH( SdResId( BMP_DOC_OPEN_H ) );
    Image aImgDocOpenH( aBmpDocOpenH, maTransparencyColor );
    Bitmap aBmpDocClosedH( SdResId( BMP_DOC_CLOSED_H ) );
    Image aImgDocClosedH( aBmpDocClosedH, maTransparencyColor );

    // Dokumentnamen einfuegen
    SvLBoxEntry* pFileEntry = InsertEntry( maDocName,
                              aImgDocOpen,
                              aImgDocClosed,
                              NULL,
                              TRUE,
                              LIST_APPEND,
                              reinterpret_cast< void* >( 1 ) );

    SetExpandedEntryBmp( pFileEntry, aImgDocOpenH, BMP_COLOR_HIGHCONTRAST );
    SetCollapsedEntryBmp( pFileEntry, aImgDocClosedH, BMP_COLOR_HIGHCONTRAST );
}

/*************************************************************************
|*
|* Prueft, ob die Seiten (PK_STANDARD) und die darauf befindlichen Objekte
|* des Docs und der TreeLB identisch sind.
|* Wird ein Doc uebergeben, wird dieses zum aktuellem Doc (Wichtig bei
|* mehreren Documenten).
|*
\************************************************************************/

BOOL SdPageObjsTLB::IsEqualToDoc( const SdDrawDocument* pInDoc )
{
    if( pInDoc )
        mpDoc = pInDoc;

    if( !mpDoc )
        return( FALSE );

    SdrObject*   pObj = NULL;
    SdPage*      pPage = NULL;
    SvLBoxEntry* pEntry = First();
    String       aName;

    // Alle Pages incl. Objekte vergleichen
    USHORT nPage = 0;
    const USHORT nMaxPages = mpDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        pPage = (SdPage*) mpDoc->GetPage( nPage );
        if( pPage->GetPageKind() == PK_STANDARD )
        {
            if( !pEntry )
                return( FALSE );
            aName = GetEntryText( pEntry );

            if( pPage->GetName() != aName )
                return( FALSE );

            pEntry = Next( pEntry );

            SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );

            while( aIter.IsMore() )
            {
                pObj = aIter.Next();

                const String aObjectName( GetObjectName( pObj ) );

                if( aObjectName.Len() )
                {
                    if( !pEntry )
                        return( FALSE );

                    aName = GetEntryText( pEntry );

                    if( aObjectName != aName )
                        return( FALSE );

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

List* SdPageObjsTLB::GetSelectEntryList( USHORT nDepth )
{
    List*        pList  = NULL;
    SvLBoxEntry* pEntry = FirstSelected();

    while( pEntry )
    {
        USHORT nListDepth = GetModel()->GetDepth( pEntry );
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
|* Alle Pages (und Objekte) des Docs zurueckgeben
|* nType == 0 -> Seiten
|* nType == 1 -> Objekte
|*
\************************************************************************/

List* SdPageObjsTLB::GetBookmarkList( USHORT nType )
{
    List* pList = NULL;

    if( GetBookmarkDoc() )
    {
        SdPage*      pPage = NULL;
        String*      pName = NULL;
        USHORT       nPage = 0;
        const USHORT nMaxPages = mpBookmarkDoc->GetSdPageCount( PK_STANDARD );

        while( nPage < nMaxPages )
        {
            pPage = mpBookmarkDoc->GetSdPage( nPage, PK_STANDARD );

            if( nType == 0 ) // Seitennamen einfuegen
            {
                if( !pList )
                    pList = new List();

                pName = new String( pPage->GetRealName() );
                pList->Insert( pName, LIST_APPEND );
            }
            else // Objektnamen einfuegen
            {
                // Ueber Objekte der Seite iterieren
                SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                while( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    String aStr( GetObjectName( pObj ) );
                    if( aStr.Len() )
                    {
                        if( !pList )
                            pList = new List();

                        pName = new String( aStr );
                        pList->Insert( pName, LIST_APPEND );
                    }
                }
            }
            nPage++;
        }
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

            Bitmap aBmpPage( SdResId( BMP_PAGE ) );
            Image aImgPage( aBmpPage, maTransparencyColor );
            Bitmap aBmpPageObjs( SdResId( BMP_PAGEOBJS ) );
            Image aImgPageObjs( aBmpPageObjs, maTransparencyColor );
            Bitmap aBmpObjects( SdResId( BMP_OBJECTS ) );
            Image aImgObjects( aBmpObjects, maTransparencyColor );
            Bitmap aBmpPageH( SdResId( BMP_PAGE_H ) );
            Image aImgPageH( aBmpPageH, maTransparencyColor );
            Bitmap aBmpPageObjsH( SdResId( BMP_PAGEOBJS_H ) );
            Image aImgPageObjsH( aBmpPageObjsH, maTransparencyColor );
            Bitmap aBmpImgObjectsH( SdResId( BMP_OBJECTS_H ) );
            Image aImgObjectsH( aBmpImgObjectsH, maTransparencyColor );

            // Dokumentname ist schon eingefuegt

            // Nur alle "normalen" Pages mit Objekten einfuegen
            USHORT nPage = 0;
            const USHORT nMaxPages = mpBookmarkDoc->GetPageCount();

            while( nPage < nMaxPages )
            {
                pPage = (SdPage*) mpBookmarkDoc->GetPage( nPage );
                if( pPage->GetPageKind() == PK_STANDARD )
                {
                    pPageEntry = InsertEntry( pPage->GetName(),
                                              aImgPage,
                                              aImgPage,
                                              pFileEntry,
                                              FALSE,
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
                                SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgOle, maImgOle, pPageEntry );

                                SetExpandedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                                SetCollapsedEntryBmp( pNewEntry, maImgOleH, BMP_COLOR_HIGHCONTRAST );
                            }
                            else if( pObj->GetObjInventor() == SdrInventor && pObj->GetObjIdentifier() == OBJ_GRAF )
                            {
                                SvLBoxEntry* pNewEntry = InsertEntry( aStr, maImgGraphic, maImgGraphic, pPageEntry ); // pEntry entspr. Parent

                                SetExpandedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                                SetCollapsedEntryBmp( pNewEntry, maImgGraphicH, BMP_COLOR_HIGHCONTRAST );
                            }
                            else
                            {
                                SvLBoxEntry* pNewEntry = InsertEntry( aStr, aImgObjects, aImgObjects, pPageEntry );

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
    if (!mpBookmarkDoc ||
        pMed && (!mpOwnMedium || mpOwnMedium->GetName() != pMed->GetName()))
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
            mxBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, TRUE);
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

    mbLinkableSelected = TRUE;

    while( pEntry && mbLinkableSelected )
    {
        if( NULL == pEntry->GetUserData() )
            mbLinkableSelected = FALSE;

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
    else
        SvTreeListBox::KeyInput( rKEvt );
}

/*************************************************************************
|*
|* StartDrag-Request
|*
\************************************************************************/

void SdPageObjsTLB::StartDrag( sal_Int8, const Point& )
{
    SdNavigatorWin* pNavWin = NULL;

    if( mpFrame->HasChildWindow( SID_NAVIGATOR ) )
        pNavWin = (SdNavigatorWin*) ( mpFrame->GetChildWindow( SID_NAVIGATOR )->GetContextWindow( SD_MOD() ) );

    if( pNavWin && pNavWin == mpParent && pNavWin->GetNavigatorDragType() != NAVIGATOR_DRAGTYPE_NONE )
    {
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
        sal_Int8        nDNDActions = DND_ACTION_COPY;

        if( eDragType == NAVIGATOR_DRAGTYPE_LINK )
            nDNDActions = DND_ACTION_LINK;  // #93240# Either COPY *or* LINK, never both!

        SvTreeListBox::ReleaseMouse();

        bIsInDrag = TRUE;

        // object is destroyed by internal reference mechanism
        ( new SdPageObjsTLB::SdPageObjsTransferable( *this, aBookmark, *pDocShell, eDragType ) )->StartDrag( this, nDNDActions );
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
    bIsInDrag = FALSE;
}

/*************************************************************************
|*
|* AcceptDrop-Event
|*
\************************************************************************/

sal_Int8 SdPageObjsTLB::AcceptDrop( const AcceptDropEvent& rEvt )
{
    return( ( !bIsInDrag && IsDropFormatSupported( FORMAT_FILE ) ) ? rEvt.mnAction : DND_ACTION_NONE );
}

/*************************************************************************
|*
|* ExecuteDrop-Event
|*
\************************************************************************/

sal_Int8 SdPageObjsTLB::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( !bIsInDrag )
    {
        SdNavigatorWin* pNavWin = NULL;
        USHORT          nId = SID_NAVIGATOR;

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
    // Return <TRUE/> as default when there is no custom show or when none
    // is used.  The page does then belong to the standard show.
    bool bBelongsToShow = true;

    if (mpDoc->getPresentationSettings().mbCustomShow)
    {
        // Get the current custom show.
        SdCustomShow* pCustomShow = NULL;
        List* pShowList = const_cast<SdDrawDocument*>(mpDoc)->GetCustomShowList();
        if (pShowList != NULL)
        {
            ULONG nCurrentShowIndex = pShowList->GetCurPos();
            void* pObject = pShowList->GetObject(nCurrentShowIndex);
            pCustomShow = static_cast<SdCustomShow*>(pObject);
        }

        // Check whether the given page is part of that custom show.
        if (pCustomShow != NULL)
        {
            bBelongsToShow = false;
            ULONG nPageCount = pCustomShow->Count();
            for (USHORT i=0; i<nPageCount && !bBelongsToShow; i++)
                if (pPage == static_cast<SdPage*>(pCustomShow->GetObject (i)))
                    bBelongsToShow = true;
        }
    }

    return bBelongsToShow;
}
