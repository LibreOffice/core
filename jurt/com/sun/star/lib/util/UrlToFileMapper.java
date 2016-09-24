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
package com.sun.star.lib.util;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLEncoder;

/**
 * Maps Java URL representations to File representations, on any Java version.
 *
 * This used to be used to do URL to File mapping in pre-Java1.4 days, but since
 * we now require Java 1.5, it is largely unnecessary.
 *
 * @since UDK 3.2.8
 */
public final class UrlToFileMapper {

    /**
     * Maps Java URL representations to File representations.
     *
     * @param url some URL, possibly null.
     * @return a corresponding File, or null on failure.
     */
    public static File mapUrlToFile(URL url) {
        if (url == null) {
            return null;
        } else {
            try {
                // where encodedUrl is url.toString(), but since that may contain
                // unsafe characters (e.g., space, " "), it is encoded, as otherwise
                // the URI constructor might throw java.net.URISyntaxException (in
                // Java 1.5, URL.toURI might be used instead).
                String encodedUrl = encode(url.toString());
                URI uri = new URI(encodedUrl);
                try {
                    return new File(uri);
                } catch (IllegalArgumentException e) {
                    return null;
                }
            } catch (URISyntaxException ex) {
                throw new RuntimeException(ex); // should never happen
            }
        }
    }

    private static String encode(String url) {
        StringBuffer buf = new StringBuffer(url.length());
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
                try {
                    String enc = URLEncoder.encode(Character.toString(c), "UTF-8");
                    buf.append(enc);
                } catch (UnsupportedEncodingException e) {
                    throw new RuntimeException(e); // should never happen
                }
            }
        }
        return buf.toString();
    }

    private UrlToFileMapper() {}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
