/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TypeDetection.java,v $
 *
 * $Revision: 1.2.412.1 $
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
 * FileTypes.java
 *
 * Created on 29. September 2003, 18:52
 */
package com.sun.star.wizards.web.data;

/**
 *
 * @author  rpiterman
 */
public class TypeDetection
{

    /**
     * a document which will open in writer.
     */
    public final static String WRITER_DOC = "writer";
    /**
     * a document which will open in calc.
     */
    public final static String CALC_DOC = "calc";
    /**
     * a document which will open in impress.
     */
    public final static String IMPRESS_DOC = "impress";
    /**
     * a document which will open in draw.
     */
    public final static String DRAW_DOC = "draw";
    /**
     * an HTML document
     */
    public final static String HTML_DOC = "html";
    /**
     * a GIF or an JPG file.
     */
    public final static String WEB_GRAPHICS = "graphics";
    /**
     * a PDF document.
     */
    public final static String PDF_DOC = "pdf";
    /**
     * a Sound file (mp3/wav ect.)
     */
    public final static String SOUND_FILE = "sound";
    /**
     * a File which can not be handled by neither SO or a Web browser (exe, jar, zip ect.)
     */
    public final static String NO_TYPE = "other";
}
