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


