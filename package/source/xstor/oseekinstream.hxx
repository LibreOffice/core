/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oseekinstream.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:49:55 $
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

#ifndef _INPUTSEEKSTREAM_HXX_
#define _INPUTSEEKSTREAM_HXX_

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#include "ocompinstream.hxx"

class OInputSeekStream : public OInputCompStream
                        , public ::com::sun::star::io::XSeekable
{
protected:
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XSeekable > m_xSeekable;

public:
    OInputSeekStream( OWriteStream_Impl& pImpl,
                      ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xStream,
                      const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                      sal_Int16 nStorageType );

    OInputSeekStream( OWriteStream_Impl& pImpl,
                      ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream > xStream,
                      const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                      sal_Int16 nStorageType );

    OInputSeekStream( ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > xStream,
                      const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                      sal_Int16 nStorageType );

    OInputSeekStream( ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream > xStream,
                      const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps,
                      sal_Int16 nStorageType );

    virtual ~OInputSeekStream();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

};

#endif

