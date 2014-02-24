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

#include <toolkit/awt/vclxprinter.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>


#include <vcl/print.hxx>
#include <vcl/jobset.hxx>
#include <vcl/svapp.hxx>

#include <tools/debug.hxx>
#include <tools/stream.hxx>

#include <toolkit/awt/vclxdevice.hxx>


#define BINARYSETUPMARKER       0x23864691

#define PROPERTY_Orientation    0
#define PROPERTY_Horizontal     1

::com::sun::star::beans::Property* ImplGetProperties( sal_uInt16& rElementCount )
{
    static ::com::sun::star::beans::Property* pProperties = NULL;
    static sal_uInt16 nElements = 0;
    if( !pProperties )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pProperties )
        {
            static ::com::sun::star::beans::Property aPropTable[] =
            {
                ::com::sun::star::beans::Property( OUString("Orientation"), PROPERTY_Orientation, ::getCppuType((const sal_Int16*)0), 0 ),
                ::com::sun::star::beans::Property( OUString("Horizontal"), PROPERTY_Horizontal, ::getBooleanCppuType(), 0 )
            };
            pProperties = aPropTable;
            nElements = sizeof( aPropTable ) / sizeof( ::com::sun::star::beans::Property );
        }
    }
    rElementCount = nElements;
    return pProperties;
}

//    ----------------------------------------------------
//    class VCLXPrinterPropertySet
//    ----------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXPrinterPropertySet, VCLXPrinterPropertySet_Base, OPropertySetHelper )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXPrinterPropertySet, VCLXPrinterPropertySet_Base, ::cppu::OPropertySetHelper )

VCLXPrinterPropertySet::VCLXPrinterPropertySet( const OUString& rPrinterName )
    : OPropertySetHelper( BrdcstHelper )
    , mpPrinter( new Printer( rPrinterName ) )
{
    SolarMutexGuard aSolarGuard;

    mnOrientation = 0;
    mbHorizontal = false;
}

VCLXPrinterPropertySet::~VCLXPrinterPropertySet()
{
    SolarMutexGuard aSolarGuard;
    mpPrinter.reset();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >  VCLXPrinterPropertySet::GetDevice()
{
    if ( !mxPrnDevice.is() )
    {
        VCLXDevice* pDev = new VCLXDevice;
        pDev->SetOutputDevice( GetPrinter() );
        mxPrnDevice = pDev;
    }
    return mxPrnDevice;
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > VCLXPrinterPropertySet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& VCLXPrinterPropertySet::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper* pPropertyArrayHelper = NULL;
    if ( !pPropertyArrayHelper )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pPropertyArrayHelper )
        {
            sal_uInt16 nElements;
            ::com::sun::star::beans::Property* pProps = ImplGetProperties( nElements );
            pPropertyArrayHelper = new ::cppu::OPropertyArrayHelper( pProps, nElements, sal_False );
        }
    }
    return *pPropertyArrayHelper ;
}

sal_Bool VCLXPrinterPropertySet::convertFastPropertyValue( ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::lang::IllegalArgumentException)
{
    ::osl::MutexGuard aGuard( Mutex );

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

void VCLXPrinterPropertySet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    ::osl::MutexGuard aGuard( Mutex );

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

void VCLXPrinterPropertySet::getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const
{
    ::osl::MutexGuard aGuard( ((VCLXPrinterPropertySet*)this)->Mutex );

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

// ::com::sun::star::awt::XPrinterPropertySet
void VCLXPrinterPropertySet::setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    ::com::sun::star::uno::Any aValue;
    aValue <<= bHorizontal;
    setFastPropertyValue( PROPERTY_Horizontal, aValue );
}

::com::sun::star::uno::Sequence< OUString > VCLXPrinterPropertySet::getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    sal_uInt16 nPaperBinCount = GetPrinter()->GetPaperBinCount();
    ::com::sun::star::uno::Sequence< OUString > aDescriptions( nPaperBinCount );
    for ( sal_uInt16 n = 0; n < nPaperBinCount; n++ )
    {
        // Format: <DisplayFormName;FormNameId;DisplayPaperBinName;PaperBinNameId;DisplayPaperName;PaperNameId>
        OUStringBuffer aDescr( "*;*;" );
        aDescr.append(GetPrinter()->GetPaperBinName( n ));
        aDescr.append(';');
        aDescr.append(OUString::number(n));
        aDescr.append(";*;*");

        aDescriptions.getArray()[n] = aDescr.makeStringAndClear();
    }
    return aDescriptions;
}

void VCLXPrinterPropertySet::selectForm( const OUString& rFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    sal_Int32 nIndex = 0;
    sal_uInt16 nPaperBin = sal::static_int_cast< sal_uInt16 >(
        rFormDescription.getToken( 3, ';', nIndex ).toInt32());
    GetPrinter()->SetPaperBin( nPaperBin );
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXPrinterPropertySet::getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    SvMemoryStream aMem;
    aMem.WriteUInt32( sal_uInt32(BINARYSETUPMARKER) );
    WriteJobSetup( aMem, GetPrinter()->GetJobSetup() );
    return ::com::sun::star::uno::Sequence<sal_Int8>( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

void VCLXPrinterPropertySet::setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    SvMemoryStream aMem( (char*) data.getConstArray(), data.getLength(), STREAM_READ );
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

sal_Bool VCLXPrinter::start( const OUString& /*rJobName*/, sal_Int16 /*nCopies*/, sal_Bool /*bCollate*/ ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    bool bDone = true;
    if ( mpPrinter.get() )
    {
        maInitJobSetup = mpPrinter->GetJobSetup();
        mpListener.reset( new vcl::OldStylePrintAdaptor( mpPrinter ) );
    }

    return bDone;
}

void VCLXPrinter::end(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    if ( mpListener.get() )
    {
        Printer::PrintJob( mpListener, maInitJobSetup );
        mpListener.reset();
    }
}

void VCLXPrinter::terminate(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    mpListener.reset();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXPrinter::startPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    if ( mpListener.get() )
    {
        mpListener->StartPage();
    }
    return GetDevice();
}

void VCLXPrinter::endPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    if ( mpListener.get() )
    {
        mpListener->EndPage();
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

// ::com::sun::star::awt::XInfoPrinter
::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXInfoPrinter::createDevice(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( Mutex );

    return GetDevice();
}

//    ----------------------------------------------------
//    class VCLXPrinterServer
//    ----------------------------------------------------

// ::com::sun::star::awt::XPrinterServer
::com::sun::star::uno::Sequence< OUString > VCLXPrinterServer::getPrinterNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    const std::vector<OUString>& rQueues = Printer::GetPrinterQueues();
    sal_uInt32 nPrinters = rQueues.size();

    ::com::sun::star::uno::Sequence< OUString > aNames( nPrinters );
    for ( sal_uInt32 n = 0; n < nPrinters; n++ )
        aNames.getArray()[n] = rQueues[n];

    return aNames;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > VCLXPrinterServer::createPrinter( const OUString& rPrinterName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > xP;
    xP = new VCLXPrinter( rPrinterName );
    return xP;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > VCLXPrinterServer::createInfoPrinter( const OUString& rPrinterName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > xP;
    xP = new VCLXInfoPrinter( rPrinterName );
    return xP;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
stardiv_Toolkit_VCLXPrinterServer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new VCLXPrinterServer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
