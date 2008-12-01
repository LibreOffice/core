/*
 ************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WebWizard.java,v $
 *
 * $Revision: 1.6.192.1 $
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
/*
 * Start1.java
 *
 * Created on 19. September 2003, 12:09
 */
package com.sun.star.wizards.web;

import com.sun.star.awt.XTopWindow;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;

/**
 * The last class in the WebWizard Dialog class hirarchy.
 * Has no functionality, is just nice to have it instanciated.
 * @author  rpiterman
 */
public class WebWizard extends WWD_Events
{

    /**
     * @param xmsf
     */
    public WebWizard(XMultiServiceFactory xmsf) throws Exception
    {
        super(xmsf);
    }

    public static void main(String args[])
    {

        String ConnectStr =
                "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.NamingService";
        //localhost  ;Lo-1.Germany.sun.com; 10.16.65.155
        try
        {
            XMultiServiceFactory xmsf = Desktop.connect(ConnectStr);
            //File dir = new File("D:\\CWS\\extras6");//("C:\\Documents and Settings\\rpiterman\\My Documents");
            //getFiles(dir,xLocMSF);

            //read Configuration
            WebWizard ww = new WebWizard(xmsf);
            ww.show();
            ww.cleanup();

        }
        catch (Exception exception)
        {
            exception.printStackTrace(System.out);
        }
    }
}