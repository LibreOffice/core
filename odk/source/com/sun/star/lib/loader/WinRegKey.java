/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

    private final String m_rootKeyName;
    private final String m_subKeyName;

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
                File libfile;
                BufferedInputStream istream = null;
                BufferedOutputStream ostream = null;
                try {
                    istream = new BufferedInputStream( is );
                    libfile = File.createTempFile( "unowinreg", ".dll" );
                    libfile.deleteOnExit(); // ensure deletion
                    ostream = new BufferedOutputStream(
                        new FileOutputStream( libfile ) );
                    int bsize = 2048; int n = 0;
                    byte[] buffer = new byte[bsize];
                    while ( ( n = istream.read( buffer, 0, bsize ) ) != -1 ) {
                        ostream.write( buffer, 0, n );
                    }
                } finally {
                    if (istream != null) {
                        istream.close();
                    }
                    if (ostream != null) {
                        ostream.close();
                    }
                }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
