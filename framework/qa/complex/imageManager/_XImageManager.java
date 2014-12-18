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
package complex.imageManager;

import com.sun.star.graphic.XGraphic;
import com.sun.star.ui.ImageType;
import com.sun.star.ui.XImageManager;


/**
 *
 */
public class _XImageManager {


    private String[]imageNames = null;
    private XGraphic[] xGraphicArray = null;
    private final XImageManager oObj;

    public _XImageManager( XImageManager oObj) {
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
