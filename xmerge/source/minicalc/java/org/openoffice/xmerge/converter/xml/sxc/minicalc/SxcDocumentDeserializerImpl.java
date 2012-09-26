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

import org.openoffice.xmerge.converter.xml.sxc.SxcDocumentDeserializer;
import org.openoffice.xmerge.converter.xml.sxc.SpreadsheetDecoder;
import org.openoffice.xmerge.converter.palm.PalmDB;
import org.openoffice.xmerge.ConvertData;
import org.openoffice.xmerge.converter.palm.PalmDocument;

import java.io.IOException;
import java.util.Enumeration;

/**
 *  <p>MiniCalc implementation of <code>DocumentDeserializer</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.minicalc.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 *
 *  <p>This converts a set of files in MiniCalc PDB format to a StarOffice DOM.</p>
 *
 */
public final class SxcDocumentDeserializerImpl extends SxcDocumentDeserializer {

    /**
     *  Creates new <code>SxcDocumentDeserializerImpl</code>.
     *
     *  @param  cd  <code>ConvertData</code>  Input data to convert.
     */
    public SxcDocumentDeserializerImpl(ConvertData cd) {
        super(cd);
    }


    /**
     *  This method will be implemented by concrete subclasses and will
     *  return an application-specific decoder.
     *
     *  @param  workbook        The WorkBook name.
     *  @param  worksheetNames  An array of WorkSheet names.
     *  @param  password        The password.
     *
     *  @return  An application-specific <code>SpreadsheetDecoder</code>.
     */
    public SpreadsheetDecoder createDecoder(String workbook,
        String[] worksheetNames, String password) throws IOException {

        return new MinicalcDecoder(workbook, worksheetNames, password);
    }


    /**
     *  This method will return the name of the WorkBook from the
     *  <code>ConvertData</code>.  Allows for situations where the
     *  WorkBook name differs from the PDB name.
     *
     *  Implemented in the Deserializer as the Decoder's constructor
     *  requires a name.
     *
     *  @param  cd  The <code>ConvertData</code>.
     *
     *  @return  The name of the WorkBook.
     */
    protected String getWorkbookName(ConvertData cd)
        throws IOException {

        Enumeration e        = cd.getDocumentEnumeration();
        PalmDocument palmDoc = (PalmDocument) e.nextElement();
        String workbookName  = palmDoc.getName();

        // Search for "-", which separates workbook from worksheet
        int end = workbookName.indexOf("-");

        if (end > 0) {
            workbookName = workbookName.substring(0, end);
        }

        return workbookName;
    }


    /**
     *  This method will return an array of WorkSheet names from the
     *  <code>ConvertData</code>.
     *
     *  @param  cd  The <code>ConvertData</code>.
     *
     *  @return  The name of the WorkSheet.
     */
    protected String[] getWorksheetNames(ConvertData cd)
        throws IOException {
        int numberOfPDBs = cd.getNumDocuments();
        String worksheetName[] = new String[numberOfPDBs];
        int i=0;
        Enumeration e = cd.getDocumentEnumeration();
        while (e.hasMoreElements()) {
                PalmDocument palmDoc = (PalmDocument) e.nextElement();
                worksheetName[i] = palmDoc.getName();

                // Search for the "-", which seperates workbook from worksheet
                int start = worksheetName[i].indexOf("-");

                if (start != -1) {
                   worksheetName[i] = worksheetName[i].substring(start + 1);
                }
                i++;
        }

        return worksheetName;
    }
}

