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

#undef SC_DLLIMPLEMENTATION

#include "scdlgfact.hxx"

#include <scuiasciiopt.hxx>
#include <scuiautofmt.hxx>
#include <corodlg.hxx>
#include <dapidata.hxx>
#include <dapitype.hxx>
#include <delcldlg.hxx>
#include <delcodlg.hxx>
#include <filldlg.hxx>
#include <gototabdlg.hxx>
#include <groupdlg.hxx>
#include <inscldlg.hxx>
#include <inscodlg.hxx>
#include <instbdlg.hxx>
#include <lbseldlg.hxx>
#include <linkarea.hxx>
#include <mtrindlg.hxx>
#include <mvtabdlg.hxx>
#include <namecrea.hxx>
#include <namepast.hxx>
#include <pfiltdlg.hxx>
#include <pvfundlg.hxx>
#include <dpgroupdlg.hxx>
#include <scendlg.hxx>
#include <shtabdlg.hxx>
#include <strindlg.hxx>
#include <tabbgcolordlg.hxx>
#include <scuiimoptdlg.hxx>
#include <attrdlg.hxx>
#include <hfedtdlg.hxx>
#include <styledlg.hxx>
#include <subtdlg.hxx>
#include <textdlgs.hxx>
#include <sortdlg.hxx>
#include <textimportoptions.hxx>
#include <opredlin.hxx>
#include <tpcalc.hxx>
#include <tpprint.hxx>
#include <tpstat.hxx>
#include <tpusrlst.hxx>
#include <tpview.hxx>
#include <tpformula.hxx>
#include <datafdlg.hxx>
#include <tpcompatibility.hxx>
#include <tpdefaults.hxx>
#include <condformatmgr.hxx>
#include <scres.hrc>
#include <svx/dialogs.hrc>
#include <sfx2/sfxdlg.hxx>
#include <conditio.hxx>

#include <vcl/abstdlgimpl.hxx>
#include <vcl/virdev.hxx>

namespace
{
template <class Base, class Dialog, template <class...> class ImplTemplate>
class ScreenshottedDialog_Impl_BASE : public ImplTemplate<Base, Dialog>
{
public:
    using ImplTemplate<Base, Dialog>::ImplTemplate;

    // screenshotting
    BitmapEx createScreenshot() const override
    {
        VclPtr<VirtualDevice> xDialogSurface(this->m_pDlg->getDialog()->screenshot());
        return xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel());
    }
    OUString GetScreenshotId() const override { return this->m_pDlg->get_help_id(); }
};

template <class Base, class Dialog>
using ScreenshottedDialog_Impl_Sync
    = ScreenshottedDialog_Impl_BASE<Base, Dialog, vcl::AbstractDialogImpl_Sync>;

template <class Base, class Dialog>
using ScreenshottedDialog_Impl_Async
    = ScreenshottedDialog_Impl_BASE<Base, Dialog, vcl::AbstractDialogImpl_Async>;
}

// =========================Factories  for createdialog ===================

namespace
{
class AbstractScImportAsciiDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScImportAsciiDlg, ScImportAsciiDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetOptions(ScAsciiOptions& rOpt) override { m_pDlg->GetOptions(rOpt); }
    void SaveParameters() override { m_pDlg->SaveParameters(); }
};
}

VclPtr<AbstractScImportAsciiDlg> ScAbstractDialogFactory_Impl::CreateScImportAsciiDlg(weld::Window* pParent,
                                                    const OUString& aDatName,
                                                    SvStream* pInStream, ScImportAsciiCall eCall)
{
    return VclPtr<AbstractScImportAsciiDlg_Impl>::Create(pParent, aDatName, pInStream, eCall);
}

namespace
{
class AbstractScTextImportOptionsDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractScTextImportOptionsDlg, ScTextImportOptionsDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    LanguageType GetLanguageType() const override { return m_pDlg->getLanguageType(); }
    bool IsDateConversionSet() const override { return m_pDlg->isDateConversionSet(); }
    bool IsScientificConversionSet() const override { return m_pDlg->isScientificConversionSet(); }
};
}

VclPtr<AbstractScTextImportOptionsDlg> ScAbstractDialogFactory_Impl::CreateScTextImportOptionsDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScTextImportOptionsDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractScAutoFormatDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScAutoFormatDlg, ScAutoFormatDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    sal_uInt16 GetIndex() const override { return m_pDlg->GetIndex(); }
    OUString GetCurrFormatName() override { return m_pDlg->GetCurrFormatName(); }
};
}

