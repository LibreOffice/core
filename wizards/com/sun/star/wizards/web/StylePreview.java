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
package com.sun.star.wizards.web;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.web.data.CGStyle;

/**
 * the style preview, which is a OOo Document Preview in 
 * an Image Control.
 * This class copies the files needed for this
 * preview from the web wizard work directory 
 * to a given temporary directory, and updates them
 * on request, according to the current style/background selection
 * of the user.
 */
public class StylePreview
{

    private FileAccess fileAccess;
    /**
     * the destination html url.
     */
    public String htmlFilename;
    /** 
     * the destination css url
     */
    private String cssFilename;
    /**
     * destination directory
     */
    public String tempDir;
    /**
     * destination background file url
     */
    private String backgroundFilename;
    /**
     * web wizard work directory
     */
    private String wwRoot;

    /**
     * copies the html file to the temp directory, and calculates the
     * destination names of the background and css files. 
     * @param wwRoot_ is the root directory of the web wizard files (
     *                usually [oo]/share/template/[lang]/wizard/web
     */
    public StylePreview(XMultiServiceFactory xmsf, String wwRoot_) throws Exception
    {
        fileAccess = new FileAccess(xmsf);

        tempDir = createTempDir(xmsf);

        htmlFilename = FileAccess.connectURLs(tempDir, "wwpreview.html");
        cssFilename = FileAccess.connectURLs(tempDir, "style.css");
        backgroundFilename = FileAccess.connectURLs(tempDir, "images/background.gif");

        wwRoot = wwRoot_;

        fileAccess.copy(FileAccess.connectURLs(wwRoot, "preview.html"), htmlFilename);
    }

    /**
     * copies the given style and background files to the temporary
     * directory.
     * @param style
     * @param background
     * @throws Exception
     */
    public void refresh(CGStyle style, String background) throws Exception
    {

        String css = FileAccess.connectURLs(wwRoot, "styles/" + style.cp_CssHref);

        if (background == null || background.equals(PropertyNames.EMPTY_STRING))
        {
            //delete the background image
            if (fileAccess.exists(backgroundFilename, false))
            {
                fileAccess.delete(backgroundFilename);
            }
        }
        else
        {
            // a solaris bug workaround
            // TODO
            //copy the background image to the temp directory.
            fileAccess.copy(background, backgroundFilename);
        }
        //copy the actual css to the temp directory 
        fileAccess.copy(css, cssFilename);
    }

    public void cleanup()
    {
        try
        {
            removeTempDir();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * creates a temporary directory.
     * @param xmsf
     * @return the url of the new directory.
     * @throws Exception
     */
    private String createTempDir(XMultiServiceFactory xmsf) throws Exception
    {
        String tempPath = FileAccess.getOfficePath(xmsf, "Temp", PropertyNames.EMPTY_STRING, PropertyNames.EMPTY_STRING);
        String s = fileAccess.createNewDir(tempPath, "wwiz");
        fileAccess.createNewDir(s, "images");
        return s;
    }

    /**
     * deletes/removes the temporary directroy.
     * @throws Exception
     */
    private void removeTempDir() throws Exception
    {
        fileAccess.delete(tempDir);
    }
}
