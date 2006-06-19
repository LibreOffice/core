/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XImageManager.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:10:03 $
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
package imageManager.interfaces;

import com.sun.star.graphic.XGraphic;
import com.sun.star.ui.ImageType;
import com.sun.star.ui.XImageManager;
import lib.TestParameters;
import share.LogWriter;

/**
 *
 */
public class _XImageManager {
    LogWriter log = null;
    TestParameters tEnv = null;
    String[]imageNames = null;
    XGraphic[] xGraphicArray = null;
    public XImageManager oObj;

    public _XImageManager(LogWriter log, TestParameters tEnv, XImageManager oObj) {
        this.log = log;
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public boolean _getAllImageNames() {
        short s = ImageType.COLOR_NORMAL + ImageType.SIZE_DEFAULT;
        imageNames = oObj.getAllImageNames(s);
        for (int i=0; i<(imageNames.length>10?10:imageNames.length); i++)
            System.out.println("###### Image: " + imageNames[i]);
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
                result &= oObj.hasImage(s, imageNames[i]);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((java.io.PrintWriter)log);
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
