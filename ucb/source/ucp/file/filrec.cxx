/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $::osl::FileBase::RCSfile: file.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:13:55 $
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
 *    ME::osl::FileBase::RCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include "filrec.hxx"

namespace fileaccess {

void ReconnectingFile::disconnect()
{
    m_aFile.close();
    m_bDisconnect = sal_True;
}

sal_Bool ReconnectingFile::reconnect()
{
    sal_Bool bResult = sal_False;
    if ( m_bFlagsSet )
    {
        disconnect();
        if ( m_aFile.open( m_nFlags ) == ::osl::FileBase::E_None
          || m_aFile.open( OpenFlag_Read ) == ::osl::FileBase::E_None )
        {
            m_bDisconnect = sal_False;
            bResult = sal_True;
        }
    }

    return bResult;
}

::osl::FileBase::RC ReconnectingFile::open( sal_uInt32 uFlags )
{
    ::osl::FileBase::RC nResult = m_aFile.open( uFlags );
    if ( nResult == ::osl::FileBase::E_None )
    {
        if ( uFlags & OpenFlag_Create )
            m_nFlags = uFlags & ( ~OpenFlag_Create ) | OpenFlag_Write;
        else
            m_nFlags = uFlags;

        m_bFlagsSet = sal_True;
    }

    return nResult;
}

::osl::FileBase::RC ReconnectingFile::close()
{
    m_nFlags = 0;
    m_bFlagsSet = sal_False;
    m_bDisconnect = sal_False;

    return m_aFile.close();
}

::osl::FileBase::RC ReconnectingFile::setPos( sal_uInt32 uHow, sal_Int64 uPos )
{
    ::osl::FileBase::RC nRes = ::osl::FileBase::E_NETWORK;

    if ( uHow == Pos_Absolut && uPos > 0 )
    {
        if ( m_bDisconnect )
        {
            if ( reconnect() )
                nRes = m_aFile.setPos( uHow, uPos );
        }
        else
        {
            // E_INVAL error code means in this case that
            // the file handler is invalid
            nRes = m_aFile.setPos( uHow, uPos );
            if ( ( nRes == ::osl::FileBase::E_NETWORK
                || nRes == ::osl::FileBase::E_INVAL )
              && reconnect() )
                nRes = m_aFile.setPos( uHow, uPos );
        }
    }
    else
    {
        if ( !m_bDisconnect )
            nRes = m_aFile.setPos( uHow, uPos );
    }

    return nRes;
}

::osl::FileBase::RC ReconnectingFile::getPos( sal_uInt64& uPos )
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.getPos( uPos );
}

::osl::FileBase::RC ReconnectingFile::isEndOfFile( sal_Bool *pIsEOF )
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.isEndOfFile( pIsEOF );
}

::osl::FileBase::RC ReconnectingFile::setSize( sal_uInt64 uSize )
{
    ::osl::FileBase::RC nRes = ::osl::FileBase::E_NETWORK;

    if ( uSize == 0 )
    {
        if ( m_bDisconnect )
        {
            if ( reconnect() )
                nRes = m_aFile.setSize( uSize );
        }
        else
        {
            // E_INVAL error code means in this case that
            // the file handler is invalid
            nRes = m_aFile.setSize( uSize );
            if ( ( nRes == ::osl::FileBase::E_NETWORK
                || nRes == ::osl::FileBase::E_INVAL )
              && reconnect() )
                nRes = m_aFile.setSize( uSize );
        }
    }
    else
    {
        if ( !m_bDisconnect )
            nRes = m_aFile.setSize( uSize );
    }

    return nRes;
}

::osl::FileBase::RC ReconnectingFile::getSize( sal_uInt64 &rSize )
{
    ::osl::FileBase::RC nRes = ::osl::FileBase::E_NETWORK;

    if ( !m_bDisconnect )
        nRes = m_aFile.getSize( rSize );

    // E_INVAL error code means in this case that
    // the file handler is invalid
    if ( ( nRes == ::osl::FileBase::E_NETWORK
        || nRes == ::osl::FileBase::E_INVAL )
      && reconnect() )
    {
        nRes = m_aFile.getSize( rSize );

        // the repairing should be disconnected, since the position might be wrong
        // but the result should be retrieved
        disconnect();
    }

    return nRes;
}

::osl::FileBase::RC ReconnectingFile::read( void *pBuffer, sal_uInt64 uBytesRequested, sal_uInt64& rBytesRead )
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.read( pBuffer, uBytesRequested, rBytesRead );
}

::osl::FileBase::RC ReconnectingFile::write(const void *pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64& rBytesWritten)
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.write( pBuffer, uBytesToWrite, rBytesWritten );
}

::osl::FileBase::RC ReconnectingFile::readLine( ::rtl::ByteSequence& aSeq )
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.readLine( aSeq );
}

::osl::FileBase::RC ReconnectingFile::sync() const
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.sync();
}

} // namespace fileaccess

