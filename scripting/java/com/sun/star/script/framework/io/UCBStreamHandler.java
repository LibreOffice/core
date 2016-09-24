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

package com.sun.star.script.framework.io;

import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XTruncate;

import com.sun.star.script.framework.log.LogUtils;
import com.sun.star.script.framework.provider.PathUtils;

import com.sun.star.ucb.XSimpleFileAccess;

import com.sun.star.uno.UnoRuntime;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandler;

import java.util.HashMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class UCBStreamHandler extends URLStreamHandler {

    public static final String separator = "/ucb/";

    private final XSimpleFileAccess m_xSimpleFileAccess;
    private final HashMap<String, InputStream> m_jarStreamMap = new HashMap<String, InputStream>(12);
    private static String m_ucbscheme;

    public UCBStreamHandler(String scheme, XSimpleFileAccess xSFA) {
        LogUtils.DEBUG("UCBStreamHandler ctor, scheme = " + scheme);
        UCBStreamHandler.m_ucbscheme = scheme;
        this.m_xSimpleFileAccess = xSFA;
    }

    @Override
    public void parseURL(URL url, String spec, int start, int limit) {

        LogUtils.DEBUG("**XUCBStreamHandler, parseURL: " + url + " spec: "
                       + spec + " start: " + start + " limit: " + limit);

        String file = url.getFile();

        if (file == null)
            file =  spec.substring(start, limit);
        else
            file += spec.substring(start, limit);

        LogUtils.DEBUG("**For scheme = " + m_ucbscheme);
        LogUtils.DEBUG("**Setting path = " + file);
        setURL(url, m_ucbscheme, null, -1, null, null, file, null, null);
    }

    @Override
    public URLConnection openConnection(URL u) throws IOException {
        return new UCBConnection(u);
    }

    private class UCBConnection extends URLConnection {

        public UCBConnection(URL url) {
            super(url);
        }

        @Override
        public void connect() {
        }

        @Override
        public InputStream getInputStream() throws IOException {
            LogUtils.DEBUG("UCBConnectionHandler GetInputStream on " + url);
            String sUrl = url.toString();

            if (sUrl.lastIndexOf(separator) == -1) {
                LogUtils.DEBUG("getInputStream straight file load");
                return getFileStreamFromUCB(sUrl);
            } else {
                String path = sUrl.substring(0, sUrl.lastIndexOf(separator));

                String file =
                    sUrl.substring(sUrl.lastIndexOf(separator) + separator.length());

                LogUtils.DEBUG("getInputStream, load of file from another file eg. "
                               + file + " from " + path);

                return getUCBStream(file, path);
            }
        }
        @Override
        public OutputStream getOutputStream() throws IOException {
            LogUtils.DEBUG("UCBConnectionHandler getOutputStream on " + url);
            OutputStream os = null;

            try {
                String sUrl = url.toString();

                if (sUrl.lastIndexOf(separator) != -1) {
                    String path = sUrl.substring(0, sUrl.lastIndexOf(separator));

                    if (m_xSimpleFileAccess.isReadOnly(path)) {
                        throw new java.io.IOException("File is read only");
                    }

                    LogUtils.DEBUG("getOutputStream, create o/p  stream  for file eg. "
                                   + path);

                    // we will only deal with simple file write
                    XOutputStream xos = m_xSimpleFileAccess.openFileWrite(path);

                    XTruncate xtrunc =
                        UnoRuntime.queryInterface(XTruncate.class, xos);

                    if (xtrunc != null) {
                        xtrunc.truncate();
                    }

                    os = new XOutputStreamWrapper(xos);
                }

                if (os == null) {
                    throw new IOException("Failed to get OutputStream for "
                                          + sUrl);
                }
            } catch (com.sun.star.ucb.CommandAbortedException cae) {
                LogUtils.DEBUG("caught exception: " + cae.toString() +
                               " getting writable stream from " + url);
                IOException newEx = new IOException(cae.getMessage());
                newEx.initCause(cae);
                throw newEx;
            } catch (com.sun.star.uno.Exception e) {
                LogUtils.DEBUG("caught unknown exception: " + e.toString() +
                               " getting writable stream from " + url);
                IOException newEx = new IOException(e.getMessage());
                newEx.initCause(e);
                throw newEx;
            }

            return os;
        }
    }

    private InputStream getUCBStream(String file, String path) throws IOException {

        InputStream is = null;
        InputStream result = null;

        try {
            if (path.endsWith(".jar")) {
                is = m_jarStreamMap.get(path);

                if (is == null) {
                    is = getFileStreamFromUCB(path);
                    m_jarStreamMap.put(path, is);
                } else {
                    try {
                        is.reset();
                    } catch (IOException e) {
                        is.close();
                        is = getFileStreamFromUCB(path);
                        m_jarStreamMap.put(path, is);
                    }
                }

                result = getFileStreamFromJarStream(file, is);
            } else {
                String fileUrl = PathUtils.make_url(path, file);
                result = getFileStreamFromUCB(fileUrl);
            }
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException ioe) {
                    LogUtils.DEBUG("Caught exception closing stream: " +
                                   ioe.getMessage());
                }
            }
        }

        return result;
    }

    private InputStream getFileStreamFromJarStream(String file,
            InputStream is) throws
        IOException {

        ZipInputStream zis = new ZipInputStream(is);

        while (zis.available() != 0) {
            ZipEntry entry = zis.getNextEntry();

            if (entry != null && entry.getName().equals(file)) {
                return zis;
            }
        }

        return null;
    }

    private InputStream getFileStreamFromUCB(String path) throws IOException {

        InputStream result = null;
        XInputStream xInputStream = null;

        try {
            LogUtils.DEBUG("Trying to read from " + path);
            xInputStream = m_xSimpleFileAccess.openFileRead(path);
            LogUtils.DEBUG("sfa appeared to read file ");
            byte[][] inputBytes = new byte[1][];

            int sz = m_xSimpleFileAccess.getSize(path);

            // TODO don't depend on result of available() or size()
            // just read stream 'till complete
            if (sz == 0 && xInputStream.available() > 0) {
                sz = xInputStream.available();
            }

            LogUtils.DEBUG("size of file " + path  + " is " + sz);
            LogUtils.DEBUG("available = " + xInputStream.available());
            inputBytes[0] = new byte[sz];

            int ln = xInputStream.readBytes(inputBytes, sz);

            if (ln != sz) {
                throw new IOException(
                    "Failed to read " + sz + " bytes from XInputStream");
            }

            result = new ByteArrayInputStream(inputBytes[0]);
        } catch (com.sun.star.io.IOException ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        } catch (com.sun.star.uno.Exception ex1) {
            IOException ex2 = new IOException();
            ex2.initCause(ex1);
            throw ex2;
        } finally {
            if (xInputStream != null) {
                try {
                    xInputStream.closeInput();
                } catch (Exception e2) {
                    LogUtils.DEBUG("Error closing XInputStream: "
                                   + e2.getMessage());
                }
            }
        }

        return result;
    }
}
