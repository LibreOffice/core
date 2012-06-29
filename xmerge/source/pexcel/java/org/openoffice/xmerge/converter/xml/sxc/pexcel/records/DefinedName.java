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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

import org.openoffice.xmerge.util.Debug;
import org.openoffice.xmerge.util.EndianConverter;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.Workbook;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula.FormulaHelper;
import org.openoffice.xmerge.converter.xml.sxc.NameDefinition;
import org.openoffice.xmerge.converter.xml.sxc.pexcel.PocketExcelConstants;

/**
 * Represents a BIFF Record representing a defined name in the workbook
 */
public class DefinedName implements BIFFRecord {

    private byte[] grbit    = new byte[2];
    private byte   cch;
    private byte[] cce      = new byte[2];
    private byte[] ixals    = new byte[2];
    private byte[] rgch;
    private byte[] rgce;
    private FormulaHelper fh = new FormulaHelper();
    private String definition = new String("");
    private Workbook wb;

    /**
      * Constructs a Defined Name from an existing <code>NameDefinition</code>
      *
      * @param  nd The name definition.
      * @param  wb The workbook we are defined in.
      */
    public DefinedName(NameDefinition nd, Workbook wb) throws IOException {

        fh.setWorkbook(wb);
        this.wb = wb;
        String name = nd.getName();

        // we have to insert an = to stop the formulaParser throwing an exception
        definition = "=" + nd.getDefinition();

        cch = (byte)name.length();
        rgch = new byte[cch*2];
        rgch = name.getBytes("UTF-16LE");
        grbit = EndianConverter.writeShort((short)0);
        ixals[0] = (byte)0xFF;ixals[1] = (byte)0xFF;
    }

    /**
      * Constructs a Defined Name from the <code>InputStream</code>
      *
      * @param  is InputStream containing the record data
      * @param  wb The workbook we are defined in.
      */
    public DefinedName(InputStream is, Workbook wb) throws IOException {

        read(is);
        fh.setWorkbook(wb);
        this.wb = wb;
    }

    /**
     * Get the hex code for this particular <code>BIFFRecord</code>
     *
     * @return the hex code for <code>DefinedName</code>
     */
    public short getBiffType() {
        return PocketExcelConstants.DEFINED_NAME;
    }

    /**
      * Reads a Defined Name from the <code>InputStream</code> The byte array
     * must be twice the size of the String as it uses unicode.
      *
      * @param  input InputStream containing the record data
      */
    public int read(InputStream input) throws IOException {

        int numOfBytesRead  = input.read(grbit);
        cch                 = (byte) input.read();
        numOfBytesRead++;
        numOfBytesRead      += input.read(cce);
        numOfBytesRead      += input.read(ixals);

        rgch = new byte[cch*2];
        input.read(rgch, 0, cch*2);

        rgce = new byte[EndianConverter.readShort(cce)];
        input.read(rgce, 0, EndianConverter.readShort(cce));

        Debug.log(Debug.TRACE, "\tgrbit : "+ EndianConverter.readShort(grbit) +
                            " cch : " + cch +
                            " cce : " + EndianConverter.readShort(cce) +
                            " ixals : " + EndianConverter.readShort(ixals) +
                            "\n\trgch : " + rgch +
                            " rgce : " + rgce);

        return numOfBytesRead;
    }

     /**
     * Write this particular <code>BIFFRecord</code> to the <code>OutputStream</code>
     *
     * @param output the <code>OutputStream</code>
     */
    public void write(OutputStream output) throws IOException {

        try {
            Debug.log(Debug.TRACE,"Writing out " + definition);
            rgce = fh.convertCalcToPXL(definition);
            cce = EndianConverter.writeShort((short) rgce.length);
        } catch(Exception e) {
            Debug.log(Debug.TRACE,"Error in Parsing Name Definition");
            cce = EndianConverter.writeShort((short) 0);
        }

        output.write(getBiffType());
        output.write(grbit);
        output.write(cch);
        output.write(cce);
        output.write(ixals);
        output.write(rgch);
        if(rgce.length!=0)
            output.write(rgce);

        Debug.log(Debug.TRACE,"Writing DefinedName record");
    }

    /**
     * Returns definition name. This is public because the
     * <code>TokenDecoder</code> has to substitue the Name token with this
     * String when writing out to sxc
     *
     * @return the <code>String</code> containing the name
     */
    public String getName() {
        String name;

        try {
            name = new String(rgch, "UTF-16LE");
        } catch (UnsupportedEncodingException e){
            name = "unknown";
        }
        return name;
    }

    /**
     * Returns a definition table which can be used by the pocket excel
     * decoder to build a complete definitions table for writing to the sxc
     * document
     */
     public NameDefinition getNameDefinition() {

         String baseCellAddress;
        getDefinition();        // This must be called first so we know the type

        baseCellAddress = "$" + wb.getSheetName(0) + ".A1";

        NameDefinition nd = new NameDefinition(getName(),definition, baseCellAddress, isRangeType(), isExpressionType());
        return nd;
     }

    /**
     * Returns the definition
     *
     * @return the <code>String</code> containing the definition
     */
    private String getDefinition() {
        // pexcel sometimes creates Name definition with no defintion, bug??
        if(EndianConverter.readShort(cce)!=0) {
            definition = fh.convertPXLToCalc(rgce);
            definition = definition.substring(1);   // remove the '='
            definition = definition.replace(',', ';');
        }
        return definition;
    }

    /**
     * Returns the defintion
     *
     * @return the <code>String</code> containing the definition
     */
    private boolean isRangeType() {

        return fh.isRangeType();
    }
    /**
     * Returns the defintion
     *
     * @return the <code>String</code> containing the definition
     */
    private boolean isExpressionType() {

        return fh.isExpressionType();
    }
}
