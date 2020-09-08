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

#include <svtools/printoptions.hxx>
#include <vcl/print.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <officecfg/Office/Common.hxx>

#include "itemholder2.hxx"

#include <sal/macros.h>
#include <tools/diagnose_ex.h>

const sal_uInt16 aDPIArray[] = { 72, 96, 150, 200, 300, 600 };

#define DPI_COUNT (SAL_N_ELEMENTS(aDPIArray))

#define ROOTNODE_START                                  "Office.Common/Print/Option"
#define ROOTNODE_PRINTOPTION                            "org.openoffice.Office.Common/Print/Option"

#define PROPERTYNAME_REDUCETRANSPARENCY                 "ReduceTransparency"
#define PROPERTYNAME_REDUCEDTRANSPARENCYMODE            "ReducedTransparencyMode"
#define PROPERTYNAME_REDUCEGRADIENTS                    "ReduceGradients"
#define PROPERTYNAME_REDUCEDGRADIENTMODE                "ReducedGradientMode"
#define PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT           "ReducedGradientStepCount"
#define PROPERTYNAME_REDUCEBITMAPS                      "ReduceBitmaps"
#define PROPERTYNAME_REDUCEDBITMAPMODE                  "ReducedBitmapMode"
#define PROPERTYNAME_REDUCEDBITMAPRESOLUTION            "ReducedBitmapResolution"
#define PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY  "ReducedBitmapIncludesTransparency"
#define PROPERTYNAME_CONVERTTOGREYSCALES                "ConvertToGreyscales"
#define PROPERTYNAME_PDFASSTANDARDPRINTJOBFORMAT        "PDFAsStandardPrintJobFormat"

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

SvtPrintOptions_Impl*   SvtPrinterOptions::m_pStaticDataContainer = nullptr;
sal_Int32               SvtPrinterOptions::m_nRefCount = 0;

SvtPrintOptions_Impl*   SvtPrintFileOptions::m_pStaticDataContainer = nullptr;
sal_Int32               SvtPrintFileOptions::m_nRefCount = 0;

class SvtPrintOptions_Impl
{
public:
    explicit SvtPrintOptions_Impl( const OUString& rConfigRoot );
    ~SvtPrintOptions_Impl();

    void        SetReduceTransparency( bool bState ) ;
    void        SetReducedTransparencyMode( sal_Int16 nMode ) ;
    void        SetReduceGradients( bool bState ) ;
    void        SetReducedGradientMode( sal_Int16 nMode ) ;
    void        SetReducedGradientStepCount( sal_Int16 nStepCount ) ;
    void        SetReduceBitmaps( bool bState ) ;
    void        SetReducedBitmapMode( sal_Int16 nMode ) ;
    void        SetReducedBitmapResolution( sal_Int16 nResolution ) ;
    void        SetReducedBitmapIncludesTransparency( bool bState ) ;
    void        SetConvertToGreyscales( bool bState ) ;
    void        SetPDFAsStandardPrintJobFormat( bool bState ) ;


//  private API


private:
    void impl_setValue (const OUString& sProp, bool bNew );
    void impl_setValue (const OUString& sProp, sal_Int16 nNew );


//  private member


private:
       css::uno::Reference< css::container::XNameAccess > m_xCfg;
    css::uno::Reference< css::container::XNameAccess > m_xNode;
};

SvtPrintOptions_Impl::SvtPrintOptions_Impl(const OUString& rConfigRoot)
{
    try
    {
        m_xCfg.set(
            ::comphelper::ConfigurationHelper::openConfig(
                comphelper::getProcessComponentContext(),
                ROOTNODE_PRINTOPTION,
                ::comphelper::EConfigurationModes::Standard),
            css::uno::UNO_QUERY);

        if (m_xCfg.is())
        {
            m_xCfg->getByName(rConfigRoot.copy(rConfigRoot.lastIndexOf('/')+1)) >>= m_xNode;
        }
    }
    catch (const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svtools.config");
        m_xNode.clear();
        m_xCfg.clear();
    }
}

void SvtPrintOptions_Impl::SetReduceTransparency(bool bState)
{
    impl_setValue(PROPERTYNAME_REDUCETRANSPARENCY, bState);
}

void SvtPrintOptions_Impl::SetReducedTransparencyMode(sal_Int16 nMode)
{
    impl_setValue(PROPERTYNAME_REDUCEDTRANSPARENCYMODE, nMode);
}

void SvtPrintOptions_Impl::SetReduceGradients(bool bState)
{
    impl_setValue(PROPERTYNAME_REDUCEGRADIENTS, bState);
}

void SvtPrintOptions_Impl::SetReducedGradientMode(sal_Int16 nMode)
{
    impl_setValue(PROPERTYNAME_REDUCEDGRADIENTMODE, nMode);
}

