/*************************************************************************
 *
 *  $RCSfile: hlmarkwn.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pw $ $Date: 2000-11-22 13:44:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

// UNO-Stuff
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XLINKTARGETSUPPLIER_HPP_
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <toolkit/unohlp.hxx>

#include "dialogs.hrc"
#include "hlmarkwn.hrc"
#include "hlmarkwn.hxx"
#include "hltpbase.hxx"

using namespace ::com::sun::star;
using namespace ::rtl;

/*************************************************************************
|*
|* Userdata-struct for tree-entries
|*
|************************************************************************/

struct TargetData
{
    OUString        aUStrLinkname;
    BOOL        bIsTarget;

    TargetData ( OUString aUStrLName, BOOL bTarget )
        :   bIsTarget ( bTarget )
    {
        if ( bIsTarget )
            aUStrLinkname = aUStrLName;
    }
};


//########################################################################
//#                                                                      #
//# Tree-Window                                                          #
//#                                                                      #
//########################################################################

SvxHlmarkTreeLBox::SvxHlmarkTreeLBox( Window* pParent, const ResId& rResId )
: SvTreeListBox ( pParent, rResId ),
  mpParentWnd   ( (SvxHlinkDlgMarkWnd*) pParent )
{}

void SvxHlmarkTreeLBox::Paint( const Rectangle& rRect )
{
    if( mpParentWnd->mnError == LERR_NOERROR )
    {
        SvTreeListBox::Paint(rRect);
    }
    else
    {
        Erase();

        Rectangle aDrawRect( Point( 0, 0 ), GetSizePixel() );

        String aStrMessage;

        switch( mpParentWnd->mnError )
        {
        case LERR_NOENTRIES :
            aStrMessage = SVX_RESSTR( RID_SVXSTR_HYPDLG_ERR_LERR_NOENTRIES );
            break;
        case LERR_DOCNOTOPEN :
            aStrMessage = SVX_RESSTR( RID_SVXSTR_HYPDLG_ERR_LERR_DOCNOTOPEN );
            break;
        }

        DrawText( aDrawRect, aStrMessage, TEXT_DRAW_LEFT | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
    }

}

//########################################################################
//#                                                                      #
//# Window-Class                                                         #
//#                                                                      #
//########################################################################

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHlinkDlgMarkWnd::SvxHlinkDlgMarkWnd( SvxHyperlinkTabPageBase *pParent )
:   FloatingWindow( (Window*)pParent, SVX_RES ( RID_SVXFLOAT_HYPERLINK_MARKWND ) ),
    maBtApply( this, ResId (BT_APPLY) ),
    maBtClose( this, ResId (BT_CLOSE) ),
    maLbTree ( this, ResId (TLB_MARK) ),
    mbUserMoved ( FALSE ),
    mbFirst     ( TRUE ),
    mpParent    ( pParent ),
    mnError     ( LERR_NOERROR )
{
    FreeResource();

    maBtApply.SetClickHdl       ( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );
    maBtClose.SetClickHdl       ( LINK ( this, SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl ) );
    maLbTree.SetDoubleClickHdl  ( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );

    // Tree-ListBox mit Linien versehen
    maLbTree.SetWindowBits( WinBits( WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS |  //WB_HASLINESATROOT |
                            WB_HSCROLL | WB_HASBUTTONSATROOT ) );

    maLbTree.SetNodeBitmaps( Bitmap( ResId( RID_SVXBMP_HYPDLG_EXPAND ) ),
                             Bitmap( ResId( RID_SVXBMP_HYPDLG_COLLAPSE ) ) );
}

SvxHlinkDlgMarkWnd::~SvxHlinkDlgMarkWnd()
{
    ClearTree();
}

/*************************************************************************
|*
|* Set an errorstatus
|*
|************************************************************************/

USHORT SvxHlinkDlgMarkWnd::SetError( USHORT nError)
{
    USHORT nOldError = mnError;
    mnError = nError;

    if( mnError != LERR_NOERROR )
        ClearTree();

    maLbTree.Invalidate();

    return nOldError;
}

/*************************************************************************
|*
|* Move window
|*
|************************************************************************/

const BOOL SvxHlinkDlgMarkWnd::MoveTo ( Point aNewPos )
{
    if ( !mbUserMoved )
    {
        BOOL bOldStatus = mbUserMoved;
        SetPosPixel ( aNewPos );
        mbUserMoved = bOldStatus;
    }

    return mbUserMoved;
}

void SvxHlinkDlgMarkWnd::Move ()
{
    Window::Move();

    if ( IsReallyVisible() )
        mbUserMoved = TRUE;
}

const BOOL SvxHlinkDlgMarkWnd::ConnectToDialog( BOOL bDoit )
{
    BOOL bOldStatus = mbUserMoved;

    mbUserMoved = !bDoit;

    return bOldStatus;
}

/*************************************************************************
|*
|* Interface to refresh tree
|*
|************************************************************************/

void SvxHlinkDlgMarkWnd::RefreshTree ( String aStrURL )
{
    String aEmptyStr;
    OUString aUStrURL;

    EnterWait();

    ClearTree();

    int nPos = aStrURL.Search ( sal_Unicode('#') );

    if( nPos != 0 )
        aUStrURL = ::rtl::OUString( aStrURL );

    if( !RefreshFromDoc ( aUStrURL ) )
        maLbTree.Invalidate();

    if ( nPos != STRING_NOTFOUND )
    {
        String aStrMark = aStrURL.Copy ( nPos+1 );
        SelectEntry ( aStrMark );
    }

    LeaveWait();

    maStrLastURL = aStrURL;
}

/*************************************************************************
|*
|* get links from document
|*
|************************************************************************/

BOOL SvxHlinkDlgMarkWnd::RefreshFromDoc( OUString aURL )
{
    mnError = LERR_NOERROR;

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    if( xFactory.is() )
    {
        uno::Reference< frame::XDesktop > xDesktop( xFactory->createInstance( OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ),
                    uno::UNO_QUERY );
        if( xDesktop.is() )
        {
            uno::Reference< lang::XComponent > xComp;

            if( aURL.getLength() )
            {
                // load from url
                uno::Reference< frame::XComponentLoader > xLoader( xDesktop, uno::UNO_QUERY );
                if( xLoader.is() )
                {
                    uno::Sequence< beans::PropertyValue > aArg(1);
                    aArg.getArray()[0].Name = OUString::createFromAscii( "Hidden" );
                    aArg.getArray()[0].Value <<= (sal_Bool) TRUE;
                    xComp = xLoader->loadComponentFromURL( aURL, OUString::createFromAscii( "_blank" ), 0,
                                                           aArg );
                }
            }
            else
            {
                // the component with user focus ( current document )
                xComp = xDesktop->getCurrentComponent();
            }

            if( xComp.is() )
            {
                uno::Reference< document::XLinkTargetSupplier > xLTS( xComp, uno::UNO_QUERY );

                if( xLTS.is() )
                {
                    if( FillTree( xLTS->getLinks() ) == 0 )
                        mnError = LERR_NOENTRIES;
                }
                else
                    mnError = LERR_DOCNOTOPEN;

                if ( aURL.getLength() )
                    xComp->dispose();
            }
            else
            {
                if( aURL.getLength() )
                    mnError=LERR_DOCNOTOPEN;
            }
        }
    }
    return (mnError==0);
}
/*
void SvxHlinkDlgMarkWnd::Error(int nNr)
{
    switch(nNr)
    {
        case 0:
        {
            Rectangle aDrawRect( Point( 0, 0 ), maLbTree.GetSizePixel() );
            //maLbTree.SetTextColor( Color(COL_BLACK) );
            //maLbTree.DrawText( aDrawRect, "Keine Ziele im Dokument vorhanden.", TEXT_DRAW_LEFT);// | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
            maLbTree.DrawText( Point(0,0), "Keine Ziele im Dokument vorhanden.");
            maLbTree.DrawLine(aDrawRect.TopLeft(), aDrawRect.BottomRight() );
        }
        break;
        case 1:
            Rectangle aDrawRect( Point( 0, 0 ), maLbTree.GetSizePixel() );
            maLbTree.DrawText( aDrawRect, "Das Dokument konnte nicht geöffnet werden.", TEXT_DRAW_LEFT | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
            break;
    }
}
*/
/*************************************************************************
|*
|* Fill Tree-Control
|*
|************************************************************************/

int SvxHlinkDlgMarkWnd::FillTree( uno::Reference< container::XNameAccess > xLinks, SvLBoxEntry* pParentEntry )
{
    int nEntries=0;
    const uno::Sequence< OUString > aNames( xLinks->getElementNames() );
    const ULONG nLinks = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();

    uno::Any aAny;

    for( ULONG i = 0; i < nLinks; i++ )
    {
        OUString aLink( *pNames++ );

        aAny = xLinks->getByName( aLink );

        uno::Reference< beans::XPropertySet > xTarget;

        if( aAny >>= xTarget )
        {
            try
            {
                // get name to display
                aAny = xTarget->getPropertyValue( OUString::createFromAscii( "LinkDisplayName" ) );
                OUString aDisplayName;
                aAny >>= aDisplayName;
                String aStrDisplayname ( aDisplayName );

                // is it a target ?
                uno::Reference< lang::XServiceInfo > xSI( xTarget, uno::UNO_QUERY );
                BOOL bIsTarget = xSI->supportsService( OUString::createFromAscii( "com.sun.star.document.LinkTarget" ) );

                // create userdata
                TargetData *pData = new TargetData ( aLink, bIsTarget );

                SvLBoxEntry* pEntry;

                try
                {
                    // get bitmap for the tree-entry
                    uno::Any aAny( xTarget->getPropertyValue( OUString::createFromAscii( "LinkDisplayBitmap" ) ) );
                    uno::Reference< awt::XBitmap > aXBitmap;
                    if( aAny >>= aXBitmap )
                    {
                        BitmapEx aBmp( VCLUnoHelper::GetBitmap( aXBitmap ) );

                        // insert Displayname into treelist with bitmaps
                        pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                        aBmp, aBmp,
                                                        pParentEntry,
                                                        FALSE, LIST_APPEND,
                                                        (void*)pData );
                        nEntries++;
                    }
                    else
                    {
                        // insert Displayname into treelist without bitmaps
                        pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                        pParentEntry,
                                                        FALSE, LIST_APPEND,
                                                        (void*)pData );
                        nEntries++;
                    }
                }
                catch(...)
                {
                    // insert Displayname into treelist without bitmaps
                    pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                    pParentEntry,
                                                    FALSE, LIST_APPEND,
                                                    (void*)pData );
                    nEntries++;
                }

                uno::Reference< document::XLinkTargetSupplier > xLTS( xTarget, uno::UNO_QUERY );
                if( xLTS.is() )
                    nEntries += FillTree( xLTS->getLinks(), pEntry );
            }
            catch(...)
            {
            }
        }
    }

    return nEntries;
}

