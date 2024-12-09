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

#include <config_features.h>
#include <config_fuzzers.h>
#include <config_wasm_strip.h>

#include "swdlgfact.hxx"
#include <svl/style.hxx>
#include <vcl/abstdlgimpl.hxx>
#include <globals.hrc>

#include <wordcountdialog.hxx>
#include <abstract.hxx>
#include <addrdlg.hxx>
#include <ascfldlg.hxx>
#include <bookmark.hxx>
#include <break.hxx>
#include <changedb.hxx>
#include <chrdlg.hxx>
#include <colwd.hxx>
#include <contentcontroldlg.hxx>
#include <contentcontrollistitemdlg.hxx>
#include <pagenumberdlg.hxx>
#include <convert.hxx>
#include <cption.hxx>
#include <dbinsdlg.hxx>
#include <docfnote.hxx>
#include <docstdlg.hxx>
#include <DateFormFieldDialog.hxx>
#include <DropDownFieldDialog.hxx>
#include <DropDownFormFieldDialog.hxx>
#include <envlop.hxx>
#include <label.hxx>
#include <drpcps.hxx>
#include <swuipardlg.hxx>
#include <pattern.hxx>
#include <pardlg.hxx>
#include <rowht.hxx>
#include <selglos.hxx>
#include <splittbl.hxx>
#include <srtdlg.hxx>
#include <tautofmt.hxx>
#include <tblnumfm.hxx>
#include <wrap.hxx>
#include <tabledlg.hxx>
#include <fldtdlg.hxx>
#include <fldedt.hxx>
#include <swrenamexnameddlg.hxx>
#include <swmodalredlineacceptdlg.hxx>
#include <frmdlg.hxx>
#include <tmpdlg.hxx>
#include <glossary.hxx>
#include <inpdlg.hxx>
#include <insfnote.hxx>
#include <instable.hxx>
#include <javaedit.hxx>
#include <linenum.hxx>
#include <titlepage.hxx>
#include <mailmrge.hxx>
#include <mergetbl.hxx>
#include <multmrk.hxx>
#include <num.hxx>
#include <outline.hxx>
#include <column.hxx>
#include <cnttab.hxx>
#include <swuicnttab.hxx>
#include <regionsw.hxx>
#include <optcomp.hxx>
#include <optload.hxx>
#include <optpage.hxx>
#include <swuiidxmrk.hxx>
#include <svx/dialogs.hrc>
#include <mailmergewizard.hxx>
#include <mailconfigpage.hxx>
#include <uiborder.hxx>
#include <mmresultdialogs.hxx>
#include <formatlinebreak.hxx>
#include <translatelangselect.hxx>
#include <copyfielddlg.hxx>

using namespace css::frame;
using namespace css::uno;

namespace
{
// This base disallows synchronous execution
template <class Base, class Dialog>
class AbstractDialogImpl_NoSync : public vcl::AbstractDialogImpl_Async<Base, Dialog>
{
public:
    using vcl::AbstractDialogImpl_Async<Base, Dialog>::AbstractDialogImpl_Async;
    short Execute() override
    {
        assert(false);
        return -1;
    }
};

class AbstractSwBreakDlg_Impl : public AbstractSwBreakDlg
{
    std::shared_ptr<weld::DialogController> m_xDlg;

public:
    explicit AbstractSwBreakDlg_Impl(std::shared_ptr<weld::DialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    OUString GetTemplateName() override;
    sal_uInt16 GetKind() override;
    std::optional<sal_uInt16> GetPageNumber() override;
    std::optional<SwLineBreakClear> GetClear() override;

    std::shared_ptr<weld::DialogController> getDialogController() override { return m_xDlg; }
};

class AbstractSwTranslateLangSelectDlg_Impl : public AbstractSwTranslateLangSelectDlg
{
    std::shared_ptr<weld::DialogController> m_xDlg;

public:
    explicit AbstractSwTranslateLangSelectDlg_Impl(std::shared_ptr<weld::DialogController> p)
        : m_xDlg(std::move(p))
    {
    }

    std::shared_ptr<weld::DialogController> getDialogController() override { return m_xDlg; }
    std::optional<SwLanguageListItem> GetSelectedLanguage() override;
};

class AbstractInsTableDlg_Impl : public AbstractInsTableDlg
{
    std::shared_ptr<weld::DialogController> m_xDlg;

public:
    explicit AbstractInsTableDlg_Impl(std::shared_ptr<weld::DialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    void GetValues(OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                   SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                   std::unique_ptr<SwTableAutoFormat>& prTAFormat) override;
    std::shared_ptr<weld::DialogController> getDialogController() override { return m_xDlg; }
};
}

OUString AbstractSwBreakDlg_Impl::GetTemplateName()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetTemplateName();

    return u""_ustr;
}

sal_uInt16 AbstractSwBreakDlg_Impl:: GetKind()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetKind();

    return 0;
}

::std::optional<sal_uInt16> AbstractSwBreakDlg_Impl:: GetPageNumber()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetPageNumber();

    return 0;
}

std::optional<SwLineBreakClear> AbstractSwBreakDlg_Impl::GetClear()
{
    SwBreakDlg* pDlg = dynamic_cast<SwBreakDlg*>(m_xDlg.get());
    if (pDlg)
        return pDlg->GetClear();

    return SwLineBreakClear::NONE;
}

void AbstractInsTableDlg_Impl::GetValues( OUString& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTableFlags, OUString& rTableAutoFormatName,
                                std::unique_ptr<SwTableAutoFormat>& prTAFormat )
{
    SwInsTableDlg* pDlg = dynamic_cast<SwInsTableDlg*>(m_xDlg.get());
    if (pDlg)
        pDlg->GetValues(rName, rRow, rCol, rInsTableFlags, rTableAutoFormatName, prTAFormat);
}

