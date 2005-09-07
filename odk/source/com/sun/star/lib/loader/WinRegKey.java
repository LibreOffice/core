/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WinRegKey.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:18:50 $
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

package com.sun.star.lib.loader;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;


/**
 * This class provides functionality for reading string values from the
 * Windows Registry. It requires the native library unowinreg.dll.
 */
final class WinRegKey {

    private String m_rootKeyName;
    private String m_subKeyName;

    // native methods to access the windows registry
    private static native boolean winreg_RegOpenClassesRoot( long[] hkresult );
    private static native boolean winreg_RegOpenCurrentConfig(
        long[] hkresult );
    private static native boolean winreg_RegOpenCurrentUser( long[] hkresult );
    private static native boolean winreg_RegOpenLocalMachine( long[] hkresult );
    private static native boolean winreg_RegOpenUsers( long[] hkresult );
    private static native boolean winreg_RegOpenKeyEx( long parent, String name,
        long[] hkresult );
    private static native boolean winreg_RegCloseKey( long hkey );
    private static native boolean winreg_RegQueryValueEx(
        long hkey, String value, long[] type,
        byte[] data, long[] size );
    private static native boolean winreg_RegQueryInfoKey(
        long hkey, long[] subkeys, long[] maxSubkeyLen,
        long[] values, long[] maxValueNameLen,
        long[] maxValueLen, long[] secDescriptor );

    // load the native library unowinreg.dll
    static {
        try {
            ClassLoader cl = WinRegKey.class.getClassLoader();
            InputStream is = cl.getResourceAsStream( "win/unowinreg.dll" );
            if ( is != null ) {
                // generate a temporary name for lib file and write to temp
                // location
                BufferedInputStream istream = new BufferedInputStream( is );
                File libfile = File.createTempFile( "unowinreg", ".dll" );
                libfile.deleteOnExit(); // ensure deletion
                BufferedOutputStream ostream = new BufferedOutputStream(
                    new FileOutputStream( libfile ) );
                int bsize = 2048; int n = 0;
                byte[] buffer = new byte[bsize];
                while ( ( n = istream.read( buffer, 0, bsize ) ) != -1 ) {
                    ostream.write( buffer, 0, n );
                }
                istream.close();
                ostream.close();
                // load library
                System.load( libfile.getPath() );
            } else {
                // If the library cannot be found as a class loader resource,
                // try the global System.loadLibrary(). The JVM will look for
                // it in the java.library.path.
                System.loadLibrary( "unowinreg" );
            }
        } catch ( java.lang.Exception e ) {
            System.err.println( "com.sun.star.lib.loader.WinRegKey: " +
                "loading of native library failed!" + e );
        }
    }

    /**
     * Constructs a <code>WinRegKey</code>.
     */
    public WinRegKey( String rootKeyName, String subKeyName ) {
        m_rootKeyName = rootKeyName;
        m_subKeyName = subKeyName;
    }

    /**
     * Reads a string value for the specified value name.
     */
    public String getStringValue( String valueName ) throws WinRegKeyException {
        byte[] data = getValue( valueName );
        // remove terminating null character
        return new String( data, 0, data.length - 1 );
    }

    /**
     * Reads a value for the specified value name.
     */
    private byte[] getValue( String valueName ) throws WinRegKeyException {

        byte[] result = null;
        long[] hkey = {0};

        // open the specified registry key
        boolean bRet = false;
        long[] hroot = {0};
        if ( m_rootKeyName.equals( "HKEY_CLASSES_ROOT" ) ) {
            bRet = winreg_RegOpenClassesRoot( hroot );
        } else if ( m_rootKeyName.equals( "HKEY_CURRENT_CONFIG" ) ) {
            bRet = winreg_RegOpenCurrentConfig( hroot );
        } else if ( m_rootKeyName.equals( "HKEY_CURRENT_USER" ) ) {
            bRet = winreg_RegOpenCurrentUser( hroot );
        } else if ( m_rootKeyName.equals( "HKEY_LOCAL_MACHINE" ) ) {
            bRet = winreg_RegOpenLocalMachine( hroot );
        } else if ( m_rootKeyName.equals( "HKEY_USERS" ) ) {
            bRet = winreg_RegOpenUsers( hroot );
        } else {
            throw new WinRegKeyException( "unknown root registry key!");
        }
        if ( !bRet ) {
            throw new WinRegKeyException( "opening root registry key " +
                "failed!" );
        }
        if ( !winreg_RegOpenKeyEx( hroot[0], m_subKeyName, hkey ) ) {
            if ( !winreg_RegCloseKey( hroot[0] ) ) {
                throw new WinRegKeyException( "opening registry key and " +
                    "releasing root registry key handle failed!" );
            }
            throw new WinRegKeyException( "opening registry key failed!" );
        }

        // get the size of the longest data component among the key's values
        long[] subkeys = {0};
        long[] maxSubkeyLen = {0};
        long[] values = {0};
        long[] maxValueNameLen = {0};
        long[] maxValueLen = {0};
        long[] secDescriptor = {0};
        if ( !winreg_RegQueryInfoKey( hkey[0], subkeys, maxSubkeyLen,
                 values, maxValueNameLen, maxValueLen, secDescriptor ) ) {
            if ( !winreg_RegCloseKey( hkey[0] ) ||
                 !winreg_RegCloseKey( hroot[0] ) ) {
                throw new WinRegKeyException( "retrieving information about " +
                    "the registry key and releasing registry key handles " +
                    "failed!" );
            }
            throw new WinRegKeyException( "retrieving information about " +
                "the registry key failed!" );
        }

        // get the data for the specified value name
        byte[] buffer = new byte[ (int) maxValueLen[0] ];
        long[] size = new long[1];
        size[0] = buffer.length;
        long[] type = new long[1];
        type[0] = 0;
        if ( !winreg_RegQueryValueEx( hkey[0], valueName, type, buffer,
                 size ) ) {
            if ( !winreg_RegCloseKey( hkey[0] ) ||
                 !winreg_RegCloseKey( hroot[0] ) ) {
                throw new WinRegKeyException( "retrieving data for the " +
                    "specified value name and releasing registry key handles " +
                    "failed!" );
            }
            throw new WinRegKeyException( "retrieving data for the " +
                "specified value name failed!" );
        }

        // release registry key handles
        if ( !winreg_RegCloseKey( hkey[0] ) ||
             !winreg_RegCloseKey( hroot[0] ) ) {
            throw new WinRegKeyException( "releasing registry key handles " +
                "failed!" );
        }

        result = new byte[ (int) size[0] ];
        System.arraycopy( buffer, 0, result, 0, (int)size[0] );

        return result;
    }
}
