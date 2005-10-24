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
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.RandomAccessFile;
import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.util.Enumeration;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;

public class XmlZipExtract
{

  public static final String CONTENT   = "Content.xml";
  public static final String OLDCONTENT   = "content.xml";
  private static final int BUFFER_SIZE = 2048;


  /**
   * Full path of the Zip file to process.
   */
  private String filename = null;


  /**
   * Constructor
   *
   * @param filename Full Path to Zip file to process
   *
   */
  public XmlZipExtract(String filename) {
        this.filename = filename;
  }

  /**
   * Copies Content.xml from zip file onto the filename passed as
   * an argument
   *
   * @param fname Full Path to file to which contents have to be copied
   *
   */
  public void getContentXml(String fname) throws IOException
  {
     try
     {
        getContentXmlInt(fname, XmlZipExtract.CONTENT);
     }
     catch (NullPointerException e1)
     {
        // If the new name of the content file failed, try
        // the older name.
        //
        System.out.println(filename + " Content.xml does not exist, trying content.xml...");
        try
        {
          getContentXmlInt(fname, XmlZipExtract.OLDCONTENT);
        }
        catch (NullPointerException e2)
        {
           System.out.println(filename + " content.xml does not exist, trying content.xml...");
           throw e2;
        }
     }
  }

  public void getContentXmlInt(String fname, String cname) throws IOException
  {
     byte b[] = getEntry(cname);

     RandomAccessFile raf=null;
     raf = new RandomAccessFile(fname, "rw");
     raf.write(b);
     raf.close();
  }

  /**
   * Get the specified entry in the zip file as a stream.
   *
   * @param entryName The name of the entry in the zipfile to get.
   *   This should be one of the constants defined above.
   *
   * @return byte[] bits for entryName
   *
   * @throws IOException if something goes wrong
   */
  public byte[] getEntry(String entryName) throws IOException
  {
        ZipFile zf = new ZipFile(filename);
        ZipEntry ze = zf.getEntry(entryName);
        byte[] bits = readStream(zf.getInputStream(ze));
        zf.close();
        return bits;
  }


  /**
   * Read an InputStream into an array of bytes.
   *
   * @param is InputStream of data from Zip file
   *
   * @return an array of Bytes
   */
  private byte[] readStream(InputStream is) throws IOException
  {
        BufferedInputStream bis = new BufferedInputStream(is);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buffer = new byte[BUFFER_SIZE];
        int eof = 0;
        while ((eof = bis.read(buffer, 0, buffer.length)) > 0) {
            baos.write(buffer, 0, eof);
        }

        return baos.toByteArray();
  }
}


