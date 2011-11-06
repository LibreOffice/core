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
