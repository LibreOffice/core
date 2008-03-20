/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestHelper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-20 15:27:27 $
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
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package complex.tempfile;

import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.io.*;

import com.sun.star.uno.AnyConverter;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

public class TestHelper {
    LogWriter m_aLogWriter;
    String m_sTestPrefix;

    public TestHelper( LogWriter aLogWriter, String sTestPrefix ) {
        m_aLogWriter = aLogWriter;
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
        if ( sTempFileURL == null || sTempFileURL == "" ) {
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
        if ( sTempFileName == null || sTempFileName == "") {
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
        m_aLogWriter.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage ) {
        m_aLogWriter.println( m_sTestPrefix + sMessage );
    }
}
