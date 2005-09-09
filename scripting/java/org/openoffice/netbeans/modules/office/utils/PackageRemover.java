/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PackageRemover.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:20:29 $
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
