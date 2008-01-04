/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: seqoutputstreamserv.cxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: obo $ $Date: 2008-01-04 16:38:36 $
*
*  The Contents of this file are made available subject to
*  the terms of GNU Lesser General Public License Version 2.1.
*
*
*    GNU Lesser General Public License Version 2.1
*    =============================================
*    Copyright 2005 by Sun Microsystems, Inc.
*    901 San Antonio Road, Palo Alto, CA 94303, USA
*
*    This library is free software; you can redistribute it and/or
*    modify it under the terms of the GNU Lesser General Public
*    License version 2.1, as published by the Free Software Foundation.
*
*    This library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    Lesser General Public License for more details.
*
*    You should have received a copy of the GNU Lesser General Public
*    License along with this library; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*    MA  02111-1307  USA
*
************************************************************************/

#include "precompiled_comphelper.hxx"

#include <sal/config.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase2.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;


::rtl::OUString SAL_CALL SequenceOutputStreamService_getImplementationName();
uno::Sequence< ::rtl::OUString > SAL_CALL SequenceOutputStreamService_getSupportedServiceNames();
uno::Reference< uno::XInterface > SAL_CALL SequenceOutputStreamService_createInstance( const uno::Reference< uno::XComponentContext >& rxContext )SAL_THROW((uno::Exception));


namespace {

class SequenceOutputStreamService:
public ::cppu::WeakImplHelper2 < lang::XServiceInfo, io::XSequenceOutputStream >
{
public:
    explicit SequenceOutputStreamService();

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw ( uno::RuntimeException );
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString & ServiceName ) throw ( uno::RuntimeException );
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( uno::RuntimeException );

    // ::com::sun::star::io::XOutputStream:
    virtual void SAL_CALL writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) throw ( io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException );
    virtual void SAL_CALL flush() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );
    virtual void SAL_CALL closeOutput() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException );

    // ::com::sun::star::io::XSequenceOutputStream:
    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getWrittenBytes(  ) throw ( io::NotConnectedException, io::IOException, uno::RuntimeException);

private:
    SequenceOutputStreamService( SequenceOutputStreamService & ); //not defined
    void operator =( SequenceOutputStreamService & ); //not defined

    virtual ~SequenceOutputStreamService() {};


    ::osl::Mutex m_aMutex;
    uno::Reference< io::XOutputStream > m_xOutputStream;
    uno::Sequence< ::sal_Int8 > m_aSequence;
};
SequenceOutputStreamService::SequenceOutputStreamService()
{
    m_xOutputStream.set( static_cast < ::cppu::OWeakObject* >( new ::comphelper::OSequenceOutputStream( m_aSequence ) ), uno::UNO_QUERY_THROW );
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL SequenceOutputStreamService::getImplementationName() throw ( uno::RuntimeException )
{
    return SequenceOutputStreamService_getImplementationName();
}

::sal_Bool SAL_CALL SequenceOutputStreamService::supportsService( ::rtl::OUString const & serviceName ) throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > serviceNames = SequenceOutputStreamService_getSupportedServiceNames();
    for ( ::sal_Int32 i = 0; i < serviceNames.getLength(); ++i ) {
        if ( serviceNames[i] == serviceName )
            return sal_True;
    }
    return sal_False;
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceOutputStreamService::getSupportedServiceNames() throw ( uno::RuntimeException )
{
    return SequenceOutputStreamService_getSupportedServiceNames();
}

// ::com::sun::star::io::XOutputStream:
void SAL_CALL SequenceOutputStreamService::writeBytes( const uno::Sequence< ::sal_Int8 > & aData ) throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->writeBytes( aData );
    m_aSequence = aData;
}

void SAL_CALL SequenceOutputStreamService::flush() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
};

void SAL_CALL SequenceOutputStreamService::closeOutput() throw ( uno::RuntimeException, io::NotConnectedException, io::BufferSizeExceededException, io::IOException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->closeOutput();
    m_xOutputStream = uno::Reference< io::XOutputStream >();
}

// ::com::sun::star::io::XSequenceOutputStream:
uno::Sequence< ::sal_Int8 > SAL_CALL SequenceOutputStreamService::getWrittenBytes() throw ( io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xOutputStream.is() )
        throw io::NotConnectedException();

    m_xOutputStream->flush();
    return m_aSequence;
}

} // anonymous namespace

::rtl::OUString SAL_CALL SequenceOutputStreamService_getImplementationName() {
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.SequenceOutputStreamService" ) );
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceOutputStreamService_getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > s( 1 );
    s[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.io.SequenceOutputStream" ) );
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL SequenceOutputStreamService_createInstance(
    const uno::Reference< uno::XComponentContext >& )
    SAL_THROW( (uno::Exception) )
{
    return static_cast< ::cppu::OWeakObject * >( new SequenceOutputStreamService());
}

