/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StandaloneDocumentInfoUnitTest.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:26:54 $
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
package complex.standalonedocumentinfo;

import complexlib.ComplexTestCase;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/* Document here
*/

public class StandaloneDocumentInfoUnitTest extends ComplexTestCase {
    private XMultiServiceFactory m_xMSF = null;

    public String[] getTestMethodNames() {
        return new String[] {
            "ExecuteTest01"};
    }

    public String[] getTestObjectNames() {
        return new String[] {"StandaloneDocumentInfoUnitTest"};
    }

    public void before() {
        try {
            m_xMSF = (XMultiServiceFactory)param.getMSF();
        } catch(Exception e) {
            failed( "Failed to create service factory!" );
        }
        if( m_xMSF ==null ) {
            failed( "Failed to create service factory!" );
        }
    }

    public void after() {
        m_xMSF = null;
    }

    public void ExecuteTest01() {
        StandaloneDocumentInfoTest aTest = new Test01 (m_xMSF, log);
        assure( "Test01 failed!", aTest.test() );
    }
}


