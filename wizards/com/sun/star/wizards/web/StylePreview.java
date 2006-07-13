/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StylePreview.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 12:28:39 $
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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.web.data.CGStyle;

/**
 * @author rpiterman
 * the style preview, which is a OOo Document Preview in
 * an Image Control.
 * This class copies the files needed for this
 * preview from the web wizard work directory
 * to a given temporary directory, and updates them
 * on request, according to the current style/background selection
 * of the user.
 */
public class StylePreview {
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
     * @param wwRoot is the root directory of the web wizard files (
     * usually [oo]/share/template/[lang]/wizard/web
     */
    public StylePreview(XMultiServiceFactory xmsf, String wwRoot_) throws Exception {
        fileAccess = new FileAccess(xmsf);

        tempDir = createTempDir(xmsf);

        htmlFilename = FileAccess.connectURLs(tempDir , "wwpreview.html");
        cssFilename = FileAccess.connectURLs(tempDir ,"style.css");
        backgroundFilename = FileAccess.connectURLs(tempDir , "images/background.gif");

        wwRoot = wwRoot_;

        fileAccess.copy(FileAccess.connectURLs(wwRoot,"preview.html"), htmlFilename);
    }

    /**
     * copies the given style and background files to the temporary
     * directory.
     * @param style
     * @param background
     * @throws Exception
     */
    public void refresh(CGStyle style, String background) throws Exception {

        String css = FileAccess.connectURLs(wwRoot , "styles/" + style.cp_CssHref);

        if (background == null || background.equals("")) {
            //delete the background image
            if (fileAccess.exists(backgroundFilename,false))
                    fileAccess.delete(backgroundFilename);
        }
        else {
            // a solaris bug workaround
            // TODO
            //copy the background image to the temp directory.
            fileAccess.copy(background,backgroundFilename);
        }
        //copy the actual css to the temp directory
        fileAccess.copy(css,cssFilename);
    }

    public void cleanup() {
        try {
            removeTempDir();
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * creates a temporary directory.
     * @param xmsf
     * @return the url of the new directory.
     * @throws Exception
     */
    private String createTempDir(XMultiServiceFactory xmsf) throws Exception {
        String tempPath = FileAccess.getOfficePath(xmsf,"Temp","", "") ;
        String s = fileAccess.createNewDir(tempPath, "wwiz");
        fileAccess.createNewDir(s,"images");
        return s;
    }

    /**
     * deletes/removes the temporary directroy.
     * @throws Exception
     */
    private void removeTempDir() throws Exception {
        fileAccess.delete(tempDir);
    }

}
