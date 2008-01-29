 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filstr.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:14:17 $
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
#ifndef _FILSTR_HXX_
#define _FILSTR_HXX_

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XASYNCOUTPUTMONITOR_HPP_
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif

#include "filrec.hxx"

namespace fileaccess {

    // forward:
    class shell;
    class XInputStreamForStream;
    class XOutputStreamForStream;

    class XStream_impl
        : public cppu::OWeakObject,
          public com::sun::star::lang::XTypeProvider,
          public com::sun::star::io::XStream,
          public com::sun::star::io::XSeekable,
          public com::sun::star::io::XInputStream,
          public com::sun::star::io::XOutputStream,
          public com::sun::star::io::XTruncate,
          public com::sun::star::io::XAsyncOutputMonitor
    {
        friend class XInputStreamForStream;
        friend class XOutputStreamForStream;

    public:

        XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath );

        /**
         *  Returns an error code as given by filerror.hxx
         */

        sal_Int32 SAL_CALL CtorSuccess();
        sal_Int32 SAL_CALL getMinorError();

        virtual ~XStream_impl();


        // OWeakObject

        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& rType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        // XStream

        virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
        getInputStream(  )
            throw( com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL
        getOutputStream(  )
            throw( com::sun::star::uno::RuntimeException );


        // XTruncate

        virtual void SAL_CALL truncate( void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );


        // XInputStream

        sal_Int32 SAL_CALL
        readBytes(
            com::sun::star::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);

        sal_Int32 SAL_CALL
        readSomeBytes(
            com::sun::star::uno::Sequence< sal_Int8 >& aData,
            sal_Int32 nMaxBytesToRead )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);


        void SAL_CALL
        skipBytes(
            sal_Int32 nBytesToSkip )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int32 SAL_CALL
        available(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        void SAL_CALL
        closeInput(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        // XSeekable

        void SAL_CALL
        seek(
            sal_Int64 location )
            throw( com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL
        getPosition(
            void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        sal_Int64 SAL_CALL
        getLength(
            void )
            throw( com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );


        // XOutputStream

        void SAL_CALL
        writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);



        void SAL_CALL
        flush()
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::BufferSizeExceededException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException);


        void SAL_CALL
        closeOutput(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL waitForCompletion()
            throw (
                com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException);

    private:

        osl::Mutex   m_aMutex;
        bool         m_bInputStreamCalled,m_bOutputStreamCalled;

        shell*       m_pMyShell;
        com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider > m_xProvider;
        sal_Bool     m_nIsOpen;

        ReconnectingFile    m_aFile;

        sal_Int32                                          m_nErrorCode;
        sal_Int32                                          m_nMinorErrorCode;

        // Implementation methods

        void SAL_CALL
        closeStream(
            void )
            throw( com::sun::star::io::NotConnectedException,
                   com::sun::star::io::IOException,
                   com::sun::star::uno::RuntimeException );

    };

}  // end namespace XStream_impl

#endif
