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

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * This class provides functionality for reading string values from the
 * Windows Registry.
 */
final class WinRegKey {

    private final String m_keyName;

    /**
     * Constructs a <code>WinRegKey</code>.
     */
    public WinRegKey( String keyName ) {
        m_keyName = keyName;
    }

    /**
     * Reads the default string value.
     */
    public String getStringValue() throws WinRegKeyException {
        try {
            Process p = Runtime.getRuntime().exec(new String[]{"reg", "QUERY", m_keyName});
            BufferedReader r = new BufferedReader(
                new InputStreamReader(p.getInputStream(), Charset.defaultCharset()));
            String v = null;
            Pattern pt = Pattern.compile("\\s+\\(Default\\)\\s+REG_SZ\\s+(.+)");
            for (;;) {
                String s = r.readLine();
                if (s == null) {
                    break;
                }
                Matcher m = pt.matcher(s);
                if (m.matches()) {
                    if (v != null) {
                        throw new WinRegKeyException("reg QUERY did not provided expected output");
                    }
                    v = m.group(1);
                }
            }
            p.waitFor();
            int e = p.exitValue();
            if (e != 0) {
                throw new WinRegKeyException("reg QUERY exited with " + e);
            }
            if (v == null) {
                throw new WinRegKeyException("reg QUERY did not provided expected output");
            }
            return v;
        } catch (WinRegKeyException e) {
            throw e;
        } catch (Exception e) {
            throw new WinRegKeyException(e);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
