/*************************************************************************
 *
 *  $RCSfile: SdXImpressDocument.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:27:34 $
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

package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

/**
* Test for object which is represented by service
* <code>com.sun.star.presentation.PresentationDocument</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XMultiServiceFactory</code></li>
*  <li> <code>com::sun::star::drawing::XMasterPagesSupplier</code></li>
*  <li> <code>com::sun::star::presentation::XCustomPresentationSupplier</code></li>
*  <li> <code>com::sun::star::document::XLinkTargetSupplier</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::drawing::XLayerSupplier</code></li>
*  <li> <code>com::sun::star::presentation::XPresentationSupplier</code></li>
*  <li> <code>com::sun::star::style::XStyleFamiliesSupplier</code></li>
*  <li> <code>com::sun::star::drawing::DrawingDocument</code></li>
*  <li> <code>com::sun::star::drawing::XDrawPageDuplicator</code></li>
*  <li> <code>com::sun::star::drawing::XDrawPagesSupplier</code></li>
* </ul>
* @see com.sun.star.presentation.PresentationDocument
* @see com.sun.star.lang.XMultiServiceFactory
* @see com.sun.star.drawing.XMasterPagesSupplier
* @see com.sun.star.presentation.XCustomPresentationSupplier
* @see com.sun.star.document.XLinkTargetSupplier
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.drawing.XLayerSupplier
* @see com.sun.star.presentation.XPresentationSupplier
* @see com.sun.star.style.XStyleFamiliesSupplier
* @see com.sun.star.drawing.DrawingDocument
* @see com.sun.star.drawing.XDrawPageDuplicator
* @see com.sun.star.drawing.XDrawPagesSupplier
* @see ifc.lang._XMultiServiceFactory
* @see ifc.drawing._XMasterPagesSupplier
* @see ifc.presentation._XCustomPresentationSupplier
* @see ifc.document._XLinkTargetSupplier
* @see ifc.beans._XPropertySet
* @see ifc.drawing._XLayerSupplier
* @see ifc.presentation._XPresentationSupplier
* @see ifc.style._XStyleFamiliesSupplier
* @see ifc.drawing._DrawingDocument
* @see ifc.drawing._XDrawPageDuplicator
* @see ifc.drawing._XDrawPagesSupplier
*/
public class SdXImpressDocument extends TestCase {
    XComponent xImpressDoc;

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress document.
    * @param tParam test parameters
    * @param tEnv the environment to cleanup
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xImpressDoc");
        xImpressDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates new impress document that is the instance of the service
    * <code>com.sun.star.presentation.PresentationDocument</code>.
    * @see com.sun.star.presentation.PresentationDocument
    */
    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        log.println( "creating a test environment" );
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a impress document" );
            xImpressDoc = SOF.createImpressDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        log.println( "creating a new environment for drawpage object" );
        TestEnvironment tEnv = new TestEnvironment( xImpressDoc );

        return tEnv;
    } // finish method getTestEnvironment


}    // finish class SdDrawPage

