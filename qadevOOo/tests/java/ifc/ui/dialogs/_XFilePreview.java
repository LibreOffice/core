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

package ifc.ui.dialogs;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.ui.dialogs.XFilePreview;

public class _XFilePreview extends MultiMethodTest {

    public XFilePreview oObj=null;

    /**
     * _getSupportedImageFormats() gets all formats and
     * stores them in an Array of short.<br>
     * Is OK is the resulting Array isn't empty
    */
    public void _getSupportedImageFormats() {
        short[] formats = oObj.getSupportedImageFormats();
        tRes.tested("getSupportedImageFormats()", formats.length > 0);
    }

    /**
     * _getTargetColorDepth() gets the color depth
     * and stores it in an int.<br>
     * Is OK is the resulting int isn't 1
    */
    public void _getTargetColorDepth() {
        int CDepth = oObj.getTargetColorDepth();
        tRes.tested("getTargetColorDepth()",CDepth != 1);
    }

    /**
     * _getAvailableWidth() gets the width
     * and stores it in an int.<br>
     * Is OK is the resulting int isn't 1
    */
    public void _getAvailableWidth() {
        int the_width = oObj.getAvailableWidth();
        tRes.tested("getAvailableWidth()", the_width != 1);
    }

    /**
     * _getAvailableHeight() gets the width
     * and stores it in an int.<br>
     * Is OK is the resulting int isn't 1
    */
    public void _getAvailableHeight() {
        int the_height = oObj.getAvailableHeight();
        tRes.tested("getAvailableHeight()", the_height != 1);
    }

    /**
    * sets the empty image.
    * Is OK if no exception no exceptions were thrown.
    */
    public void _setImage() {
        boolean bOK = true;
        try {
            oObj.setImage
                (com.sun.star.ui.dialogs.FilePreviewImageFormats.BITMAP,null);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            bOK = false;
            throw new StatusException( "Can't set empty image", e );
        }
        tRes.tested("setImage()", bOK);
    }

    boolean prev_state;

    /**
     * _setShowState() sets the state
     * to the opposite value returned by getShowState.<br>
     * Is OK is the returned result is false or if
     * the value that was set is equal to the value
     * that was returned by getShowState.
    */
    public void _setShowState() {
        requiredMethod("getShowState()");
        boolean success = oObj.setShowState(!prev_state);
        boolean res_state = oObj.getShowState();
        tRes.tested("setShowState()", !success || res_state != prev_state);
    }

    /**
     * _getShowState() gets the state
     * and sets it to the opposite.<br>
     * Is OK if no exceptions were thrown
    */
    public void _getShowState() {
        prev_state = oObj.getShowState();
        tRes.tested("getShowState()", true);
    }

}

