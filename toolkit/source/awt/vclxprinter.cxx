/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"


#include <toolkit/awt/vclxprinter.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>


#include <vcl/print.hxx>
#include <vcl/jobset.hxx>
#include <vcl/svapp.hxx>

#include <tools/debug.hxx>
#include <tools/stream.hxx>

#include <toolkit/awt/vclxdevice.hxx>


#define BINARYSETUPMARKER   0x23864691

#define PROPERTY_Orientation    0
#define PROPERTY_Horizontal     1

::com::sun::star::beans::Property* ImplGetProperties( sal_uInt16& rElementCount )
{
    static ::com::sun::star::beans::Property* pProperties = NULL;
    static sal_uInt16 nElements = 0;
    if( !pProperties )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pProperties )
        {
            static ::com::sun::star::beans::Property __FAR_DATA aPropTable[] =
            {
                ::com::sun::star::beans::Property( ::rtl::OUString::createFromAscii( "Orientation" ), PROPERTY_Orientation, ::getCppuType((const sal_Int16*)0), 0 ),
                ::com::sun::star::beans::Property( ::rtl::OUString::createFromAscii( "Horizontal" ), PROPERTY_Horizontal, ::getBooleanCppuType(), 0 )
            };
            pProperties = aPropTable;
            nElements = sizeof( aPropTable ) / sizeof( ::com::sun::star::beans::Property );
        }
    }
    rElementCount = nElements;
    return pProperties;
}

//  ----------------------------------------------------
//  class VCLXPrinterPropertySet
//  ----------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXPrinterPropertySet, VCLXPrinterPropertySet_Base, OPropertySetHelper )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXPrinterPropertySet, VCLXPrinterPropertySet_Base, ::cppu::OPropertySetHelper )

VCLXPrinterPropertySet::VCLXPrinterPropertySet( const String& rPrinterName )
    : OPropertySetHelper( BrdcstHelper )
    , mpPrinter( new Printer( rPrinterName ) )
{
    osl::Guard< vos::IMutex > aSolarGuard( Application::GetSolarMutex() );

    mnOrientation = 0;
    mbHorizontal = sal_False;
}

VCLXPrinterPropertySet::~VCLXPrinterPropertySet()
{
    osl::Guard< vos::IMutex > aSolarGuard( Application::GetSolarMutex() );
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
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
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
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    sal_Bool bDifferent = sal_False;
    switch ( nHandle )
    {
        case PROPERTY_Orientation:
        {
            sal_Int16 n;
            if( ( rValue >>= n ) && ( n != mnOrientation ) )
            {
                rConvertedValue <<= n;
                rOldValue <<= mnOrientation;
                bDifferent = sal_True;
            }
        }
        break;
        case PROPERTY_Horizontal:
        {
            sal_Bool b;
            if( ( rValue >>= b ) && ( b != mbHorizontal ) )
            {
                rConvertedValue <<= b;
                rOldValue <<= mbHorizontal;
                bDifferent = sal_True;
            }
        }
        break;
        default:
        {
            DBG_ERROR( "VCLXPrinterPropertySet_Impl::convertFastPropertyValue - invalid Handle" );
        }
    }
    return bDifferent;
}

void VCLXPrinterPropertySet::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

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
            DBG_ERROR( "VCLXPrinterPropertySet_Impl::convertFastPropertyValue - invalid Handle" );
        }
    }
}

void VCLXPrinterPropertySet::getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const
{
    ::osl::Guard< ::osl::Mutex > aGuard( ((VCLXPrinterPropertySet*)this)->Mutex );

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
            DBG_ERROR( "VCLXPrinterPropertySet_Impl::convertFastPropertyValue - invalid Handle" );
        }
    }
}

// ::com::sun::star::awt::XPrinterPropertySet
void VCLXPrinterPropertySet::setHorizontal( sal_Bool bHorizontal ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    ::com::sun::star::uno::Any aValue;
    aValue <<= bHorizontal;
    setFastPropertyValue( PROPERTY_Horizontal, aValue );
}

::com::sun::star::uno::Sequence< ::rtl::OUString > VCLXPrinterPropertySet::getFormDescriptions(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    sal_uInt16 nPaperBinCount = GetPrinter()->GetPaperBinCount();
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aDescriptions( nPaperBinCount );
    for ( sal_uInt16 n = 0; n < nPaperBinCount; n++ )
    {
        // Format: <DisplayFormName;FormNameId;DisplayPaperBinName;PaperBinNameId;DisplayPaperName;PaperNameId>
        String aDescr( RTL_CONSTASCII_USTRINGPARAM( "*;*;" ) );
        aDescr += GetPrinter()->GetPaperBinName( n );
        aDescr += ';';
        aDescr += n;
        aDescr.AppendAscii( ";*;*", 4 );

        aDescriptions.getArray()[n] = aDescr;
    }
    return aDescriptions;
}

