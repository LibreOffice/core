/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CrashThread.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:56:56 $
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

package complex.framework.recovery;

import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

/**
 * Thread to crash the office. This thread dies after the office process
 * is nopt longer available.
 */
public class CrashThread extends Thread {
    public XComponent xDoc = null;
    public XMultiServiceFactory msf = null;

    public CrashThread(XComponent xDoc, XMultiServiceFactory msf) {
        this.xDoc = xDoc;
        this.msf = msf;
    }

    public void run() {
        try{
            XModel xModel = (XModel) UnoRuntime.queryInterface(XModel.class, xDoc);

            XController xController = xModel.getCurrentController();
            XDispatchProvider xDispProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                                  XDispatchProvider.class,
                                                  xController);
            XURLTransformer xParser = (XURLTransformer) UnoRuntime.queryInterface(
                                              XURLTransformer.class,
                                              msf.createInstance(
                                                      "com.sun.star.util.URLTransformer"));

            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:Crash";
            xParser.parseStrict(aParseURL);

            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);

            if (xDispatcher != null) {
                xDispatcher.dispatch(aURL, null);
            }
        } catch (Exception e){}
    }
}
