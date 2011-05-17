/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
        (void)bSuccess;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OFileNotation::get(NOTATION _eOutputNotation)
    {
        switch (_eOutputNotation)
        {
            case N_SYSTEM: return m_sSystem;
            case N_URL: return m_sFileURL;
        }

        OSL_FAIL("OFileNotation::get: inavlid enum value!");
        return ::rtl::OUString();
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
