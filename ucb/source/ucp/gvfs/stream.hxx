/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stream.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:28:09 $
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
#ifndef _GVFSSTREAM_HXX_
#define _GVFSSTREAM_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#include <libgnomevfs/gnome-vfs-handle.h>

namespace gvfs
{

class Stream : public ::com::sun::star::io::XStream,
           public ::com::sun::star::io::XInputStream,
           public ::com::sun::star::io::XOutputStream,
           public ::com::sun::star::io::XTruncate,
           public ::com::sun::star::io::XSeekable,
           public ::cppu::OWeakObject
{
private:
    GnomeVFSHandle  *m_handle;
    GnomeVFSFileInfo m_info;
    osl::Mutex       m_aMutex;
    sal_Bool         m_eof;
    sal_Bool         m_bInputStreamCalled;
    sal_Bool         m_bOutputStreamCalled;

    void throwOnError( GnomeVFSResult result )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::BufferSizeExceededException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

    void closeStream( void )
        throw( ::com::sun::star::io::NotConnectedException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

public:
    Stream ( GnomeVFSHandle         *handle,
         const GnomeVFSFileInfo *aInfo );
    virtual ~Stream();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & type )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire( void )
        throw ()
            { OWeakObject::acquire(); }
    virtual void SAL_CALL release( void )
        throw()
            { OWeakObject::release(); }

    // XStream
    virtual com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL getInputStream(  )
        throw( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  )
        throw( com::sun::star::uno::RuntimeException );

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(
              ::com::sun::star::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nBytesToRead )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::BufferSizeExceededException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL readSomeBytes(
            ::com::sun::star::uno::Sequence< sal_Int8 > & aData,
            sal_Int32 nMaxBytesToRead )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::BufferSizeExceededException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::BufferSizeExceededException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL available( void )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL closeInput( void )
                throw( ::com::sun::star::io::NotConnectedException,
                          ::com::sun::star::io::IOException,
                          ::com::sun::star::uno::RuntimeException );

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL getPosition()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL getLength()
        throw( ::com::sun::star::io::IOException,
               ::com::sun::star::uno::RuntimeException );

        // XOutputStream
    virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL flush( void )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::BufferSizeExceededException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);


        virtual void SAL_CALL closeOutput( void )
        throw( com::sun::star::io::NotConnectedException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );

    // XTruncate
    virtual void SAL_CALL truncate( void )
        throw( com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException );
};

} // namespace gvfs
#endif // _GVFSSTREAM_HXX_
