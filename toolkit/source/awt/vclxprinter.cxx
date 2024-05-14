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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <awt/vclxprinter.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/string.hxx>

#include <vcl/print.hxx>
#include <vcl/jobset.hxx>
#include <vcl/oldprintadaptor.hxx>
#include <vcl/svapp.hxx>

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <o3tl/string_view.hxx>

#include <toolkit/awt/vclxdevice.hxx>


#define BINARYSETUPMARKER       0x23864691

#define PROPERTY_Orientation    0
#define PROPERTY_Horizontal     1

//    ----------------------------------------------------
//    class VCLXPrinterPropertySet
//    ----------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXPrinterPropertySet, VCLXPrinterPropertySet_Base, OPropertySetHelper )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXPrinterPropertySet, VCLXPrinterPropertySet_Base, ::cppu::OPropertySetHelper )

VCLXPrinterPropertySet::VCLXPrinterPropertySet( const OUString& rPrinterName )
    : OPropertySetHelper( m_aBHelper )
    , mxPrinter(VclPtrInstance< Printer >(rPrinterName))
{
    SolarMutexGuard aSolarGuard;

    mnOrientation = 0;
    mbHorizontal = false;
}

VCLXPrinterPropertySet::~VCLXPrinterPropertySet()
{
    SolarMutexGuard aSolarGuard;
    mxPrinter.reset();
}

css::uno::Reference< css::awt::XDevice > const &  VCLXPrinterPropertySet::GetDevice()
{
    if ( !mxPrnDevice.is() )
    {
        rtl::Reference<VCLXDevice> pDev = new VCLXDevice;
        pDev->SetOutputDevice( GetPrinter() );
        mxPrnDevice = pDev;
    }
    return mxPrnDevice;
}

css::uno::Reference< css::beans::XPropertySetInfo > VCLXPrinterPropertySet::getPropertySetInfo(  )
{
    static css::uno::Reference< css::beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& VCLXPrinterPropertySet::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper s_PropertyArrayHelper(
            css::uno::Sequence<css::beans::Property>{
                    css::beans::Property( u"Orientation"_ustr, PROPERTY_Orientation, cppu::UnoType<sal_Int16>::get(), 0 ),
                    css::beans::Property( u"Horizontal"_ustr, PROPERTY_Horizontal, cppu::UnoType<bool>::get(), 0 )},
            false);

    return s_PropertyArrayHelper;
}

sal_Bool VCLXPrinterPropertySet::convertFastPropertyValue( css::uno::Any & rConvertedValue, css::uno::Any & rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bDifferent = false;
    switch ( nHandle )
    {
        case PROPERTY_Orientation:
        {
            sal_Int16 n;
            if( ( rValue >>= n ) && ( n != mnOrientation ) )
            {
                rConvertedValue <<= n;
                rOldValue <<= mnOrientation;
                bDifferent = true;
            }
        }
        break;
        case PROPERTY_Horizontal:
        {
            bool b;
            if( ( rValue >>= b ) && ( b != mbHorizontal ) )
            {
                rConvertedValue <<= b;
                rOldValue <<= mbHorizontal;
                bDifferent = true;
            }
        }
        break;
        default:
        {
            OSL_FAIL( "VCLXPrinterPropertySet_Impl::convertFastPropertyValue - invalid Handle" );
        }
    }
    return bDifferent;
}

void VCLXPrinterPropertySet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    switch( nHandle )
    {
        case PROPERTY_Orientation:
        {
            rValue >>= mnOrientation;
        }
        break;
        case PROPERTY_Horizontal:
        {
            rValue >>= mbHorizontal;
        }
        break;
        default:
        {
            OSL_FAIL( "VCLXPrinterPropertySet_Impl::convertFastPropertyValue - invalid Handle" );
        }
    }
}

void VCLXPrinterPropertySet::getFastPropertyValue( css::uno::Any& rValue, sal_Int32 nHandle ) const
{
    ::osl::MutexGuard aGuard( const_cast<VCLXPrinterPropertySet*>(this)->m_aMutex );

    switch( nHandle )
    {
        case PROPERTY_Orientation:
            rValue <<= mnOrientation;
        break;
        case PROPERTY_Horizontal:
            rValue <<= mbHorizontal;
        break;
        default:
        {
            OSL_FAIL( "VCLXPrinterPropertySet_Impl::convertFastPropertyValue - invalid Handle" );
        }
    }
}

// css::awt::XPrinterPropertySet
void VCLXPrinterPropertySet::setHorizontal( sal_Bool bHorizontal )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    css::uno::Any aValue;
    aValue <<= bHorizontal;
    setFastPropertyValue( PROPERTY_Horizontal, aValue );
}

css::uno::Sequence< OUString > VCLXPrinterPropertySet::getFormDescriptions(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    const sal_uInt16 nPaperBinCount = GetPrinter()->GetPaperBinCount();
    css::uno::Sequence< OUString > aDescriptions( nPaperBinCount );
    for ( sal_uInt16 n = 0; n < nPaperBinCount; n++ )
    {
        // Format: <DisplayFormName;FormNameId;DisplayPaperBinName;PaperBinNameId;DisplayPaperName;PaperNameId>
        aDescriptions.getArray()[n] = "*;*;" + GetPrinter()->GetPaperBinName( n ) + ";" +
                                      OUString::number(n) + ";*;*";
    }
    return aDescriptions;
}