std::optional<SwLanguageListItem> AbstractSwTranslateLangSelectDlg_Impl::GetSelectedLanguage()
{
#if HAVE_FEATURE_CURL && !ENABLE_WASM_STRIP_EXTRA
    return SwTranslateLangSelectDlg::GetSelectedLanguage();
#else
    return {};
#endif
}

namespace
{
class AbstractSwInsertAbstractDlg_Impl
    : public AbstractDialogImpl_NoSync<AbstractSwInsertAbstractDlg, SwInsertAbstractDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    sal_uInt8 GetLevel() const override { return m_pDlg->GetLevel(); }
    sal_uInt8 GetPara() const override { return m_pDlg->GetPara(); }
};
}

VclPtr<AbstractSwInsertAbstractDlg> SwAbstractDialogFactory_Impl::CreateSwInsertAbstractDlg(weld::Window* pParent)
{
    return VclPtr<AbstractSwInsertAbstractDlg_Impl>::Create(pParent);
}

namespace
{
template <class Dialog>
class SwAbstractSfxController_Impl : public vcl::AbstractDialogImpl_Async<SfxAbstractDialog, Dialog>
{
public:
    using vcl::AbstractDialogImpl_Async<SfxAbstractDialog, Dialog>::AbstractDialogImpl_Async;
    const SfxItemSet* GetOutputItemSet() const override { return this->m_pDlg->GetOutputItemSet(); }
    void SetText(const OUString& rStr) override { this->m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAddressAbstractDlg(weld::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl<SwAddrDlg>>::Create(pParent, rSet);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwDropCapsDialog(weld::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl<SwDropCapsDlg>>::Create(pParent, rSet);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwBackgroundDialog(weld::Window* pParent,
                                                                  const SfxItemSet& rSet)
{
    return VclPtr<SwAbstractSfxController_Impl<SwBackgroundDlg>>::Create(pParent, rSet);
}

namespace
{
class AbstractNumFormatDlg_Impl
    : public vcl::AbstractDialogImpl_Async<SfxAbstractDialog, SwNumFormatDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateNumFormatDialog(weld::Widget* pParent, const SfxItemSet& rSet)
{
    return VclPtr<AbstractNumFormatDlg_Impl>::Create(pParent, rSet);
}

namespace
{
class AbstractSwAsciiFilterDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractSwAsciiFilterDlg, SwAsciiFilterDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void FillOptions(SwAsciiOptions& rOptions) override { m_pDlg->FillOptions(rOptions); }
};
}

VclPtr<AbstractSwAsciiFilterDlg> SwAbstractDialogFactory_Impl::CreateSwAsciiFilterDlg(weld::Window* pParent,
       SwDocShell& rDocSh, SvStream* pStream)
{
    return VclPtr<AbstractSwAsciiFilterDlg_Impl>::Create(pParent, rDocSh, pStream);
}

namespace
{
template <class Dialog>
using AbstractGenericDialog_Impl = vcl::AbstractDialogImpl_Async<VclAbstractDialog, Dialog>;
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwInsertBookmarkDlg(weld::Window *pParent,
        SwWrtShell &rSh, OUString const*const pSelected)
{
    return VclPtr<AbstractGenericDialog_Impl<SwInsertBookmarkDlg>>::Create(pParent, rSh, pSelected);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwContentControlDlg(weld::Window* pParent,
                                                                                  SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl<SwContentControlDlg>>::Create(pParent, rSh);
}

VclPtr<AbstractSwContentControlListItemDlg>
SwAbstractDialogFactory_Impl::CreateSwContentControlListItemDlg(weld::Window* pParent,
                                                                SwContentControlListItem& rItem)
{
    using AbstractSwContentControlListItemDlg_Impl
        = vcl::AbstractDialogImpl_Async<AbstractSwContentControlListItemDlg,
                                        SwContentControlListItemDlg>;
    return VclPtr<AbstractSwContentControlListItemDlg_Impl>::Create(pParent, rItem);
}

std::shared_ptr<AbstractSwBreakDlg> SwAbstractDialogFactory_Impl::CreateSwBreakDlg(weld::Window* pParent, SwWrtShell &rSh)
{
    return std::make_shared<AbstractSwBreakDlg_Impl>(std::make_unique<SwBreakDlg>(pParent, rSh));
}

std::shared_ptr<AbstractSwTranslateLangSelectDlg> SwAbstractDialogFactory_Impl::CreateSwTranslateLangSelectDlg(weld::Window* pParent, SwWrtShell &rSh)
{
#if HAVE_FEATURE_CURL && !ENABLE_WASM_STRIP_EXTRA
    return std::make_shared<AbstractSwTranslateLangSelectDlg_Impl>(std::make_unique<SwTranslateLangSelectDlg>(pParent, rSh));
#else
    (void) pParent;
    (void) rSh;
    return nullptr;
#endif
}

namespace
{
class AbstractChangeDbDialog_Impl
    : public AbstractDialogImpl_NoSync<AbstractChangeDbDialog, SwChangeDBDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    virtual void UpdateFields() override { m_pDlg->UpdateFields(); }
};
}

VclPtr<AbstractChangeDbDialog> SwAbstractDialogFactory_Impl::CreateSwChangeDBDlg(SwView& rVw)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    return VclPtr<AbstractChangeDbDialog_Impl>::Create(rVw);
#else
    (void) rVw;
    return nullptr;
#endif
}

namespace
{
template<class AbstractTabDialog_t, class Dialog>
class AbstractTabController_Impl_BASE
    : public vcl::AbstractDialogImpl_Async<AbstractTabDialog_t, Dialog>
{
public:
    using vcl::AbstractDialogImpl_Async<AbstractTabDialog_t, Dialog>::AbstractDialogImpl_Async;
    void SetCurPageId(const OUString& rName) override { this->m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return this->m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return this->m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { this->m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { this->m_pDlg->set_title(rStr); }
};

template <class Dialog>
using AbstractTabController_Impl = AbstractTabController_Impl_BASE<SfxAbstractTabDialog, Dialog>;
}

VclPtr<SfxAbstractTabDialog>  SwAbstractDialogFactory_Impl::CreateSwCharDlg(weld::Window* pParent, SwView& pVw,
    const SfxItemSet& rCoreSet, SwCharDlgMode nDialogMode, const OUString* pFormatStr)
{
    return VclPtr<AbstractTabController_Impl<SwCharDlg>>::Create(pParent, pVw, rCoreSet, nDialogMode, pFormatStr);
}

namespace
{
class AbstractSwConvertTableDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractSwConvertTableDlg, SwConvertTableDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void GetValues(sal_Unicode& rDelim, SwInsertTableOptions& rInsTableFlags,
                   SwTableAutoFormat const*& prTAFormat) override
    {
        m_pDlg->GetValues(rDelim, rInsTableFlags, prTAFormat);
    }
};
}

VclPtr<AbstractSwConvertTableDlg> SwAbstractDialogFactory_Impl::CreateSwConvertTableDlg(SwView& rView, bool bToTable)
{
    return VclPtr<AbstractSwConvertTableDlg_Impl>::Create(rView, bToTable);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwCaptionDialog(weld::Window *pParent, SwView &rV)
{
    return VclPtr<AbstractGenericDialog_Impl<SwCaptionDialog>>::Create(pParent, rV);
}

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
namespace
{
class AbstractSwInsertDBColAutoPilot_Impl
    : public AbstractDialogImpl_NoSync<AbstractSwInsertDBColAutoPilot, SwInsertDBColAutoPilot>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void DataToDoc(const css::uno::Sequence<css::uno::Any>& rSelection,
                   css::uno::Reference<css::sdbc::XDataSource> rxSource,
                   css::uno::Reference<css::sdbc::XConnection> xConnection,
                   css::uno::Reference<css::sdbc::XResultSet> xResultSet) override
    {
        m_pDlg->DataToDoc(rSelection, rxSource, xConnection, xResultSet);
    }
};
}
#endif

VclPtr<AbstractSwInsertDBColAutoPilot> SwAbstractDialogFactory_Impl::CreateSwInsertDBColAutoPilot( SwView& rView,
        uno::Reference< sdbc::XDataSource> rxSource,
        uno::Reference<sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    return VclPtr<AbstractSwInsertDBColAutoPilot_Impl>::Create(rView, rxSource, xColSupp, rData);
#else
    (void) rView;
    (void) rxSource;
    (void) xColSupp;
    (void) rData;
    return nullptr;
#endif
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwFootNoteOptionDlg(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractTabController_Impl<SwFootNoteOptionDlg>>::Create(pParent, rSh);
}

namespace
{
class AbstractDropDownFieldDialog_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractDropDownFieldDialog, sw::DropDownFieldDialog>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    bool PrevButtonPressed() const override { return m_pDlg->PrevButtonPressed(); }
    bool NextButtonPressed() const override { return m_pDlg->NextButtonPressed(); }
};
}

VclPtr<AbstractDropDownFieldDialog> SwAbstractDialogFactory_Impl::CreateDropDownFieldDialog(weld::Widget *pParent,
    SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton)
{
    return VclPtr<AbstractDropDownFieldDialog_Impl>::Create(pParent, rSh, pField, bPrevButton, bNextButton);
}

namespace
{
class AbstractDropDownFormFieldDialog_Impl
    : public AbstractDialogImpl_NoSync<AbstractDropDownFormFieldDialog, sw::DropDownFormFieldDialog>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractDropDownFormFieldDialog> SwAbstractDialogFactory_Impl::CreateDropDownFormFieldDialog(weld::Widget *pParent, sw::mark::Fieldmark* pDropDownField)
{
    return VclPtr<AbstractDropDownFormFieldDialog_Impl>::Create(pParent, pDropDownField);
}

namespace
{
class AbstractDateFormFieldDialog_Impl
    : public AbstractDialogImpl_NoSync<AbstractDateFormFieldDialog, sw::DateFormFieldDialog>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractDateFormFieldDialog> SwAbstractDialogFactory_Impl::CreateDateFormFieldDialog(weld::Widget *pParent, sw::mark::DateFieldmark* pDateField, SwDoc& rDoc)
{
    return VclPtr<AbstractDateFormFieldDialog_Impl>::Create(pParent, pDateField, rDoc);
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                                 SwWrtShell* pWrtSh, Printer* pPrt,
                                                                 bool bInsert)
{
    return VclPtr<AbstractTabController_Impl<SwEnvDlg>>::Create(pParent, rSet, pWrtSh,pPrt, bInsert);
}

namespace
{
class AbstractSwLabDlg_Impl : public vcl::AbstractDialogImpl_Sync<AbstractSwLabDlg, SwLabDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
    const OUString& GetBusinessCardStr() const override { return m_pDlg->GetBusinessCardStr(); }
    Printer* GetPrt() override { return m_pDlg->GetPrt(); }
};
}

VclPtr<AbstractSwLabDlg> SwAbstractDialogFactory_Impl::CreateSwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
                                                     SwDBManager* pDBManager, bool bLabel)
{
    return VclPtr<AbstractSwLabDlg_Impl>::Create(pParent, rSet, pDBManager, bLabel);
}

SwLabDlgMethod SwAbstractDialogFactory_Impl::GetSwLabDlgStaticMethod ()
{
    return SwLabDlg::UpdateFieldInformation;
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwParaDlg(weld::Window *pParent, SwView& rVw,
                                                                           const SfxItemSet& rCoreSet,
                                                                           bool bDraw ,
                                                                           const OUString& sDefPage)
{
    return VclPtr<AbstractTabController_Impl<SwParaDlg>>::Create(pParent, rVw, rCoreSet, DLG_STD, nullptr, bDraw, sDefPage);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwAutoMarkDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl<SwAuthMarkModalDlg>>::Create(pParent, rSh);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwColumnDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractGenericDialog_Impl<SwColumnDlg>>::Create(pParent, rSh);
}

namespace
{
class AbstractSwTableHeightDlg_Impl
    : public AbstractDialogImpl_NoSync<AbstractSwTableHeightDlg, SwTableHeightDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractSwTableHeightDlg> SwAbstractDialogFactory_Impl::CreateSwTableHeightDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwTableHeightDlg_Impl>::Create(pParent, rSh);
}

namespace
{
class AbstractSwSortDlg_Impl : public AbstractDialogImpl_NoSync<AbstractSwSortDlg, SwSortDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractSwSortDlg> SwAbstractDialogFactory_Impl::CreateSwSortingDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSwSortDlg_Impl>::Create(pParent, rSh);
}

namespace
{
class AbstractSplitTableDialog_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractSplitTableDialog, SwSplitTableDlg> // add for
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    SplitTable_HeadlineOption GetSplitMode() override { return m_pDlg->GetSplitMode(); }
};
}

VclPtr<AbstractSplitTableDialog> SwAbstractDialogFactory_Impl::CreateSplitTableDialog(weld::Window *pParent, SwWrtShell &rSh)
{
    return VclPtr<AbstractSplitTableDialog_Impl>::Create(pParent, rSh);
}

namespace
{
class AbstractSwSelGlossaryDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractSwSelGlossaryDlg, SwSelGlossaryDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void InsertGlos(const OUString& rRegion, const OUString& rGlosName) override
    {
        m_pDlg->InsertGlos(rRegion, rGlosName);
    }
    sal_Int32 GetSelectedIdx() const override { return m_pDlg->GetSelectedIdx(); }
    void SelectEntryPos(sal_Int32 nIdx) override { m_pDlg->SelectEntryPos(nIdx); }
};
}

VclPtr<AbstractSwSelGlossaryDlg> SwAbstractDialogFactory_Impl::CreateSwSelGlossaryDlg(weld::Window *pParent, const OUString &rShortName)
{
    return VclPtr<AbstractSwSelGlossaryDlg_Impl>::Create(pParent, rShortName);
}

namespace
{
class AbstractSwAutoFormatDlg_Impl
    : public AbstractDialogImpl_NoSync<AbstractSwAutoFormatDlg, SwAutoFormatDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    std::unique_ptr<SwTableAutoFormat> FillAutoFormatOfIndex() const override
    {
        return m_pDlg->FillAutoFormatOfIndex();
    }
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractSwAutoFormatDlg> SwAbstractDialogFactory_Impl::CreateSwAutoFormatDlg(weld::Window* pParent,
    SwWrtShell* pShell, bool bSetAutoFormat, const SwTableAutoFormat* pSelFormat)
{
    return VclPtr<AbstractSwAutoFormatDlg_Impl>::Create(pParent, pShell, bSetAutoFormat, pSelFormat);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwBorderDlg(weld::Window* pParent, SfxItemSet& rSet, SwBorderModes nType )
{
    return VclPtr<SwAbstractSfxController_Impl<SwBorderDlg>>::Create(pParent, rSet, nType);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwWrapDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pSh)
{
    return VclPtr<SwAbstractSfxController_Impl<SwWrapDlg>>::Create(pParent, rSet, pSh, true/*bDrawMode*/);
}

namespace
{
class AbstractSwTableWidthDlg_Impl
    : public AbstractDialogImpl_NoSync<AbstractSwTableWidthDlg, SwTableWidthDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractSwTableWidthDlg> SwAbstractDialogFactory_Impl::CreateSwTableWidthDlg(weld::Window *pParent, SwWrtShell *pShell)
{
    return VclPtr<AbstractSwTableWidthDlg_Impl>::Create(pParent, pShell);
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateSwTableTabDlg(weld::Window* pParent,
    const SfxItemSet* pItemSet, SwWrtShell* pSh)
{
    return VclPtr<AbstractTabController_Impl<SwTableTabDlg>>::Create(pParent, pItemSet, pSh);
}

namespace
{
class AbstractSwFieldDlg_Impl : public vcl::AbstractDialogImpl_Async<AbstractSwFieldDlg, SwFieldDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    bool StartExecuteAsync(AsyncContext& rCtx) override;
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
    void ShowReferencePage() override { m_pDlg->ShowReferencePage(); }
    void Initialize(SfxChildWinInfo* pInfo) override { m_pDlg->Initialize(pInfo); }
    void ReInitDlg() override { m_pDlg->ReInitDlg(); }
    void ActivateDatabasePage() override { m_pDlg->ActivateDatabasePage(); }
    std::shared_ptr<SfxDialogController> GetController() override { return m_pDlg; }
};

bool AbstractSwFieldDlg_Impl::StartExecuteAsync(AsyncContext& rCtx)
{
    return SfxTabDialogController::runAsync(m_pDlg,
                                            [rCtx, pDlg = m_pDlg](sal_Int32 nResult)
                                            {
                                                pDlg->Close();
                                                if (rCtx.isSet())
                                                    rCtx.maEndDialogFn(nResult);
                                            });
}
}

VclPtr<AbstractSwFieldDlg> SwAbstractDialogFactory_Impl::CreateSwFieldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, weld::Window *pParent)
{
    return VclPtr<AbstractSwFieldDlg_Impl>::Create(pB, pCW, pParent);
}

VclPtr<SfxAbstractDialog> SwAbstractDialogFactory_Impl::CreateSwFieldEditDlg(SwView& rVw)
{
    auto xDlg = std::make_shared<SwFieldEditDlg>(rVw);
    // without TabPage no dialog
    if (!xDlg->GetTabPage())
        return nullptr;
    return VclPtr<SwAbstractSfxController_Impl<SwFieldEditDlg>>::Create(std::move(xDlg));
}

namespace
{
class AbstractSwRenameXNamedDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractSwRenameXNamedDlg, SwRenameXNamedDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void SetForbiddenChars(const OUString& rSet) override { m_pDlg->SetForbiddenChars(rSet); }
    void SetAlternativeAccess(css::uno::Reference<css::container::XNameAccess>& xSecond,
                              css::uno::Reference<css::container::XNameAccess>& xThird) override
    {
        m_pDlg->SetAlternativeAccess(xSecond, xThird);
    }
};
}

VclPtr<AbstractSwRenameXNamedDlg> SwAbstractDialogFactory_Impl::CreateSwRenameXNamedDlg(weld::Widget* pParent,
    css::uno::Reference< css::container::XNamed > & xNamed,
    css::uno::Reference< css::container::XNameAccess > & xNameAccess)
{
    return VclPtr<AbstractSwRenameXNamedDlg_Impl>::Create(pParent,xNamed, xNameAccess);
}

VclPtr<AbstractSwModalRedlineAcceptDlg> SwAbstractDialogFactory_Impl::CreateSwModalRedlineAcceptDlg(weld::Window *pParent)
{
    using AbstractSwModalRedlineAcceptDlg_Impl
        = AbstractDialogImpl_NoSync<AbstractSwModalRedlineAcceptDlg, SwModalRedlineAcceptDlg>;
    return VclPtr<AbstractSwModalRedlineAcceptDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractCopyFieldDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractCopyFieldDlg, CopyFieldDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
};
}

VclPtr<AbstractCopyFieldDlg> SwAbstractDialogFactory_Impl::CreateCopyFieldDlg(weld::Widget* pParent,
    const rtl::OUString& rFieldValue)
{
    return VclPtr<AbstractCopyFieldDlg_Impl>::Create(pParent, rFieldValue);
}

namespace
{
/// Interface implementation for the insert -> fields -> page number wizard dialog
class AbstractSwPageNumberDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractSwPageNumberDlg, SwPageNumberDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    int GetPageNumberPosition() const override { return m_pDlg->GetPageNumberPosition(); };
    int GetPageNumberAlignment() const override { return m_pDlg->GetPageNumberAlignment(); }
    bool GetMirrorOnEvenPages() const override { return m_pDlg->GetMirrorOnEvenPages(); }
    bool GetIncludePageTotal() const override { return m_pDlg->GetIncludePageTotal(); }
    bool GetIncludePageRangeTotal() const override { return m_pDlg->GetIncludePageRangeTotal(); }
    bool GetFitIntoExistingMargins() const override { return m_pDlg->GetFitIntoExistingMargins(); }
    SvxNumType GetPageNumberType() const override { return m_pDlg->GetPageNumberType(); }
    void SetPageNumberType(SvxNumType nSet) override { m_pDlg->SetPageNumberType(nSet); }
};
}

VclPtr<AbstractSwPageNumberDlg> SwAbstractDialogFactory_Impl::CreateSwPageNumberDlg(weld::Window *pParent)
{
    return VclPtr<AbstractSwPageNumberDlg_Impl>::Create(pParent);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTableMergeDialog(weld::Window* pParent, bool& rWithPrev)
{
    using AbstractSwMergeTableDlg_Impl
        = vcl::AbstractDialogImpl_Sync<VclAbstractDialog, SwMergeTableDlg>;
    return VclPtr<AbstractSwMergeTableDlg_Impl>::Create(pParent, rWithPrev);
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateFrameTabDialog(const OUString &rDialogType,
                                                SfxViewFrame& rFrame, weld::Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool        bNewFrame,
                                                const OUString&  sDefPage )
{
    return VclPtr<AbstractTabController_Impl<SwFrameDlg>>::Create(rFrame, pParent, rCoreSet, bNewFrame, rDialogType, false/*bFormat*/, sDefPage, nullptr);
}

namespace
{
class AbstractApplyTabController_Impl
    : public AbstractTabController_Impl_BASE<SfxAbstractApplyTabDialog, SwTemplateDlgController>
{
public:
    using AbstractTabController_Impl_BASE::AbstractTabController_Impl_BASE;
    DECL_LINK(ApplyHdl, weld::Button&, void);

private:
    Link<LinkParamNone*, void> m_aHandler;
    void SetApplyHdl(const Link<LinkParamNone*, void>& rLink) override;
};

IMPL_LINK_NOARG(AbstractApplyTabController_Impl, ApplyHdl, weld::Button&, void)
{
    if (m_pDlg->Apply())
    {
        m_aHandler.Call(nullptr);
        m_pDlg->Applied();
    }
}

void AbstractApplyTabController_Impl::SetApplyHdl(const Link<LinkParamNone*, void>& rLink)
{
    m_aHandler = rLink;
    m_pDlg->SetApplyHandler(LINK(this, AbstractApplyTabController_Impl, ApplyHdl));
}
}

VclPtr<SfxAbstractApplyTabDialog> SwAbstractDialogFactory_Impl::CreateTemplateDialog(
                                                weld::Window *pParent,
                                                SfxStyleSheetBase&  rBase,
                                                SfxStyleFamily      nRegion,
                                                const OUString&     sPage,
                                                SwWrtShell*         pActShell,
                                                bool                bNew )
{
    return VclPtr<AbstractApplyTabController_Impl>::Create(pParent, rBase, nRegion, sPage, pActShell, bNew);
}

namespace
{
class AbstractGlossaryDlg_Impl
    : public AbstractDialogImpl_NoSync<AbstractGlossaryDlg, SwGlossaryDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
    OUString GetCurrGrpName() const override { return m_pDlg->GetCurrGrpName(); }
    OUString GetCurrShortName() const override { return m_pDlg->GetCurrShortName(); }
};
}

VclPtr<AbstractGlossaryDlg> SwAbstractDialogFactory_Impl::CreateGlossaryDlg(SfxViewFrame& rViewFrame, SwGlossaryHdl* pGlosHdl,
                                                                            SwWrtShell *pWrtShell)
{
    return VclPtr<AbstractGlossaryDlg_Impl>::Create(rViewFrame, pGlosHdl, pWrtShell);
}

namespace
{
class AbstractFieldInputDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractFieldInputDlg, SwFieldInputDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void EndDialog(sal_Int32 n) override { m_pDlg->response(n); }
    bool PrevButtonPressed() const override { return m_pDlg->PrevButtonPressed(); }
    bool NextButtonPressed() const override { return m_pDlg->NextButtonPressed(); }
};
}

VclPtr<AbstractFieldInputDlg> SwAbstractDialogFactory_Impl::CreateFieldInputDlg(weld::Widget *pParent,
    SwWrtShell &rSh, SwField* pField, bool bPrevButton, bool bNextButton)
{
    return VclPtr<AbstractFieldInputDlg_Impl>::Create(pParent, rSh, pField, bPrevButton, bNextButton);
}

namespace
{
class AbstractInsFootNoteDlg_Impl
    : public AbstractDialogImpl_NoSync<AbstractInsFootNoteDlg, SwInsFootNoteDlg>
{
public:
    using AbstractDialogImpl_NoSync::AbstractDialogImpl_NoSync;
    void Apply() override { m_pDlg->Apply(); }
    OUString GetFontName() override { return m_pDlg->GetFontName(); }
    bool IsEndNote() override { return m_pDlg->IsEndNote(); }
    OUString GetStr() override { return m_pDlg->GetStr(); }
    // from class Window
    void SetHelpId(const OUString& sHelpId) override { m_pDlg->set_help_id(sHelpId); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<AbstractInsFootNoteDlg> SwAbstractDialogFactory_Impl::CreateInsFootNoteDlg(
    weld::Window * pParent, SwWrtShell &rSh, bool bEd )
{
    return VclPtr<AbstractInsFootNoteDlg_Impl>::Create(pParent, rSh, bEd);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateTitlePageDlg(weld::Window *pParent)
{
    return VclPtr<AbstractGenericDialog_Impl<SwTitlePageDlg>>::Create(pParent);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateVclSwViewDialog(SwView& rView)
{
    return VclPtr<AbstractGenericDialog_Impl<SwLineNumberingDlg>>::Create(rView);
}

std::shared_ptr<AbstractInsTableDlg> SwAbstractDialogFactory_Impl::CreateInsTableDlg(SwView& rView)
{
    return std::make_shared<AbstractInsTableDlg_Impl>(std::make_shared<SwInsTableDlg>(rView));
}

namespace
{
class AbstractJavaEditDialog_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractJavaEditDialog, SwJavaEditDialog>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    OUString GetScriptText() const override { return m_pDlg->GetScriptText(); }
    OUString GetScriptType() const override { return m_pDlg->GetScriptType(); }
    bool IsUrl() const override { return m_pDlg->IsUrl(); }
    bool IsNew() const override { return m_pDlg->IsNew(); }
    bool IsUpdate() const override { return m_pDlg->IsUpdate(); }
};
}

VclPtr<AbstractJavaEditDialog> SwAbstractDialogFactory_Impl::CreateJavaEditDialog(
    weld::Window* pParent, SwWrtShell* pWrtSh)
{
    return VclPtr<AbstractJavaEditDialog_Impl>::Create(pParent, pWrtSh);
}

namespace
{
class AbstractMailMergeDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractMailMergeDlg, SwMailMergeDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    DBManagerOptions GetMergeType() override { return m_pDlg->GetMergeType(); }
    const OUString& GetSaveFilter() const override { return m_pDlg->GetSaveFilter(); }
    css::uno::Sequence<css::uno::Any> GetSelection() const override
    {
        return m_pDlg->GetSelection();
    }
    css::uno::Reference<css::sdbc::XResultSet> GetResultSet() const override
    {
        return m_pDlg->GetResultSet();
    }
    bool IsSaveSingleDoc() const override { return m_pDlg->IsSaveSingleDoc(); }
    bool IsGenerateFromDataBase() const override { return m_pDlg->IsGenerateFromDataBase(); }
    bool IsFileEncryptedFromDataBase() const override
    {
        return m_pDlg->IsFileEncryptedFromDataBase();
    }
    OUString GetColumnName() const override { return m_pDlg->GetColumnName(); }
    OUString GetPasswordColumnName() const override { return m_pDlg->GetPasswordColumnName(); }
    OUString GetTargetURL() const override { return m_pDlg->GetTargetURL(); }
};
}

VclPtr<AbstractMailMergeDlg> SwAbstractDialogFactory_Impl::CreateMailMergeDlg(
                                                weld::Window* pParent, SwWrtShell& rSh,
                                                const OUString& rSourceName,
                                                const OUString& rTableName,
                                                sal_Int32 nCommandType,
                                                const uno::Reference< sdbc::XConnection>& xConnection )
{
    return VclPtr<AbstractMailMergeDlg_Impl>::Create(pParent, rSh, rSourceName, rTableName, nCommandType, xConnection, nullptr);
}

namespace
{
class AbstractMailMergeCreateFromDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractMailMergeCreateFromDlg, SwMailMergeCreateFromDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    bool IsThisDocument() const override { return m_pDlg->IsThisDocument(); }
};
}

VclPtr<AbstractMailMergeCreateFromDlg> SwAbstractDialogFactory_Impl::CreateMailMergeCreateFromDlg(weld::Window* pParent)
{
    return VclPtr<AbstractMailMergeCreateFromDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractMailMergeFieldConnectionsDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractMailMergeFieldConnectionsDlg,
                                          SwMailMergeFieldConnectionsDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    bool IsUseExistingConnections() const override { return m_pDlg->IsUseExistingConnections(); }
};
}

VclPtr<AbstractMailMergeFieldConnectionsDlg> SwAbstractDialogFactory_Impl::CreateMailMergeFieldConnectionsDlg(weld::Window* pParent)
{
    return VclPtr<AbstractMailMergeFieldConnectionsDlg_Impl>::Create(pParent);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateMultiTOXMarkDlg(weld::Window* pParent, SwTOXMgr &rTOXMgr)
{
    using AbstractMultiTOXMarkDlg_Impl
        = vcl::AbstractDialogImpl_Sync<VclAbstractDialog, SwMultiTOXMarkDlg>;
    return VclPtr<AbstractMultiTOXMarkDlg_Impl>::Create(pParent, rTOXMgr);
}

namespace
{
class AbstractNumBulletDialog_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractNumBulletDialog, SwSvxNumBulletTabDialog>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    const SfxItemSet* GetInputItemSet() const override { return m_pDlg->GetInputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<AbstractNumBulletDialog> SwAbstractDialogFactory_Impl::CreateSvxNumBulletTabDialog(weld::Window* pParent,
                                                const SfxItemSet& rSwItemSet,
                                                SwWrtShell & rWrtSh)
{
    return VclPtr<AbstractNumBulletDialog_Impl>::Create(pParent, rSwItemSet, rWrtSh);
}

VclPtr<SfxAbstractTabDialog> SwAbstractDialogFactory_Impl::CreateOutlineTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell & rWrtSh )
{
    return VclPtr<AbstractTabController_Impl<SwOutlineTabDialog>>::Create(pParent, pSwItemSet, rWrtSh);
}

namespace
{
class AbstractMultiTOXTabDialog_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractMultiTOXTabDialog, SwMultiTOXTabDialog>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    CurTOXType GetCurrentTOXType() const override { return m_pDlg->GetCurrentTOXType(); }
    SwTOXDescription& GetTOXDescription(CurTOXType eTOXTypes) override
    {
        return m_pDlg->GetTOXDescription(eTOXTypes);
    }
    // from SfxTabDialog
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
};
}

VclPtr<AbstractMultiTOXTabDialog> SwAbstractDialogFactory_Impl::CreateMultiTOXTabDialog(weld::Widget* pParent, const SfxItemSet& rSet,
                                                                                        SwWrtShell &rShell, SwTOXBase* pCurTOX, bool bGlobal)
{
    return VclPtr<AbstractMultiTOXTabDialog_Impl>::Create(pParent, rSet, rShell, pCurTOX, USHRT_MAX, bGlobal);
}

namespace
{
class AbstractEditRegionDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractEditRegionDlg, SwEditRegionDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void SelectSection(const OUString& rSectionName) override
    {
        m_pDlg->SelectSection(rSectionName);
    }
};
}

VclPtr<AbstractEditRegionDlg> SwAbstractDialogFactory_Impl::CreateEditRegionDlg(weld::Window* pParent, SwWrtShell& rWrtSh)
{
    return VclPtr<AbstractEditRegionDlg_Impl>::Create(pParent, rWrtSh);
}

namespace
{
class AbstractInsertSectionTabDialog_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractInsertSectionTabDialog, SwInsertSectionTabDialog>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void SetSectionData(SwSectionData const& rSect) override { m_pDlg->SetSectionData(rSect); }
};
}

VclPtr<AbstractInsertSectionTabDialog> SwAbstractDialogFactory_Impl::CreateInsertSectionTabDialog(weld::Window* pParent,
        const SfxItemSet& rSet, SwWrtShell& rSh)
{
    return VclPtr<AbstractInsertSectionTabDialog_Impl>::Create(pParent, rSet, rSh);
}

namespace
{
class AbstractIndexMarkFloatDlg_Impl
    : public vcl::AbstractDialogImpl_Sync_Shared<AbstractMarkFloatDlg, SwIndexMarkFloatDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void ReInitDlg(SwWrtShell& rWrtShell) override { m_pDlg->ReInitDlg(rWrtShell); }
    std::shared_ptr<SfxDialogController> GetController() override { return m_pDlg; }
};
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateIndexMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo )
{
    return VclPtr<AbstractIndexMarkFloatDlg_Impl>::Create(pBindings, pChild, pParent, pInfo, true/*bNew*/);
}

namespace
{
class AbstractAuthMarkFloatDlg_Impl
    : public vcl::AbstractDialogImpl_Sync_Shared<AbstractMarkFloatDlg, SwAuthMarkFloatDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void ReInitDlg(SwWrtShell& rWrtShell) override { m_pDlg->ReInitDlg(rWrtShell); }
    std::shared_ptr<SfxDialogController> GetController() override { return m_pDlg; }
};
}

VclPtr<AbstractMarkFloatDlg> SwAbstractDialogFactory_Impl::CreateAuthMarkFloatDlg(
                                                       SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       weld::Window *pParent,
                                                       SfxChildWinInfo* pInfo)
{
    return VclPtr<AbstractAuthMarkFloatDlg_Impl>::Create(pBindings, pChild, pParent, pInfo, true/*bNew*/);
}

namespace
{
class AbstractSwWordCountFloatDlg_Impl
    : public vcl::AbstractDialogImpl_Sync_Shared<AbstractSwWordCountFloatDlg, SwWordCountFloatDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void UpdateCounts() override { m_pDlg->UpdateCounts(); }
    void SetCounts(const SwDocStat& rCurrCnt, const SwDocStat& rDocStat) override
    {
        m_pDlg->SetCounts(rCurrCnt, rDocStat);
    }
    std::shared_ptr<SfxDialogController> GetController() override { return m_pDlg; }
};
}

VclPtr<AbstractSwWordCountFloatDlg> SwAbstractDialogFactory_Impl::CreateSwWordCountDialog(
                                                                              SfxBindings* pBindings,
                                                                              SfxChildWindow* pChild,
                                                                              weld::Window *pParent,
                                                                              SfxChildWinInfo* pInfo)
{
    return VclPtr<AbstractSwWordCountFloatDlg_Impl>::Create(pBindings, pChild, pParent, pInfo);
}

VclPtr<VclAbstractDialog> SwAbstractDialogFactory_Impl::CreateIndexMarkModalDlg(weld::Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark )
{
    return VclPtr<AbstractGenericDialog_Impl<SwIndexMarkModalDlg>>::Create(pParent, rSh, pCurTOXMark);
}

namespace
{
class AbstractMailMergeWizard_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractMailMergeWizard, SwMailMergeWizard>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    bool StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx) override;