void SvtPrintOptions_Impl::SetReducedGradientStepCount(sal_Int16 nStepCount )
{
    impl_setValue(PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT, nStepCount);
}

void SvtPrintOptions_Impl::SetReduceBitmaps(bool bState )
{
    impl_setValue(PROPERTYNAME_REDUCEBITMAPS, bState);
}

void SvtPrintOptions_Impl::SetReducedBitmapMode(sal_Int16 nMode )
{
    impl_setValue(PROPERTYNAME_REDUCEDBITMAPMODE, nMode);
}

void SvtPrintOptions_Impl::SetReducedBitmapResolution(sal_Int16 nResolution )
{
    impl_setValue(PROPERTYNAME_REDUCEDBITMAPRESOLUTION, nResolution);
}

void SvtPrintOptions_Impl::SetReducedBitmapIncludesTransparency(bool bState )
{
    impl_setValue(PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY, bState);
}

void SvtPrintOptions_Impl::SetConvertToGreyscales(bool bState)
{
    impl_setValue(PROPERTYNAME_CONVERTTOGREYSCALES, bState);
}

void SvtPrintOptions_Impl::SetPDFAsStandardPrintJobFormat(bool bState)
{
    impl_setValue(PROPERTYNAME_PDFASSTANDARDPRINTJOBFORMAT, bState);
}

SvtPrintOptions_Impl::~SvtPrintOptions_Impl()
{
    m_xNode.clear();
    m_xCfg.clear();
}

void SvtPrintOptions_Impl::impl_setValue (const OUString& sProp, bool bNew )
{
    try
    {
        if ( ! m_xNode.is())
            return;

        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if ( ! xSet.is())
            return;

        bool bOld = ! bNew;
        if ( ! (xSet->getPropertyValue(sProp) >>= bOld))
            return;

        if (bOld != bNew)
        {
            xSet->setPropertyValue(sProp, css::uno::makeAny(bNew));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svtools.config");
    }
}

void SvtPrintOptions_Impl::impl_setValue (const OUString& sProp,
                                                ::sal_Int16      nNew )
{
    try
    {
        if ( ! m_xNode.is())
            return;

        css::uno::Reference<css::beans::XPropertySet> xSet(m_xNode, css::uno::UNO_QUERY);
        if ( ! xSet.is())
            return;

        ::sal_Int16 nOld = nNew+1;
        if ( ! (xSet->getPropertyValue(sProp) >>= nOld))
            return;

        if (nOld != nNew)
        {
            xSet->setPropertyValue(sProp, css::uno::makeAny(nNew));
            ::comphelper::ConfigurationHelper::flush(m_xCfg);
        }
    }
    catch(const css::uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svtools.config");
    }
}

SvtBasePrintOptions::SvtBasePrintOptions()
    : m_pDataContainer(nullptr)
{
}

SvtBasePrintOptions::~SvtBasePrintOptions()
{
}

Mutex& SvtBasePrintOptions::GetOwnStaticMutex()
{
    static Mutex ourMutex;

    return ourMutex;
}

bool SvtBasePrintOptions::IsReduceTransparency()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReduceTransparency::get();
}

sal_Int16 SvtBasePrintOptions::GetReducedTransparencyMode()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReducedTransparencyMode::get();
}

bool SvtBasePrintOptions::IsReduceGradients()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReduceGradients::get();
}

sal_Int16 SvtBasePrintOptions::GetReducedGradientMode()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReducedGradientMode::get();
}

sal_Int16 SvtBasePrintOptions::GetReducedGradientStepCount()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReducedGradientStepCount::get();
}

bool SvtBasePrintOptions::IsReduceBitmaps()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReduceBitmaps::get();
}

sal_Int16 SvtBasePrintOptions::GetReducedBitmapMode()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReducedBitmapMode::get();
}

sal_Int16 SvtBasePrintOptions::GetReducedBitmapResolution()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReducedBitmapResolution::get();
}

bool SvtBasePrintOptions::IsReducedBitmapIncludesTransparency()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ReducedBitmapIncludesTransparency::get();
}

bool SvtBasePrintOptions::IsConvertToGreyscales()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::ConvertToGreyscales::get();
}

bool SvtBasePrintOptions::IsPDFAsStandardPrintJobFormat()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return officecfg::Office::Common::Print::Option::Printer::PDFAsStandardPrintJobFormat::get();
}

void SvtBasePrintOptions::SetReduceTransparency( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReduceTransparency( bState ) ;
}

void SvtBasePrintOptions::SetReducedTransparencyMode( sal_Int16 nMode )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedTransparencyMode( nMode );
}

void SvtBasePrintOptions::SetReduceGradients( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReduceGradients( bState );
}

void SvtBasePrintOptions::SetReducedGradientMode( sal_Int16 nMode )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedGradientMode( nMode );
}

