/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: otransactedfilestream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:34:19 $
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
#ifndef _COMPHELPER_OTRANSACTEDFILESTREAM_HXX
#define _COMPHELPER_OTRANSACTEDFILESTREAM_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

namespace comphelper
{


//==================================================================
// OTransactedFileStream
// allows to get transacted stream access to a file
// the file contents are automatically truncated, but the file
// itself is changed only on commit
// after the first commit the stream switches to direct mode
//==================================================================

struct TTFileStreamData_Impl;
class COMPHELPER_DLLPUBLIC OTruncatedTransactedFileStream
        : public ::cppu::WeakImplHelper7 <
                                        ::com::sun::star::io::XStream,
                                        ::com::sun::star::io::XInputStream,
                                        ::com::sun::star::io::XOutputStream,
                                        ::com::sun::star::io::XTruncate,
                                        ::com::sun::star::io::XSeekable,
                                        ::com::sun::star::beans::XPropertySetInfo,
                                        ::com::sun::star::beans::XPropertySet >
{
    ::osl::Mutex    m_aMutex;
    TTFileStreamData_Impl* m_pStreamData;

    void CommonInit_Impl(
            const ::rtl::OUString& aURL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
            sal_Bool bDeleteOptionIsProvided );

    void CloseAll_Impl();

public:

    OTruncatedTransactedFileStream( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    OTruncatedTransactedFileStream( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    OTruncatedTransactedFileStream( const ::rtl::OUString& aURL, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >& xFileAccess, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory, sal_Bool bDeleteIfNotCommited );

    ~OTruncatedTransactedFileStream();

    void Commit_Impl();
    void Revert_Impl();

// com::sun::star::io::XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL available(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XTruncate
    virtual void SAL_CALL truncate(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// com::sun::star::io::XSeekable
    virtual void SAL_CALL seek( ::sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertySetInfo
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const ::rtl::OUString& aName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

};

} // namespace comphelper

#endif //_COMPHELPER_OTRANSACTEDFILESTREAM_HXX