    OUString GetReloadDocument() const override { return m_pDlg->GetReloadDocument(); }
    void ShowPage(sal_uInt16 nLevel) override { m_pDlg->skipUntil(nLevel); }
    sal_uInt16 GetRestartPage() const override { return m_pDlg->GetRestartPage(); }
};

bool AbstractMailMergeWizard_Impl::StartExecuteAsync(AsyncContext& rCtx)
{
    // SwMailMergeWizardExecutor wants to run the lifecycle of this dialog
    // so clear mxOwner here and leave it up to SwMailMergeWizardExecutor
    rCtx.mxOwner.clear();
    return AbstractDialogImpl_BASE::StartExecuteAsync(rCtx);
}
}

VclPtr<AbstractMailMergeWizard> SwAbstractDialogFactory_Impl::CreateMailMergeWizard(
                                    SwView& rView, std::shared_ptr<SwMailMergeConfigItem>& rConfigItem)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    return VclPtr<AbstractMailMergeWizard_Impl>::Create(rView, rConfigItem);
#else
    (void) rView;
    (void) rConfigItem;
    return nullptr;
#endif
}

GlossaryGetCurrGroup    SwAbstractDialogFactory_Impl::GetGlossaryCurrGroupFunc()
{
    return SwGlossaryDlg::GetCurrGroup;
}

