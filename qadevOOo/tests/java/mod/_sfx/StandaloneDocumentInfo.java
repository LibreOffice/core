/*************************************************************************
 *
 *  $RCSfile: StandaloneDocumentInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:28:45 $
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

package mod._sfx;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.document.StandaloneDocumentInfo</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::document::DocumentInfo</code></li>
 *  <li> <code>com::sun::star::document::XDocumentInfo</code></li>
 *  <li> <code>com::sun::star::document::XStandaloneDocumentInfo</code></li>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 * </ul> <p>
 * The following files used by this test :
 * <ul>
 *  <li><b> SfxStandaloneDocInfoObject.sdw </b> : is copied
 *   to SOffice temporary directory and is used to load and
 *   save its info. </li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.document.DocumentInfo
 * @see com.sun.star.document.XDocumentInfo
 * @see com.sun.star.document.XStandaloneDocumentInfo
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.beans.XPropertySet
 * @see ifc.lang._XComponent
 * @see ifc.document._DocumentInfo
 * @see ifc.document._XDocumentInfo
 * @see ifc.document._XStandaloneDocumentInfo
 * @see ifc.beans._XFastPropertySet
 * @see ifc.beans._XPropertySet
 */
public class StandaloneDocumentInfo extends TestCase {

    String destUrl = null ;

    /**
     * Copies file 'SfxStandaloneDocInfoObject.sdw' to tempopary
     * location (overwriting the old file if exists).
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        String srcUrl = utils.getFullTestURL("SfxStandaloneDocInfoObject.sdw") ;
        destUrl = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF()) +
            "SfxStandaloneDocInfoObject.sdw";

        utils.overwriteFile((XMultiServiceFactory)tParam.getMSF(), srcUrl, destUrl) ;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.document.StandaloneDocumentInfo</code>.
     *     Object relations created :
     * <ul>
     *  <li> <code>'DOCURL'</code> for
     *      {@link ifc.document._XStandaloneDocumentInfo} :
     *      the Writer file URL in temporary location. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)xMSF.createInstance
                ("com.sun.star.document.StandaloneDocumentInfo");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);
        XStandaloneDocumentInfo the_info = (XStandaloneDocumentInfo)
                UnoRuntime.queryInterface(XStandaloneDocumentInfo.class, oObj);
        try {
            the_info.loadFromURL(destUrl);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't load document",e);
        }
        tEnv.addObjRelation("DOCURL",destUrl);
        return tEnv;
    }

}    // finish class TestCase

