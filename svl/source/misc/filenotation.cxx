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
#include "precompiled_svl.hxx"
#include <svl/filenotation.hxx>
#include <osl/file.h>
#include <osl/diagnose.h>
#include <tools/urlobj.hxx>

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OFileNotation
    //=====================================================================
    //---------------------------------------------------------------------
    OFileNotation::OFileNotation( const ::rtl::OUString& _rUrlOrPath )
    {
        construct( _rUrlOrPath );
    }

    //---------------------------------------------------------------------
    OFileNotation::OFileNotation( const ::rtl::OUString& _rUrlOrPath, NOTATION _eInputNotation )
    {
        if ( _eInputNotation == N_URL )
        {
            INetURLObject aParser( _rUrlOrPath );
            if ( aParser.GetProtocol() == INET_PROT_FILE )
                implInitWithURLNotation( _rUrlOrPath );
            else
                m_sSystem = m_sFileURL = _rUrlOrPath;
        }
        else
            implInitWithSystemNotation( _rUrlOrPath );
    }

    //---------------------------------------------------------------------
    bool OFileNotation::implInitWithSystemNotation( const ::rtl::OUString& _rSystemPath )
    {
        bool bSuccess = false;

        m_sSystem = _rSystemPath;
        if  (  ( osl_File_E_None != osl_getFileURLFromSystemPath( m_sSystem.pData, &m_sFileURL.pData ) )
            && ( 0 == m_sFileURL.getLength() )
            )
        {
            if ( _rSystemPath.getLength() )
            {
                INetURLObject aSmartParser;
                aSmartParser.SetSmartProtocol( INET_PROT_FILE );
                if ( aSmartParser.SetSmartURL( _rSystemPath ) )
                {
                    m_sFileURL = aSmartParser.GetMainURL( INetURLObject::NO_DECODE );
                    osl_getSystemPathFromFileURL( m_sFileURL.pData, &m_sSystem.pData );
                    bSuccess = true;
                }
            }
        }
        else
            bSuccess = true;
        return bSuccess;
    }

    //---------------------------------------------------------------------
    bool OFileNotation::implInitWithURLNotation( const ::rtl::OUString& _rURL )
    {
        m_sFileURL = _rURL;
        osl_getSystemPathFromFileURL( _rURL.pData, &m_sSystem.pData );
        return true;
    }

    //---------------------------------------------------------------------
    void OFileNotation::construct( const ::rtl::OUString& _rUrlOrPath )
    {
        bool bSuccess = false;
        // URL notation?
        INetURLObject aParser( _rUrlOrPath );
        switch ( aParser.GetProtocol() )
        {
            case INET_PROT_FILE:
                // file URL
                bSuccess = implInitWithURLNotation( _rUrlOrPath );
                break;

            case INET_PROT_NOT_VALID:
                // assume system notation
                bSuccess = implInitWithSystemNotation( _rUrlOrPath );
                break;

            default:
                // it's a known scheme, but no file-URL -> assume that bothe the URL representation and the
                // system representation are the URL itself
                m_sSystem = m_sFileURL = _rUrlOrPath;
                bSuccess = true;
                break;
        }

        OSL_ENSURE( bSuccess, "OFileNotation::OFileNotation: could not detect the format!" );
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFileNotation::get(NOTATION _eOutputNotation)
    {
        switch (_eOutputNotation)
        {
            case N_SYSTEM: return m_sSystem;
            case N_URL: return m_sFileURL;
        }

        OSL_ENSURE(sal_False, "OFileNotation::get: inavlid enum value!");
        return ::rtl::OUString();
    }

//.........................................................................
}   // namespace svt
//.........................................................................

