/*************************************************************************
 *
 *  $RCSfile: TestComponentB.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:14:03 $
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
package JavaComp;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.test.XSomethingB;
import com.sun.star.uno.Type;

// TestComponentB implements all necessary interfaces self, this is only
// for demonstration. More convenient is to use the impelmentation WeakBase or
// ComponentBase, see implementation of TestComponentA.
public class TestComponentB implements XTypeProvider, XServiceInfo, XSomethingB {
    static final String __serviceName= "JavaTestComponentB";

       static byte[] _implementationId;
    private XComponentContext context;
    private Object[] args;

    public TestComponentB(XComponentContext context, Object[] args) {
        this.context= context;
        this.args= args;
    }

    // XSomethingB
    public String methodTwo(String val) {
        if (args.length > 0 && args[0] instanceof String )
            return (String) args[0];
        return val;
    }

    //XTypeProvider
    public com.sun.star.uno.Type[] getTypes(  ) {
        Type[] retValue= new Type[3];
        retValue[0]= new Type( XServiceInfo.class);
        retValue[1]= new Type( XTypeProvider.class);
        retValue[2]= new Type( XSomethingB.class);
        return retValue;
    }
    //XTypeProvider
    synchronized public byte[] getImplementationId(  ) {
        if (_implementationId == null) {
            _implementationId= new byte[16];
            int hash = hashCode();
            _implementationId[0] = (byte)(hash & 0xff);
            _implementationId[1] = (byte)((hash >>> 8) & 0xff);
            _implementationId[2] = (byte)((hash >>> 16) & 0xff);
            _implementationId[3] = (byte)((hash >>>24) & 0xff);
        }
        return _implementationId;
    }

    //XServiceInfo
    public String getImplementationName(  ) {
        return getClass().getName();
    }

    // XServiceInfo
    public boolean supportsService( /*IN*/String serviceName ) {
        if ( serviceName.equals( __serviceName))
            return true;
        return false;
    }
    //XServiceInfo
    public String[] getSupportedServiceNames(  ) {
        String[] retValue= new String[0];
        retValue[0]= __serviceName;
        return retValue;
    }
}
