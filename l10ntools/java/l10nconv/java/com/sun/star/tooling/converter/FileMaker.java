/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FileMaker.java,v $
 * $Revision: 1.3 $
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