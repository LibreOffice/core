/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XFilePreview.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:38:01 $
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