void VCLXPrinterPropertySet::selectForm( const ::rtl::OUString& rFormDescription ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    sal_Int32 nIndex = 0;
    sal_uInt16 nPaperBin = sal::static_int_cast< sal_uInt16 >(
        rFormDescription.getToken( 3, ';', nIndex ).toInt32());
    GetPrinter()->SetPaperBin( nPaperBin );
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXPrinterPropertySet::getBinarySetup(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    SvMemoryStream aMem;
    aMem << BINARYSETUPMARKER;
    aMem << GetPrinter()->GetJobSetup();
    return ::com::sun::star::uno::Sequence<sal_Int8>( (sal_Int8*) aMem.GetData(), aMem.Tell() );
}

void VCLXPrinterPropertySet::setBinarySetup( const ::com::sun::star::uno::Sequence< sal_Int8 >& data ) throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    SvMemoryStream aMem( (char*) data.getConstArray(), data.getLength(), STREAM_READ );
    sal_uInt32 nMarker;
    aMem >> nMarker;
    DBG_ASSERT( nMarker == BINARYSETUPMARKER, "setBinarySetup - invalid!" );
    if ( nMarker == BINARYSETUPMARKER )
    {
        JobSetup aSetup;
        aMem >> aSetup;
        GetPrinter()->SetJobSetup( aSetup );
    }
}


//  ----------------------------------------------------
//  class VCLXPrinter
//  ----------------------------------------------------
VCLXPrinter::VCLXPrinter( const String& rPrinterName )
    : VCLXPrinter_Base( rPrinterName )
{
}

VCLXPrinter::~VCLXPrinter()
{
}

sal_Bool VCLXPrinter::start( const ::rtl::OUString& /*rJobName*/, sal_Int16 /*nCopies*/, sal_Bool /*bCollate*/ ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    sal_Bool bDone = sal_True;
    if ( mpListener.get() )
    {
        maInitJobSetup = mpPrinter->GetJobSetup();
        mpListener.reset( new vcl::OldStylePrintAdaptor( mpPrinter ) );
    }

    return bDone;
}

void VCLXPrinter::end(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    if ( mpListener.get() )
    {
        Printer::PrintJob( mpListener, maInitJobSetup );
        mpListener.reset();
    }
}

void VCLXPrinter::terminate(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    mpListener.reset();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXPrinter::startPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    if ( mpListener.get() )
    {
        mpListener->StartPage();
    }
    return GetDevice();
}

void VCLXPrinter::endPage(  ) throw(::com::sun::star::awt::PrinterException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    if ( mpListener.get() )
    {
        mpListener->EndPage();
    }
}


//  ----------------------------------------------------
//  class VCLXInfoPrinter
//  ----------------------------------------------------

VCLXInfoPrinter::VCLXInfoPrinter( const String& rPrinterName )
    : VCLXInfoPrinter_Base( rPrinterName )
{
}

VCLXInfoPrinter::~VCLXInfoPrinter()
{
}

// ::com::sun::star::awt::XInfoPrinter
::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXInfoPrinter::createDevice(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( Mutex );

    return GetDevice();
}

//  ----------------------------------------------------
//  class VCLXPrinterServer
//  ----------------------------------------------------

// ::com::sun::star::awt::XPrinterServer
::com::sun::star::uno::Sequence< ::rtl::OUString > VCLXPrinterServer::getPrinterNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    const std::vector<rtl::OUString>& rQueues = Printer::GetPrinterQueues();
    sal_uInt32 nPrinters = rQueues.size();

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  aNames( nPrinters );
    for ( sal_uInt32 n = 0; n < nPrinters; n++ )
        aNames.getArray()[n] = rQueues[n];

    return aNames;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > VCLXPrinterServer::createPrinter( const ::rtl::OUString& rPrinterName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPrinter > xP;
    xP = new VCLXPrinter( rPrinterName );
    return xP;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > VCLXPrinterServer::createInfoPrinter( const ::rtl::OUString& rPrinterName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XInfoPrinter > xP;
    xP = new VCLXInfoPrinter( rPrinterName );
    return xP;
}


