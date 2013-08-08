/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SFX2_RECENTDOCSVIEW_HXX__
#define __SFX2_RECENTDOCSVIEW_HXX__

#include <sfx2/thumbnailview.hxx>
#include <sfx2/recentdocsviewitem.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

struct LoadRecentFile
{
    ::com::sun::star::util::URL                                                 aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   aArgSeq;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      xDispatch;
};

class SFX2_DLLPUBLIC RecentDocsView :   protected ::comphelper::OBaseMutex,
                                        public ThumbnailView
{
public:
    RecentDocsView( Window* pParent );
    virtual ~RecentDocsView();

    void insertItem(const OUString &rURL, const OUString &rTitle);
    void loadRecentDocs();

    virtual Size GetOptimalSize() const;

    void SetThumbnailSize(long ThumbnailWidth, long ThumbnailHeight);
    void SetHeight(long Height);

    DECL_STATIC_LINK( RecentDocsView, ExecuteHdl_Impl, LoadRecentFile* );

protected:
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem);

    long    mnItemMaxWidth;
    long    mnItemMaxHeight;
    long    mnItemPadding;
    long    mnItemMaxTextLength;
    long    mnItemThumbnailMaxHeight;
    long    mnItemMaxHeightSub;

    long mnHeight;

    int     mnMaxThumbnailItems;
};

#endif  // __SFX2_RECENTDOCSVIEW_HXX__

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