VclPtr<AbstractScAutoFormatDlg> ScAbstractDialogFactory_Impl::CreateScAutoFormatDlg(weld::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData& rViewData)
{
    return VclPtr<AbstractScAutoFormatDlg_Impl>::Create(pParent, pAutoFormat, pSelFormatData, rViewData);
}

namespace
{
class AbstractScColRowLabelDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScColRowLabelDlg, ScColRowLabelDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    bool IsCol() override { return m_pDlg->IsCol(); }
    bool IsRow() override { return m_pDlg->IsRow(); }
};
}

VclPtr<AbstractScColRowLabelDlg>  ScAbstractDialogFactory_Impl::CreateScColRowLabelDlg(weld::Window* pParent,
                                                                bool bCol, bool bRow)
{
    return VclPtr<AbstractScColRowLabelDlg_Impl>::Create(pParent, bCol, bRow);
}

VclPtr<AbstractScSortWarningDlg> ScAbstractDialogFactory_Impl::CreateScSortWarningDlg(weld::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText)
{
    using AbstractScSortWarningDlg_Impl
        = vcl::AbstractDialogImpl_Sync<AbstractScSortWarningDlg, ScSortWarningDlg>;
    return VclPtr<AbstractScSortWarningDlg_Impl>::Create(pParent, rExtendText, rCurrentText);
}

namespace
{
class AbstractScCondFormatManagerDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScCondFormatManagerDlg, ScCondFormatManagerDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    std::unique_ptr<ScConditionalFormatList> GetConditionalFormatList() override
    {
        return m_pDlg->GetConditionalFormatList();
    }
    bool CondFormatsChanged() const override { return m_pDlg->CondFormatsChanged(); }
    void SetModified() override { return m_pDlg->SetModified(); }
    ScConditionalFormat* GetCondFormatSelected() override
    {
        return m_pDlg->GetCondFormatSelected();
    }
};
}

VclPtr<AbstractScCondFormatManagerDlg> ScAbstractDialogFactory_Impl::CreateScCondFormatMgrDlg(weld::Window* pParent, ScDocument& rDoc, const ScConditionalFormatList* pFormatList )
{
    return VclPtr<AbstractScCondFormatManagerDlg_Impl>::Create(pParent, rDoc, pFormatList);
}

namespace
{
class AbstractScDataPilotDatabaseDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScDataPilotDatabaseDlg, ScDataPilotDatabaseDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetValues(ScImportSourceDesc& rDesc) override { m_pDlg->GetValues(rDesc); }
};
}

VclPtr<AbstractScDataPilotDatabaseDlg> ScAbstractDialogFactory_Impl::CreateScDataPilotDatabaseDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScDataPilotDatabaseDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractScDataPilotSourceTypeDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScDataPilotSourceTypeDlg,
                                            ScDataPilotSourceTypeDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    bool IsDatabase() const override { return m_pDlg->IsDatabase(); }
    bool IsExternal() const override { return m_pDlg->IsExternal(); }
    bool IsNamedRange() const override { return m_pDlg->IsNamedRange(); }
    OUString GetSelectedNamedRange() const override { return m_pDlg->GetSelectedNamedRange(); }
    void AppendNamedRange(const OUString& rName) override { m_pDlg->AppendNamedRange(rName); }
};
}

VclPtr<AbstractScDataPilotSourceTypeDlg> ScAbstractDialogFactory_Impl::CreateScDataPilotSourceTypeDlg(
    weld::Window* pParent, bool bEnableExternal)
{
    return VclPtr<AbstractScDataPilotSourceTypeDlg_Impl>::Create(pParent, bEnableExternal);
}

namespace
{
class AbstractScDataPilotServiceDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScDataPilotServiceDlg, ScDataPilotServiceDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    OUString GetServiceName() const override { return m_pDlg->GetServiceName(); }
    OUString GetParSource() const override { return m_pDlg->GetParSource(); }
    OUString GetParName() const override { return m_pDlg->GetParName(); }
    OUString GetParUser() const override { return m_pDlg->GetParUser(); }
    OUString GetParPass() const override { return m_pDlg->GetParPass(); }
};
}

VclPtr<AbstractScDataPilotServiceDlg> ScAbstractDialogFactory_Impl::CreateScDataPilotServiceDlg(weld::Window* pParent,
                                                                        const std::vector<OUString>& rServices)
{
    return VclPtr<AbstractScDataPilotServiceDlg_Impl>::Create(pParent, rServices);
}

