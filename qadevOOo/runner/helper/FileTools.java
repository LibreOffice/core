/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FileTools.java,v $
 * $Revision: 1.6 $
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
package helper;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;


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

        // if (! cleanDir(dir)) return false;

        // The directory is now empty so delete it
        // return dir.delete();
        return cleanDir(dir);
    }

    /**
     * Deletes all files and subdirectories under dir.
     * Returns true if all deletions were successful.
     * If a deletion fails, the method continues to delete rest of the files.
     * @return Returns true if all deletions were successful, else false.
     * @param dir the directory to clean from content
     */
    // public static boolean cleanDir(File dir){
    //
    //     boolean success = true;
    //     if (dir.isDirectory()){
    //         File [] theFiles = dir.listFiles();
    //
    //         if (theFiles.length != 0 )
    //             for (int i = 0; i < theFiles.length; i++){
    //                 success &= theFiles[i].delete();
    //             }
    //     }
    //     return success;
    // }

   public static boolean cleanDir(File dir)
        {
            if (dir.isDirectory())
            {
                String[] children = dir.list();
                for (int i=0; i<children.length; i++)
                {
                    boolean success = cleanDir(new File(dir, children[i]));
                    if (!success)
                    {
                        return false;
                    }
                }
            }

            // The directory is now empty so delete it
            return dir.delete();
        }
}
