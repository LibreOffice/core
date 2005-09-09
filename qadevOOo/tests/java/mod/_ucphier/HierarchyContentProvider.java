/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HierarchyContentProvider.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:53:12 $
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

package mod._ucphier;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifierFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class HierarchyContentProvider extends TestCase {

    public TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log ) {

        XInterface oObj = null;
        Object oInterface = null;
        Object ucb = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.comp.ucb.HierarchyContentProvider" );
            ucb = xMSF.createInstance
                ( "com.sun.star.comp.ucb.UniversalContentBroker" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        XContentIdentifierFactory CIF = (XContentIdentifierFactory)
            UnoRuntime.queryInterface(XContentIdentifierFactory.class,ucb);

        System.out.println("ImplementationName: "+util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );

        //Adding ObjRelation for XContentProvider
        tEnv.addObjRelation("FACTORY",CIF);
        tEnv.addObjRelation("CONTENT1","vnd.sun.star.hier:/");

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class HierarchyContentProvider

