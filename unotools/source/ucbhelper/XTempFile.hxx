/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XTempFile.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:28:11 $
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
#ifndef _XTEMPFILE_HXX_
#define _XTEMPFILE_HXX_

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTEMPFILE_HPP_
#include <com/sun/star/io/XTempFile.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _CPPUHELPER_PROPERTYSETMIXIN_HXX_
#include <cppuhelper/propertysetmixin.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

class SvStream;
namespace utl { class TempFile; }

typedef  ::cppu::WeakImplHelper5<   ::com::sun::star::io::XTempFile
                                    ,   ::com::sun::star::io::XInputStream
                                                  , ::com::sun::star::io::XOutputStream
                                                  , ::com::sun::star::io::XTruncate
                                                  , ::com::sun::star::lang::XServiceInfo
                                                  >
                                    OTempFileBase;

class OTempFileService :
    public OTempFileBase,
    public ::cppu::PropertySetMixin< ::com::sun::star::io::XTempFile >
{
protected:
    ::utl::TempFile*    mpTempFile;
    ::osl::Mutex        maMutex;
    SvStream*           mpStream;
    sal_Bool            mbRemoveFile;
    sal_Bool            mbInClosed;
    sal_Bool            mbOutClosed;

    sal_Int64           mnCachedPos;
    sal_Bool            mbHasCachedPos;

    void checkError () const;
    void checkConnected ();

public:
    OTempFileService (::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

    //Methods
    //  XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw ();
    virtual void SAL_CALL release(  )
        throw ();
    //  XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getImplementationId(  )
        throw (::com::sun::star::uno::RuntimeException);

    //  XTempFile
    virtual ::sal_Bool SAL_CALL getRemoveFile()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRemoveFile( ::sal_Bool _removefile )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getUri()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getResourceName()
        throw (::com::sun::star::uno::RuntimeException);

    // XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL available(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  )
        throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  )
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  )
        throw (::com::sun::star::uno::RuntimeException);
    // XTruncate
    virtual void SAL_CALL truncate()
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    //::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > SAL_CALL XTempFile_createInstance( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);
    static ::rtl::OUString getImplementationName_Static ();
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > getSupportedServiceNames_Static();

    static ::com::sun::star::uno::Reference < com::sun::star::lang::XSingleComponentFactory > createServiceFactory_Static( com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > const & rServiceFactory );

private:
    OTempFileService( OTempFileService & );
    virtual ~OTempFileService ();

};
#endif
