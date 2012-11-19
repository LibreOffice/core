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
#ifndef _OSL_PIPE_HXX_
#define _OSL_PIPE_HXX_

#include <osl/pipe_decl.hxx>

namespace osl
{
    //______________________________________________________________________________
    inline Pipe::Pipe()
        : m_handle( 0 )
    {}

    //______________________________________________________________________________
    inline Pipe::Pipe(const ::rtl::OUString& strName, oslPipeOptions Options )
        : m_handle( osl_createPipe( strName.pData, Options , 0 ) )
    {}

    //______________________________________________________________________________
    inline Pipe::Pipe(const ::rtl::OUString& strName, oslPipeOptions Options,const Security & rSecurity)
        : m_handle( osl_createPipe( strName.pData, Options , rSecurity.getHandle() ) )
    {}

    //______________________________________________________________________________
    inline Pipe::Pipe(const Pipe& pipe )
        : m_handle( pipe.m_handle )
    {
        if( m_handle )
            osl_acquirePipe( m_handle );
    }

    //______________________________________________________________________________
    inline Pipe::Pipe( oslPipe pipe, __sal_NoAcquire )
        : m_handle ( pipe )
    {}

    //______________________________________________________________________________
    inline Pipe::Pipe(oslPipe pipe)
        : m_handle( pipe )
    {
        if( m_handle )
            osl_acquirePipe( m_handle );
    }

    //______________________________________________________________________________
    inline Pipe::~Pipe()
    {
        if( m_handle )
            osl_releasePipe( m_handle );
    }

    //______________________________________________________________________________
    inline sal_Bool Pipe::create( const ::rtl::OUString & strName,
                                  oslPipeOptions Options, const Security &rSec )
    {
        *this = Pipe( strName, Options, rSec );
        return is();
    }

    //______________________________________________________________________________
    inline sal_Bool Pipe::create( const ::rtl::OUString & strName, oslPipeOptions Options  )
    {
        *this = Pipe( strName, Options );
        return is();
    }
    //______________________________________________________________________________
    inline Pipe& SAL_CALL Pipe::operator= (const Pipe& pipe)
    {
        *this = pipe.getHandle();
        return *this;
    }

    //______________________________________________________________________________
    inline Pipe & SAL_CALL Pipe::operator=( oslPipe pipe)
    {
        if( pipe )
            osl_acquirePipe( pipe );
        if( m_handle )
            osl_releasePipe( m_handle );
        m_handle = pipe;
        return *this;
    }

    //______________________________________________________________________________
    inline sal_Bool SAL_CALL Pipe::is() const
    {
        return m_handle != 0;
    }

    //______________________________________________________________________________
    inline sal_Bool SAL_CALL Pipe::operator==( const Pipe& rPipe ) const
    {
        return m_handle == rPipe.m_handle;
    }

    //______________________________________________________________________________
    inline void SAL_CALL Pipe::close()
    {
        osl_closePipe( m_handle );
    }

    //______________________________________________________________________________
    inline void SAL_CALL Pipe::clear()
    {
        if( m_handle )
        {
            osl_releasePipe( m_handle );
            m_handle = 0;
        }
    }

    //______________________________________________________________________________
    inline oslPipeError SAL_CALL Pipe::accept(StreamPipe& Connection)
    {
        Connection = StreamPipe( osl_acceptPipe( m_handle ), SAL_NO_ACQUIRE);
        if( Connection.is() )
            return osl_Pipe_E_None;
        else
            return getError();
    }

    //______________________________________________________________________________
    inline oslPipeError SAL_CALL Pipe::getError() const
    {
        return osl_getLastPipeError( 0 );
    }

    //______________________________________________________________________________
    inline oslPipe SAL_CALL Pipe::getHandle() const
    {
        return m_handle;
    }

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(){}

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(oslPipe hPipe)
        : Pipe( hPipe )
    {
    }

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options, const Security &rSec )
        : Pipe( strName, Options , rSec )
    {}

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options  )
        : Pipe( strName, Options )
    {}

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(const StreamPipe& aPipe)
        : Pipe( aPipe )
    {}
    //______________________________________________________________________________
    inline StreamPipe::StreamPipe( oslPipe pipe, __sal_NoAcquire noacquire )
        : Pipe( pipe , noacquire )
    {}

    //______________________________________________________________________________
    inline sal_Int32 SAL_CALL StreamPipe::read(void* pBuffer, sal_Int32 n) const
    {
        return osl_readPipe( m_handle, pBuffer, n );
    }

    //______________________________________________________________________________
    inline sal_Int32 SAL_CALL StreamPipe::write(const void* pBuffer, sal_Int32 n) const
    {
        return osl_writePipe( m_handle, pBuffer , n );
    }

    //______________________________________________________________________________
    inline sal_Int32 SAL_CALL StreamPipe::recv(void* pBuffer, sal_Int32 BytesToRead) const
    {
        return osl_receivePipe( m_handle, pBuffer , BytesToRead );
    }

    //______________________________________________________________________________
    inline sal_Int32 SAL_CALL StreamPipe::send(const void* pBuffer, sal_Int32 BytesToSend) const
    {
        return osl_sendPipe( m_handle, pBuffer , BytesToSend );
    }

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
