/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_RECENTDOCSVIEW_HXX
#define INCLUDED_SFX2_RECENTDOCSVIEW_HXX

#include <sfx2/thumbnailview.hxx>
#include <sfx2/recentdocsviewitem.hxx>
#include <vcl/image.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>

struct LoadRecentFile
{
    ::com::sun::star::util::URL                                                 aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   aArgSeq;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >      xDispatch;
};

enum ApplicationType
{
    TYPE_NONE     =      0,
    TYPE_WRITER   = 1 << 0,
    TYPE_CALC     = 1 << 1,
    TYPE_IMPRESS  = 1 << 2,
    TYPE_DRAW     = 1 << 3,
    TYPE_DATABASE = 1 << 4,
    TYPE_MATH     = 1 << 5,
    TYPE_OTHER    = 1 << 6
};


class SFX2_DLLPUBLIC RecentDocsView :   protected ::comphelper::OBaseMutex,
                                        public ThumbnailView
{
public:
    RecentDocsView( Window* pParent );
    virtual ~RecentDocsView();

    void insertItem(const OUString &rURL, const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId);
    void loadRecentDocs();

    long GetThumbnailSize() const;

    static bool typeMatchesExtension(ApplicationType type, const OUString &rExt);
    static BitmapEx getDefaultThumbnail(const OUString &rURL);

    int     mnFileTypes;

    virtual void Clear() SAL_OVERRIDE;

    DECL_STATIC_LINK( RecentDocsView, ExecuteHdl_Impl, LoadRecentFile* );

protected:
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) SAL_OVERRIDE;
    void OpenItem( const ThumbnailViewItem *pItem );
    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;

    virtual void LoseFocus() SAL_OVERRIDE;

    bool isAcceptedFile(const OUString &rURL) const;

    long    mnItemMaxSize;
    long    mnTextHeight;
    long    mnItemPadding;
    long    mnItemMaxTextLength;

    Image   maWelcomeImage;
    OUString maWelcomeLine1;
    OUString maWelcomeLine2;
};

#endif // INCLUDED_SFX2_RECENTDOCSVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
