/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SmModel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:22:51 $
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

package mod._sm;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

/**
 * Test for object which is represented by
 * <code>'StarMath'</code> document. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::formula::FormulaProperties</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.formula.FormulaProperties
 * @see ifc.formula._FormulaProperties
 */
public class SmModel extends TestCase {
    XComponent xMathDoc;

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates a <code>StarMath</code> document and passes it as
     * tested component.
     */
    public synchronized TestEnvironment createTestEnvironment
            ( TestParameters Param, PrintWriter log )
            throws StatusException {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );
        try {
            xMathDoc = SOF.openDoc("smath","_blank");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.io.IOException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        }

        String Iname = util.utils.getImplName(xMathDoc);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(xMathDoc);
        return tEnv;
    }

    /**
     * Disposes the document created in <code>createTestEnvironment</code>
     * method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }


}    // finish class SmModel

