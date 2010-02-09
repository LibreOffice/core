/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConverterException.java,v $
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
 * a simple exception
 * just to seperate
 * it from other
 * exceptions
 */
package com.sun.star.tooling.converter;

/**
 * @author Christian Schmidt
 *
 * Thrown if an error occurs during converting from one file format to another
 * that does not belong to an other exception
 */
public class ConverterException extends Exception {

    /**
     *
     */
    public ConverterException() {
        super();

    }

    /**
     * @param arg0
     */
    public ConverterException(String arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     */
    public ConverterException(Throwable arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     * @param arg1
     */
    public ConverterException(String arg0, Throwable arg1) {
        super(arg0, arg1);

    }

}