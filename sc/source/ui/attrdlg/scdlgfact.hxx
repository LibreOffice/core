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
#pragma once

#include <scabstdlg.hxx>

//AbstractDialogFactory_Impl implementations
class ScAbstractDialogFactory_Impl : public ScAbstractDialogFactory
{

public:
    virtual ~ScAbstractDialogFactory_Impl() {}

    virtual VclPtr<AbstractScImportAsciiDlg> CreateScImportAsciiDlg(weld::Window* pParent,
                                                                    const OUString& aDatName,
                                                                    SvStream* pInStream,
                                                                    ScImportAsciiCall eCall) override;

    virtual VclPtr<AbstractScTextImportOptionsDlg> CreateScTextImportOptionsDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScAutoFormatDlg> CreateScAutoFormatDlg(weld::Window* pParent,
                                                                ScAutoFormat* pAutoFormat,
                                                                const ScAutoFormatData* pSelFormatData,
                                                                ScViewData& rViewData) override;
    virtual VclPtr<AbstractScColRowLabelDlg> CreateScColRowLabelDlg (weld::Window* pParent,
                                                                bool bCol,
                                                                bool bRow) override;

    virtual VclPtr<AbstractScSortWarningDlg> CreateScSortWarningDlg(weld::Window* pParent, const OUString& rExtendText, const OUString& rCurrentText ) override;

    virtual VclPtr<AbstractScCondFormatManagerDlg> CreateScCondFormatMgrDlg(weld::Window* pParent, ScDocument& rDoc, const ScConditionalFormatList* pFormatList ) override;

    virtual VclPtr<AbstractScDataPilotDatabaseDlg> CreateScDataPilotDatabaseDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScDataPilotSourceTypeDlg> CreateScDataPilotSourceTypeDlg(weld::Window* pParent,
        bool bEnableExternal) override;

    virtual VclPtr<AbstractScDataPilotServiceDlg> CreateScDataPilotServiceDlg(weld::Window* pParent,
                                                                              const std::vector<OUString>& rServices) override;
    virtual VclPtr<AbstractScDeleteCellDlg> CreateScDeleteCellDlg(weld::Window* pParent, bool bDisallowCellMove ) override;

    //for dataform
    virtual VclPtr<AbstractScDataFormDlg> CreateScDataFormDlg(weld::Window* pParent, ScTabViewShell* pTabViewShell) override;

    virtual VclPtr<AbstractScDeleteContentsDlg> CreateScDeleteContentsDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScFillSeriesDlg> CreateScFillSeriesDlg(weld::Window*        pParent,
                                                            ScDocument&     rDocument,
                                                            FillDir         eFillDir,
                                                            FillCmd         eFillCmd,
                                                            FillDateCmd     eFillDateCmd,
                                                            const OUString& aStartStr,
                                                            double          fStep,
                                                            double          fMax,
                                                            SCSIZE          nSelectHeight,
                                                            SCSIZE          nSelectWidth,
                                                            sal_uInt16       nPossDir) override;
    virtual VclPtr<AbstractScGroupDlg> CreateAbstractScGroupDlg(weld::Window* pParent, bool bUnGroup = false) override;

    virtual VclPtr<AbstractScInsertCellDlg> CreateScInsertCellDlg(weld::Window* pParent,
                                                                  bool bDisallowCellMove) override;

    virtual VclPtr<AbstractScInsertContentsDlg> CreateScInsertContentsDlg(weld::Window* pParent,
                                                                          const OUString* pStrTitle = nullptr) override;

    virtual VclPtr<AbstractScInsertTableDlg> CreateScInsertTableDlg(weld::Window* pParent, ScViewData& rViewData,
        SCTAB nTabCount, bool bFromFile) override;

    virtual VclPtr<AbstractScSelEntryDlg> CreateScSelEntryDlg(weld::Window* pParent, const std::vector<OUString> &rEntryList) override;

    virtual VclPtr<AbstractScLinkedAreaDlg> CreateScLinkedAreaDlg(weld::Widget* pParent) override;

    virtual VclPtr<AbstractScMetricInputDlg> CreateScMetricInputDlg(weld::Window* pParent,
                                                                const OUString&  sDialogName,
                                                                tools::Long            nCurrent,
                                                                tools::Long            nDefault,
                                                                FieldUnit       eFUnit,
                                                                sal_uInt16      nDecimals,
                                                                tools::Long            nMaximum,
                                                                tools::Long            nMinimum  = 0 ) override;

    virtual VclPtr<AbstractScMoveTableDlg> CreateScMoveTableDlg(weld::Window * pParent,
        const OUString& rDefault) override;