void SvtBasePrintOptions::SetReducedGradientStepCount( sal_Int16 nStepCount )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedGradientStepCount( nStepCount );
}

void SvtBasePrintOptions::SetReduceBitmaps( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReduceBitmaps( bState );
}

void SvtBasePrintOptions::SetReducedBitmapMode( sal_Int16 nMode )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedBitmapMode( nMode );
}

void SvtBasePrintOptions::SetReducedBitmapResolution( sal_Int16 nResolution )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedBitmapResolution( nResolution );
}

void SvtBasePrintOptions::SetReducedBitmapIncludesTransparency( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetReducedBitmapIncludesTransparency( bState );
}

void SvtBasePrintOptions::SetConvertToGreyscales( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetConvertToGreyscales( bState );
}

void SvtBasePrintOptions::SetPDFAsStandardPrintJobFormat( bool bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetPDFAsStandardPrintJobFormat( bState );
}

void SvtBasePrintOptions::GetPrinterOptions( PrinterOptions& rOptions )
{
    rOptions.SetReduceTransparency( IsReduceTransparency() );
    rOptions.SetReducedTransparencyMode( static_cast<PrinterTransparencyMode>(GetReducedTransparencyMode()) );
    rOptions.SetReduceGradients( IsReduceGradients() );
    rOptions.SetReducedGradientMode( static_cast<PrinterGradientMode>(GetReducedGradientMode()) );
    rOptions.SetReducedGradientStepCount( GetReducedGradientStepCount() );
    rOptions.SetReduceBitmaps( IsReduceBitmaps() );
    rOptions.SetReducedBitmapMode( static_cast<PrinterBitmapMode>(GetReducedBitmapMode()) );
    rOptions.SetReducedBitmapResolution( aDPIArray[ std::min( static_cast<sal_uInt16>(GetReducedBitmapResolution()), sal_uInt16( DPI_COUNT - 1 ) ) ] );
    rOptions.SetReducedBitmapIncludesTransparency( IsReducedBitmapIncludesTransparency() );
    rOptions.SetConvertToGreyscales( IsConvertToGreyscales() );
    rOptions.SetPDFAsStandardPrintJobFormat( IsPDFAsStandardPrintJobFormat() );
}

void SvtBasePrintOptions::SetPrinterOptions( const PrinterOptions& rOptions )
{
    SetReduceTransparency( rOptions.IsReduceTransparency() );
    SetReducedTransparencyMode(
        sal::static_int_cast< sal_Int16 >(
            rOptions.GetReducedTransparencyMode()) );
    SetReduceGradients( rOptions.IsReduceGradients() );
    SetReducedGradientMode(
        sal::static_int_cast< sal_Int16 >(rOptions.GetReducedGradientMode()) );
    SetReducedGradientStepCount( rOptions.GetReducedGradientStepCount() );
    SetReduceBitmaps( rOptions.IsReduceBitmaps() );
    SetReducedBitmapMode(
        sal::static_int_cast< sal_Int16 >(rOptions.GetReducedBitmapMode()) );
    SetReducedBitmapIncludesTransparency( rOptions.IsReducedBitmapIncludesTransparency() );
    SetConvertToGreyscales( rOptions.IsConvertToGreyscales() );
    SetPDFAsStandardPrintJobFormat( rOptions.IsPDFAsStandardPrintJobFormat() );

    const sal_uInt16 nDPI = rOptions.GetReducedBitmapResolution();

    if( nDPI < aDPIArray[ 0 ] )
        SetReducedBitmapResolution( 0 );
    else
    {
        for( long i = DPI_COUNT - 1; i >= 0; i-- )
        {
            if( nDPI >= aDPIArray[ i ] )
            {
                SetReducedBitmapResolution( static_cast<sal_Int16>(i) );
                i = -1;
            }
        }
    }
}

SvtPrinterOptions::SvtPrinterOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already!
    if( m_pStaticDataContainer == nullptr )
    {
        m_pStaticDataContainer = new SvtPrintOptions_Impl( ROOTNODE_START "/Printer" );
        svtools::ItemHolder2::holdConfigItem(EItem::PrintOptions);
    }

    SetDataContainer( m_pStaticDataContainer );
}

SvtPrinterOptions::~SvtPrinterOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease our refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy our static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pStaticDataContainer;
        m_pStaticDataContainer = nullptr;
    }
}

SvtPrintFileOptions::SvtPrintFileOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already!
    if( m_pStaticDataContainer == nullptr )
    {
        m_pStaticDataContainer = new SvtPrintOptions_Impl( ROOTNODE_START "/File" );
        svtools::ItemHolder2::holdConfigItem(EItem::PrintFileOptions);
    }

    SetDataContainer( m_pStaticDataContainer );
}

SvtPrintFileOptions::~SvtPrintFileOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease our refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy our static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pStaticDataContainer;
        m_pStaticDataContainer = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
