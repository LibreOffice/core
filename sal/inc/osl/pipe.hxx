/*************************************************************************
 *
 *  $RCSfile: pipe.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:28:31 $
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
#ifndef _OSL_PIPE_HXX_
#define _OSL_PIPE_HXX_

#ifndef _OSL_PIPE_DECL_HXX_
#include <osl/pipe_decl.hxx>
#endif

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
    inline Pipe::Pipe( oslPipe pipe, __sal_NoAcquire noacquire )
        : m_handle ( pipe )
    {}

    //______________________________________________________________________________
    inline Pipe::Pipe(oslPipe Pipe)
        : m_handle( Pipe )
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
    inline StreamPipe::StreamPipe(oslPipe Pipe)
        : Pipe( Pipe )
    {}

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options, const Security &rSec )
        : Pipe( strName, Options , rSec )
    {}

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(const ::rtl::OUString& strName, oslPipeOptions Options  )
        : Pipe( strName, Options )
    {}

    //______________________________________________________________________________
    inline StreamPipe::StreamPipe(const StreamPipe& Pipe)
        : Pipe( Pipe )
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