    virtual VclPtr<AbstractScNameCreateDlg> CreateScNameCreateDlg(weld::Window * pParent,
        CreateNameFlags nFlags) override;

    virtual VclPtr<AbstractScNamePasteDlg> CreateScNamePasteDlg(weld::Window * pParent, ScDocShell* pShell) override;

    virtual VclPtr<AbstractScPivotFilterDlg> CreateScPivotFilterDlg(weld::Window* pParent, const SfxItemSet& rArgSet,
                                                                    sal_uInt16 nSourceTab) override;

    virtual VclPtr<AbstractScDPFunctionDlg> CreateScDPFunctionDlg(weld::Widget* pParent,
                                                                  const ScDPLabelDataVector& rLabelVec,
                                                                  const ScDPLabelData& rLabelData,
                                                                  const ScPivotFuncData& rFuncData ) override;

    virtual VclPtr<AbstractScDPSubtotalDlg> CreateScDPSubtotalDlg(weld::Widget* pParent,
                                                                  ScDPObject& rDPObj,
                                                                  const ScDPLabelData& rLabelData,
                                                                  const ScPivotFuncData& rFuncData,
                                                                  const ScDPNameVec& rDataFields ) override;

    virtual VclPtr<AbstractScDPNumGroupDlg> CreateScDPNumGroupDlg(weld::Window* pParent,
                                                                  const ScDPNumGroupInfo& rInfo) override;

    virtual VclPtr<AbstractScDPDateGroupDlg> CreateScDPDateGroupDlg(weld::Window* pParent,
                                                                    const ScDPNumGroupInfo& rInfo,
                                                                    sal_Int32 nDatePart,
                                                                    const Date& rNullDate) override;

    virtual VclPtr<AbstractScDPShowDetailDlg> CreateScDPShowDetailDlg(weld::Window* pParent,
                                                                ScDPObject& rDPObj,
                                                                css::sheet::DataPilotFieldOrientation nOrient) override;

    virtual VclPtr<AbstractScNewScenarioDlg> CreateScNewScenarioDlg(weld::Window* pParent, const OUString& rName,
                                                                    bool bEdit, bool bSheetProtected) override;
    virtual VclPtr<AbstractScShowTabDlg> CreateScShowTabDlg(weld::Window* pParent) override;
    virtual VclPtr<AbstractScGoToTabDlg> CreateScGoToTabDlg(weld::Window* pParent) override;

    virtual VclPtr<AbstractScStringInputDlg> CreateScStringInputDlg(weld::Window* pParent,
                                                                    const OUString& rTitle,
                                                                    const OUString& rEditTitle,
                                                                    const OUString& rDefault,
                                                                    const OUString& rHelpId,
                                                                    const OUString& rEditHelpId) override;

    virtual VclPtr<AbstractScTabBgColorDlg> CreateScTabBgColorDlg(weld::Window* pParent,
                                                                  const OUString& rTitle, //Dialog Title
                                                                  const OUString& rTabBgColorNoColorText, //Label for no tab color
                                                                  const Color& rDefaultColor) override; //Currently selected Color

    virtual VclPtr<AbstractScImportOptionsDlg> CreateScImportOptionsDlg(weld::Window* pParent, bool bAscii,
                                                                        const ScImportOptions* pOptions,
                                                                        const OUString* pStrTitle,
                                                                        bool bOnlyDbtoolsEncodings,
                                                                        bool bImport = true) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScAttrDlg(weld::Window* pParent,
                                                         const SfxItemSet* pCellAttrs) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScHFEditDlg(weld::Window*       pParent,
                                                    const SfxItemSet&   rCoreSet,
                                                    const OUString&     rPageStyle,
                                                    sal_uInt16          nResId ) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScStyleDlg(weld::Window* pParent,
                                                          SfxStyleSheetBase& rStyleBase,
                                                          bool bPage) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScDrawStyleDlg(weld::Window* pParent,
                                                              SfxStyleSheetBase& rStyleBase,
                                                              SdrView* pView) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScSubTotalDlg(weld::Window* pParent,
                                                             const SfxItemSet& rArgSet) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateScCharDlg(weld::Window* pParent,
        const SfxItemSet* pAttr, const SfxObjectShell* pDocShell, bool bDrawText) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateScParagraphDlg(weld::Window* pParent,
        const SfxItemSet* pAttr) override;

    virtual std::shared_ptr<ScAsyncTabController> CreateScSortDlg(weld::Window* pParent, const SfxItemSet* pArgSet) override;

    // For TabPage
    virtual CreateTabPage                GetTabPageCreatorFunc( sal_uInt16 nId ) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
