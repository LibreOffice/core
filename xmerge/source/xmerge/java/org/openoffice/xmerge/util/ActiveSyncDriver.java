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
               
        ConverterInfoReader cirPlugin = new ConverterInfoReader(pluginJar.toURL().toString(), false);     
        
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
            return false;
        }
        
        if (dataOut == null) {
            return false;
        }

        // Get the document and write it out.
        Document doc = (Document)dataOut.getDocumentEnumeration().nextElement();      
        if (doc == null) {
            return false;
        }
        
        doc.write(fos);
        fos.flush();
        fos.close();
        
        return true;         
    }
}
