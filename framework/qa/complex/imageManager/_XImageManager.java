/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
