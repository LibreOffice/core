/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProcessErrors.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:57:15 $
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
package com.sun.star.wizards.web;

/**
 * @author rpiterman
 * Error IDs for errors that can accure
 * in the interaction with the Process class.
 */
public interface ProcessErrors {

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
