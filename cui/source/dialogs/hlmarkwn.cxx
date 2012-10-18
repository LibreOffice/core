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

#include <vcl/wrkwin.hxx>
#include <dialmgr.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

// UNO-Stuff
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <toolkit/unohlp.hxx>

#include <cuires.hrc>
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
    sal_Bool        bIsTarget;

    TargetData ( OUString aUStrLName, sal_Bool bTarget )
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
{
    SetNodeDefaultImages();
}

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
            aStrMessage = CUI_RESSTR( RID_SVXSTR_HYPDLG_ERR_LERR_NOENTRIES );
            break;
        case LERR_DOCNOTOPEN :
            aStrMessage = CUI_RESSTR( RID_SVXSTR_HYPDLG_ERR_LERR_DOCNOTOPEN );
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
:   ModalDialog( (Window*)pParent, CUI_RES ( RID_SVXFLOAT_HYPERLINK_MARKWND ) ),
    maBtApply( this, CUI_RES (BT_APPLY) ),
    maBtClose( this, CUI_RES (BT_CLOSE) ),
    maLbTree ( this, CUI_RES (TLB_MARK) ),
    mbUserMoved ( sal_False ),
    mpParent    ( pParent ),
    mnError     ( LERR_NOERROR )
{
    FreeResource();

    maBtApply.SetClickHdl       ( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );
    maBtClose.SetClickHdl       ( LINK ( this, SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl ) );
    maLbTree.SetDoubleClickHdl  ( LINK ( this, SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl ) );

    // add lines to the Tree-ListBox
    maLbTree.SetStyle( maLbTree.GetStyle() | WB_TABSTOP | WB_BORDER | WB_HASLINES |
                            WB_HASBUTTONS |  //WB_HASLINESATROOT |
                            WB_HSCROLL | WB_HASBUTTONSATROOT );

    maLbTree.SetAccessibleName(String(CUI_RES(STR_MARK_TREE)));

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

sal_uInt16 SvxHlinkDlgMarkWnd::SetError( sal_uInt16 nError)
{
    sal_uInt16 nOldError = mnError;
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

sal_Bool SvxHlinkDlgMarkWnd::MoveTo ( Point aNewPos )
{
    if ( !mbUserMoved )
    {
        sal_Bool bOldStatus = mbUserMoved;
        SetPosPixel ( aNewPos );
        mbUserMoved = bOldStatus;
    }

    return mbUserMoved;
}

void SvxHlinkDlgMarkWnd::Move ()
{
    Window::Move();

    if ( IsReallyVisible() )
        mbUserMoved = sal_True;
}

sal_Bool SvxHlinkDlgMarkWnd::ConnectToDialog( sal_Bool bDoit )
{
    sal_Bool bOldStatus = mbUserMoved;

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

    xub_StrLen nPos = aStrURL.Search ( sal_Unicode('#') );

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

sal_Bool SvxHlinkDlgMarkWnd::RefreshFromDoc( OUString aURL )
{
    mnError = LERR_NOERROR;

    uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    if( xFactory.is() )
    {
        uno::Reference< frame::XDesktop > xDesktop( xFactory->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" )) ),
                    uno::UNO_QUERY );
        if( xDesktop.is() )
        {
            uno::Reference< lang::XComponent > xComp;

            if( !aURL.isEmpty() )
            {
                // load from url
                uno::Reference< frame::XComponentLoader > xLoader( xDesktop, uno::UNO_QUERY );
                if( xLoader.is() )
                {
                    try
                    {
                        uno::Sequence< beans::PropertyValue > aArg(1);
                        aArg.getArray()[0].Name = OUString(RTL_CONSTASCII_USTRINGPARAM( "Hidden" ));
                        aArg.getArray()[0].Value <<= (sal_Bool) sal_True;
                        xComp = xLoader->loadComponentFromURL( aURL, OUString(RTL_CONSTASCII_USTRINGPARAM( "_blank" )), 0, aArg );
                    }
                    catch( const io::IOException& )
                    {

                    }
                    catch( const lang::IllegalArgumentException& )
                    {

                    }
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

                if ( !aURL.isEmpty() )
                    xComp->dispose();
            }
            else
            {
                if( !aURL.isEmpty() )
                    mnError=LERR_DOCNOTOPEN;
            }
        }
    }
    return (mnError==0);
}
/*************************************************************************
|*
|* Fill Tree-Control
|*
|************************************************************************/

int SvxHlinkDlgMarkWnd::FillTree( uno::Reference< container::XNameAccess > xLinks, SvTreeListEntry* pParentEntry )
{
    int nEntries=0;
    const uno::Sequence< OUString > aNames( xLinks->getElementNames() );
    const sal_uLong nLinks = aNames.getLength();
    const OUString* pNames = aNames.getConstArray();

    Color aMaskColor( COL_LIGHTMAGENTA );
    const OUString aProp_LinkDisplayName( RTL_CONSTASCII_USTRINGPARAM( "LinkDisplayName" ) );
    const OUString aProp_LinkTarget( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.LinkTarget" ) );
    const OUString aProp_LinkDisplayBitmap( RTL_CONSTASCII_USTRINGPARAM( "LinkDisplayBitmap" ) );
    for( sal_uLong i = 0; i < nLinks; i++ )
    {
        uno::Any aAny;
        OUString aLink( *pNames++ );

        sal_Bool bError = sal_False;
        try
        {
            aAny = xLinks->getByName( aLink );
        }
        catch(const uno::Exception&)
        {
            // if the name of the target was invalid (like empty headings)
            // no object can be provided
            bError = sal_True;
        }
        if(bError)
            continue;

        uno::Reference< beans::XPropertySet > xTarget;

        if( aAny >>= xTarget )
        {
            try
            {
                // get name to display
                aAny = xTarget->getPropertyValue( aProp_LinkDisplayName );
                OUString aDisplayName;
                aAny >>= aDisplayName;
                String aStrDisplayname ( aDisplayName );

                // is it a target ?
                uno::Reference< lang::XServiceInfo > xSI( xTarget, uno::UNO_QUERY );
                sal_Bool bIsTarget = xSI->supportsService( aProp_LinkTarget );

                // create userdata
                TargetData *pData = new TargetData ( aLink, bIsTarget );

                SvTreeListEntry* pEntry;

                try
                {
                    // get bitmap for the tree-entry
                    uno::Reference< awt::XBitmap > aXBitmap( xTarget->getPropertyValue( aProp_LinkDisplayBitmap ), uno::UNO_QUERY );
                    if( aXBitmap.is() )
                    {
                        Image aBmp( VCLUnoHelper::GetBitmap( aXBitmap ).GetBitmap(), aMaskColor );
                        // insert Displayname into treelist with bitmaps
                        pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                        aBmp, aBmp,
                                                        pParentEntry,
                                                        sal_False, LIST_APPEND,
                                                        (void*)pData );
                        nEntries++;
                    }
                    else
                    {
                        // insert Displayname into treelist without bitmaps
                        pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                        pParentEntry,
                                                        sal_False, LIST_APPEND,
                                                        (void*)pData );
                        nEntries++;
                    }
                }
                catch(const com::sun::star::uno::Exception&)
                {
                    // insert Displayname into treelist without bitmaps
                    pEntry = maLbTree.InsertEntry ( aStrDisplayname,
                                                    pParentEntry,
                                                    sal_False, LIST_APPEND,
                                                    (void*)pData );
                    nEntries++;
                }

                uno::Reference< document::XLinkTargetSupplier > xLTS( xTarget, uno::UNO_QUERY );
                if( xLTS.is() )
                    nEntries += FillTree( xLTS->getLinks(), pEntry );
            }
            catch(const com::sun::star::uno::Exception&)
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
    SvTreeListEntry* pEntry = maLbTree.First();

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

SvTreeListEntry* SvxHlinkDlgMarkWnd::FindEntry ( String aStrName )
{
    sal_Bool bFound=sal_False;
    SvTreeListEntry* pEntry = maLbTree.First();

    while ( pEntry && !bFound )
    {
        TargetData* pUserData = ( TargetData * ) pEntry->GetUserData ();
        if ( aStrName == String( pUserData->aUStrLinkname ) )
            bFound = sal_True;
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
    SvTreeListEntry* pEntry = FindEntry ( aStrMark );
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

IMPL_LINK_NOARG(SvxHlinkDlgMarkWnd, ClickApplyHdl_Impl)
{
    SvTreeListEntry* pEntry = maLbTree.GetCurEntry();

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

IMPL_LINK_NOARG(SvxHlinkDlgMarkWnd, ClickCloseHdl_Impl)
{
    Close();

    return( 0L );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