namespace
{
class AbstractScDeleteCellDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScDeleteCellDlg, ScDeleteCellDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    DelCellCmd GetDelCellCmd() const override { return m_pDlg->GetDelCellCmd(); }
};
}

VclPtr<AbstractScDeleteCellDlg> ScAbstractDialogFactory_Impl::CreateScDeleteCellDlg(weld::Window* pParent,
    bool bDisallowCellMove)
{
    return VclPtr<AbstractScDeleteCellDlg_Impl>::Create(pParent, bDisallowCellMove);
}

VclPtr<AbstractScDataFormDlg> ScAbstractDialogFactory_Impl::CreateScDataFormDlg(weld::Window* pParent,
    ScTabViewShell* pTabViewShell)
{
    // for dataform
    using AbstractScDataFormDlg_Impl
        = ScreenshottedDialog_Impl_Sync<AbstractScDataFormDlg, ScDataFormDlg>;
    return VclPtr<AbstractScDataFormDlg_Impl>::Create(pParent, pTabViewShell);
}

namespace
{
class AbstractScDeleteContentsDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractScDeleteContentsDlg, ScDeleteContentsDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void DisableObjects() override { m_pDlg->DisableObjects(); }
    InsertDeleteFlags GetDelContentsCmdBits() const override
    {
        return m_pDlg->GetDelContentsCmdBits();
    }
};
}

VclPtr<AbstractScDeleteContentsDlg> ScAbstractDialogFactory_Impl::CreateScDeleteContentsDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScDeleteContentsDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractScFillSeriesDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScFillSeriesDlg, ScFillSeriesDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    FillDir GetFillDir() const override { return m_pDlg->GetFillDir(); }
    FillCmd GetFillCmd() const override { return m_pDlg->GetFillCmd(); }
    FillDateCmd GetFillDateCmd() const override { return m_pDlg->GetFillDateCmd(); }
    double GetStart() const override { return m_pDlg->GetStart(); }
    double GetStep() const override { return m_pDlg->GetStep(); }
    double GetMax() const override { return m_pDlg->GetMax(); }
    OUString GetStartStr() const override { return m_pDlg->GetStartStr(); }
    void SetEdStartValEnabled(bool bFlag) override { m_pDlg->SetEdStartValEnabled(bFlag); }
};
}

VclPtr<AbstractScFillSeriesDlg> ScAbstractDialogFactory_Impl::CreateScFillSeriesDlg(weld::Window*       pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            const SCSIZE    nSelectHeight,
                                                            const SCSIZE    nSelectWidth,
                                                            sal_uInt16      nPossDir)
{
    return VclPtr<AbstractScFillSeriesDlg_Impl>::Create(pParent, rDocument,eFillDir, eFillCmd,eFillDateCmd, aStartStr,fStep,fMax,nSelectHeight,nSelectWidth,nPossDir);
}

namespace
{
class AbstractScGroupDlg_Impl : public vcl::AbstractDialogImpl_Async<AbstractScGroupDlg, ScGroupDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    bool GetColsChecked() const override { return m_pDlg->GetColsChecked(); }
};
}

VclPtr<AbstractScGroupDlg> ScAbstractDialogFactory_Impl::CreateAbstractScGroupDlg(weld::Window* pParent, bool bUnGroup)
{
    return VclPtr<AbstractScGroupDlg_Impl>::Create(pParent, bUnGroup, true);
}

namespace
{
class AbstractScInsertCellDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScInsertCellDlg, ScInsertCellDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    InsCellCmd GetInsCellCmd() const override { return m_pDlg->GetInsCellCmd(); }
    size_t GetCount() const override { return m_pDlg->GetCount(); }
};
}

VclPtr<AbstractScInsertCellDlg> ScAbstractDialogFactory_Impl::CreateScInsertCellDlg(weld::Window* pParent,
                                                                bool bDisallowCellMove)
{
    return VclPtr<AbstractScInsertCellDlg_Impl>::Create(pParent, bDisallowCellMove);
}

