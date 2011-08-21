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
package complex.imageManager;

import com.sun.star.graphic.XGraphic;
import com.sun.star.ui.ImageType;
import com.sun.star.ui.XImageManager;
import lib.TestParameters;


/**
 *
 */
public class _XImageManager {

    TestParameters tEnv = null;
    String[]imageNames = null;
    XGraphic[] xGraphicArray = null;
    public XImageManager oObj;

    public _XImageManager( TestParameters tEnv, XImageManager oObj) {

        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public boolean _getAllImageNames() {
        short s = ImageType.COLOR_NORMAL + ImageType.SIZE_DEFAULT;
        imageNames = oObj.getAllImageNames(s);
        for (int i=0; i<(imageNames.length>10?10:imageNames.length); i++)
        {
            System.out.println("###### Image: " + imageNames[i]);
        }
        return imageNames != null;
    }

    public boolean _getImages() {
        short s = ImageType.COLOR_NORMAL + ImageType.SIZE_DEFAULT;
        try {
            xGraphicArray = oObj.getImages(s, imageNames);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
        }
        return xGraphicArray != null;
    }

    public boolean _hasImage() {
        boolean result = true;
        short s = ImageType.COLOR_NORMAL + ImageType.SIZE_DEFAULT;
        try { // check the first image names, 10 at max
            for (int i=0; i<(imageNames.length>10?10:imageNames.length); i++)
            {
                result &= oObj.hasImage(s, imageNames[i]);
            }
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            result = false;
        }
        return result;
    }

    public boolean _insertImages() {
        try {
            oObj.insertImages((short)imageNames.length, imageNames, xGraphicArray);
        }
        catch(com.sun.star.container.ElementExistException e) {
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
        }
        return true;
    }

    public boolean _removeImages() {
        try {
            oObj.removeImages((short)(imageNames.length-1), imageNames);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
        }
        catch(com.sun.star.lang.IllegalAccessException e) {
        }
        return true;
    }

    public boolean _replaceImages() {
        return true;
    }

    public boolean _reset() {
        return true;
    }
}
