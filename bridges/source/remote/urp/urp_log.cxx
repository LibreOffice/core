/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_log.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"
#include <osl/time.h>
#include "urp_bridgeimpl.hxx"
#include "urp_log.hxx"

using namespace ::rtl;
using namespace ::osl;
namespace bridges_urp
{
#ifdef BRIDGES_URP_PROT
    Mutex g_logFileMutex;

    class FileAccess
    {
    public:
        FileAccess( urp_BridgeImpl *pBridgeImpl_ ) :
            pBridgeImpl( pBridgeImpl_ ),
            guard( g_logFileMutex )
            {
                if( pBridgeImpl->m_pLogFile )
                {
                    f = pBridgeImpl->m_pLogFile;
                }
                else
                {
                    f = fopen( pBridgeImpl->m_sLogFileName.getStr() , "a" );
                }
            }
        ~FileAccess()
            {
                if( ! pBridgeImpl->m_pLogFile )
                {
                    fclose( f );
                }
            }
        FILE *getFile()
            {
                return f;
            }
    private:
        urp_BridgeImpl *pBridgeImpl;
        MutexGuard guard;
        FILE *f;
    };

    void urp_logCall( urp_BridgeImpl *pBridgeImpl, sal_Int32 nSize, sal_Int32 nUseData, sal_Bool bSynchron ,
                      const ::rtl::OUString &sMethodName )
    {
        if( pBridgeImpl->m_sLogFileName.getLength() && getenv( "PROT_REMOTE_ACTIVATE" ) )
        {
            OString sOperation = OUStringToOString( sMethodName,RTL_TEXTENCODING_ASCII_US );

            FileAccess access( pBridgeImpl );
            fprintf( access.getFile() ,
                     "%06u: calling [size=%d(usedata=%d)] [synchron=%d] [name=%s]\n" ,
                     static_cast< unsigned int > (osl_getGlobalTimer()),
                     static_cast< int > (nSize),
                     static_cast< int > (nUseData),
                     bSynchron, sOperation.pData->buffer );
        }
    }

    void urp_logServingRequest( urp_BridgeImpl *pBridgeImpl,
                                sal_Int32 nSize, sal_Int32 nUseData, sal_Bool bSynchron ,
                                const ::rtl::OUString &sMethodName )
    {
        if( pBridgeImpl->m_sLogFileName.getLength() && getenv( "PROT_REMOTE_ACTIVATE" ) )
        {
            OString sOperation = OUStringToOString( sMethodName,RTL_TEXTENCODING_ASCII_US );

            FileAccess access( pBridgeImpl );
            fprintf(
                access.getFile(),
                "%06u: serving request [size=%d(usedata=%d)] [synchron=%d] [name=%s]\n",
                static_cast< unsigned int > (osl_getGlobalTimer()),
                static_cast< int > (nSize),
                static_cast< int > (nUseData),
                bSynchron,
                sOperation.pData->buffer
                );
        }
    }

    void urp_logGettingReply( urp_BridgeImpl *pBridgeImpl,
                              sal_Int32 nSize, sal_Int32 nUseData,
                              const ::rtl::OUString &sMethodName )
    {
        if( pBridgeImpl->m_sLogFileName.getLength() && getenv( "PROT_REMOTE_ACTIVATE" ) )
        {
            OString sOperation = OUStringToOString( sMethodName,RTL_TEXTENCODING_ASCII_US );
            FileAccess access( pBridgeImpl );
            fprintf( access.getFile(),
                     "%06u: getting reply [size=%d(usedata=%d)][name=%s]\n" ,
                     static_cast< unsigned int > (osl_getGlobalTimer()),
                     static_cast< int > (nSize),
                     static_cast< int > (nUseData),
                     sOperation.pData->buffer);
        }
    }

    void urp_logReplying( urp_BridgeImpl *pBridgeImpl,
                          sal_Int32 nSize , sal_Int32 nUseData,
                          const ::rtl::OUString &sMethodName )
    {
        if( pBridgeImpl->m_sLogFileName.getLength() && getenv( "PROT_REMOTE_ACTIVATE" ) )
        {
            OString sOperation = OUStringToOString(sMethodName,RTL_TEXTENCODING_ASCII_US);

            FileAccess access( pBridgeImpl );
            fprintf( access.getFile(),
                     "%06u: replying [size=%d(usedata=%d)] [name=%s]\n",
                     static_cast< unsigned int > (osl_getGlobalTimer()),
                     static_cast< int > (nSize),
                     static_cast< int > (nUseData),
                     sOperation.pData->buffer);
        }
    }
#endif
}