void VCLXPrinterPropertySet::selectForm( const OUString& rFormDescription )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_uInt16 nPaperBin = sal::static_int_cast< sal_uInt16 >(
        o3tl::toInt32(o3tl::getToken(rFormDescription, 3, ';' )));
    GetPrinter()->SetPaperBin( nPaperBin );
}

css::uno::Sequence< sal_Int8 > VCLXPrinterPropertySet::getBinarySetup(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvMemoryStream aMem;
    aMem.WriteUInt32( BINARYSETUPMARKER );
    WriteJobSetup( aMem, GetPrinter()->GetJobSetup() );
    return css::uno::Sequence<sal_Int8>( static_cast<sal_Int8 const *>(aMem.GetData()), aMem.Tell() );
}

void VCLXPrinterPropertySet::setBinarySetup( const css::uno::Sequence< sal_Int8 >& data )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    SvMemoryStream aMem( const_cast<signed char*>(data.getConstArray()), data.getLength(), StreamMode::READ );
    sal_uInt32 nMarker;
    aMem.ReadUInt32( nMarker );
    DBG_ASSERT( nMarker == BINARYSETUPMARKER, "setBinarySetup - invalid!" );
    if ( nMarker == BINARYSETUPMARKER )
    {
        JobSetup aSetup;
        ReadJobSetup( aMem, aSetup );
        GetPrinter()->SetJobSetup( aSetup );
    }
}


//    ----------------------------------------------------
//    class VCLXPrinter
//    ----------------------------------------------------
VCLXPrinter::VCLXPrinter( const OUString& rPrinterName )
    : VCLXPrinter_Base( rPrinterName )
{
}

VCLXPrinter::~VCLXPrinter()
{
}

sal_Bool VCLXPrinter::start( const OUString& /*rJobName*/, sal_Int16 /*nCopies*/, sal_Bool /*bCollate*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mxPrinter)
    {
        maInitJobSetup = mxPrinter->GetJobSetup();
        mxListener = std::make_shared<vcl::OldStylePrintAdaptor>(mxPrinter, nullptr);
    }

    return true;
}

void VCLXPrinter::end(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mxListener)
    {
        Printer::PrintJob(mxListener, maInitJobSetup);
        mxListener.reset();
    }
}

void VCLXPrinter::terminate(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    mxListener.reset();
}

css::uno::Reference< css::awt::XDevice > VCLXPrinter::startPage(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mxListener)
    {
        mxListener->StartPage();
    }
    return GetDevice();
}

void VCLXPrinter::endPage(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (mxListener)
    {
        mxListener->EndPage();
    }
}


//    ----------------------------------------------------
//    class VCLXInfoPrinter
//    ----------------------------------------------------

VCLXInfoPrinter::VCLXInfoPrinter( const OUString& rPrinterName )
    : VCLXInfoPrinter_Base( rPrinterName )
{
}

VCLXInfoPrinter::~VCLXInfoPrinter()
{
}

// css::awt::XInfoPrinter
css::uno::Reference< css::awt::XDevice > VCLXInfoPrinter::createDevice(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return GetDevice();
}

//    ----------------------------------------------------
//    class VCLXPrinterServer
//    ----------------------------------------------------

// css::awt::XPrinterServer2
css::uno::Sequence< OUString > VCLXPrinterServer::getPrinterNames(  )
{
    const std::vector<OUString>& rQueues = Printer::GetPrinterQueues();
    sal_uInt32 nPrinters = rQueues.size();

    css::uno::Sequence< OUString > aNames( nPrinters );
    for ( sal_uInt32 n = 0; n < nPrinters; n++ )
        aNames.getArray()[n] = rQueues[n];

    return aNames;
}

OUString VCLXPrinterServer::getDefaultPrinterName()
{
    return Printer::GetDefaultPrinterName();
}

css::uno::Reference< css::awt::XPrinter > VCLXPrinterServer::createPrinter( const OUString& rPrinterName )
{
    css::uno::Reference< css::awt::XPrinter > xP = new VCLXPrinter( rPrinterName );
    return xP;
}

css::uno::Reference< css::awt::XInfoPrinter > VCLXPrinterServer::createInfoPrinter( const OUString& rPrinterName )
{
    css::uno::Reference< css::awt::XInfoPrinter > xP = new VCLXInfoPrinter( rPrinterName );
    return xP;
}

OUString VCLXPrinterServer::getImplementationName()
{
    return u"stardiv.Toolkit.VCLXPrinterServer"_ustr;
}

sal_Bool VCLXPrinterServer::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> VCLXPrinterServer::getSupportedServiceNames()
{
    return css::uno::Sequence<OUString>{
        u"com.sun.star.awt.PrinterServer"_ustr, u"stardiv.vcl.PrinterServer"_ustr};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
stardiv_Toolkit_VCLXPrinterServer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXPrinterServer);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
