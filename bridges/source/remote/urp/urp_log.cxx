/*************************************************************************
 *
 *  $RCSfile: urp_log.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2001-02-27 18:01:31 $
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
                     "%06d: calling [size=%d(usedata=%d)] [synchron=%d] [name=%s]\n" ,
                     osl_getGlobalTimer(),
                     nSize, nUseData, bSynchron, sOperation.pData->buffer );
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
                "%06d: serving request [size=%d(usedata=%d)] [synchron=%d] [name=%s]\n",
                osl_getGlobalTimer(),
                nSize,
                nUseData,
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
                     "%06d: getting reply [size=%d(usedata=%d)][name=%s]\n" ,
                     osl_getGlobalTimer(),
                     nSize, nUseData,
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
                     "%06d: replying [size=%d(usedata=%d)] [name=%s]\n",
                     osl_getGlobalTimer(),
                     nSize, nUseData,
                     sOperation.pData->buffer);
        }
    }
#endif
}
