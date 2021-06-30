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
package complex.tempfile;

import com.sun.star.io.*;
import com.sun.star.uno.*;
import com.sun.star.uno.AnyConverter;
import com.sun.star.ucb.XSimpleFileAccess;
import java.io.*;

public class TestHelper {

    private String m_sTestPrefix;

    public TestHelper( String sTestPrefix ) {
        m_sTestPrefix = sTestPrefix;
    }

    public void SetTempFileRemove( XTempFile xTempFile, boolean b ) {
        xTempFile.setRemoveFile( b );
    }

    public String GetTempFileURL ( XTempFile xTempFile ) throws java.lang.Exception {
        String sTempFileURL = AnyConverter.toString( xTempFile.getUri() );
        if ( sTempFileURL == null || sTempFileURL.equals("") ) {
            throw new java.lang.Exception( "Temporary file not valid." );
        }
        return sTempFileURL;
    }

    public String GetTempFileName( XTempFile xTempFile ) throws java.lang.Exception {
        String sTempFileName = AnyConverter.toString( xTempFile.getResourceName() );
        if ( sTempFileName == null || sTempFileName.equals("") ) {
            throw new java.lang.Exception( "Temporary file not valid." );
        }
        return sTempFileName;
    }

    public boolean CompareFileNameAndURL ( String sTempFileName, String sTempFileURL ) throws java.lang.Exception {
        boolean bRet = sTempFileURL.endsWith( sTempFileName.replaceAll( "\\\\" , "/" ) );
        if (!bRet)
            throw new java.lang.Exception("FILE NAME AND URL DO NOT MATCH." );
        return bRet;
    }

    public void WriteBytesWithStream( byte [] pBytes, XTempFile xTempFile ) throws java.lang.Exception {
        XOutputStream xOutTemp = xTempFile.getOutputStream();
        if ( xOutTemp == null )
            throw new java.lang.Exception( "Cannot get output stream." );
        xOutTemp.writeBytes( pBytes );
        xOutTemp.flush();
        Message ( "Write " + pBytes.length + " bytes to tempfile successfully." );
    }

    public void ReadBytesWithStream( byte [][] pBytes, int nBytes, XTempFile xTempFile ) throws java.lang.Exception {
        XInputStream xInTemp = xTempFile.getInputStream();
        if ( xInTemp == null )
            throw new java.lang.Exception( "Cannot get input stream from tempfile." );
        int n = xInTemp.readBytes( pBytes, nBytes );
        Message ( "Read " + n + " bytes from tempfile successfully." );
    }

    public void ReadDirectlyFromTempFile( byte [][] pBytes, int nBytes,  XSimpleFileAccess xSFA, String sTempFileURL )
         throws java.lang.Exception
    {
        Message ( "Attempting to read directly from " + sTempFileURL );
        XInputStream xInTemp = xSFA.openFileRead( sTempFileURL );
        if ( xInTemp == null )
            throw new java.lang.Exception("Cannot create input stream from URL.");
        int n = xInTemp.readBytes( pBytes, nBytes );
        xInTemp.closeInput();
        Message ( "Read " + n + " bytes directly from tempfile successfully. " + sTempFileURL );
    }

    public void CloseTempFile( XTempFile xTempFile ) throws java.lang.Exception {
        XOutputStream xOutTemp = null;
        XInputStream xInTemp = null;
        xOutTemp = xTempFile.getOutputStream();
        if ( xOutTemp == null ) {
            throw new java.lang.Exception( "Cannot get output stream." );
        }
        xOutTemp.closeOutput();
        xInTemp = xTempFile.getInputStream();
        if ( xInTemp == null ) {
            throw new java.lang.Exception( "Cannot get input stream." );
        }
        xInTemp.closeInput();
        Message ( "Tempfile closed successfully." );
    }

    public void KillTempFile ( String sTempFileURL, XSimpleFileAccess xSFA ) throws com.sun.star.uno.Exception {
        xSFA.kill( sTempFileURL );
        Message ( "Tempfile killed successfully." );
    }

    public boolean IfTempFileExists( XSimpleFileAccess xSFA, String sTempFileURL )
        throws com.sun.star.uno.Exception
    {
        boolean bRet = false;
        bRet = xSFA.exists( sTempFileURL );
        Message ( "Tempfile " + ( bRet ? "still " : "no longer " ) + "exists." );
        return bRet;
    }

    public void Message( String sMessage ) {
        System.out.println( m_sTestPrefix + sMessage );
    }
}
