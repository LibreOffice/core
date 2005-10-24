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

package org.openoffice.xmerge.converter.palm;

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;

import java.io.OutputStream;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;

import org.openoffice.xmerge.Document;

/**
 *  <p> A <code>PalmDocument</code> is palm implementaion of the
 *  <code>Docuemnt</code> interface.</p>
 *
 *  <p>This implementation allows the Palm device format to be
 *  read via an <code>InputStream</code> and written via an
 *  <code>OutputStream</code>.</p>
 *
 *  @author  Martin Maher
 */

public class PalmDocument
    implements Document {

    /**
     *  The internal representation of a pdb.
     */
    private PalmDB pdb;

    /**
     *  The file name.
     */
    private String fileName;

    /**
     *  Constructor to create a <code>PalmDocument</code>
     *  from an <code>InputStream</code>.
     *
     *  @param  is  <code>InputStream</code> containing a PDB.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public PalmDocument(InputStream is) throws IOException {
        read(is);
    }


    /**
     *  Constructor to create a <code>PalmDocument</code> with
     *  <code>Record</code> objects.  <code>recs.length</code>
     *  can be zero for an empty PDB.
     *
     *  @param  name       Suggested PDB name in <code>String</code>.
     *  @param  creatorID  The PDB Creator ID.
     *  @param  typeID     The PDB Type ID.
     *  @param  version    The PDB header version.
     *  @param  attribute  The PDB header attribute.
     *  @param  recs       Array of <code>Record</code> objects.
     *
     *  @throws  NullPointerException  If <code>recs</code> is null.
     */
    public PalmDocument(String name, int creatorID, int typeID, int version,
        short attribute, Record[] recs)
        throws UnsupportedEncodingException {
        pdb = new PalmDB(name, creatorID, typeID, version, attribute, recs);
        fileName = pdb.getPDBNameString();
    }


    /**
     *  Reads in a file from the <code>InputStream</code>.
     *
     *  @param  is  <code>InputStream</code> to read in its content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */

    public void read(InputStream is) throws IOException {
        PdbDecoder decoder = new PdbDecoder();
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int n = 0;
    while ((n = is.read(buf)) > 0) {
        baos.write(buf, 0, n);
        }
        byte[] bytearr = baos.toByteArray();
        pdb = decoder.parse(bytearr);
        fileName = pdb.getPDBNameString();
    }


    /**
     *  Writes the <code>PalmDocument</code> to an <code>OutputStream</code>.
     *
     *  @param  is  The <code>OutputStream</code> to write the content.
     *
     *  @throws  IOException  If any I/O error occurs.
     */
    public void write(OutputStream os) throws IOException {
        PdbEncoder encoder = new PdbEncoder(pdb);
        encoder.write(os);
    }


    /**
     *  Returns the <code>PalmDB</code> contained in this object.
     *
     *  @return  The <code>PalmDB</code>.
     */
    public PalmDB getPdb() {
        return pdb;
    }


    /**
     *  Sets the <code>PalmDocument</code> to a new <code>PalmDB</code>
     *  value.
     *
     *  @param  pdb  The new <code>PalmDB</code> value.
     */
    public void setPdb(PalmDB pdb) {
        this.pdb = pdb;

        String name = pdb.getPDBNameString();
        fileName = name;
    }


    /**
     *  Returns the name of the file.
     *
     *  @return  The name of the file represented in the
     *           <code>PalmDocument</code>.
     */
    public String getFileName() {
        return fileName + ".pdb";
    }


    /**
     *  Returns the <code>Document</code> name.
     *
     *  @return  The <code>Document</code> name.
     */
    public String getName() {
        return fileName;
    }
}

