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
package complex.tempfile;



import com.sun.star.io.*;

import com.sun.star.uno.AnyConverter;
import com.sun.star.ucb.XSimpleFileAccess;


public class TestHelper {

    String m_sTestPrefix;

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

    public boolean GetTempFileRemove ( XTempFile xTempFile ) {
        boolean b = false;
        try {
            b = xTempFile.getRemoveFile();
        } catch( Exception e) {
            Error( "Cannot get TempFileRemove. exception: " + e );
        }
        return b;
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
                    Error ( "Cannot creat input stream from URL." );
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
            if ( sTempFileURL != null ) {
                if ( xSFA != null ) {
                    xSFA.kill( sTempFileURL );
                    Message ( "Tempfile killed successfully." );
                }
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
            if ( sTempFileURL != null ) {
                if ( xSFA != null ) {
                    bRet = xSFA.exists( sTempFileURL );
                    Message ( "Tempfile " + ( bRet ? "still " : "no longer " ) + "exists." );
                }
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