namespace
{
class AbstractScInsertContentsDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractScInsertContentsDlg, ScInsertContentsDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    InsertDeleteFlags GetInsContentsCmdBits() const override
    {
        return m_pDlg->GetInsContentsCmdBits();
    }
    ScPasteFunc GetFormulaCmdBits() const override { return m_pDlg->GetFormulaCmdBits(); }
    bool IsSkipEmptyCells() const override { return m_pDlg->IsSkipEmptyCells(); }
    bool IsLink() const override { return m_pDlg->IsLink(); }
    void SetFillMode(bool bSet) override { m_pDlg->SetFillMode(bSet); }
    void SetOtherDoc(bool bSet) override { m_pDlg->SetOtherDoc(bSet); }
    bool IsTranspose() const override { return m_pDlg->IsTranspose(); }
    void SetChangeTrack(bool bSet) override { m_pDlg->SetChangeTrack(bSet); }
    void SetCellShiftDisabled(CellShiftDisabledFlags nDisable) override
    {
        m_pDlg->SetCellShiftDisabled(nDisable);
    }
    InsCellCmd GetMoveMode() override { return m_pDlg->GetMoveMode(); }
};
}

VclPtr<AbstractScInsertContentsDlg> ScAbstractDialogFactory_Impl::CreateScInsertContentsDlg(weld::Window* pParent,
                                                                                            const OUString* pStrTitle)
{
    return VclPtr<AbstractScInsertContentsDlg_Impl>::Create(pParent, pStrTitle);
}

namespace
{
class AbstractScInsertTableDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScInsertTableDlg, ScInsertTableDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    bool GetTablesFromFile() override { return m_pDlg->GetTablesFromFile(); }
    bool GetTablesAsLink() override { return m_pDlg->GetTablesAsLink(); }
    const OUString* GetFirstTable(sal_uInt16* pN) override { return m_pDlg->GetFirstTable(pN); }
    ScDocShell* GetDocShellTables() override { return m_pDlg->GetDocShellTables(); }
    bool IsTableBefore() override { return m_pDlg->IsTableBefore(); }
    sal_uInt16 GetTableCount() override { return m_pDlg->GetTableCount(); }
    const OUString* GetNextTable(sal_uInt16* pN) override { return m_pDlg->GetNextTable(pN); }
};
}

VclPtr<AbstractScInsertTableDlg> ScAbstractDialogFactory_Impl::CreateScInsertTableDlg(weld::Window* pParent, ScViewData& rViewData,
    SCTAB nTabCount, bool bFromFile)
{
    return VclPtr<AbstractScInsertTableDlg_Impl>::Create(pParent, rViewData,nTabCount, bFromFile);
}

namespace
{
class AbstractScSelEntryDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScSelEntryDlg, ScSelEntryDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    OUString GetSelectedEntry() const override { return m_pDlg->GetSelectedEntry(); }
};
}

VclPtr<AbstractScSelEntryDlg> ScAbstractDialogFactory_Impl::CreateScSelEntryDlg(weld::Window* pParent,
                                                                                const std::vector<OUString> &rEntryList)
{
    return VclPtr<AbstractScSelEntryDlg_Impl>::Create(pParent, rEntryList);
}

namespace
{
class AbstractScLinkedAreaDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScLinkedAreaDlg, ScLinkedAreaDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void InitFromOldLink(const OUString& rFile, const OUString& rFilter, const OUString& rOptions,
                         const OUString& rSource, sal_Int32 nRefreshDelaySeconds) override
    {
        m_pDlg->InitFromOldLink(rFile, rFilter, rOptions, rSource, nRefreshDelaySeconds);
    }
    OUString GetURL() override { return m_pDlg->GetURL(); }
    OUString GetFilter() override { return m_pDlg->GetFilter(); }
    OUString GetOptions() override { return m_pDlg->GetOptions(); }
    OUString GetSource() override { return m_pDlg->GetSource(); }
    sal_Int32 GetRefreshDelaySeconds() override { return m_pDlg->GetRefreshDelaySeconds(); }
};
}

VclPtr<AbstractScLinkedAreaDlg> ScAbstractDialogFactory_Impl::CreateScLinkedAreaDlg(weld::Widget* pParent)
{
    return VclPtr<AbstractScLinkedAreaDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractScMetricInputDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScMetricInputDlg, ScMetricInputDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    int GetInputValue() const override { return m_pDlg->GetInputValue(); }
};
}

VclPtr<AbstractScMetricInputDlg> ScAbstractDialogFactory_Impl::CreateScMetricInputDlg(weld::Window* pParent,
                                                                const OUString& sDialogName,
                                                                tools::Long            nCurrent,
                                                                tools::Long            nDefault,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                tools::Long            nMaximum ,
                                                                tools::Long            nMinimum )
{
    return VclPtr<AbstractScMetricInputDlg_Impl>::Create(pParent, sDialogName, nCurrent, nDefault,
                                                      eFUnit, nDecimals, nMaximum, nMinimum);
}

