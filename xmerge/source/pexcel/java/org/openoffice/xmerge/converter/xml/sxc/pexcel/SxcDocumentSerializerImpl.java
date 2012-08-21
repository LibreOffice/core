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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;

import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocumentSerializer;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;

/**
 *  <p>Pocket Excel implementation of <code>SxcDocumentDeserializer</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.pexcel.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>This converts StarOffice XML format to a set of files in
 *  Pocket Excel PXL format.</p>
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

        // TODO - get real values for password when implemented in XML
        // Passwords are not currently stored in StarCalc XML format.
        String password = null;

        encoder = new PocketExcelEncoder(docName);

        // get dom document
        org.w3c.dom.Document domDoc = sxcDoc.getContentDOM();

        // load the styles
        loadStyles(sxcDoc);
        //  Traverse to the office:body element.
        //  There should only be one.
        NodeList list = domDoc.getElementsByTagName(TAG_OFFICE_BODY);
        int len = list.getLength();

        if (len > 0) {
            Node node = list.item(0);
            traverseBody(node);
        }

        // get settings for this document
        org.w3c.dom.Document settingsDoc = sxcDoc.getSettingsDOM();
        if(settingsDoc!=null) {
            NodeList settingsList = settingsDoc.getElementsByTagName(TAG_OFFICE_SETTINGS);
            int slen = settingsList.getLength();

            if (slen > 0) {
                Node settingsNode = settingsList.item(0);
                traverseSettings(settingsNode);
            }
        }

        // Get the number of sheets in the workbook
        // This will equal the number of PDBs we need
        ConvertData cd = new ConvertData();
        Workbook wb = ((PocketExcelEncoder) encoder).getWorkbook();
        cd.addDocument(wb);

        return cd;
    }


    /**
     *  A cell reference in a StarOffice formula looks like
     *  [.C2] (for cell C2).  MiniCalc is expecting cell references
     *  to look like C2.  This method strips out the braces and
     *  the period.
     *
     *  @param  formula  A StarOffice formula <code>String</code>.
     *
     *  @return  A MiniCalc formula <code>String</code>.
     */
    protected String parseFormula(String formula) {

        return null;
    }
}

