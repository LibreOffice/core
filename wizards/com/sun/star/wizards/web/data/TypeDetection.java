/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/*
 * FileTypes.java
 *
 * Created on 29. September 2003, 18:52
 */
package com.sun.star.wizards.web.data;

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
