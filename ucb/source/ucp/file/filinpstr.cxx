 /*************************************************************************
 *
 *  $RCSfile: filinpstr.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-17 09:55:46 $
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
#ifndef _FILINPSTR_HXX_
#include "filinpstr.hxx"
#endif
#ifndef _FILERROR_HXX_
#include "filerror.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif
#ifndef _PROV_HXX_
#include "prov.hxx"
#endif


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



XInputStream_impl::XInputStream_impl( shell* pMyShell,const rtl::OUString& aUncPath )
    : m_pMyShell( pMyShell ),
      m_aFile( aUncPath ),
      m_xProvider( pMyShell->m_pProvider ),
      m_nErrorCode( TASKHANDLER_NO_ERROR ),
      m_nMinorErrorCode( TASKHANDLER_NO_ERROR )
{
    osl::FileBase::RC err = m_aFile.open( OpenFlag_Read );
    if( err != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();

        m_nErrorCode = TASKHANDLING_OPEN_FOR_INPUTSTREAM;
        m_nMinorErrorCode = err;
    }
    else
        m_nIsOpen = true;
}


XInputStream_impl::~XInputStream_impl()
{
    try
    {
        closeInput();
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


sal_Int32 SAL_CALL XInputStream_impl::CtorSuccess()
{
    return m_nErrorCode;
};



sal_Int32 SAL_CALL XInputStream_impl::getMinorError()
{
    return m_nMinorErrorCode;
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XServiceInfo
//////////////////////////////////////////////////////////////////////////////////////////

rtl::OUString SAL_CALL
XInputStream_impl::getImplementationName()
    throw(uno::RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.io.comp.XInputStream");
}



sal_Bool SAL_CALL
XInputStream_impl::supportsService( const rtl::OUString& ServiceName )
    throw(uno::RuntimeException)
{
    return false;
}



uno::Sequence< rtl::OUString > SAL_CALL
XInputStream_impl::getSupportedServiceNames()
    throw( uno::RuntimeException )
{
    uno::Sequence< rtl::OUString > ret( 0 );
    return ret;
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////


XTYPEPROVIDER_IMPL_4( XInputStream_impl,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      io::XSeekable,
                      io::XInputStream )



uno::Any SAL_CALL
XInputStream_impl::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( io::XInputStream*,this ),
                                          SAL_STATIC_CAST( lang::XTypeProvider*,this ),
                                          SAL_STATIC_CAST( lang::XServiceInfo*,this ),
                                          SAL_STATIC_CAST( io::XSeekable*,this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL
XInputStream_impl::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XInputStream_impl::release(
    void )
    throw()
{
    OWeakObject::release();
}



sal_Int32 SAL_CALL
XInputStream_impl::readBytes(
                 uno::Sequence< sal_Int8 >& aData,
                 sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    if( ! m_nIsOpen ) throw io::IOException();

    aData.realloc(nBytesToRead);
        //TODO! translate memory exhaustion (if it were detectable...) into
        // io::BufferSizeExceededException

    sal_uInt64 nrc(0);
    if(m_aFile.read( aData.getArray(),sal_uInt64(nBytesToRead),nrc )
       != osl::FileBase::E_None)
        throw io::IOException();

    // Shrink aData in case we read less than nBytesToRead (XInputStream
    // documentation does not tell whether this is required, and I do not know
    // if any code relies on this, so be conservative---SB):
    if (nrc != nBytesToRead)
        aData.realloc(sal_Int32(nrc));
    return ( sal_Int32 ) nrc;
}

sal_Int32 SAL_CALL
XInputStream_impl::readSomeBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL
XInputStream_impl::skipBytes(
    sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XInputStream_impl::available(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    return 0;
}


void SAL_CALL
XInputStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if( m_nIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();
        if( err != osl::FileBase::E_None )
            throw io::IOException();
        m_nIsOpen = false;
    }
}


void SAL_CALL
XInputStream_impl::seek(
    sal_Int64 location )
    throw( lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException )
{
    if( location < 0 )
        throw lang::IllegalArgumentException();
    if( osl::FileBase::E_None != m_aFile.setPos( Pos_Absolut, sal_uInt64( location ) ) )
        throw io::IOException();
}


sal_Int64 SAL_CALL
XInputStream_impl::getPosition(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    sal_uInt64 uPos;
    if( osl::FileBase::E_None != m_aFile.getPos( uPos ) )
        throw io::IOException();
    return sal_Int64( uPos );
}

sal_Int64 SAL_CALL
XInputStream_impl::getLength(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    sal_uInt64 uEndPos;
    if ( m_aFile.getSize(uEndPos) != osl::FileBase::E_None )
        throw io::IOException();
    else
        return sal_Int64( uEndPos );
}
