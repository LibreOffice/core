/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include "StylePresetsPanel.hxx"

#include <swtypes.hxx>
#include <cmdid.h>

#include <svl/intitem.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>

#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/DocumentTemplates.hpp>
#include <com/sun/star/frame/XDocumentTemplates.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <sfx2/doctempl.hxx>

#include "shellio.hxx"
#include "docsh.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/string.hxx>

namespace sw { namespace sidebar {

VclPtr<vcl::Window> StylePresetsPanel::Create (vcl::Window* pParent,
                                        const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                        SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw css::lang::IllegalArgumentException("no parent Window given to PagePropertyPanel::Create", NULL, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to PagePropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw css::lang::IllegalArgumentException("no SfxBindings given to PagePropertyPanel::Create", NULL, 2);

    return VclPtr<vcl::Window>(new StylePresetsPanel(pParent, rxFrame, pBindings), SAL_NO_ACQUIRE);
}

StylePresetsPanel::StylePresetsPanel(vcl::Window* pParent,
                               const css::uno::Reference<css::frame::XFrame>& rxFrame,
                               SfxBindings* pBindings)
    : PanelLayout(pParent, "StylePresetsPanel", "modules/swriter/ui/sidebarstylepresets.ui", rxFrame)
    , mpBindings(pBindings)
{

    get(mpListBox, "listbox");

    mpListBox->SetDoubleClickHdl(LINK(this, StylePresetsPanel, DoubleClickHdl));


    SfxDocumentTemplates aTemplates;

    sal_uInt16 nCount = aTemplates.GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(aTemplates.GetFullRegionName(i));
        if (aRegionName == "styles")
        {
            for (sal_uInt16 j = 0; j < aTemplates.GetCount(i); ++j)
            {
                OUString aName = aTemplates.GetName(i,j);
                OUString aURL = aTemplates.GetPath(i,j);
                sal_Int32 nIndex = mpListBox->InsertEntry(aName);
                maTemplateEntries.push_back(std::unique_ptr<TemplateEntry>(new TemplateEntry(aName, aURL)));
                mpListBox->SetEntryData(nIndex, maTemplateEntries.back().get());
            }
        }
    }
}

StylePresetsPanel::~StylePresetsPanel()
{
    disposeOnce();
}

void StylePresetsPanel::dispose()
{
    mpListBox.disposeAndClear();

    PanelLayout::dispose();
}

IMPL_LINK_NOARG(StylePresetsPanel, DoubleClickHdl)
{
    sal_Int32 nIndex = mpListBox->GetSelectEntryPos();
    TemplateEntry* pEntry = static_cast<TemplateEntry*>(mpListBox->GetEntryData(nIndex));

    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    if (pDocSh)
    {
        SwgReaderOption aOption;
        aOption.SetTextFormats(true);
        aOption.SetNumRules(true);
        pDocSh->LoadStylesFromFile(pEntry->maURL, aOption, false);
    }

    return 1;
}

void StylePresetsPanel::NotifyItemUpdate(const sal_uInt16 /*nSId*/,
                                         const SfxItemState /*eState*/,
                                         const SfxPoolItem* /*pState*/,
                                         const bool /*bIsEnabled*/)
{
}

}} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
