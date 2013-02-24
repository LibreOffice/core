/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "inputstream.hxx"


using namespace chelp;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



XInputStream_impl::XInputStream_impl( const rtl::OUString& aUncPath )
    : m_bIsOpen( false ),
      m_aFile( aUncPath )
{
    m_bIsOpen = ( osl::FileBase::E_None == m_aFile.open( osl_File_OpenFlag_Read ) );
}


XInputStream_impl::~XInputStream_impl()
{
    closeInput();
}


bool SAL_CALL XInputStream_impl::CtorSuccess()
{
    return m_bIsOpen;
};


uno::Any SAL_CALL
XInputStream_impl::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          (static_cast< io::XInputStream* >(this)),
                                          (static_cast< io::XSeekable* >(this)) );
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
    if( ! m_bIsOpen )
        throw io::IOException();

    aData.realloc(nBytesToRead);
    //TODO! translate memory exhaustion (if it were detectable...) into
    // io::BufferSizeExceededException

    sal_uInt64 nrc;
    m_aFile.read( aData.getArray(),sal_uInt64(nBytesToRead),nrc );

    // Shrink aData in case we read less than nBytesToRead (XInputStream
    // documentation does not tell whether this is required, and I do not know
    // if any code relies on this, so be conservative---SB):
    if (nrc != sal::static_int_cast<sal_uInt64>( nBytesToRead) )
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
    if (m_aFile.setPos(osl_Pos_Current, sal_uInt64(nBytesToSkip)) != osl::FileBase::E_None)
    {
        throw io::IOException(::rtl::OUString(
            "XInputStream_impl::skipBytes failed seek"), uno::Reference< uno::XInterface >());
    }
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
    if( m_bIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();
        if( err != osl::FileBase::E_None )
            throw io::IOException();
        m_bIsOpen = false;
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
    if( osl::FileBase::E_None != m_aFile.setPos( osl_Pos_Absolut, sal_uInt64( location ) ) )
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
    osl::FileBase::RC   err;
    sal_uInt64          uCurrentPos, uEndPos;

    err = m_aFile.getPos( uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( osl_Pos_End, 0 );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.getPos( uEndPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();

    err = m_aFile.setPos( osl_Pos_Absolut, uCurrentPos );
    if( err != osl::FileBase::E_None )
        throw io::IOException();
    else
        return sal_Int64( uEndPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
