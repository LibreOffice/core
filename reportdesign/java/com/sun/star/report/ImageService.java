/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImageService.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:22:20 $
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
package com.sun.star.report;

import java.io.InputStream;
import java.awt.Dimension;

/**
 *
 * @author oj93728
 */
public interface ImageService
{

    /**
     * @return the mime-type of the image as string.
     */
    public String getMimeType(final InputStream image) throws ReportExecutionException;

    /**
     * @return the mime-type of the image as string.
     */
    public String getMimeType(final byte[] image) throws ReportExecutionException;

    /**
     * @returns the dimension in 100th mm.
     **/
    public Dimension getImageSize(final InputStream image) throws ReportExecutionException;

    /**
     * @returns the dimension in 100th mm.
     **/
    public Dimension getImageSize(final byte[] image) throws ReportExecutionException;
}

