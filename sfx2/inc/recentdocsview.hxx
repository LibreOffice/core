/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/thumbnailview.hxx>
#include <vcl/image.hxx>

#include <o3tl/typed_flags_set.hxx>

#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

struct ImplSVEvent;
namespace com::sun::star::frame { class XDispatch; }

namespace sfx2
{

class RecentDocsView;

struct LoadRecentFile
{
    css::util::URL                                    aTargetURL;
    css::uno::Sequence< css::beans::PropertyValue >   aArgSeq;
    css::uno::Reference< css::frame::XDispatch >      xDispatch;
    RecentDocsView*                                   pView;
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

class RecentDocsView final : public ThumbnailView
{
public:
    RecentDocsView(std::unique_ptr<weld::ScrolledWindow> xWindow, std::unique_ptr<weld::Menu> xMenu);
    virtual ~RecentDocsView() override;

    void insertItem(const OUString &rURL, const OUString &rTitle, const BitmapEx &rThumbnail, sal_uInt16 nId);

    static bool typeMatchesExtension(ApplicationType type, std::u16string_view rExt);
    static BitmapEx getDefaultThumbnail(const OUString &rURL, bool bCheckEncrypted = true);

    ApplicationType mnFileTypes;

    virtual void Clear() override;

    /// Update the information in the view.
    virtual void Reload() override;

    // launch load of recently used file
    void PostLoadRecentUsedFile(LoadRecentFile* pLoadRecentFile);

    // received on load of recently used file
    void DispatchedLoadRecentUsedFile();

private:
    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual bool MouseButtonUp( const MouseEvent& rMEvt ) override;

    virtual void OnItemDblClicked(ThumbnailViewItem *pItem) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void LoseFocus() override;

    bool isAcceptedFile(const OUString &rURL) const;

    DECL_LINK( ExecuteHdl_Impl, void*, void );

    tools::Long mnItemMaxSize;
    size_t mnLastMouseDownItem;

    /// Image that appears when there is no recent document.
    BitmapEx maWelcomeImage;
    OUString maWelcomeLine1;
    OUString maWelcomeLine2;

    sfx2::LoadRecentFile* mpLoadRecentFile;
    ImplSVEvent* m_nExecuteHdlId;
};

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
