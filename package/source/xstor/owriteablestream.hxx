/*************************************************************************
 *
 *  $RCSfile: owriteablestream.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-30 09:48:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in::compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _WRITESTREAM_HXX_
#define _WRITESTREAM_HXX_

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#ifndef _COM_SUN_STAR_PACKAGES_XDATASINKENCRSUPPORT_HPP_
#include <com/sun/star/packages/XDataSinkEncrSupport.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XENCRYPTIONPROTECTEDSOURCE_HPP_
#include <com/sun/star/embed/XEncryptionProtectedSource.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif


#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#include <list>

#include "ocompinstream.hxx"
#include "mutexholder.hxx"


struct PreCreationStruct
{
    SotMutexHolderRef m_rMutexRef;

    PreCreationStruct()
    : m_rMutexRef( new SotMutexHolder )
    {}

};

struct WSInternalData_Impl
{
    SotMutexHolderRef m_rSharedMutexRef;
    ::cppu::OInterfaceContainerHelper m_aListenersContainer; // list of listeners

    // the mutex reference MUST NOT be empty
    WSInternalData_Impl( const SotMutexHolderRef rMutexRef )
    : m_rSharedMutexRef( rMutexRef )
    , m_aListenersContainer( rMutexRef->GetMutex() )
    {}
};

typedef ::std::list< OInputCompStream* > InputStreamsList_Impl;

struct OStorage_Impl;
class OWriteStream;

struct OWriteStream_Impl : public PreCreationStruct
{
    friend class OWriteStream;
    friend class OInputCompStream;

    OWriteStream*   m_pAntiImpl;
    ::rtl::OUString m_aTempURL;

    InputStreamsList_Impl m_aInputStreamsList;

    sal_Bool                        m_bIsModified;    // only modified elements will be sent to the original content
    sal_Bool                        m_bCommited;      // sending the streams is coordinated by the root storage of the package

    ::com::sun::star::uno::Reference< ::com::sun::star::packages::XDataSinkEncrSupport > m_xPackageStream;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    OStorage_Impl* m_pParent;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aProps;

    sal_Bool m_bForceEncrypted;

    sal_Bool m_bHasCachedPassword;
    ::com::sun::star::uno::Sequence< sal_Int8 > m_aKey;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > m_xPackage;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > GetServiceFactory();

    ::rtl::OUString GetFilledTempFile();
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >       GetTempFileAsStream();
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  GetTempFileAsInputStream();

    void CopyTempFileToOutput( ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > xOutStream );

    ::com::sun::star::uno::Reference<   ::com::sun::star::io::XStream > GetStream_Impl( sal_Int32 nStreamMode );

    ::com::sun::star::uno::Sequence< sal_Int8 > GetCommonRootPass();

public:
    OWriteStream_Impl( OStorage_Impl* pParent,
                       ::com::sun::star::uno::Reference< ::com::sun::star::packages::XDataSinkEncrSupport > xPackageStream,
                       ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xPackage,
                       ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory,
                       sal_Bool bForceEncrypted );

    ~OWriteStream_Impl();

    void InsertIntoPackageFolder(
            const ::rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xParentPackageFolder );

    void SetToBeCommited() { m_bCommited = sal_True; }

    sal_Bool HasCachedPassword() { return m_bHasCachedPassword; }
    ::com::sun::star::uno::Sequence< sal_Int8 > GetCachedPassword() { return m_aKey; }
    sal_Bool IsModified() { return m_bIsModified; }

    sal_Bool IsEncrypted();

    void DisposeWrappers();

    void Commit();
    void Revert();

    void Free( sal_Bool bMust ); // allows to try to disconnect from the temporary stream
                                 // in case bMust is set to sal_True the method
                                // will throw exception in case the file is still busy

    void SetModified(); // can be done only by parent storage after renaming

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetStreamProperties();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStream(
                        sal_Int32 nStreamMode,
                        const ::com::sun::star::uno::Sequence< sal_Int8 >& aKey );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStream(
                        sal_Int32 nStreamMode );


    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetRawInStream();

    void InputStreamDisposed( OInputCompStream* pStream );
};

class OWriteStream : public cppu::WeakImplHelper8 < ::com::sun::star::io::XInputStream
                                                    ,::com::sun::star::io::XOutputStream
                                                    ,::com::sun::star::io::XStream
                                                    ,::com::sun::star::io::XSeekable
                                                    ,::com::sun::star::io::XTruncate
                                                    ,::com::sun::star::lang::XComponent
                                                    ,::com::sun::star::embed::XEncryptionProtectedSource
                                                    ,::com::sun::star::beans::XPropertySet >
{
    friend struct OWriteStream_Impl;

protected:
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > m_xInStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > m_xOutStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XSeekable > m_xSeekable;

    OWriteStream_Impl* m_pImpl;
    WSInternalData_Impl* m_pData;

    OWriteStream( OWriteStream_Impl* pImpl, ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > xStream );

    void CloseOutput_Impl();

public:

    virtual ~OWriteStream();

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XOutputStream
    virtual void SAL_CALL writeBytes( const ::com::sun::star::uno::Sequence< sal_Int8 >& aData ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);

    // XTruncate
    virtual void SAL_CALL truncate() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    //XEncryptionProtectedSource
    virtual void SAL_CALL setEncryptionKey( const ::com::sun::star::uno::Sequence< sal_Int8 >& aKey ) throw (::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

};

#endif

