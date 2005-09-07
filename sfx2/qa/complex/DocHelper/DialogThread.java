/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DialogThread.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:25:52 $
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
package complex.framework.DocHelper;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

import java.lang.Thread;


/**
 * This class opens a given dialog in a separate Thread by dispatching an url
 *
 */
public class DialogThread extends Thread {
    public XComponent m_xDoc = null;
    public XMultiServiceFactory m_xMSF = null;
    public String m_url = "";

    public DialogThread(XComponent xDoc, XMultiServiceFactory msf, String url) {
        this.m_xDoc = xDoc;
        this.m_xMSF = msf;
        this.m_url = url;
    }

    public void run() {
        XModel aModel = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                           m_xDoc);

        XController xController = aModel.getCurrentController();

        //Opening Dialog
        try {
            XDispatchProvider xDispProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                                  XDispatchProvider.class,
                                                  xController.getFrame());
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer) UnoRuntime.queryInterface(
                                              XURLTransformer.class,
                                              m_xMSF.createInstance(
                                                      "com.sun.star.util.URLTransformer"));

            // Because it's an in/out parameter
            // we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = m_url;
            xParser.parseStrict(aParseURL);

            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", com.sun.star.frame.FrameSearchFlag.SELF |
                                    com.sun.star.frame.FrameSearchFlag.CHILDREN);
            PropertyValue[] dispatchArguments = new PropertyValue[0];

            if (xDispatcher != null) {
                xDispatcher.dispatch(aURL, dispatchArguments);
            } else {
                System.out.println("xDispatcher is null");
            }
        } catch (com.sun.star.uno.Exception e) {
            System.out.println("Couldn't open dialog");
        }
    }
}