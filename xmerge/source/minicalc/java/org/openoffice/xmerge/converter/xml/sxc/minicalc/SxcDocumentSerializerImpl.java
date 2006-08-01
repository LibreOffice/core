/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SxcDocumentSerializerImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 12:45:26 $
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

package org.openoffice.xmerge.converter.xml.sxc.minicalc;

import java.awt.Color;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.converter.palm.Record;
import org.openoffice.xmerge.converter.palm.PalmDocument;

import jmc.JMCconstants;

import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocumentSerializer;

/**
 *  <p>MiniCalc implementation of <code>SxcDocumentDeserializer</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.minicalc.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>This converts StarOffice XML format to a set of files in
 *  MiniCalc PDB format.</p>
 *
 *  @author      Paul Rank
 *  @author      Mark Murnane
 */
public final class SxcDocumentSerializerImpl extends SxcDocumentSerializer {


    /**
     *  Constructor.
     *
     *  @param  document  The <code>Document</code> to convert.
     */
    public SxcDocumentSerializerImpl(Document document) {
        super(document);
    }


    public ConvertData serialize() throws ConvertException, IOException {


        // Get the server side document name.  This value should not
        // contain a path or the file extension.
        String docName = sxcDoc.getName();

        // TODO - get real values for password when implemnted in XML
        // Passwords are not currently stored in StarCalc XML format.
        String password = null;

        encoder = new MinicalcEncoder(docName, password);

        // get dom document
        org.w3c.dom.Document domDoc = sxcDoc.getContentDOM();

        //  Traverse to the office:body element.
        //  There should only be one.
        NodeList list = domDoc.getElementsByTagName(TAG_OFFICE_BODY);
        int len = list.getLength();

        if (len > 0) {
            Node node = list.item(0);
            traverseBody(node);
        }

        // Get the number of sheets in the workbook
        // This will equal the number of PDBs we need
        ConvertData cd = new ConvertData();
        int numSheets = encoder.getNumberOfSheets();

        for (int i = 0; i < numSheets; i++) {

            // Get records for sheet i
            Record records[] = ((MinicalcEncoder) encoder).getRecords(i);

            // Get the sheet name for sheet i
            String fullSheetName = new String(docName
                                              + "-"
                                              + encoder.getSheetName(i));

            // Create a PalmDB object
            PalmDocument palmDoc = new PalmDocument(fullSheetName,
                 MinicalcConstants.CREATOR_ID,
                 MinicalcConstants.TYPE_ID, JMCconstants.AppVersion,
                 PalmDB.PDB_HEADER_ATTR_BACKUP, records);

            cd.addDocument(palmDoc);
        }


        // OutputStream os = new FileOutputStream(docName);

        //pdbSet.write(os);
        //os.flush();

        //ConvertDataEntry cde = new ConvertDataOutputStream(os, docName);
        //cd.addCDE(cde);

        return cd;
    }



}

