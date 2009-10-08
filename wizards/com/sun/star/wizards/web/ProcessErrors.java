/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ProcessErrors.java,v $
 * $Revision: 1.5 $
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
package com.sun.star.wizards.web;

/**
 * @author rpiterman
 * Error IDs for errors that can accure
 * in the interaction with the Process class.
 */
public interface ProcessErrors
{

    public final static int ERROR_MKDIR = 0;
    public final static int ERROR_EXPORT = 1;
    public final static int ERROR_EXPORT_MKDIR = 2;
    public final static int ERROR_DOC_VALIDATE = 3;
    public final static int ERROR_EXPORT_IO = 4;
    public final static int ERROR_EXPORT_SECURITY = 5;
    public final static int ERROR_GENERATE_XSLT = 6;
    public final static int ERROR_GENERATE_COPY = 7;
    public final static int ERROR_PUBLISH = 8;
    public final static int ERROR_PUBLISH_MEDIA = 9;
    public final static int ERROR_CLEANUP = 10;
}
