/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

    private byte[] grbit	= new byte[2];
    private byte   cch;
    private byte[] cce		= new byte[2];
    private byte[] ixals	= new byte[2];
    private byte[] rgch;
    private byte[] rgce;
    private FormulaHelper fh = new FormulaHelper();    
    private String definition = new String("");
    private Workbook wb;

    /**
      * Constructs a Defined Name from the <code>InputStream</code> 
      *
      * @param	is InputStream containing the record data 
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
      * @param	is InputStream containing the record data 
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
      * @param	is InputStream containing the record data 
      */
    public int read(InputStream input) throws IOException {
        
        int numOfBytesRead	= input.read(grbit);    
        cch				 	= (byte) input.read();
        numOfBytesRead++;
        numOfBytesRead		+= input.read(cce);
        numOfBytesRead		+= input.read(ixals);    
        
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
     * @param ouput the <code>OutputStream</code>
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
        getDefinition();		// This must be called first so we know the type

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
            definition = definition.substring(1);	// remove the '='
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