GlossarySetActGroup SwAbstractDialogFactory_Impl::SetGlossaryActGroupFunc()
{
    return SwGlossaryDlg::SetActGroup;
}

// Factories for TabPages
CreateTabPage SwAbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    CreateTabPage pRet = nullptr;
    switch ( nId )
    {
        case RID_SW_TP_OPTCOMPATIBILITY_PAGE :
            pRet = SwCompatibilityOptPage::Create;
            break;
        case RID_SW_TP_OPTLOAD_PAGE :
            pRet = SwLoadOptPage::Create;
            break;
        case RID_SW_TP_OPTCAPTION_PAGE:
            return SwCaptionOptPage::Create;
        case RID_SW_TP_CONTENT_OPT:
        case RID_SW_TP_HTML_CONTENT_OPT:
            pRet = SwContentOptPage::Create;
            break;
        case RID_SW_TP_OPTSHDWCRSR:
        case RID_SW_TP_HTML_OPTSHDWCRSR:
            pRet = SwShdwCursorOptionsTabPage::Create;
            break;
        case RID_SW_TP_REDLINE_OPT :
            pRet = SwRedlineOptionsTabPage::Create;
            break;
        case RID_SW_TP_OPTTEST_PAGE :
#ifdef DBG_UTIL
            pRet = SwTestTabPage::Create;
#endif
            break;
        case TP_OPTPRINT_PAGE :
        case RID_SW_TP_HTML_OPTPRINT_PAGE:
        case RID_SW_TP_OPTPRINT_PAGE:
            pRet = SwAddPrinterTabPage::Create;
            break;
        case RID_SW_TP_STD_FONT:
        case RID_SW_TP_STD_FONT_CJK:
        case RID_SW_TP_STD_FONT_CTL:
            pRet = SwStdFontTabPage::Create;
            break;
        case RID_SW_TP_HTML_OPTTABLE_PAGE:
        case RID_SW_TP_OPTTABLE_PAGE:
            pRet = SwTableOptionsTabPage::Create;
            break;
        case RID_SW_TP_DOC_STAT :
            pRet = SwDocStatPage::Create;
            break;
        case RID_SW_TP_MAILCONFIG:
            pRet = SwMailConfigPage::Create;
        break;
        case RID_SW_TP_COMPARISON_OPT :
            pRet = SwCompareOptionsTabPage::Create;
        break;
    }

    return pRet;
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultSaveDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SwMMResultSaveDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultPrintDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SwMMResultPrintDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

void SwAbstractDialogFactory_Impl::ExecuteMMResultEmailDialog(weld::Window* pParent)
{
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    SwMMResultEmailDialog aDialog(pParent);
    aDialog.run();
#else
    (void) pParent;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
