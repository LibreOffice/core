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

package org.openoffice.xmerge.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.util.StringTokenizer;

import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConverterFactory;
import org.openoffice.xmerge.util.registry.ConverterInfoMgr;
import org.openoffice.xmerge.util.registry.ConverterInfoReader;

public class ActiveSyncDriver {
    public static void main(String[] args) {
        if (args.length != 4) {
            return;
        }

        ActiveSyncDriver asd = new ActiveSyncDriver();

        try {
            // At the moment can't really signal back to the calling DLL
            asd.Convert(args[0], args[1], args[2], args[3]);
        }
        catch (Exception e) {
            return;
        }
    }


    private boolean Convert(String srcMime, String dstMime, String srcFile, String dstFile) throws Exception {
        /*
         * The classpath passed in by XMergeSync.dll should contain all of the
         * jar files, but at the least it will contain xmerge.jar, so strip off
         * the xmerge.jar portion and use the remainder to provide a root for
         * the Pocket Word and Pocket Excel plugins.
         */
        String ooClassDir = null;
        String strClassPath = System.getProperty("java.class.path");

        StringTokenizer st = new StringTokenizer(strClassPath, ";");

        // There should be at least one element, but just in case
        while (st.hasMoreTokens()) {
            String s = st.nextToken();

            if (s.endsWith("xmerge.jar")) {
                ooClassDir = s.substring(0, s.indexOf("xmerge.jar"));
            }
        }

        if (ooClassDir == null) {
            return true;
        }


        /*
         * The XMergeSync.dll should will have checked for the presence of the
         * jars at the same location already.
         *
         * Because they can be installed separately, though, the MIME types need
         * to be check to see which one to load.
         */
        File pluginJar = null;
        if (srcMime.equals("staroffice/sxw") || srcMime.equals("application/x-pocket-word"))
        {
            pluginJar = new File(ooClassDir + "pocketWord.jar");
        }
        else if (srcMime.equals("staroffice/sxc") || srcMime.equals("application/x-pocket-excel"))
        {
            pluginJar = new File(ooClassDir + "pexcel.jar");
        }

        ConverterInfoReader cirPlugin = new ConverterInfoReader(pluginJar.toURI().toURL().toString(), false);

        ConverterInfoMgr.addPlugIn(cirPlugin.getConverterInfoEnumeration());

        ConverterFactory cf = new ConverterFactory();
        Convert conv = cf.getConverter(srcMime, dstMime);

        if (conv == null) {
            return false;
        }

        // Everything is registered so do the conversion
        FileInputStream fis = new FileInputStream(srcFile);
        FileOutputStream fos = new FileOutputStream(dstFile);

        conv.addInputStream(srcFile, fis);

        ConvertData dataOut;
        try {
            dataOut = conv.convert();
        }
        catch (Exception e) {
            fos.close();
            return false;
        }

        if (dataOut == null) {
            fos.close();
            return false;
        }

        // Get the document and write it out.
        Document doc = (Document)dataOut.getDocumentEnumeration().next();
        if (doc == null) {
            fos.close();
            return false;
        }

        doc.write(fos);
        fos.flush();
        fos.close();

        return true;
    }
}
