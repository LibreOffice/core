/*************************************************************************
*
*  $RCSfile: UCBStreamHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: svesik $ $Date: 2004-04-19 23:07:24 $
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

package com.sun.star.script.framework.io;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;

import com.sun.star.uno.XComponentContext;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.io.XInputStream;

import com.sun.star.script.framework.log.LogUtils;

public class UCBStreamHandler extends URLStreamHandler {

    public final static String UCBSCHEME = "vnd.sun.star.pkg";
    public final static String separator = "/ucb/";

    private XComponentContext m_xContext = null;
    private XMultiComponentFactory m_xMultiComponentFactory = null;
    private XSimpleFileAccess m_xSimpleFileAccess = null;
    private HashMap m_jarStreamMap = new HashMap(12);

    public UCBStreamHandler(XComponentContext ctxt) {
        this.m_xContext = ctxt;
        this.m_xMultiComponentFactory = m_xContext.getServiceManager();

        try {
            Object obj = m_xMultiComponentFactory.createInstanceWithContext(
                "com.sun.star.ucb.SimpleFileAccess", ctxt);
            m_xSimpleFileAccess = (XSimpleFileAccess)
                UnoRuntime.queryInterface( XSimpleFileAccess.class, obj );
        }
        catch (Exception e) {}
    }

    public void parseURL(URL url, String spec, int start, int limit) {
        String file = url.getFile();
        if (file == null)
            file = spec.substring(start, limit);
        else
            file += spec.substring(start, limit);

        setURL(url, url.getProtocol(), null, -1, null, null, file, null, null);
    }

    public URLConnection openConnection(URL u) throws IOException {
        return new UCBConnection(u);
    }

    private class UCBConnection extends URLConnection {

        public UCBConnection(URL url) {
            super(url);
        }

        public void connect() {
        }

        public InputStream getInputStream() throws IOException {
            String url = UCBSCHEME + ":" + getURL().getFile().toString();
            if (url.lastIndexOf(separator) == -1) {
                return getFileStreamFromUCB(url);
            }
            else {
                String path = url.substring(0, url.lastIndexOf(separator));
                String file = url.substring(
                    url.lastIndexOf(separator) + separator.length());

                // System.out.println("[Loading: " + file + " from: " +
                //     path.substring(path.lastIndexOf("/")) + "]");

                return getUCBStream(file, path);
            }
        }
    }

    private InputStream getUCBStream(String file, String path)
        throws IOException
    {
        InputStream is = null;
        InputStream result = null;

        try {
            if (path.endsWith(".jar")) {
                is = (InputStream)m_jarStreamMap.get(path);

                if (is == null) {
                    is = getFileStreamFromUCB(path);
                    m_jarStreamMap.put(path, is);
                }
                else {
                    try {
                        is.reset();
                    }
                    catch (IOException e) {
                        is.close();
                        is = getFileStreamFromUCB(path);
                        m_jarStreamMap.put(path, is);
                    }
                }
                result = getFileStreamFromJarStream(file, is);
            }
            else
            {
                path += file;
                result = getFileStreamFromUCB(path);
            }
        }
        finally {
            if (is != null) {
                try {
                    is.close();
                }
                catch (IOException ioe) {
                    LogUtils.DEBUG("Caught exception closing stream: " +
                        ioe.getMessage());
                }
            }
        }
        return result;
    }

    private InputStream getFileStreamFromJarStream(String file, InputStream is)
        throws IOException
    {
        ZipInputStream zis = null;
        ZipEntry entry = null;
        boolean found = false;

        zis = new ZipInputStream(is);

        while (zis.available() != 0) {
            entry = (ZipEntry)zis.getNextEntry();

            if (entry.getName().equals(file)) {
                return zis;
            }
        }
        return null;
    }

    private InputStream getFileStreamFromUCB(String path)
        throws IOException
    {
        InputStream result = null;
        XInputStream xInputStream = null;

        try {
            xInputStream = m_xSimpleFileAccess.openFileRead(path);
            byte[][] inputBytes = new byte[1][];

            int ln = 0;
            int sz = m_xSimpleFileAccess.getSize(path);
            inputBytes[0] = new byte[sz];

            ln = xInputStream.readBytes(inputBytes, sz);

            if (ln != sz) {
                throw new IOException(
                    "Failed to read " + sz + " bytes from XInputStream");
            }

            result = new ByteArrayInputStream(inputBytes[0]);
        }
        catch (com.sun.star.io.IOException ioe) {
            throw new IOException(ioe.getMessage());
        }
        catch (com.sun.star.uno.Exception e) {
            throw new IOException(e.getMessage());
        }
        finally
        {
            if (xInputStream != null) {
                try {
                    xInputStream.closeInput();
                }
                catch (Exception e2) {
                    LogUtils.DEBUG(
                        "Error closing XInputStream:" + e2.getMessage());
                }
            }
        }
        return result;
    }

    private String convertClassNameToFileName(String name) {
        return name.replace('.', File.separatorChar) + ".class";
    }
}
