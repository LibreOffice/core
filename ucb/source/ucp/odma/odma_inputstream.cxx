/*************************************************************************
 *
 *  $RCSfile: odma_inputstream.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:47:43 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef ODMA_INPUTSTREAM_HXX
#include "odma_inputstream.hxx"
#endif
#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include "com/sun/star/io/IOException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef ODMA_CONTENTPROPS_HXX
#include "odma_contentprops.hxx"
#endif
#ifndef ODMA_PROVIDER_HXX
#include "odma_provider.hxx"
#endif

using namespace odma;
using namespace com::sun::star;

class OActiveDataStreamer : public ::cppu::WeakImplHelper1< ::com::sun::star::io::XActiveDataStreamer>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
public:
    OActiveDataStreamer(){}
    virtual void SAL_CALL setStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& _rStream ) throw (::com::sun::star::uno::RuntimeException)
    {
        m_xStream = _rStream;
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > SAL_CALL getStream(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return m_xStream;
    }
};
// -----------------------------------------------------------------------------
OOdmaStream::OOdmaStream(::ucb::Content* _pContent,
                         ContentProvider* _pProvider,
                         const ::vos::ORef<ContentProperties>& _rProp)
 :m_pContent(_pContent)
 ,m_bInputStreamCalled(sal_False)
 ,m_bOutputStreamCalled(sal_False)
 ,m_bModified(sal_False)
 ,m_pProvider(_pProvider)
 ,m_aProp(_rProp)
{
}
// -----------------------------------------------------------------------------
OOdmaStream::~OOdmaStream()
{
    try
    {
        closeStream();
        delete m_pContent;
    }
    catch (io::IOException const &)
    {
        OSL_ENSURE(false, "unexpected situation");
    }
    catch (uno::RuntimeException const &)
    {
        OSL_ENSURE(false, "unexpected situation");
    }
}
// -----------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OOdmaStream::getInputStream(  ) throw( uno::RuntimeException)
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bInputStreamCalled = sal_True;
    }
    return uno::Reference< io::XInputStream >( this );
}
// -----------------------------------------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OOdmaStream::getOutputStream(  ) throw( uno::RuntimeException )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bOutputStreamCalled = sal_True;
    }
    return uno::Reference< io::XOutputStream >( this );
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OOdmaStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    ensureInputStream();

    return m_xInput->readBytes(aData,nBytesToRead);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OOdmaStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::skipBytes( sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    ensureInputStream();
    m_xInput->skipBytes(nBytesToSkip );
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OOdmaStream::available()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    ensureInputStream();
    return m_xInput->available();
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    ensureOutputStream();
    m_xOutput->writeBytes(aData);
    m_bModified = sal_True;
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::closeStream() throw( io::NotConnectedException,io::IOException,uno::RuntimeException )
{
    if( m_xInput.is() )
    {
        m_xInput->closeInput();
        m_xInput        = NULL;
        m_xInputSeek    = NULL;
    }
    if(m_xOutput.is())
    {
        m_xOutput->closeOutput();
        m_xOutput       = NULL;
        m_xTruncate     = NULL;
        if(m_bModified)
            m_pProvider->saveDocument(m_aProp->m_sDocumentId);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::closeInput()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bInputStreamCalled = sal_False;

    if( ! m_bOutputStreamCalled )
        closeStream();
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::closeOutput()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bOutputStreamCalled = sal_False;

    if( ! m_bInputStreamCalled )
        closeStream();
}
// -----------------------------------------------------------------------------
void SAL_CALL OOdmaStream::flush()
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    ensureOutputStream();
    m_xOutput->flush();
}
// -----------------------------------------------------------------------------
void OOdmaStream::ensureInputStream() throw( io::IOException )
{
    try
    {
        if(!m_xInput.is())
        {
            m_xInput = m_pContent->openStream();
            m_xInputSeek = uno::Reference< io::XSeekable>(m_xInput,uno::UNO_QUERY);
        }
    }
    catch(const uno::Exception&)
    {
    }
    if(!m_xInput.is())
        throw io::IOException();
}
// -----------------------------------------------------------------------------
void OOdmaStream::ensureOutputStream() throw( io::IOException )
{
    try
    {
        if(!m_xOutput.is())
        {
            ::com::sun::star::ucb::OpenCommandArgument2 aCommand;
            aCommand.Mode = ::com::sun::star::ucb::OpenMode::DOCUMENT;
            uno::Reference< io::XActiveDataStreamer > xActiveStreamer = new OActiveDataStreamer();
            aCommand.Sink = xActiveStreamer;
            m_pContent->executeCommand(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("open")),uno::makeAny(aCommand));
            if(xActiveStreamer.is())
            {
                uno::Reference< io::XStream> xStream = xActiveStreamer->getStream();
                if(xStream.is())
                    m_xOutput = xStream->getOutputStream();
            }
        }
    }
    catch(const uno::Exception&)
    {
    }
    if(!m_xOutput.is())
        throw io::IOException();
    m_xTruncate = uno::Reference< io::XTruncate>(m_xOutput,uno::UNO_QUERY);
}
// -----------------------------------------------------------------------------
// XTruncate
void SAL_CALL OOdmaStream::truncate( void )
    throw( com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    if(m_xTruncate.is())
        m_xTruncate->truncate();
}
// -----------------------------------------------------------------------------
// XSeekable
void SAL_CALL OOdmaStream::seek(sal_Int64 location )
    throw( com::sun::star::lang::IllegalArgumentException,
           com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    ensureInputStream();
    if(m_xInputSeek.is())
        m_xInputSeek->seek(location);
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OOdmaStream::getPosition()
    throw( com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    ensureInputStream();
    return m_xInputSeek.is() ? m_xInputSeek->getPosition() : sal_Int64(0);
}
// -----------------------------------------------------------------------------
sal_Int64 SAL_CALL OOdmaStream::getLength()
    throw( com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    ensureInputStream();
    return m_xInputSeek.is() ? m_xInputSeek->getLength() : sal_Int64(0);
}
// -----------------------------------------------------------------------------
