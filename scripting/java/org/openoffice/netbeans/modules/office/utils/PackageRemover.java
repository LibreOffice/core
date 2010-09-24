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

package org.openoffice.netbeans.modules.office.utils;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;

import org.openoffice.idesupport.zip.ParcelZipper;

public class PackageRemover {
    private PackageRemover() {
    }

    public static void removeDeclaration(File source) throws IOException {
        File tmp = new File(source.getAbsolutePath() + ".tmp");

        BufferedReader in = new BufferedReader(new FileReader(source));
        BufferedWriter out = new BufferedWriter(new FileWriter(tmp));

        try {
            String line;
            while ((line = in.readLine()) != null) {
                if (line.startsWith("package")) {
                    String newDeclaration = evaluate(line);
                    if (newDeclaration != null) {
                        out.write(newDeclaration, 0, newDeclaration.length());
                        out.newLine();
                    }
                }
                else {
                    out.write(line, 0, line.length());
                    out.newLine();
                }
            }
        }
        finally {
            if (in != null) {
                in.close();
            }
            if (out != null) {
                out.close();
            }
        }

        if (source.delete() == false) {
            tmp.delete();
            throw new IOException("Could not overwrite " + source);
        }
        else {
            tmp.renameTo(source);
        }
    }

    public static String evaluate(String line) {

        int idx = line.indexOf(ParcelZipper.CONTENTS_DIRNAME);
        if (idx == -1)
            return line;

        idx = idx + ParcelZipper.CONTENTS_DIRNAME.length();
        if (line.charAt(idx) == '.')
            return "package " + line.substring(idx + 1);;

        return null;
    }

    public static void main(String[] args) {
        File source = new File(args[0]);

        try {
            removeDeclaration(source);
        }
        catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }
}
