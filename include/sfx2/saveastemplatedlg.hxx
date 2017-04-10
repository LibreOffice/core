/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_INC_SAVEASTEMPLATEDLG_HXX
#define INCLUDED_SFX2_INC_SAVEASTEMPLATEDLG_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/doctempl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>

class Edit;
class ListBox;
class CheckBox;

//  class SfxSaveAsTemplateDialog -------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxSaveAsTemplateDialog : public ModalDialog
{

private:
    VclPtr<ListBox>         mpLBCategory;
    VclPtr<CheckBox>        mpCBXDefault;
    VclPtr<Edit>            mpTemplateNameEdit;
    VclPtr<PushButton>      mpOKButton;

    OUString                msSelectedCategory;
    OUString                msTemplateName;
    sal_uInt16              mnRegionPos;

    std::vector<OUString>   msCategories;

    SfxDocumentTemplates    maDocTemplates;

    css::uno::Reference< css::frame::XModel > m_xModel;

public:
    DECL_LINK(OkClickHdl, Button*, void);
    DECL_LINK(TemplateNameEditHdl, Edit&, void);
    DECL_LINK(SelectCategoryHdl, ListBox&, void);

    void setDocumentModel (const css::uno::Reference<css::frame::XModel> &rModel);

    void initialize();
    void SetCategoryLBEntries(std::vector<OUString> names);

    /*Check whether template name is unique or not in a region*/
    bool IsTemplateNameUnique();

    bool SaveTemplate();

public:

    explicit SfxSaveAsTemplateDialog();

    virtual ~SfxSaveAsTemplateDialog() override;
    virtual void dispose() override;
};

#endif // INCLUDED_SFX2_INC_SAVEASTEMPLATEDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
