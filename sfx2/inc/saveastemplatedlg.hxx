/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <sfx2/doctempl.hxx>
#include <vcl/weld.hxx>

#include <com/sun/star/frame/XModel.hpp>

//  class SfxSaveAsTemplateDialog -------------------------------------------------------------------
class SfxSaveAsTemplateDialog final : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::TreeView> m_xLBCategory;
    std::unique_ptr<weld::CheckButton> m_xCBXDefault;
    std::unique_ptr<weld::Entry> m_xTemplateNameEdit;
    std::unique_ptr<weld::Button> m_xOKButton;

    OUString msSelectedCategory;
    OUString msTemplateName;
    sal_uInt16 mnRegionPos;

    std::vector<OUString> msCategories;

    SfxDocumentTemplates maDocTemplates;

    css::uno::Reference<css::frame::XModel> m_xModel;

public:
    DECL_LINK(OkClickHdl, weld::Button&, void);
    DECL_LINK(TemplateNameEditHdl, weld::Entry&, void);
    DECL_LINK(SelectCategoryHdl, weld::TreeView&, void);

    void initialize();
    void SetCategoryLBEntries(const std::vector<OUString>& names);

    /*Check whether template name is unique or not in a region*/
    bool IsTemplateNameUnique();

    bool SaveTemplate();

public:
    SfxSaveAsTemplateDialog(weld::Window* pParent,
                            const css::uno::Reference<css::frame::XModel>& rModel);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
