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
package com.sun.star.lib.util;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLDecoder;
import java.net.URLEncoder;

/**
 * Maps Java URL representations to File representations, on any Java version.
 *
 * @since UDK 3.2.8
 */
public final class UrlToFileMapper {

    // java.net.URLEncoder.encode(String, String) and java.net.URI are only
    // available since Java 1.4:
    private static Method urlEncoderEncode;
    private static Constructor<?> uriConstructor;
    private static Constructor<File> fileConstructor;
    static {
        try {
            urlEncoderEncode = URLEncoder.class.getMethod(
                "encode", new Class[] { String.class, String.class });
            Class<?> uriClass = Class.forName("java.net.URI");
            uriConstructor = uriClass.getConstructor(
                new Class[] { String.class });
            fileConstructor = File.class.getConstructor(
                new Class[] { uriClass });
        } catch (ClassNotFoundException e) {
        } catch (NoSuchMethodException e) {
        }
    }

    /**
     * Maps Java URL representations to File representations.
     *
     * @param url some URL, possibly null.
     * @return a corresponding File, or null on failure.
     */
    public static File mapUrlToFile(URL url) {
        if (url == null) {
            return null;
        } else if (fileConstructor == null) {
            // If java.net.URI is not available, hope that the following works
            // well:  First, check that the given URL has a certain form.
            // Second, use the URLDecoder to decode the URL path (taking care
            // not to change any plus signs to spaces), hoping that the used
            // default encoding is the proper one for file URLs.  Third, create
            // a File from the decoded path.
            return url.getProtocol().equalsIgnoreCase("file")
                && url.getAuthority() == null && url.getQuery() == null
                && url.getRef() == null
                ? new File(URLDecoder.decode(
                               StringHelper.replace(url.getPath(), '+', "%2B")))
                : null;
        } else {
            // If java.net.URI is available, do
            //   URI uri = new URI(encodedUrl);
            //   try {
            //       return new File(uri);
            //   } catch (IllegalArgumentException e) {
            //       return null;
            //   }
            // where encodedUrl is url.toString(), but since that may contain
            // unsafe characters (e.g., space, " "), it is encoded, as otherwise
            // the URI constructor might throw java.net.URISyntaxException (in
            // Java 1.5, URL.toURI might be used instead).
            String encodedUrl = encode(url.toString());
            try {
                Object uri = uriConstructor.newInstance(
                    new Object[] { encodedUrl });
                try {
                    return fileConstructor.newInstance(
                        new Object[] { uri });
                } catch (InvocationTargetException e) {
                    if (e.getTargetException() instanceof
                        IllegalArgumentException) {
                        return null;
                    } else {
                        throw e;
                    }
                }
            } catch (InstantiationException e) {
                throw new RuntimeException("This cannot happen: " + e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException("This cannot happen: " + e);
            } catch (InvocationTargetException e) {
                if (e.getTargetException() instanceof Error) {
                    throw (Error) e.getTargetException();
                } else if (e.getTargetException() instanceof RuntimeException) {
                    throw (RuntimeException) e.getTargetException();
                } else {
                    throw new RuntimeException("This cannot happen: " + e);
                }
            }
        }
    }



    private static String encode(String url) {
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < url.length(); ++i) {
            char c = url.charAt(i);
            // The RFC 2732 <uric> characters: !$&'()*+,-./:;=?@[]_~ plus digits
            // and letters; additionally, do not encode % again.
            if (c >= 'a' && c <= 'z' || c >= '?' && c <= '['
                || c >= '$' && c <= ';' || c == '!' || c == '=' || c == ']'
                || c == '_' || c == '~')
            {
                buf.append(c);
            } else if (c == ' ') {
                buf.append("%20");
            } else {
                String enc;
                try {
                    enc = (String) urlEncoderEncode.invoke(
                        null,
                        new Object[] {Character.toString(c), "UTF-8" });
                } catch (IllegalAccessException e) {
                    throw new RuntimeException("This cannot happen: " + e);
                } catch (InvocationTargetException e) {
                    throw new RuntimeException("This cannot happen: " + e);
                }
                buf.append(enc);
            }
        }
        return buf.toString();
    }

    private UrlToFileMapper() {}
}
