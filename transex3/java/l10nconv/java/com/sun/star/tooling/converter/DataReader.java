/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataReader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 09:08:34 $
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
package com.sun.star.tooling.converter;

import java.io.*;
import java.util.*;

/**
 * Gets Line counting from LineNumberReader all Converter Reader classes inherit
 * from this.
 * The abstract parent class of all converter reader classes
 *
 * @author Christian Schmidt
 */
abstract public class DataReader extends LineNumberReader {

    /**
     * Creates a new instance of DataReader
     *
     * @param isr
     *            InputStreamReader used as Source for this class
     */
    public DataReader(InputStreamReader isr) {
        super(isr);
    }

    /**

     * @throws java.io.IOException
     * @throws ConverterException
     *
     * TODO this should no longer use an array as return type better a Map
     *
     */
    /**
     * The next block of the SDF file is reviewed and the Line including the
     * source language and the Line including the target Language are given back
     * in an array
     *
     *
     * @return    A Map including the source language
     *            and the target Language content are given back
     *
     * @throws java.io.IOException
     * @throws ConverterException
     */
    public Map getData() throws java.io.IOException, ConverterException {
        return null;
    }


}