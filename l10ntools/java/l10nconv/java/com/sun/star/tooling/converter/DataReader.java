/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataReader.java,v $
 * $Revision: 1.3 $
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