/*************************************************************************
 *
 *  $RCSfile: FileTools.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2005-02-02 13:55:42 $
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
package helper;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import lib.TestParameters;

import util.DesktopTools;


/**
 * This class deliver some functionality to copy files.
 */
public class FileTools {

    /**
     * Copies all files under srcDir to dstDir.
     * If dstDir does not exist, it will be created.
     * @param srcDir the source directory
     * @param dstDir the destination direcotry
     * @throws java.io.IOException throws java.io.IOException if something failes
     */
    public static void copyDirectory(File srcDir, File dstDir)
           throws java.io.IOException {
        copyDirectory(srcDir, dstDir, new String[]{});
    }
    /**
     * Copies all files under srcDir to dstDir except Files given in the
     * ignore list. This files will not be copied.
     * If dstDir does not exist, it will be created.
     * @param srcDir the source directory
     * @param dstDir the destination direcotry
     * @param ignore a list of files which should not be copied
     * @throws java.io.IOException throws java.io.IOException if something failes
     */
    public static void copyDirectory(File srcDir, File dstDir, String[] ignore)
           throws java.io.IOException {

        for (int i=0; i<ignore.length;i++){
            if (srcDir.getName().endsWith(ignore[i])) {
                return;
            }
        }

        if (srcDir.isDirectory()) {
            if (!dstDir.exists()) {
                dstDir.mkdir();
            }

            String[] files = srcDir.list();
            for (int i=0; i< files.length; i++) {
                copyDirectory(new File(srcDir, files[i]), new File(dstDir, files[i]), ignore);
            }
        } else {
            // This method is implemented in e1071 Copying a File
            copyFile(srcDir, dstDir);
        }
    }

    /**
     * Copies src file to dst file. If the dst file does not exist, it is created
     * @param src the source file
     * @param dst the destination file
     * @throws java.io.IOException throws java.io.IOException if something failes
     */
    public static void copyFile(File src, File dst) throws java.io.IOException {
        InputStream in = new FileInputStream(src);
        OutputStream out = new FileOutputStream(dst);

        // Transfer bytes from in to out
        byte[] buf = new byte[1024];
        int len;
        while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
        }
        in.close();
        out.close();
    }
    /**
     * Deletes all files and subdirectories under dir and the directory itself.
     * Returns true if all deletions were successful.
     * If the deletion fails, the method the method continues to delete rest
     * of the files and returns false.
     * @return Returns true if all deletions were successful, else false.
     * @param dir the directory to delete
     */
    public static boolean deleteDir(File dir) {

        if (! cleanDir(dir)) return false;

        // The directory is now empty so delete it
        return dir.delete();
    }

    /**
     * Deletes all files and subdirectories under dir.
     * Returns true if all deletions were successful.
     * If a deletion fails, the method continues to delete rest of the files.
     * @return Returns true if all deletions were successful, else false.
     * @param dir the directory to clean from content
     */
    public static boolean cleanDir(File dir){

        boolean success = true;
        if (dir.isDirectory()){
            File [] theFiles = dir.listFiles();

            if (theFiles.length != 0 )
                for (int i = 0; i < theFiles.length; i++){
                    success &= theFiles[i].delete();
                }
        }
        return success;
    }
}