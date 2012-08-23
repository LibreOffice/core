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
package com.sun.star.report;

import com.sun.star.awt.Size;

import java.io.InputStream;

/**
 *
 * @author oj93728
 */
public interface ImageService
{

    /**
     * @param image
     * @return the mime-type of the image as string.
     * @throws ReportExecutionException
     */
    String getMimeType(final InputStream image) throws ReportExecutionException;

    /**
     * @param image
     * @return the mime-type of the image as string.
     * @throws ReportExecutionException
     */
    String getMimeType(final byte[] image) throws ReportExecutionException;

    /**
     * @param image
     * @returns the dimension in 100th mm.
     *
     * @throws ReportExecutionException
     * @return*/
    Size getImageSize(final InputStream image) throws ReportExecutionException;

    /**
     * @param image
     * @returns the dimension in 100th mm.
     *
     * @throws ReportExecutionException
     * @return*/
    Size getImageSize(final byte[] image) throws ReportExecutionException;
}

