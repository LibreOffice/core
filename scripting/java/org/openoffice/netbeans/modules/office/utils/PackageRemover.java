/*************************************************************************
 *
 *  $RCSfile: PackageRemover.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: toconnor $ $Date: 2003-06-12 11:28:18 $
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
