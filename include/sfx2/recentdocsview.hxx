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

#include <o3tl/typed_flags_set.hxx>

#include <com/sun/star/frame/XDispatch.hpp>

namespace sfx2
{

struct LoadRecentFile
{
    css::util::URL                                    aTargetURL;
    css::uno::Sequence< css::beans::PropertyValue >   aArgSeq;
    css::uno::Reference< css::frame::XDispatch >      xDispatch;
    VclPtr< ThumbnailView >                           pView;
};

enum class ApplicationType
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

} // namespace sfx2

namespace o3tl {

template<> struct typed_flags<sfx2::ApplicationType> : is_typed_flags<sfx2::ApplicationType, 0x7f> {};

} // namespace o3tl

namespace sfx2
{

class SFX2_DLLPUBLIC RecentDocsView : public ThumbnailView
{
public:
    RecentDocsView( vcl::Window* pParent );

    void insertItem(const OUString &rURL, const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId);

    static bool typeMatchesExtension(ApplicationType type, const OUString &rExt);
    static BitmapEx getDefaultThumbnail(const OUString &rURL);

    ApplicationType mnFileTypes;

    virtual void Clear() override;

    /// Update the information in the view.
    virtual void Reload() override;

    DECL_STATIC_LINK( RecentDocsView, ExecuteHdl_Impl, void*, void );

protected:
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;

    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

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

} // namespace sfx2

#endif // INCLUDED_SFX2_RECENTDOCSVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
