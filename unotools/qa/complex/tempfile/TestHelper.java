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

import com.sun.star.uno.AnyConverter;
import com.sun.star.ucb.XSimpleFileAccess;


public class TestHelper {

    private String m_sTestPrefix;

    public TestHelper( String sTestPrefix ) {

        m_sTestPrefix = sTestPrefix;
    }
    public void SetTempFileRemove( XTempFile xTempFile, boolean b ) {
        try {
            xTempFile.setRemoveFile( b );
        } catch( Exception e ) {
            Error( "Cannot set TempFileRemove. exception: " + e );
        }
    }

    public String GetTempFileURL ( XTempFile xTempFile ) {
        String sTempFileURL = null;
        try {
            sTempFileURL = AnyConverter.toString( xTempFile.getUri() );
        } catch (Exception e) {
            Error ( "Cannot get TempFileURL. exception: " + e );
        }
        if ( sTempFileURL == null || sTempFileURL.equals("") ) {
            Error ( "Temporary file not valid." );
        }
        return sTempFileURL;
    }

    public String GetTempFileName( XTempFile xTempFile ) {
        String sTempFileName = null;
        try {
            sTempFileName = AnyConverter.toString( xTempFile.getResourceName() );
        } catch ( Exception e ) {
            Error( "Cannot get TempFileName. exception: " + e );
        }
        if ( sTempFileName == null || sTempFileName.equals("") ) {
            Error( "Temporary file not valid." );
        }
        return sTempFileName;
    }

    public boolean CompareFileNameAndURL ( String sTempFileName, String sTempFileURL ) {
        boolean bRet = false;
        try {
            bRet = sTempFileURL.endsWith( sTempFileName.replaceAll( "\\\\" , "/" ) );
            Message ( "Compare file name and URL: " +
                    ( bRet ? "OK." : "ERROR: FILE NAME AND URL DO NOT MATCH." ) );
        }
        catch ( Exception e ) {
            Error ( "exception: " + e);
        }
        return bRet;
    }

    public void WriteBytesWithStream( byte [] pBytes, XTempFile xTempFile ) {
        try {
            XOutputStream xOutTemp = xTempFile.getOutputStream();
            if ( xOutTemp == null ) {
                Error( "Cannot get output stream." );
            } else {
                xOutTemp.writeBytes( pBytes );
                Message ( "Write to tempfile successfully." );
            }
        } catch ( Exception e ) {
            Error( "Cannot write to stream. exception: " + e );
        }
    }

    public void ReadBytesWithStream( byte [][] pBytes, int nBytes, XTempFile xTempFile ) {
        try {
            XInputStream xInTemp = xTempFile.getInputStream();
            if ( xInTemp == null ) {
                Error( "Cannot get input stream from tempfile." );
            } else {
                xInTemp.readBytes( pBytes, nBytes );
                Message ( "Read from tempfile successfully." );
            }
        } catch ( Exception e ) {
            Error( "Cannot read from stream. exception: " + e );
        }
    }
    public void ReadDirectlyFromTempFile( byte [][] pBytes, int nBytes,  XSimpleFileAccess xSFA, String sTempFileURL )
    {
        try
        {
            if ( xSFA != null ) {
                XInputStream xInTemp = xSFA.openFileRead( sTempFileURL );
                if ( xInTemp != null )
                {
                    xInTemp.readBytes( pBytes, nBytes );
                    xInTemp.closeInput();
                    Message ( "Read directly from tempfile successfully." );
                } else {
                    Error ( "Cannot create input stream from URL." );
                }
            }
        }
        catch ( Exception e)
        {
            Error( "Exception caught in TestHelper." +
                    "ReadDirectlyFromTempFile(). exception: " + e );
        }
    }

    public void CloseTempFile( XTempFile xTempFile ) {
        XOutputStream xOutTemp = null;
        XInputStream xInTemp = null;
        try {
            xOutTemp = xTempFile.getOutputStream();
            if ( xOutTemp == null ) {
                Error( "Cannot get output stream." );
            }
        } catch ( Exception e ) {
            Error( "Cannot get output stream. exception:" + e );
        }
        try {
            xOutTemp.closeOutput();
        } catch( Exception e ) {
            Error( "Cannot close output stream. exception:" + e );
        }
        try {
            xInTemp = xTempFile.getInputStream();
            if ( xInTemp == null ) {
                Error( "Cannot get input stream." );
            }
        } catch ( Exception e ) {
            Error( "Cannot get input stream. exception:" + e );
        }
        try {
            xInTemp.closeInput();
            Message ( "Tempfile closed successfully." );
        } catch( Exception e ) {
            Error( "Cannot close input stream. exception:" + e );
        }
    }

    public void KillTempFile ( String sTempFileURL, XSimpleFileAccess xSFA ) {
        try {
            if ( sTempFileURL != null && xSFA != null ) {
                xSFA.kill( sTempFileURL );
                Message ( "Tempfile killed successfully." );
            }
        }
        catch ( Exception e ) {
            Error ( "Exception caught in TestHelper." +
                    "KillTempFile(): " + e);
        }
    }

    public boolean IfTempFileExists( XSimpleFileAccess xSFA, String sTempFileURL ) {
        boolean bRet = false;
        try {
            if ( sTempFileURL != null && xSFA != null ) {
                bRet = xSFA.exists( sTempFileURL );
                Message ( "Tempfile " + ( bRet ? "still " : "no longer " ) + "exists." );
            }
        }
        catch( Exception e ) {
            Error( "Exception caught in TestHelper." +
                    "IfTempFileExists(): " + e );
        }
        return bRet;
    }

    public void Error( String sError ) {
        System.out.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage ) {
        System.out.println( m_sTestPrefix + sMessage );
    }
}
