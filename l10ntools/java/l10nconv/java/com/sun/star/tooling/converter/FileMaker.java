/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
     * Create a new file if overwriting is not allowed
     * ask if existing files should be overwritten
     *
     * @param fileName the files name to overwrite
     * @param overwrite indicates whether the file can be overwritten
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