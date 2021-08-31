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

#include <config_options.h>

#include <vcl/dllapi.h>
#include <vcl/printer/NupOrderType.hxx>
#include <vcl/rendercontext/DrawModeFlags.hxx>

#include <com/sun/star/view/PrintableState.hpp>

namespace weld
{
class Window;
}

class GDIMetaFile;
class Printer;

namespace vcl::print
{
class ImplPrinterControllerData;
class PrinterOptions;
class PrinterController;

class VCL_DLLPUBLIC PrinterController
{
    std::unique_ptr<print::ImplPrinterControllerData> mpImplData;

protected:
    PrinterController(const VclPtr<Printer>&, weld::Window* pDialogParent);

public:
    struct MultiPageSetup
    {
        // all metrics in 100th mm
        int nRows;
        int nColumns;
        Size aPaperSize;
        tools::Long nLeftMargin;
        tools::Long nTopMargin;
        tools::Long nRightMargin;
        tools::Long nBottomMargin;
        tools::Long nHorizontalSpacing;
        tools::Long nVerticalSpacing;
        bool bDrawBorder;
        print::NupOrderType nOrder;

        MultiPageSetup()
            : nRows(1)
            , nColumns(1)
            , aPaperSize(21000, 29700)
            , nLeftMargin(0)
            , nTopMargin(0)
            , nRightMargin(0)
            , nBottomMargin(0)
            , nHorizontalSpacing(0)
            , nVerticalSpacing(0)
            , bDrawBorder(false)
            , nOrder(print::NupOrderType::LRTB)
        {
        }
    };

    struct PageSize
    {
        /// In 100th mm
        Size aSize;

        /// Full paper, not only imageable area is printed
        bool bFullPaper;

        PageSize(const Size& i_rSize = Size(21000, 29700), bool i_bFullPaper = false)
            : aSize(i_rSize)
            , bFullPaper(i_bFullPaper)
        {
        }
    };

    virtual ~PrinterController();

    const VclPtr<Printer>& getPrinter() const;
    weld::Window* getWindow() const;

    /** For implementations: get current job properties as changed by e.g. print dialog

        this gets the current set of properties initially told to Printer::PrintJob

        For convenience a second sequence will be merged in to get a combined sequence.
        In case of duplicate property names, the value of i_MergeList wins.
    */
    css::uno::Sequence<css::beans::PropertyValue>
    getJobProperties(const css::uno::Sequence<css::beans::PropertyValue>& i_rMergeList) const;

    /// Get the PropertyValue of a Property
    css::beans::PropertyValue* getValue(const OUString& i_rPropertyName);
    const css::beans::PropertyValue* getValue(const OUString& i_rPropertyName) const;

    /** Get a bool property

        in case the property is unknown or not convertible to bool, i_bFallback is returned
    */
    bool getBoolProperty(const OUString& i_rPropertyName, bool i_bFallback) const;

    /** Get an int property

        in case the property is unknown or not convertible to bool, i_nFallback is returned
    */
    sal_Int32 getIntProperty(const OUString& i_rPropertyName, sal_Int32 i_nFallback) const;

    /// Set a property value - can also be used to add another UI property
    void setValue(const OUString& i_rPropertyName, const css::uno::Any& i_rValue);
    void setValue(const css::beans::PropertyValue& i_rValue);

    /** @return The currently active UI options. These are the same that were passed to setUIOptions. */
    const css::uno::Sequence<css::beans::PropertyValue>& getUIOptions() const;

    /** Set possible UI options.

        should only be done once before passing the PrinterListener to Printer::PrintJob
    */
    void setUIOptions(const css::uno::Sequence<css::beans::PropertyValue>&);

    /// Enable/disable an option; this can be used to implement dialog logic.
    bool isUIOptionEnabled(const OUString& rPropName) const;
    bool isUIChoiceEnabled(const OUString& rPropName, sal_Int32 nChoice) const;

    /** MakeEnabled will change the property rPropName depends on to the value

        that makes rPropName enabled. If the dependency itself is also disabled,
        no action will be performed.

        @return The property name rPropName depends on or an empty string if no change was made.
    */
    OUString makeEnabled(const OUString& rPropName);

    /// App must override this
    virtual int getPageCount() const = 0;

    /** Get the page parameters

        namely the jobsetup that should be active for the page
        (describing among others the physical page size) and the "page size". In writer
        case this would probably be the same as the JobSetup since writer sets the page size
        draw/impress for example print their page on the paper set on the printer,
        possibly adjusting the page size to fit. That means the page size can be different from
        the paper size.

        App must override this

        @return Page size in 1/100th mm
    */
    virtual css::uno::Sequence<css::beans::PropertyValue> getPageParameters(int i_nPage) const = 0;
    /// App must override this
    virtual void printPage(int i_nPage) const = 0;

    /// Will be called after a possible dialog has been shown and the real printjob starts
    virtual void jobStarted();
    virtual void jobFinished(css::view::PrintableState);

    css::view::PrintableState getJobState() const;

    void abortJob();

    bool isShowDialogs() const;
    bool isDirectPrint() const;

    void dialogsParentClosing();

    // implementation details, not usable outside vcl
    // don't use outside vcl. Some of these are exported for
    // the benefit of vcl's plugins.
    // Still: DO NOT USE OUTSIDE VCL
    int getFilteredPageCount() const;
    VCL_DLLPRIVATE PageSize getPageFile(int i_inUnfilteredPage, GDIMetaFile& rMtf,
                                        bool i_bMayUseCache = false);
    PageSize getFilteredPageFile(int i_nFilteredPage, GDIMetaFile& o_rMtf,
                                 bool i_bMayUseCache = false);
    void printFilteredPage(int i_nPage);
    VCL_DLLPRIVATE void setPrinter(const VclPtr<Printer>&);
    void createProgressDialog();
    bool isProgressCanceled() const;
    VCL_DLLPRIVATE void setMultipage(const MultiPageSetup&);
    VCL_DLLPRIVATE const MultiPageSetup& getMultipage() const;
    void setLastPage(bool i_bLastPage);
    VCL_DLLPRIVATE void setReversePrint(bool i_bReverse);
    VCL_DLLPRIVATE void setPapersizeFromSetup(bool i_bPapersizeFromSetup);
    VCL_DLLPRIVATE bool getPapersizeFromSetup() const;
    VCL_DLLPRIVATE Size& getPaperSizeSetup() const;
    VCL_DLLPRIVATE void setPaperSizeFromUser(Size i_aUserSize);
    VCL_DLLPRIVATE Size& getPaperSizeFromUser() const;
    VCL_DLLPRIVATE bool isPaperSizeFromUser() const;
    void setPrinterModified(bool i_bPapersizeFromSetup);
    bool getPrinterModified() const;
    VCL_DLLPRIVATE void pushPropertiesToPrinter();
    VCL_DLLPRIVATE void resetPaperToLastConfigured();
    void setJobState(css::view::PrintableState);
    VCL_DLLPRIVATE void setupPrinter(weld::Window* i_pDlgParent);

    VCL_DLLPRIVATE int getPageCountProtected() const;
    VCL_DLLPRIVATE css::uno::Sequence<css::beans::PropertyValue>
    getPageParametersProtected(int i_nPage) const;

    VCL_DLLPRIVATE DrawModeFlags removeTransparencies(GDIMetaFile const& i_rIn,
                                                      GDIMetaFile& o_rOut);
    VCL_DLLPRIVATE void resetPrinterOptions(bool i_bFileOutput);
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
