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

#include <app.hxx>

#include <config_version.h>
#include <config_feature_opencl.h>
#include <config_folders.h>

#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>

#include <officecfg/Office/Calc.hxx>
#include <officecfg/Office/Common.hxx>

#include <comphelper/propertyvalue.hxx>
#include <svl/documentlockfile.hxx>
#include <comphelper/diagnose_ex.hxx>

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
#include <osl/process.h>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace desktop {

#if HAVE_FEATURE_OPENCL

static bool testOpenCLDriver()
{
    // A simple OpenCL test run in a separate process in order to test
    // whether the driver crashes (asserts,etc.) when trying to use OpenCL.
    SAL_INFO("opencl", "Starting CL driver test");

    OUString testerURL(u"$BRAND_BASE_DIR/" LIBO_BIN_FOLDER "/opencltest"_ustr);
    rtl::Bootstrap::expandMacros(testerURL); //TODO: detect failure

    OUString deviceName, platformName;
    openclwrapper::getOpenCLDeviceName( deviceName, platformName );
    rtl_uString* args[] = { deviceName.pData, platformName.pData };
    sal_Int32 numArgs = 2;

    oslProcess process;
    oslSecurity security = osl_getCurrentSecurity();
    oslProcessError error = osl_executeProcess(testerURL.pData, args, numArgs,
        osl_Process_SEARCHPATH | osl_Process_HIDDEN, security,
        nullptr, nullptr, 0, &process );
    osl_freeSecurityHandle( security );
    if( error != osl_Process_E_None )
    {
        SAL_WARN( "opencl", "failed to start CL driver test: " << error );
        return false;
    }
    // If the driver takes more than 10 seconds, it's probably broken/useless.
    TimeValue timeout( 10, 0 );
    error = osl_joinProcessWithTimeout( process, &timeout );
    if( error == osl_Process_E_None )
    {
        oslProcessInfo info;
        info.Size = sizeof( info );
        error = osl_getProcessInfo( process, osl_Process_EXITCODE, &info );
        if( error == osl_Process_E_None )
        {
            if( info.Code == 0 )
            {
                SAL_INFO( "opencl", "CL driver test passed" );
                osl_freeProcessHandle( process );
                return true;
            }
            else
            {
                SAL_WARN( "opencl", "CL driver test failed - disabling: " << info.Code );
                osl_freeProcessHandle( process );
                return false;
            }
        }
    }
    SAL_WARN( "opencl", "CL driver test did not finish - disabling: " << error );
    osl_terminateProcess( process );
    osl_freeProcessHandle( process );
    return false;
}

static bool testOpenCLCompute(const Reference< XDesktop2 > &xDesktop, const OUString &rURL)
{
    bool bSuccess = false;
    css::uno::Reference< css::lang::XComponent > xComponent;

    sal_uInt64 nKernelFailures = openclwrapper::kernelFailures;

    SAL_INFO("opencl", "Starting CL test spreadsheet");

    // A stale lock file would make the loading fail, so make sure to remove it.
    try {
        ::svt::DocumentLockFile lockFile( rURL );
        lockFile.RemoveFileDirectly();
    }
    catch (const css::uno::Exception&)
    {
    }

    try {
        css::uno::Reference< css::frame::XComponentLoader > xLoader(xDesktop, css::uno::UNO_QUERY_THROW);

        css::uno::Sequence< css::beans::PropertyValue > aArgs{ comphelper::makePropertyValue(u"Hidden"_ustr,
                                                                                             true) };

        xComponent.set(xLoader->loadComponentFromURL(rURL, u"_blank"_ustr, 0, aArgs));

        // What an unpleasant API to use.
        css::uno::Reference< css::sheet::XCalculatable > xCalculatable( xComponent, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::sheet::XSpreadsheetDocument > xSpreadDoc( xComponent, css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), css::uno::UNO_SET_THROW );
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
        xCell->setFormula(u"=MAX(results)"_ustr);
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
    catch (const css::uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("opencl", "OpenCL testing failed - disabling");
    }

    if (nKernelFailures != openclwrapper::kernelFailures)
    {
        // tdf#100883 - defeat SEH exception handling fallbacks.
        SAL_WARN("opencl", "OpenCL kernels failed to compile, "
                 "or took SEH exceptions "
                 << nKernelFailures << " != " << openclwrapper::kernelFailures);
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
    if (!openclwrapper::canUseOpenCL() || Application::IsSafeModeEnabled())
        return;

    SAL_INFO("opencl", "Initiating test of OpenCL device");
    OpenCLZone aZone;
    OpenCLInitialZone aInitialZone;

    OUString aDevice = officecfg::Office::Calc::Formula::Calculation::OpenCLDevice::get();
    OUString aSelectedCLDeviceVersionID;
    if (!openclwrapper::switchOpenCLDevice(
            aDevice,
            officecfg::Office::Calc::Formula::Calculation::OpenCLAutoSelect::get(),
            false /* bForceEvaluation */,
            aSelectedCLDeviceVersionID))
    {
        SAL_WARN("opencl", "Failed to initialize OpenCL for test");
        OpenCLZone::hardDisable();
        return;
    }

    // Append our app version as well.
    aSelectedCLDeviceVersionID += "--" LIBO_VERSION_DOTTED;

    // Append timestamp of the file.
    OUString aURL(u"$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/opencl/cl-test.ods"_ustr);
    rtl::Bootstrap::expandMacros(aURL);

    DirectoryItem aItem;
    (void)DirectoryItem::get( aURL, aItem );
    FileStatus aFileStatus( osl_FileStatus_Mask_ModifyTime );
    (void)aItem.getFileStatus( aFileStatus );
    TimeValue aTimeVal = aFileStatus.getModifyTime();
    aSelectedCLDeviceVersionID += "--" +
        OUString::number(aTimeVal.Seconds);

    if (aSelectedCLDeviceVersionID == officecfg::Office::Common::Misc::SelectedOpenCLDeviceIdentifier::get())
        return;

    // OpenCL device changed - sanity check it and disable if bad.

    sal_Int32 nOrigMinimumSize = officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::get();
    { // set the minimum group size to something small for quick testing.
        std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::set(3 /* small */, xBatch);
        xBatch->commit();
    }

    // Hopefully at least basic functionality always works and broken OpenCL implementations break
    // only when they are used to compute something. If this assumptions turns out to be not true,
    // the driver check needs to be moved sooner.
    bool bSucceeded = testOpenCLDriver() && testOpenCLCompute(xDesktop, aURL);

    { // restore the minimum group size
        std::shared_ptr<comphelper::ConfigurationChanges> xBatch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Calc::Formula::Calculation::OpenCLMinimumDataSize::set(nOrigMinimumSize, xBatch);
        officecfg::Office::Common::Misc::SelectedOpenCLDeviceIdentifier::set(aSelectedCLDeviceVersionID, xBatch);
        xBatch->commit();
    }

    if (!bSucceeded)
        OpenCLZone::hardDisable();
}
#endif // HAVE_FEATURE_OPENCL

} // end namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
