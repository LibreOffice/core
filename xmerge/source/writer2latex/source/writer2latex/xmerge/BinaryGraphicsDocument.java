/************************************************************************
 *
 *  BinaryGraphicsDocument.java
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
 *  Copyright: 2002-2003 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.2 (2003-02-27)
 *
 */

package writer2latex.xmerge;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import org.openoffice.xmerge.Document;

/**
 * <p>Class representing a binary graphics document.
 * This class is used for representing graphics documents that are <i>not</i>
 * interpreted in any way, but simply copied verbatim from the source format
 * to the target format.</p>
 *
 * <p><code>GraphicsDocument</code> is used to create new graphics documents.</p>
 *
 */
public class BinaryGraphicsDocument implements Document {

    private final static int BUFFERSIZE = 1024;

    private String docName;

    private byte[] data;
    private int nOff;
    private int nLen;

    private String sFileExtension;
    private String sMimeType;

    /**
     * <p>Constructs a new graphics document.</p>
     *
     * <p>This new document does not contain any information.  Document data must
     *    either be added using appropriate methods, or an existing file can be
     *    {@link #read(InputStream) read} from an <code>InputStream</code>.</p>
     *
     * @param   name    The name of the <code>GraphicsDocument</code>.
     */
    public BinaryGraphicsDocument(String name, String sFileExtension, String sMimeType) {
        this.sFileExtension = sFileExtension;
        this.sMimeType = sMimeType;
        docName = trimDocumentName(name);
    }


    /**
     * <p>This method reads <code>byte</code> data from the InputStream.</p>
     *
     * @param   is      InputStream containing a binary data file.
     *
     * @throws  IOException     In case of any I/O errors.
     */
    public void read(InputStream docData) throws IOException {

        if (docData == null) {
            throw new IOException ("No input stream to convert");
        }
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        int len = 0;
        byte buffer[] = new byte[BUFFERSIZE];
        while ((len = docData.read(buffer)) > 0) {
            baos.write(buffer, 0, len);
        }
        data = baos.toByteArray();
    }

    public void read(byte[] data) {
        read(data,0,data.length);
    }

    public void read(byte[] data, int nOff, int nLen) {
        this.data = data;
        this.nOff = nOff;
        this.nLen = nLen;
    }

    /*
     * Utility method to make sure the document name is stripped of any file
     * extensions before use.
     */
    private String trimDocumentName(String name) {
        String temp = name.toLowerCase();

        if (temp.endsWith(getFileExtension())) {
            // strip the extension
            int nlen = name.length();
            int endIndex = nlen - getFileExtension().length();
            name = name.substring(0,endIndex);
        }

        return name;
    }

    /**
     * <p>Returns the <code>Document</code> name with no file extension.</p>
     *
     * @return  The <code>Document</code> name with no file extension.
     */
    public String getName() {
        return docName;
    }

    /**
     * <p>Returns the <code>Document</code> name with file extension.</p>
     *
     * @return  The <code>Document</code> name with file extension.
     */
    public String getFileName() {
        return new String(docName + getFileExtension());
    }


    /**
     * <p>Writes out the <code>Document</code> content to the specified
     * <code>OutputStream</code>.</p>
     *
     * <p>This method may not be thread-safe.
     * Implementations may or may not synchronize this
     * method.  User code (i.e. caller) must make sure that
     * calls to this method are thread-safe.</p>
     *
     * @param  os  <code>OutputStream</code> to write out the
     *             <code>Document</code> content.
     *
     * @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        os.write(data, nOff, nLen);
    }

    /**
     *  Returns the file extension for this type of
     *  <code>Document</code>.
     *
     *  @return  The file extension of <code>Document</code>.
     */
    public String getFileExtension(){ return sFileExtension; }

    /**
     * Method to return the MIME type of the document.
     *
     * @return  String  The document's MIME type.
     */
    public String getDocumentMIMEType(){ return sMimeType; }

}