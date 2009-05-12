/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: KlickButtonThread.java,v $
 * $Revision: 1.4 $
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

package complex.framework.recovery;

import com.sun.star.awt.XWindow;
import com.sun.star.lang.XMultiServiceFactory;
import util.UITools;

/**
 * Thread to crash the office. This thread dies after the office process
 * is nopt longer available.
 */
public class KlickButtonThread extends Thread {
    private XWindow xWindow = null;
    private XMultiServiceFactory xMSF = null;
    private String buttonName = null;

    public KlickButtonThread(XMultiServiceFactory xMSF, XWindow xWindow, String buttonName) {
        this.xWindow = xWindow;
        this.xMSF = xMSF;
        this.buttonName = buttonName;
    }

    public void run() {
        try{
            UITools oUITools = new UITools(xMSF, xWindow);

            oUITools.clickButton(buttonName);

        } catch (Exception e){}
    }
}
