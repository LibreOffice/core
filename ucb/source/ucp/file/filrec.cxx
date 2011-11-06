/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            m_nFlags = (uFlags & ( ~OpenFlag_Create )) | OpenFlag_Write;
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

::osl::FileBase::RC ReconnectingFile::sync() const
{
    if ( m_bDisconnect )
        return ::osl::FileBase::E_NETWORK;

    return m_aFile.sync();
}

} // namespace fileaccess