namespace
{
class AbstractScMoveTableDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScMoveTableDlg, ScMoveTableDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    sal_uInt16 GetSelectedDocument() const override { return m_pDlg->GetSelectedDocument(); }
    sal_uInt16 GetSelectedTable() const override { return m_pDlg->GetSelectedTable(); }
    bool GetCopyTable() const override { return m_pDlg->GetCopyTable(); }
    bool GetRenameTable() const override { return m_pDlg->GetRenameTable(); }
    void GetTabNameString(OUString& rString) const override { m_pDlg->GetTabNameString(rString); }
    void SetForceCopyTable() override { return m_pDlg->SetForceCopyTable(); }
    void EnableRenameTable(bool bFlag) override { return m_pDlg->EnableRenameTable(bFlag); }
};
}

VclPtr<AbstractScMoveTableDlg> ScAbstractDialogFactory_Impl::CreateScMoveTableDlg(weld::Window* pParent,
    const OUString& rDefault)
{
    return VclPtr<AbstractScMoveTableDlg_Impl>::Create(pParent, rDefault);
}

namespace
{
class AbstractScNameCreateDlg_Impl
    : public ScreenshottedDialog_Impl_Sync<AbstractScNameCreateDlg, ScNameCreateDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    CreateNameFlags GetFlags() const override { return m_pDlg->GetFlags(); }
};
}

VclPtr<AbstractScNameCreateDlg> ScAbstractDialogFactory_Impl::CreateScNameCreateDlg(weld::Window * pParent, CreateNameFlags nFlags)
{
    return VclPtr<AbstractScNameCreateDlg_Impl>::Create(pParent, nFlags);
}

namespace
{
class AbstractScNamePasteDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScNamePasteDlg, ScNamePasteDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    std::vector<OUString> GetSelectedNames() const override { return m_pDlg->GetSelectedNames(); }
};
}

VclPtr<AbstractScNamePasteDlg> ScAbstractDialogFactory_Impl::CreateScNamePasteDlg(weld::Window * pParent, ScDocShell* pShell)
{
    return VclPtr<AbstractScNamePasteDlg_Impl>::Create(pParent, pShell);
}

namespace
{
class AbstractScPivotFilterDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScPivotFilterDlg, ScPivotFilterDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    const ScQueryItem& GetOutputItem() override { return m_pDlg->GetOutputItem(); }
};
}

VclPtr<AbstractScPivotFilterDlg> ScAbstractDialogFactory_Impl::CreateScPivotFilterDlg(weld::Window* pParent,
    const SfxItemSet& rArgSet, sal_uInt16 nSourceTab)
{
    return VclPtr<AbstractScPivotFilterDlg_Impl>::Create(pParent, rArgSet, nSourceTab);
}

namespace
{
class AbstractScDPFunctionDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScDPFunctionDlg, ScDPFunctionDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    PivotFunc GetFuncMask() const override { return m_pDlg->GetFuncMask(); }
    css::sheet::DataPilotFieldReference GetFieldRef() const override
    {
        return m_pDlg->GetFieldRef();
    }
    void Response(int nResponse) override { m_pDlg->response(nResponse); }
};
}

VclPtr<AbstractScDPFunctionDlg> ScAbstractDialogFactory_Impl::CreateScDPFunctionDlg(weld::Widget* pParent,
                                                                                    const ScDPLabelDataVector& rLabelVec,
                                                                                    const ScDPLabelData& rLabelData,
                                                                                    const ScPivotFuncData& rFuncData)
{
    return VclPtr<AbstractScDPFunctionDlg_Impl>::Create(pParent, rLabelVec, rLabelData, rFuncData);
}

namespace
{
class AbstractScDPSubtotalDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScDPSubtotalDlg, ScDPSubtotalDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    PivotFunc GetFuncMask() const override { return m_pDlg->GetFuncMask(); }
    void FillLabelData(ScDPLabelData& rLabelData) const override
    {
        m_pDlg->FillLabelData(rLabelData);
    }
    void Response(int nResponse) override { m_pDlg->response(nResponse); }
};
}

VclPtr<AbstractScDPSubtotalDlg> ScAbstractDialogFactory_Impl::CreateScDPSubtotalDlg(weld::Widget* pParent,
                                                                                    ScDPObject& rDPObj,
                                                                                    const ScDPLabelData& rLabelData,
                                                                                    const ScPivotFuncData& rFuncData,
                                                                                    const ScDPNameVec& rDataFields)
{
    return VclPtr<AbstractScDPSubtotalDlg_Impl>::Create(pParent, rDPObj, rLabelData, rFuncData, rDataFields, true/*bEnableLayout*/);
}

