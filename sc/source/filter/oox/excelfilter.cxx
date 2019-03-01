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

#include <excelfilter.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <excelvbaproject.hxx>
#include <stylesbuffer.hxx>
#include <themebuffer.hxx>
#include <workbookfragment.hxx>
#include <xestream.hxx>

#include <addressconverter.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <scerrors.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>

namespace oox {
namespace xls {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

using ::oox::drawingml::table::TableStyleListPtr;

ExcelFilter::ExcelFilter( const Reference< XComponentContext >& rxContext ) :
    XmlFilterBase( rxContext ),
    mpBookGlob( nullptr )
{
}

ExcelFilter::~ExcelFilter()
{
    OSL_ENSURE( !mpBookGlob, "ExcelFilter::~ExcelFilter - workbook data not cleared" );
}

void ExcelFilter::registerWorkbookGlobals( WorkbookGlobals& rBookGlob )
{
    mpBookGlob = &rBookGlob;
}

WorkbookGlobals& ExcelFilter::getWorkbookGlobals() const
{
    OSL_ENSURE( mpBookGlob, "ExcelFilter::getWorkbookGlobals - missing workbook data" );
    return *mpBookGlob;
}

void ExcelFilter::unregisterWorkbookGlobals()
{
    mpBookGlob = nullptr;
}

bool ExcelFilter::importDocument()
{
    /*  To activate the XLSX/XLSB dumper, insert the full path to the file
        file:///<path-to-oox-module>/source/dump/xlsbdumper.ini
        into the environment variable OOO_XLSBDUMPER and start the office with
        this variable (nonpro only). */
    //OOX_DUMP_FILE( ::oox::dump::xlsb::Dumper );

    OUString aWorkbookPath = getFragmentPathFromFirstTypeFromOfficeDoc( "officeDocument" );
    if( aWorkbookPath.isEmpty() )
        return false;

    try
    {
        try
        {
            importDocumentProperties();
        }
        catch( const Exception& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("sc", "exception when importing document properties " << exceptionToString(ex));
        }
        catch( ... )
        {
            SAL_WARN("sc", "exception when importing document properties");
        }
        /*  Construct the WorkbookGlobals object referred to by every instance of
            the class WorkbookHelper, and execute the import filter by constructing
            an instance of WorkbookFragment and loading the file. */
        WorkbookGlobalsRef xBookGlob(WorkbookHelper::constructGlobals(*this));
        if (xBookGlob.get())
        {
            rtl::Reference<FragmentHandler> xWorkbookFragment( new WorkbookFragment(*xBookGlob, aWorkbookPath));
            bool bRet = importFragment( xWorkbookFragment);
            if (bRet)
            {
                const WorkbookFragment* pWF = static_cast<const WorkbookFragment*>(xWorkbookFragment.get());
                const AddressConverter& rAC = pWF->getAddressConverter();
                if (rAC.isTabOverflow() || rAC.isColOverflow() || rAC.isRowOverflow())
                {
                    const ScDocument& rDoc = pWF->getScDocument();
                    if (rDoc.IsUserInteractionEnabled())
                    {
                        // Show data loss warning.

                        INetURLObject aURL( getFileUrl());
                        SfxErrorContext aContext( ERRCTX_SFX_OPENDOC,
                                aURL.getName( INetURLObject::LAST_SEGMENT, true,
                                    INetURLObject::DecodeMechanism::WithCharset),
                                nullptr, RID_ERRCTX);

                        OUString aWarning;
                        aContext.GetString( ERRCODE_NONE.MakeWarning(), aWarning);
                        aWarning += ":\n";

                        OUString aMsg;
                        if (rAC.isTabOverflow())
                        {
                            if (ErrorHandler::GetErrorString( SCWARN_IMPORT_SHEET_OVERFLOW, aMsg))
                                aWarning += aMsg;
                        }
                        if (rAC.isColOverflow())
                        {
                            if (!aMsg.isEmpty())
                                aWarning += "\n";
                            if (ErrorHandler::GetErrorString( SCWARN_IMPORT_COLUMN_OVERFLOW, aMsg))
                                aWarning += aMsg;
                        }
                        if (rAC.isRowOverflow())
                        {
                            if (!aMsg.isEmpty())
                                aWarning += "\n";
                            if (ErrorHandler::GetErrorString( SCWARN_IMPORT_ROW_OVERFLOW, aMsg))
                                aWarning += aMsg;
                        }

                        /* XXX displaying a dialog here is ugly and should
                         * rather happen at UI level instead of at the filter
                         * level, but it seems there's no way to transport
                         * detailed information other than returning true or
                         * false at this point? */

                        vcl::Window* pWin = ScDocShell::GetActiveDialogParent();
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                   aWarning));
                        xWarn->run();
                    }
                }
            }
            return bRet;
        }
    }
    catch (...)
    {
    }

    return false;
}

bool ExcelFilter::exportDocument() throw()
{
    return false;
}

const ::oox::drawingml::Theme* ExcelFilter::getCurrentTheme() const
{
    return &WorkbookHelper( getWorkbookGlobals() ).getTheme();
}

::oox::vml::Drawing* ExcelFilter::getVmlDrawing()
{
    return nullptr;
}

const TableStyleListPtr ExcelFilter::getTableStyles()
{
    return TableStyleListPtr();
}

::oox::drawingml::chart::ChartConverter* ExcelFilter::getChartConverter()
{
    return WorkbookHelper( getWorkbookGlobals() ).getChartConverter();
}

void ExcelFilter::useInternalChartDataTable( bool bInternal )
{
    return WorkbookHelper( getWorkbookGlobals() ).useInternalChartDataTable( bInternal );
}

GraphicHelper* ExcelFilter::implCreateGraphicHelper() const
{
    return new ExcelGraphicHelper( getWorkbookGlobals() );
}

::oox::ole::VbaProject* ExcelFilter::implCreateVbaProject() const
{
    return new ExcelVbaProject( getComponentContext(), Reference< XSpreadsheetDocument >( getModel(), UNO_QUERY ) );
}

sal_Bool SAL_CALL ExcelFilter::filter( const css::uno::Sequence< css::beans::PropertyValue >& rDescriptor )
{
    if ( XmlFilterBase::filter( rDescriptor ) )
        return true;

    if ( isExportFilter() )
    {
        bool bExportVBA = exportVBA();
        Reference< XExporter > xExporter(
            new XclExpXmlStream( getComponentContext(), bExportVBA, isExportTemplate() ) );

        Reference< XComponent > xDocument( getModel(), UNO_QUERY );
        Reference< XFilter > xFilter( xExporter, UNO_QUERY );

        if ( xFilter.is() )
        {
            xExporter->setSourceDocument( xDocument );
            if ( xFilter->filter( rDescriptor ) )
                return true;
        }
    }

    return false;
}

OUString ExcelFilter::getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.xls.ExcelFilter" );
}

} // namespace xls
} // namespace oox


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_oox_xls_ExcelFilter_get_implementation(css::uno::XComponentContext* context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new oox::xls::ExcelFilter(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
