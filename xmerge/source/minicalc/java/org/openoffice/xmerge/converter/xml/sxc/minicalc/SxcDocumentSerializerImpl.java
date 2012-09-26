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