namespace
{
class AbstractScDPNumGroupDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScDPNumGroupDlg, ScDPNumGroupDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    ScDPNumGroupInfo GetGroupInfo() const override { return m_pDlg->GetGroupInfo(); }
};
}

VclPtr<AbstractScDPNumGroupDlg> ScAbstractDialogFactory_Impl::CreateScDPNumGroupDlg(weld::Window* pParent, const ScDPNumGroupInfo& rInfo)
{
    return VclPtr<AbstractScDPNumGroupDlg_Impl>::Create(pParent, rInfo);
}

namespace
{
class AbstractScDPDateGroupDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScDPDateGroupDlg, ScDPDateGroupDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    ScDPNumGroupInfo GetGroupInfo() const override { return m_pDlg->GetGroupInfo(); }
    sal_Int32 GetDatePart() const override { return m_pDlg->GetDatePart(); }
};
}

VclPtr<AbstractScDPDateGroupDlg> ScAbstractDialogFactory_Impl::CreateScDPDateGroupDlg(
        weld::Window* pParent, const ScDPNumGroupInfo& rInfo, sal_Int32 nDatePart, const Date& rNullDate)
{
    return VclPtr<AbstractScDPDateGroupDlg_Impl>::Create(pParent, rInfo, nDatePart, rNullDate);
}

namespace
{
class AbstractScDPShowDetailDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScDPShowDetailDlg, ScDPShowDetailDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    OUString GetDimensionName() const override { return m_pDlg->GetDimensionName(); }
};
}

VclPtr<AbstractScDPShowDetailDlg> ScAbstractDialogFactory_Impl::CreateScDPShowDetailDlg (
        weld::Window* pParent, ScDPObject& rDPObj, css::sheet::DataPilotFieldOrientation nOrient )
{
    return VclPtr<AbstractScDPShowDetailDlg_Impl>::Create(pParent, rDPObj, nOrient);
}

namespace
{
class AbstractScNewScenarioDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScNewScenarioDlg, ScNewScenarioDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;

    void SetScenarioData(const OUString& rName, const OUString& rComment, const Color& rColor,
                         ScScenarioFlags nFlags) override
    {
        m_pDlg->SetScenarioData(rName, rComment, rColor, nFlags);
    }

    void GetScenarioData(OUString& rName, OUString& rComment, Color& rColor,
                         ScScenarioFlags& rFlags) const override
    {
        m_pDlg->GetScenarioData(rName, rComment, rColor, rFlags);
    }
};
}

VclPtr<AbstractScNewScenarioDlg> ScAbstractDialogFactory_Impl::CreateScNewScenarioDlg(weld::Window* pParent, const OUString& rName,
    bool bEdit, bool bSheetProtected)
{
    return VclPtr<AbstractScNewScenarioDlg_Impl>::Create(pParent, rName, bEdit, bSheetProtected);
}

namespace
{
class AbstractScShowTabDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScShowTabDlg, ScShowTabDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void Insert(const OUString& rString, bool bSelected) override
    {
        m_pDlg->Insert(rString, bSelected);
    }
    void SetDescription(const OUString& rTitle, const OUString& rFixedText,
                        const OUString& sDlgHelpId, const OUString& sLbHelpId) override
    {
        m_pDlg->SetDescription(rTitle, rFixedText, sDlgHelpId, sLbHelpId);
    }
    OUString GetEntry(sal_Int32 nPos) const override { return m_pDlg->GetEntry(nPos); }
    std::vector<sal_Int32> GetSelectedRows() const override { return m_pDlg->GetSelectedRows(); }
};
}

VclPtr<AbstractScShowTabDlg> ScAbstractDialogFactory_Impl::CreateScShowTabDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScShowTabDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractScGoToTabDlg_Impl
    : public vcl::AbstractDialogImpl_Async<AbstractScGoToTabDlg, ScGoToTabDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void Insert(const OUString& rString, bool bSelected) override
    {
        m_pDlg->Insert(rString, bSelected);
    }
    void SetDescription(const OUString& rTitle, const OUString& rEntryLabel,
                        const OUString& rListLabel, const OUString& rDlgHelpId,
                        const OUString& rEnHelpId, const OUString& rLbHelpId) override
    {
        m_pDlg->SetDescription(rTitle, rEntryLabel, rListLabel, rDlgHelpId, rEnHelpId, rLbHelpId);
    }
    OUString GetSelectedEntry() const override { return m_pDlg->GetSelectedEntry(); }
};
}

