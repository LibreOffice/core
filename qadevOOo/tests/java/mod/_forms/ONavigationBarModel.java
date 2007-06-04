/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ONavigationBarModel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-06-04 13:37:07 $
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

package mod._forms;

import java.io.PrintWriter;
import lib.TestEnvironment;
import lib.TestParameters;

public class ONavigationBarModel extends GenericModelTest {

    /**
     * Set some member variable of the super class <CODE>GenericModelTest</CODE>:
     * <pre>
     *    super.m_kindOfControl="NavigationToolBar";
     *    super.m_ObjectName = "om.sun.star.form.component.NavigationToolBar";
     *    super.m_LCShape_Type = "NavigationToolBar";
     * </pre>
     * Then <CODE>super.initialize()</CODE> was called.
     * @param tParam the test parameter
     * @param log the log writer
     */

    protected void initialize(TestParameters tParam, PrintWriter log) {

        super.initialize(tParam, log);

        super.m_kindOfControl="NavigationToolBar";

        super.m_ObjectName = "com.sun.star.form.component.NavigationToolBar";

        super.m_LCShape_Type = "NavigationToolBar";

    }
    /**
     * calls <CODE>cleanup()</CODE> from it's super class
     * @param tParam the test parameter
     * @param log the log writer
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        super.cleanup(tParam, log);
    }


    /**
     * calls <CODE>createTestEnvironment()</CODE> from it's super class
     * @param Param the test parameter
     * @param log the log writer
     * @return lib.TestEnvironment
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) {
        return super.createTestEnvironment(Param, log);
    }

}    // finish class ONavigationBarModel