/*************************************************************************
|*
|* Clear Tree
|*
|************************************************************************/

void SvxHlinkDlgMarkWnd::ClearTree()
{
    SvLBoxEntry* pEntry = maLbTree.First();

    while ( pEntry )
    {
        TargetData* pUserData = ( TargetData * ) pEntry->GetUserData();
        delete pUserData;

        pEntry = maLbTree.Next( pEntry );
    }

    maLbTree.Clear();
}

/*************************************************************************
|*
|* Find Entry for Strng
|*
|************************************************************************/

SvLBoxEntry* SvxHlinkDlgMarkWnd::FindEntry ( String aStrName )
{
    BOOL bFound=FALSE;
    SvLBoxEntry* pEntry = maLbTree.First();

    while ( pEntry && !bFound )
    {
        TargetData* pUserData = ( TargetData * ) pEntry->GetUserData ();
        if ( aStrName == String( pUserData->aUStrLinkname ) )
            bFound = TRUE;
        else
            pEntry = maLbTree.Next( pEntry );
    }

    return pEntry;
}

/*************************************************************************
|*
|* Select Entry
|*
|************************************************************************/

void SvxHlinkDlgMarkWnd::SelectEntry ( String aStrMark )
{
    SvLBoxEntry* pEntry = FindEntry ( aStrMark );
    if ( pEntry )
    {
        maLbTree.Select ( pEntry );
        maLbTree.MakeVisible ( pEntry );
    }
}

/*************************************************************************
|*
|* Click on Apply-Button / Doubleclick on item in tree
|*
|************************************************************************/

IMPL_LINK ( SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl, void *, EMPTYARG )
{
    SvLBoxEntry* pEntry = maLbTree.GetCurEntry();

    if ( pEntry )
    {
        TargetData *pData = ( TargetData * )pEntry->GetUserData();

        if ( pData->bIsTarget )
        {
            String aStrMark ( pData->aUStrLinkname );
            mpParent->SetMarkStr ( aStrMark );
        }
    }

    return( 0L );
}

/*************************************************************************
|*
|* Click on Close-Button
|*
|************************************************************************/

IMPL_LINK ( SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl, void *, EMPTYARG )
{
    Hide();

    return( 0L );
}