VclPtr<AbstractScGoToTabDlg> ScAbstractDialogFactory_Impl::CreateScGoToTabDlg(weld::Window* pParent)
{
    return VclPtr<AbstractScGoToTabDlg_Impl>::Create(pParent);
}

namespace
{
class AbstractScStringInputDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScStringInputDlg, ScStringInputDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    OUString GetInputString() const override { return m_pDlg->GetInputString(); }
};
}

VclPtr<AbstractScStringInputDlg> ScAbstractDialogFactory_Impl::CreateScStringInputDlg(weld::Window* pParent,
        const OUString& rTitle, const OUString& rEditTitle, const OUString& rDefault, const OUString& rHelpId,
        const OUString& rEditHelpId)
{
    return VclPtr<AbstractScStringInputDlg_Impl>::Create(pParent, rTitle, rEditTitle, rDefault,
                                                      rHelpId, rEditHelpId);
}

namespace
{
class AbstractScTabBgColorDlg_Impl
    : public ScreenshottedDialog_Impl_Async<AbstractScTabBgColorDlg, ScTabBgColorDlg>
{
public:
    using ScreenshottedDialog_Impl_BASE::ScreenshottedDialog_Impl_BASE;
    void GetSelectedColor(Color& rColor) const override { m_pDlg->GetSelectedColor(rColor); }
};
}

VclPtr<AbstractScTabBgColorDlg> ScAbstractDialogFactory_Impl::CreateScTabBgColorDlg(
                                                            weld::Window* pParent,
                                                            const OUString& rTitle,
                                                            const OUString& rTabBgColorNoColorText,
                                                            const Color& rDefaultColor)
{
    return VclPtr<AbstractScTabBgColorDlg_Impl>::Create(pParent, rTitle, rTabBgColorNoColorText, rDefaultColor);
}

namespace
{
class AbstractScImportOptionsDlg_Impl
    : public vcl::AbstractDialogImpl_Sync<AbstractScImportOptionsDlg, ScImportOptionsDlg>
{
public:
    using AbstractDialogImpl_BASE::AbstractDialogImpl_BASE;
    void GetImportOptions(ScImportOptions& rOptions) const override
    {
        m_pDlg->GetImportOptions(rOptions);
    }
    void SaveImportOptions() const override { m_pDlg->SaveImportOptions(); }
};
}

VclPtr<AbstractScImportOptionsDlg> ScAbstractDialogFactory_Impl::CreateScImportOptionsDlg(weld::Window* pParent,
                                                                                          bool bAscii,
                                                                                          const ScImportOptions* pOptions,
                                                                                          const OUString* pStrTitle,
                                                                                          bool bOnlyDbtoolsEncodings,
                                                                                          bool bImport)
{
    return VclPtr<AbstractScImportOptionsDlg_Impl>::Create(pParent, bAscii, pOptions, pStrTitle, true/*bMultiByte*/, bOnlyDbtoolsEncodings, bImport);
}

