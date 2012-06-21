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
