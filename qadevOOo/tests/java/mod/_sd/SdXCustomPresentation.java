/*************************************************************************
 *
 *  $RCSfile: SdXCustomPresentation.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:27:17 $
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

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.presentation.XCustomPresentationSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.presentation.CustomPresentation</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::container::XIndexContainer</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexReplace</code></li>
* </ul>
* @see com.sun.star.presentation.CustomPresentation
* @see com.sun.star.container.XNamed
* @see com.sun.star.container.XIndexContainer
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XIndexReplace
* @see ifc.container._XNamed
* @see ifc.container._XIndexContainer
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XIndexReplace
*/
public class SdXCustomPresentation extends TestCase {
    XComponent xImpressDoc;

    /**
    * Creates Impress document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xImpressDoc = SOF.createImpressDoc(null);;
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Impress document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xImpressDoc");
        xImpressDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the CustomPresentation from the document
    * using the interface <code>XCustomPresentationSupplier</code>. Creates and
    * inserts new instance of the presentation to the collection. The created
    * instance is the instance of the service
    * <code>com.sun.star.presentation.CustomPresentation</code>. Obtains
    * the collection of draw pages from the document and takes one of them.
    * Inserts the obtained draw page to the created instance. Inserts some new
    * draw pages.
    * Object relations created :
    * <ul>
    *  <li> <code>'XIndexContainerINDEX'</code> for
    *      {@link ifc.container._XIndexContainer}(the string representation of
    *      the zero)</li>
    *  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> for
    *      {@link ifc.container._XIndexContainer}(the new inserted draw pages)</li>
    * </ul>
    * @see com.sun.star.presentation.XCustomPresentationSupplier
    * @see com.sun.star.presentation.CustomPresentation
    */
    protected TestEnvironment createTestEnvironment(
                                TestParameters Param, PrintWriter log) {


        log.println( "creating a test environment" );

        log.println( "get presentation" );
        XCustomPresentationSupplier oPS = (XCustomPresentationSupplier)
            UnoRuntime.queryInterface(
                XCustomPresentationSupplier.class, xImpressDoc);
        XInterface oObj = oPS.getCustomPresentations();

        XSingleServiceFactory oSingleMSF = (XSingleServiceFactory)
            UnoRuntime.queryInterface(XSingleServiceFactory.class, oObj);

        XInterface oInstance = null;
        try {
            oInstance = (XInterface) oSingleMSF.createInstance();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        XNameContainer aContainer = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, oObj);

        try {
            aContainer.insertByName("FirstPresentation", oInstance);
        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert Presentation", e);
        } catch (com.sun.star.container.ElementExistException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert Presentation", e);
        } catch (com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert Presentation", e);
        }

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, xImpressDoc);
        XDrawPages oDPn = oDPS.getDrawPages();
        XIndexAccess oDPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oDPn);

        XDrawPage oDrawPage = null;
        try {
            oDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        }

        XIndexContainer aIContainer = (XIndexContainer)
            UnoRuntime.queryInterface(XIndexContainer.class,oInstance);

        try {
            aIContainer.insertByIndex(0, oDrawPage);
        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert DrawPage", e);
        } catch (com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert DrawPage", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't insert DrawPage", e);
        }

        log.println( "creating a new environment for XPresentation object" );
        TestEnvironment tEnv = new TestEnvironment( oInstance );

        int THRCNT=1;
        if ((String)Param.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)Param.get("THRCNT"));
        }

        // INDEX : _XNameContainer
        log.println( "adding XIndexContainerINDEX as mod relation to environment" );
        tEnv.addObjRelation("XIndexContainerINDEX", "0");

        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println( "adding INSTANCEn as mod relation to environment" );
        try {
            for (int n = 1; n < (2*THRCNT+1) ;n++ ) {
                log.println( "adding INSTANCE" + n
                    +" as mod relation to environment" );
                oDPn.insertNewByIndex(0);
                oDrawPage = (XDrawPage) AnyConverter.toObject(
                        new Type(XDrawPage.class),oDPi.getByIndex(0));
                tEnv.addObjRelation("INSTANCE" + n, oDrawPage);
            }
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't adding INSTANCEn", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't adding INSTANCEn", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't adding INSTANCEn", e);
        }

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SdXCustomPresentation
