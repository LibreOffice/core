/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * This module exists to validate the OpenCL implementation,
 * where necessary during startup; and before we load or
 * calculate using OpenCL.
 */

#include "app.hxx"

#include <config_version.h>
#include <config_folders.h>

#include <rtl/bootstrap.hxx>

#include <officecfg/Office/Calc.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/table/XCell2.hpp>
#include <com/sun/star/sheet/XCalculatable.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#if HAVE_FEATURE_OPENCL
#include <opencl/openclwrapper.hxx>
#endif
#include <opencl/OpenCLZone.hxx>

#include <osl/file.hxx>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace desktop {

#if HAVE_FEATURE_OPENCL

bool testOpenCLCompute(const Reference< XDesktop2 > &xDesktop, const OUString &rURL)
{
    bool bSuccess = false;
    css::uno::Reference< css::lang::XComponent > xComponent;

    sal_uInt64 nKernelFailures = opencl::kernelFailures;

    SAL_INFO("opencl", "Starting CL test spreadsheet");

    try {
        css::uno::Reference< css::frame::XComponentLoader > xLoader(xDesktop, css::uno::UNO_QUERY_THROW);

        css::uno::Sequence< css::beans::PropertyValue > aArgs(1);
        aArgs[0].Name = "Hidden";
        aArgs[0].Value = makeAny(true);

        xComponent.set(xLoader->loadComponentFromURL(rURL, "_blank", 0, aArgs));

        // What an unpleasant API to use.
        css::uno::Reference< css::sheet::XCalculatable > xCalculatable( xComponent, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::sheet::XSpreadsheetDocument > xSpreadDoc( xComponent, css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::container::XIndexAccess > xIndex( xSheets, css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), css::uno::UNO_QUERY_THROW);

        // So we insert our MAX call at the end on a named range.
        css::uno::Reference< css::table::XCell2 > xThresh( xSheet->getCellByPosition(1,1), css::uno::UNO_QUERY_THROW ); // B2
        double fThreshold = xThresh->getValue();

        // We need pure OCL formulae all the way through the
        // dependency chain, or we fall-back.
        xCalculatable->calculateAll();

        // So we insert our MAX call at the end on a named range.
        css::uno::Reference< css::table::XCell2 > xCell( xSheet->getCellByPosition(1,0), css::uno::UNO_QUERY_THROW );
        xCell->setFormula("=MAX(results)");
        double fResult = xCell->getValue();

        // Ensure the maximum variance is below our tolerance.
        if (fResult > fThreshold)
        {
            SAL_WARN("opencl", "OpenCL results unstable - disabling; result: "
                     << fResult << " vs. " << fThreshold);
        }
        else
        {
            SAL_INFO("opencl", "calculating smoothly; result: " << fResult);
            bSuccess = true;
        }
    }
    catch (const css::uno::Exception &e)
    {
        (void)e;
        SAL_WARN("opencl", "OpenCL testing failed - disabling: " << e.Message);
    }

    if (nKernelFailures != opencl::kernelFailures)
    {
        // tdf#100883 - defeat SEH exception handling fallbacks.
        SAL_WARN("opencl", "OpenCL kernels failed to compile, "
                 "or took SEH exceptions "
                 << nKernelFailures << " != " << opencl::kernelFailures);
        bSuccess = false;
    }

    if (!bSuccess)
        OpenCLZone::hardDisable();
    if (xComponent.is())
        xComponent->dispose();


    return bSuccess;
}

void Desktop::CheckOpenCLCompute(const Reference< XDesktop2 > &xDesktop)
{
    if (getenv("SAL_DISABLE_OPENCL") ||
        !officecfg::Office::Common::Misc::UseOpenCL::get())
        return;

    SAL_INFO("opencl", "Initiating test of OpenCL device");
    OpenCLZone aZone;

    OUString aDevice = officecfg::Office::Calc::Formula::Calculation::OpenCLDevice::get();
    OUString aSelectedCLDeviceVersionID;
    if (!opencl::switchOpenCLDevice(
            &aDevice,
            officecfg::Office::Calc::Formula::Calculation::OpenCLAutoSelect::get(),
            false /* bForceEvaluation */,
            aSelectedCLDeviceVersionID))
    {
        SAL_WARN("opencl", "Failed to initialize OpenCL for test");
        OpenCLZone::hardDisable();
        return;
    }

    // Append our app version as well.
    aSelectedCLDeviceVersionID += "--";
    aSelectedCLDeviceVersionID += LIBO_VERSION_DOTTED;

    // Append timestamp of the file.
    OUString aURL("$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/opencl/cl-test.ods");
    rtl::Bootstrap::expandMacros(aURL);

    DirectoryItem aItem;
    DirectoryItem::get( aURL, aItem );
    FileStatus aFileStatus( osl_FileStatus_Mask_ModifyTime );
    aItem.getFileStatus( aFileStatus );
    TimeValue aTimeVal = aFileStatus.getModifyTime();
    aSelectedCLDeviceVersionID += "--";
    aSelectedCLDeviceVersionID += OUString::number(aTimeVal.Seconds);

    if (aSelectedCLDeviceVersionID != officecfg::Office::Common::Misc::SelectedOpenCLDeviceIdentifier::get())
    {
        // OpenCL device changed - sanity check it and disable if bad.

        boost::optional<sal_Int32> nOrigMinimumSize = officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::get();
        { // set the minumum group size to something small for quick testing.
            std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
            officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::set(3 /* small */, xBatch);
            xBatch->commit();
        }

        bool bSucceeded = testOpenCLCompute(xDesktop, aURL);

        { // restore the minimum group size
            std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
            officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::set(nOrigMinimumSize, xBatch);
            officecfg::Office::Common::Misc::SelectedOpenCLDeviceIdentifier::set(aSelectedCLDeviceVersionID, xBatch);
            xBatch->commit();
        }

        if (!bSucceeded)
            OpenCLZone::hardDisable();
    }
}
#endif // HAVE_FEATURE_OPENCL

} // end namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
