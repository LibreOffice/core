/************************************************************************
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

package org.openoffice.xmerge.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.util.StringTokenizer;

import java.net.URL;

import org.openoffice.xmerge.Convert;
import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConverterFactory;
import org.openoffice.xmerge.util.registry.ConverterInfo;
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
         */
        File jarWord = new File(ooClassDir + "pocketword.jar");
        File jarExcel = new File(ooClassDir + "pexcel.jar");

        ConverterInfoReader cirWord  = new ConverterInfoReader(jarWord.toURL().toString(), false);
        ConverterInfoReader cirExcel = new ConverterInfoReader(jarExcel.toURL().toString(), false);

        ConverterInfoMgr.addPlugIn(cirWord.getConverterInfoEnumeration());
        ConverterInfoMgr.addPlugIn(cirExcel.getConverterInfoEnumeration());

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
