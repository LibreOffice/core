/*************************************************************************
 *
 *  $RCSfile: _XInteractionHandler.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:11:44 $
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

package ifc.task;



/**
* <code>com.sun.star.task.XInteractionHandler</code> interface testing.
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInteractionHandler.Request'</code>
*    (of type <code>com.sun.star.task.XInteractionRequest</code>):
*    this interface implementation is handler specific and is
*    passed as argument to method <code>handle</code>. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.task.XInteractionHandler
*/
import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XInteractionRequest;

/**
* <code>com.sun.star.task.XInteractionHandler</code> interface testing.
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInteractionHandler.Request'</code>
*    (of type <code>com.sun.star.task.XInteractionRequest</code>):
*    this interface implementation is handler specific and is
*    passed as argument to method <code>handle</code>. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.task.XInteractionHandler
*/
public class _XInteractionHandler extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XInteractionHandler oObj = null ;

    private XInteractionRequest request = null ;

    /**
    * Retrieves an object relation. <p>
    * @throws StatusException If the relation is not found.
    */
    public void before() {
        request = (XInteractionRequest)
            tEnv.getObjRelation("XInteractionHandler.Request") ;

        //if (request == null)
        //    throw new StatusException(Status.failed("Reelation not found")) ;
    }

    /**
    * Sinse this test is interactive (dialog window can't be
    * disposed using API) it is skipped. <p>
    * Always has <b>SKIPPED.OK</b> status .
    */
    public void _handle() {

        /*

        final XMultiServiceFactory msf = (XMultiServiceFactory)tParam.getMSF() ;
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );
        XComponent textDoc = null ;
        try {
            textDoc = SOF.createTextDoc( null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create Document") ;
            tRes.tested("handle()", false) ;
            return ;
        }
        final XComponent fTextDoc = textDoc ;
        final XModel xModelDoc = (XModel)
            UnoRuntime.queryInterface(XModel.class, textDoc);

        Thread thr = new Thread( new Runnable() {
            public void run() {
                try {
                    Thread.sleep(1000) ;
                } catch (InterruptedException e ) {}

                //fTextDoc.dispose() ;

                XFrame docFr = xModelDoc.getCurrentController().getFrame() ;
                docFr.dispose() ;

                ///*
                try {

                    Object dsk = msf.createInstance
                        ("com.sun.star.frame.Desktop");

                    XFrame xDsk = (XFrame)
                        UnoRuntime.queryInterface(XFrame.class, dsk) ;

                    XFrame fr = xDsk.findFrame("_top", 55) ;

                    XWindow win = fr.getContainerWindow() ;

                    String name = null ;
                    if (fr != null) {
                        name = fr.getName() ;
                        docFr.dispose() ;
                    }

                } catch (com.sun.star.uno.Exception e) {
                    e.printStackTrace();
                } //
            }
        }) ;
        thr.start() ;

        oObj.handle(request) ;
        try {
            thr.join(500) ;
        } catch (InterruptedException e ) {}
        */

        tRes.tested("handle()", Status.skipped(true)) ;
    }
}


