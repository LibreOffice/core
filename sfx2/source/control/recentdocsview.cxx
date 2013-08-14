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

#include <sfx2/recentdocsview.hxx>

#include <sfx2/templateabstractview.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/sfxresid.hxx>
#include <unotools/historyoptions.hxx>
#include <vcl/builder.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

static const char SFX_REFERER_USER[] = "private:user";

RecentDocsView::RecentDocsView( Window* pParent )
    : ThumbnailView(pParent)
    , mnItemMaxSize(100)
    , mnTextHeight(30)
    , mnItemPadding(5)
    , mnItemMaxTextLength(30)
{
    SetStyle(GetStyle() | WB_VSCROLL);
    setItemMaxTextLength( mnItemMaxTextLength );
    setItemDimensions( mnItemMaxSize, mnItemMaxSize, mnTextHeight, mnItemPadding );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeRecentDocsView(Window *pParent, VclBuilder::stringmap &)
{
    return new RecentDocsView(pParent);
}

RecentDocsView::~RecentDocsView()
{
}

bool RecentDocsView::isFilteredExtension(APPLICATION_FILTER filter, const OUString &rExt)
{
    bool bRet = true;

    if (filter == FILTER_WRITER)
    {
        bRet = rExt == "odt" || rExt == "doc" || rExt == "docx" ||
            rExt == "rtf" || rExt == "txt";
    }
    else if (filter == FILTER_CALC)
    {
        bRet = rExt == "ods" || rExt == "xls" || rExt == "xlsx";
    }
    else if (filter == FILTER_IMPRESS)
    {
        bRet = rExt == "odp" || rExt == "pps" || rExt == "ppt" ||
            rExt == "pptx";
    }
    else if (filter == FILTER_DRAW)
    {
        bRet = rExt == "odg";
    }
    else if (filter == FILTER_DATABASE)
    {
        bRet = rExt == "odb";
    }
    else if (filter == FILTER_MATH)
    {
        bRet = rExt == "odf";
    }

    return bRet;
}

bool RecentDocsView::isUnfilteredFile(const OUString &rURL) const
{
    INetURLObject aUrl(rURL);
    return isFilteredExtension(mFilter, aUrl.getExtension());
}

BitmapEx RecentDocsView::getDefaultThumbnail(const OUString &rURL)
{
    BitmapEx aImg;
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();

    if ( isFilteredExtension( FILTER_WRITER, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_TEXT ) );
    else if ( isFilteredExtension( FILTER_CALC, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_SHEET ) );
    else if ( isFilteredExtension( FILTER_IMPRESS, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_PRESENTATION ) );
    else if ( isFilteredExtension( FILTER_DRAW, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_DRAWING ) );
    else if ( isFilteredExtension( FILTER_DATABASE, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_DATABASE ) );
    else if ( isFilteredExtension( FILTER_MATH, aExt) )
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_MATH ) );
    else
        aImg = BitmapEx ( SfxResId( SFX_FILE_THUMBNAIL_DEFAULT ) );

    return aImg;
}

void RecentDocsView::insertItem(const OUString &rURL, const OUString &rTitle)
{
    RecentDocsViewItem *pChild = new RecentDocsViewItem(*this, rURL, rTitle);

    AppendItem(pChild);
}

void RecentDocsView::loadRecentDocs()
{
    Clear();

    Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
    for ( int i = 0; i < aHistoryList.getLength(); i++ )
    {
        Sequence< PropertyValue >& rRecentEntry = aHistoryList[i];

        OUString aURL;
        OUString aTitle;

        for ( int j = 0; j < rRecentEntry.getLength(); j++ )
        {
            Any a = rRecentEntry[j].Value;

            if (rRecentEntry[j].Name == "URL")
                a >>= aURL;
            else if (rRecentEntry[j].Name == "Title")
                a >>= aTitle;
        }

        if( isUnfilteredFile(aURL) )
        {
            insertItem(aURL, aTitle);
        }
    }

    CalculateItemPositions();
    Invalidate();
}

void RecentDocsView::OnItemDblClicked(ThumbnailViewItem *pItem)
{
    RecentDocsViewItem* pRecentItem = dynamic_cast<RecentDocsViewItem*>(pItem);

    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    css::util::URL                    aTargetURL;
    Sequence< PropertyValue >         aArgsList;

    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( ::comphelper::getProcessComponentContext() );
    uno::Reference< frame::XFrame > xActiveFrame = xDesktop->getActiveFrame();

    osl::ClearableMutexGuard aLock( m_aMutex );
    xDispatchProvider = Reference< XDispatchProvider >( xActiveFrame, UNO_QUERY );
    aLock.clear();

    aTargetURL.Complete = pRecentItem->maURL;
    Reference< ::com::sun::star::util::XURLTransformer > xTrans(
        ::com::sun::star::util::URLTransformer::create(
            ::comphelper::getProcessComponentContext() ) );
    xTrans->parseStrict( aTargetURL );

    sal_Int32 nSize = 2;
    aArgsList.realloc( nSize );
    aArgsList[0].Name = "Referer";
    aArgsList[0].Value = makeAny( OUString( SFX_REFERER_USER ) );

    // documents will never be opened as templates
    aArgsList[1].Name = "AsTemplate";
    aArgsList[1].Value = makeAny( (sal_Bool) sal_False );

    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, "_default", 0 );

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        LoadRecentFile* pLoadRecentFile = new LoadRecentFile;
        pLoadRecentFile->xDispatch  = xDispatch;
        pLoadRecentFile->aTargetURL = aTargetURL;
        pLoadRecentFile->aArgSeq    = aArgsList;

        Application::PostUserEvent( STATIC_LINK(0, RecentDocsView, ExecuteHdl_Impl), pLoadRecentFile );
    }
}

void RecentDocsView::SetThumbnailSize(long thumbnailSize)
{
    mnItemMaxSize = thumbnailSize;
    setItemDimensions( mnItemMaxSize, mnItemMaxSize, mnTextHeight, mnItemPadding );
}

long RecentDocsView::GetThumbnailSize() const
{
    return mnItemMaxSize;
}

void RecentDocsView::SetFilter(APPLICATION_FILTER filter)
{
    mFilter = filter;
}

IMPL_STATIC_LINK_NOINSTANCE( RecentDocsView, ExecuteHdl_Impl, LoadRecentFile*, pLoadRecentFile )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pLoadRecentFile->xDispatch->dispatch( pLoadRecentFile->aTargetURL, pLoadRecentFile->aArgSeq );
    }
    catch ( const Exception& )
    {
    }

    delete pLoadRecentFile;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
