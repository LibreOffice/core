/*************************************************************************
 *
 *  $RCSfile: oinputstreamcontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-26 20:39:25 $
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
 *  except in compliance with the License. You may obtain a copy of the
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


#include "oinputstreamcontainer.hxx"

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

using namespace ::com::sun::star;

//-----------------------------------------------
OInputStreamContainer::OInputStreamContainer( const uno::Reference< io::XInputStream >& xStream )
: m_xInputStream( xStream )
, m_xSeekable( xStream, uno::UNO_QUERY )
{
    m_bSeekable = m_xSeekable.is();
}

//-----------------------------------------------
OInputStreamContainer::~OInputStreamContainer()
{
}

//-----------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OInputStreamContainer::getTypes()
        throw ( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( m_aMutex ) ;

        if ( pTypeCollection == NULL )
        {
            if ( m_bSeekable )
            {
                static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const uno::Reference< io::XStream >* )NULL ),
                        ::getCppuType(( const uno::Reference< io::XInputStream >* )NULL ),
                        ::getCppuType(( const uno::Reference< io::XSeekable >* )NULL ) );

                pTypeCollection = &aTypeCollection ;
            }
            else
            {
                static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const uno::Reference< io::XStream >* )NULL ),
                        ::getCppuType(( const uno::Reference< io::XInputStream >* )NULL ) );

                pTypeCollection = &aTypeCollection ;
            }
        }
    }

    return pTypeCollection->getTypes() ;

}

//-----------------------------------------------
uno::Any SAL_CALL OInputStreamContainer::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    // Attention:
    //  Don't use mutex or guard in this method!!! Is a method of XInterface.

    uno::Any aReturn;
    if ( m_bSeekable )
        aReturn = uno::Any( ::cppu::queryInterface( rType,
                                           static_cast< io::XStream* >( this ),
                                           static_cast< io::XInputStream* >( this ),
                                           static_cast< io::XSeekable* >( this ) ) );
    else
        aReturn = uno::Any( ::cppu::queryInterface( rType,
                                           static_cast< io::XStream* >( this ),
                                           static_cast< io::XInputStream* >( this ) ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    return ::cppu::OWeakObject::queryInterface( rType ) ;
}

//-----------------------------------------------
void SAL_CALL OInputStreamContainer::acquire()
        throw()
{
    ::cppu::OWeakObject::acquire();
}

//-----------------------------------------------
void SAL_CALL OInputStreamContainer::release()
        throw()
{
    ::cppu::OWeakObject::release();
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputStreamContainer::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputStreamContainer::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->readSomeBytes( aData, nMaxBytesToRead );
}

//-----------------------------------------------
void SAL_CALL OInputStreamContainer::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    m_xInputStream->skipBytes( nBytesToSkip );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputStreamContainer::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->available();
}

//-----------------------------------------------
void SAL_CALL OInputStreamContainer::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    if ( !m_xInputStream.is() )
        throw uno::RuntimeException();

    return m_xInputStream->closeInput();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OInputStreamContainer::getInputStream()
        throw ( uno::RuntimeException )
{
    if ( !m_xInputStream.is() )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OInputStreamContainer::getOutputStream()
        throw ( uno::RuntimeException )
{
    return uno::Reference< io::XOutputStream >();
}

//-----------------------------------------------
void SAL_CALL OInputStreamContainer::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

//-----------------------------------------------
sal_Int64 SAL_CALL OInputStreamContainer::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

//-----------------------------------------------
sal_Int64 SAL_CALL OInputStreamContainer::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

