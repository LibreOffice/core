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

package org.openoffice.xmerge.converter.xml.sxc.pexcel;

import java.awt.Color;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Element;

import java.io.IOException;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.ConvertException;
import org.openoffice.xmerge.converter.xml.sxc.Format;
import org.openoffice.xmerge.converter.xml.sxc.SxcDocumentSerializer;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.converter.xml.StyleCatalog;

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

        // TODO - get real values for password when implemnted in XML
        // Passwords are not currently stored in StarCalc XML format.
        String password = null;

        encoder = new PocketExcelEncoder(docName, password);

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