namespace
{
template<class Dialog>
class ScAbstractTabController_Impl
    : public vcl::AbstractDialogImpl_Async<SfxAbstractTabDialog, Dialog>
{
public:
    using vcl::AbstractDialogImpl_Async<SfxAbstractTabDialog, Dialog>::AbstractDialogImpl_Async;
    void SetCurPageId(const OUString& rName) override { this->m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return this->m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return this->m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { this->m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { this->m_pDlg->set_title(rStr); }

    // screenshotting
    std::vector<OUString> getAllPageUIXMLDescriptions() const override
    {
        return this->m_pDlg->getAllPageUIXMLDescriptions();
    }
    bool selectPageByUIXMLDescription(const OUString& rUIXMLDescription) override
    {
        return this->m_pDlg->selectPageByUIXMLDescription(rUIXMLDescription);
    }
    BitmapEx createScreenshot() const override { return this->m_pDlg->createScreenshot(); }
    OUString GetScreenshotId() const override { return this->m_pDlg->GetScreenshotId(); }
};
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScAttrDlg(weld::Window* pParent, const SfxItemSet* pCellAttrs)
{
    return VclPtr<ScAbstractTabController_Impl<ScAttrDlg>>::Create(pParent, pCellAttrs);
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScHFEditDlg( weld::Window*         pParent,
                                                                        const SfxItemSet&   rCoreSet,
                                                                        const OUString&     rPageStyle,
                                                                        sal_uInt16          nResId )
{
    switch (nResId)
    {
        case RID_SCDLG_HFED_HEADER:
        case RID_SCDLG_HFEDIT_HEADER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditHeaderDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFED_FOOTER:
        case RID_SCDLG_HFEDIT_FOOTER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditFooterDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_SHAREDFIRSTHEADER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditSharedFirstHeaderDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_SHAREDLEFTHEADER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditSharedLeftHeaderDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_SHAREDFIRSTFOOTER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditSharedFirstFooterDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_SHAREDLEFTFOOTER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditSharedLeftFooterDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_LEFTHEADER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditLeftHeaderDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_RIGHTHEADER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditRightHeaderDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_LEFTFOOTER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditLeftFooterDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_RIGHTFOOTER:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditRightFooterDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_SHDR:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditSharedHeaderDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_SFTR:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditSharedFooterDlg>>::Create(pParent, rCoreSet, rPageStyle);
        case RID_SCDLG_HFEDIT_ALL:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditAllDlg>>::Create(pParent, rCoreSet, rPageStyle);
        default:
        case RID_SCDLG_HFEDIT:
            return VclPtr<ScAbstractTabController_Impl<ScHFEditActiveDlg>>::Create(pParent, rCoreSet, rPageStyle);
    }
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScStyleDlg(weld::Window* pParent,
                                                                            SfxStyleSheetBase& rStyleBase,
                                                                            bool bPage)
{
    return VclPtr<ScAbstractTabController_Impl<ScStyleDlg>>::Create(pParent, rStyleBase, bPage);
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScDrawStyleDlg(weld::Window* pParent,
                                                                                SfxStyleSheetBase& rStyleBase,
                                                                                SdrView* pView)
{
    return VclPtr<ScAbstractTabController_Impl<ScDrawStyleDlg>>::Create(pParent, rStyleBase, pView);
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScSubTotalDlg(weld::Window* pParent, const SfxItemSet& rArgSet)
{
    return VclPtr<ScAbstractTabController_Impl<ScSubTotalDlg>>::Create(pParent, rArgSet);
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScCharDlg(
    weld::Window* pParent, const SfxItemSet* pAttr, const SfxObjectShell* pDocShell, bool bDrawText)
{
    return VclPtr<ScAbstractTabController_Impl<ScCharDlg>>::Create(pParent, pAttr, pDocShell, bDrawText);
}

VclPtr<SfxAbstractTabDialog> ScAbstractDialogFactory_Impl::CreateScParagraphDlg(
    weld::Window* pParent, const SfxItemSet* pAttr)
{
    return VclPtr<ScAbstractTabController_Impl<ScParagraphDlg>>::Create(pParent, pAttr);
}

namespace
{
class ScAsyncTabController_Impl : public ScAsyncTabController
{
    std::shared_ptr<SfxTabDialogController> m_pDlg;

public:
    explicit ScAsyncTabController_Impl(std::shared_ptr<SfxTabDialogController> p)
        : m_pDlg(std::move(p))
    {
    }
    bool StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx) override
    {
        return SfxTabDialogController::runAsync(m_pDlg, rCtx.maEndDialogFn);
    }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
};
}

std::shared_ptr<ScAsyncTabController> ScAbstractDialogFactory_Impl::CreateScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet)
{
    return std::make_shared<ScAsyncTabController_Impl>(std::make_shared<ScSortDlg>(pParent, pArgSet));
}

//------------------ Factories for TabPages--------------------
CreateTabPage ScAbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    switch (nId)
    {
        case SID_SC_TP_CHANGES:
            return ScRedlineOptionsTabPage::Create;
        case SID_SC_TP_CALC:
            return ScTpCalcOptions::Create;
        case SID_SC_TP_FORMULA:
            return ScTpFormulaOptions::Create;
        case SID_SC_TP_COMPATIBILITY:
            return ScTpCompatOptions::Create;
        case RID_SC_TP_DEFAULTS:
            return ScTpDefaultsOptions::Create;
        case RID_SC_TP_PRINT:
            return ScTpPrintOptions::Create;
        case SID_SC_TP_STAT:
            return ScDocStatPage::Create;
        case SID_SC_TP_USERLISTS:
             return ScTpUserLists::Create;
        case SID_SC_TP_CONTENT:
            return ScTpContentOptions::Create;
        case SID_SC_TP_LAYOUT:
            return ScTpLayoutOptions::Create;
        default:
            break;
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
