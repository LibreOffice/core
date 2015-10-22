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
    css::util::URL                                    aTargetURL;
    css::uno::Sequence< css::beans::PropertyValue >   aArgSeq;
    css::uno::Reference< css::frame::XDispatch >      xDispatch;
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


class SFX2_DLLPUBLIC RecentDocsView : public ThumbnailView
{
public:
    RecentDocsView( vcl::Window* pParent );

    void insertItem(const OUString &rURL, const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId);

    long GetThumbnailSize() const { return mnItemMaxSize;}

    static bool typeMatchesExtension(ApplicationType type, const OUString &rExt);
    static BitmapEx getDefaultThumbnail(const OUString &rURL);

    int     mnFileTypes;

    virtual void Clear() override;

    /// Update the information in the view.
    virtual void Reload() override;

    DECL_STATIC_LINK_TYPED( RecentDocsView, ExecuteHdl_Impl, void*, void );

protected:
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;

    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;

    virtual void LoseFocus() override;

    bool isAcceptedFile(const OUString &rURL) const;

    long mnItemMaxSize;
    long mnTextHeight;
    long mnItemPadding;
    long mnItemMaxTextLength;
    size_t mnLastMouseDownItem;

    /// Image that appears when there is no recent document.
    Image maWelcomeImage;
    OUString maWelcomeLine1;
    OUString maWelcomeLine2;
};

#endif // INCLUDED_SFX2_RECENTDOCSVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
