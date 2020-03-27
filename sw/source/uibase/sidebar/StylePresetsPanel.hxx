/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_STYLEPRESETSPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_STYLEPRESETSPANEL_HXX

#include <memory>
#include <com/sun/star/frame/XFrame.hpp>

#include <svx/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

#include <svtools/valueset.hxx>

namespace sw { namespace sidebar {

class StylePresetsPanel : public PanelLayout,
                       public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
    friend class VclPtr<StylePresetsPanel>;
public:
    static VclPtr<vcl::Window> Create(vcl::Window* pParent,
                                  const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual void NotifyItemUpdate(const sal_uInt16 nSId,
                                  const SfxItemState eState,
                                  const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

private:
    struct TemplateEntry
    {
        explicit TemplateEntry(const OUString& rURL)
            : maURL(rURL)
        {}

        OUString const maURL;
    };

    void RefreshList();

    StylePresetsPanel(vcl::Window* pParent,
                   const css::uno::Reference<css::frame::XFrame>& rxFrame);

    virtual ~StylePresetsPanel() override;
    virtual void dispose() override;

    VclPtr<ValueSet> mpValueSet;

    std::vector<std::unique_ptr<TemplateEntry>> maTemplateEntries;

    DECL_LINK(DoubleClickHdl, ValueSet*, void);
};

}} // end of namespace sw::sidebar

#endif // INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_STYLEPRESETSPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
