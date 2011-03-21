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
    private static Constructor uriConstructor;
    private static Constructor fileConstructor;
    static {
        try {
            urlEncoderEncode = URLEncoder.class.getMethod(
                "encode", new Class[] { String.class, String.class });
            Class uriClass = Class.forName("java.net.URI");
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
            // If java.net.URI is avaliable, do
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
                    return (File) fileConstructor.newInstance(
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
