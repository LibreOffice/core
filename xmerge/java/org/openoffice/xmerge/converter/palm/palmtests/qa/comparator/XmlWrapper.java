
/************************************************************************
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

import java.io.IOException;
import java.io.File;
import java.util.zip.*;

public class XmlWrapper
{
    public static void main(String args[]) throws IOException
    {
        System.out.println("args.length is " + args.length);
        if (args.length < 2) {
            System.out.println("Usage: java XmlWrapper [<zipfile1> <zipfile2>].");
            //return;
             System.exit(-1);

        }

        XmlWrapper w = new XmlWrapper();
        File currdirfp = null;
        try {
         currdirfp = new File(".");
        } catch (Exception fx) {
         System.out.println("Could not get File instance for current directory \n");
         //return;
         System.exit(-1);
        }

        File f1 = null;
        File f2 = null;
        String fname1,fname2;
        try {
         f1 = File.createTempFile("xmlcomp", ".tmp", currdirfp);
         f2 = File.createTempFile("xmlcomp", ".tmp", currdirfp);
        } catch (Exception tx) {
         System.out.println("Could not create TempFile ");
         System.out.println("Exception: " + tx.toString());
         //return;
         System.exit(-1);
        }

        fname1 = f1.getAbsolutePath();
        fname2 = f2.getAbsolutePath();

        // get content.xml file from zip file and copy it to temporary
        // filename
        XmlZipExtract xw1 = new XmlZipExtract(args[0]);
        try {
          xw1.getContentXml(fname1);
        } catch (ZipException e) {
         System.out.println("Exception: file is not a ZIP file: " + args[0]);
         f1.delete();
         f2.delete();
         //return;
         System.exit(-1);
        } catch (Exception e) {
         System.out.println("Exception: Could not extract XML from " + args[0]);
         System.out.println("Exception: " + e.toString());
         f1.delete();
         f2.delete();
         //return;
         System.exit(-1);
        }

        // get content.xml file from zip file and copy it to temporary
        // filename
        XmlZipExtract xw2 = new XmlZipExtract(args[1]);
        try {
         xw2.getContentXml(fname2);
        } catch (ZipException e) {
         System.out.println("Exception: file is not a ZIP file: " + args[0]);
         f1.delete();
         f2.delete();
         //return;
         System.exit(-1);
        } catch (Exception ex) {
         System.out.println(ex.getMessage());
         System.out.println("Exception: Could not extract XML from " + args[1]);
         System.out.println("Exception: " + ex.toString());
         f1.delete();
         f2.delete();
         //return;
         System.exit(-1);
        }

        boolean same = false;

        try
        {
           XmlDiff xmldiff = new XmlDiff();

           if (args.length == 2) {
               same = xmldiff.diff(fname1, fname2);
           } else {
               same = xmldiff.diff();
           }
        }
        catch (Exception ex)
        {
         System.out.println("XmlDiff failed");
         System.out.println("Exception: " + ex.toString());
         f1.delete();
         f2.delete();
         //return;
         System.exit(-1);
        }

        System.out.println("Diff result: " + same);
        if (same)
        {
          System.out.println("XMLDIFFRESULT:PASSED");
        } else {
          System.out.println("XMLDIFFRESULT:FAILED");
        }

        f1.delete();
        f2.delete();

        if (same)
        {
            System.exit(2);
        }
        else
        {
            System.exit(3);
        }
    }
}
