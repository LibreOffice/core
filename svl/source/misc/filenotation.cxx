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

#include <svl/filenotation.hxx>
#include <osl/file.h>
#include <osl/diagnose.h>
#include <tools/urlobj.hxx>

namespace svt
{

    //= OFileNotation
    OFileNotation::OFileNotation( const OUString& _rUrlOrPath )
    {
        construct( _rUrlOrPath );
    }

    OFileNotation::OFileNotation( const OUString& _rUrlOrPath, NOTATION _eInputNotation )
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

    bool OFileNotation::implInitWithSystemNotation( const OUString& _rSystemPath )
    {
        bool bSuccess = false;

        m_sSystem = _rSystemPath;
        if  (  ( osl_File_E_None != osl_getFileURLFromSystemPath( m_sSystem.pData, &m_sFileURL.pData ) )
            && ( m_sFileURL.isEmpty() )
            )
        {
            if ( !_rSystemPath.isEmpty() )
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

    bool OFileNotation::implInitWithURLNotation( const OUString& _rURL )
    {
        m_sFileURL = _rURL;
        osl_getSystemPathFromFileURL( _rURL.pData, &m_sSystem.pData );
        return true;
    }

    void OFileNotation::construct( const OUString& _rUrlOrPath )
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
        (void)bSuccess;
    }

    OUString OFileNotation::get(NOTATION _eOutputNotation)
    {
        switch (_eOutputNotation)
        {
            case N_SYSTEM: return m_sSystem;
            case N_URL: return m_sFileURL;
        }

        OSL_FAIL("OFileNotation::get: invalid enum value!");
        return OUString();
    }

}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
