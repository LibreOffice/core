/*************************************************************************
 *
 *  $RCSfile: _XSynchronousFrameLoader.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:40:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.frame;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.SOfficeFactory;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XSynchronousFrameLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

/**
 * Testing <code>com.sun.star.frame.XSynchronousFrameLoader</code>
 * interface methods :
 * <ul>
 *  <li><code> load()</code></li>
 *  <li><code> cancel()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'FrameLoader.URL'</code> (of type <code>String</code>):
 *   an url of component to be loaded </li>
 *  <li> <code>'FrameLoader.Frame'</code> <b>(optional)</b>
 *  (of type <code>com.sun.star.frame.XFrame</code>):
 *   a target frame where component to be loaded. If this
 *   relation is ommited then a text document created and its
 *   frame is used. </li>
 * <ul> <p>
 *
 * @see com.sun.star.frame.XSynchronousFrameLoader
 */
public class _XSynchronousFrameLoader extends MultiMethodTest {

    public XSynchronousFrameLoader oObj = null; // oObj filled by MultiMethodTest
    private String url = null ;
    private XFrame frame = null ;
    private XComponent frameSup = null ;
    private PropertyValue[] descr = null;

    /**
     * Retrieves all relations. If optional relation
     * <code>FrameLoader.Frame</code> not found
     * creates a new document and otains its frame for loading. <p>
     *
     * Also <code>MediaDescriptor</code> is created using
     * URL from <code>FrameLoader.URL</code> relation.
     *
     * @throws StatusException If one of required relations not found.
     */
    public void before() {
        url = (String) tEnv.getObjRelation("FrameLoader.URL") ;
        frame = (XFrame) tEnv.getObjRelation("FrameLoader.Frame") ;

        if (url == null) {
            throw new StatusException(Status.failed("Some relations not found")) ;
        }

        SOfficeFactory SOF = SOfficeFactory.getFactory(
                             (XMultiServiceFactory)tParam.getMSF() );

        XURLTransformer xURLTrans = null;

        // if frame is not contained in relations the writer frmame will be used.
        if (frame == null) {
            try {
                log.println( "creating a textdocument" );
                frameSup = SOF.createTextDoc( null );

                Object oDsk = (
                    (XMultiServiceFactory)tParam.getMSF()).createInstance
                    ("com.sun.star.frame.Desktop") ;
                XDesktop dsk = (XDesktop) UnoRuntime.queryInterface
                    (XDesktop.class, oDsk) ;
                frame = dsk.getCurrentFrame() ;

                Object o = (
                    (XMultiServiceFactory)tParam.getMSF()).createInstance
                    ("com.sun.star.util.URLTransformer") ;
                xURLTrans = (XURLTransformer) UnoRuntime.queryInterface
                    (XURLTransformer.class, o) ;

            } catch ( com.sun.star.uno.Exception e ) {
                // Some exception occures.FAILED
                e.printStackTrace( log );
                throw new StatusException( "Couldn't create a document.", e );
            }
        }

        URL[] urlS = new URL[1];
        urlS[0] = new URL();
        urlS[0].Complete = url;
        boolean res = xURLTrans.parseStrict(urlS);
        log.println("Parsing URL '" + url + "': " + res);
        descr = new PropertyValue[1] ;
        descr[0] = new PropertyValue();
        descr[0].Name = "URL" ;
        descr[0].Value = urlS[0] ;
    }


    /**
     * Tries to load component to a frame. <p>
     * Has <b> OK </b> status if <code>true</code> is returned.
     */
    public void _load() {
        boolean result = oObj.load(descr, frame) ;

        tRes.tested("load()", result) ;
    }

    /**
     * Tries to load component to a frame in separate thread to
     * avoid blocking of the current thread and imediately
     * cancels loading. <p>
     *
     * Has <b> OK </b> status if <code>flase</code> is returned,
     * i.e. loading was not completed.
     */
    public void _cancel() {
        requiredMethod("load()") ;

        final boolean[] result = new boolean[1] ;

        (new Thread() {
            public void run() {
                result[0] = oObj.load(descr, frame);
            }
        }).start();

        oObj.cancel() ;

        try {
            Thread.sleep(1000);
        } catch (InterruptedException ex) {}


        tRes.tested("cancel()", !result[0]) ;
    }

    /**
     * Disposes document if it was created for frame supplying.
     */
    protected void after() {
        if (frameSup != null) {
            frameSup.dispose();
        }
    }
}


