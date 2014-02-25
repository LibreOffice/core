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

    //     boolean success = true;
    //     if (dir.isDirectory()){
    //         File [] theFiles = dir.listFiles();

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
