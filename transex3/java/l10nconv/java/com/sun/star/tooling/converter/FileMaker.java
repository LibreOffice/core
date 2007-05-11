/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FileMaker.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:09:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
/*
 * creates new files
 * checks if they exist or
 * can be overwritten
 *
 */
package com.sun.star.tooling.converter;

import java.io.File;
import java.io.IOException;

/**
 * Creates new files only if the file  does not yet exist
 * or overwriting is allowed
 *
 * @author Christian Schmidt 2005
 *
 */
public final class FileMaker {

    /**
     * Create a new file if overwriting is not alowed
     * ask if existing files should be overwritten
     *
     * @param fileName the files name to overwrite
     * @param overwrite indicates wether the file can be overwritten
     * @return the File created from the fileName
     * @throws IOException
     */
    public final static File newFile(String fileName, boolean overwrite)
            throws IOException {
        File file = new File(fileName);
        if (file.exists()) {
            if (!overwrite) {
                char c = 0;

                System.out.print("Warning: File " + fileName
                        + " already exist.\n" + "Overwrite (y/n) ? :");
                byte[] waste = new byte[10];
                System.in.read(waste);
                c = (char) waste[0];
                if (c == 'y') {
                    OutputHandler.out("...overwriting " + fileName);
                } else {
                    OutputHandler.out(
                            "\nPlease set '-o' switch at command line to overwrite.\n\nProgramm Aborted.");
                    System.exit(-1);
                }
            } else {
                OutputHandler.out("...overwriting " + fileName);
            }
        } else {
            OutputHandler.out("...creating new target file " + fileName);
        }
        return file;
    }

}