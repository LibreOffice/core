/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: LineErrorException.java,v $
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
/*
 * Exception for errors in
 * SDFLines
 */
package com.sun.star.tooling.converter;


/**
 * This Exeption is thrown if a DataReader finds an error in a read Line
 * f. e. wrong column number
 *
 * @author Christian Schmidt 2005
 *
 */
public class LineErrorException extends Exception {

     public int tokenCount;
    /**
     *
     */
    public LineErrorException() {
        super();

    }

    /**
     * @param arg0
     */
    public LineErrorException(String arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     */
    public LineErrorException(Throwable arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     * @param arg1
     */
    public LineErrorException(String arg0, Throwable arg1) {
        super(arg0, arg1);

    }


}