/*************************************************************************
 *
 *  $RCSfile: _XPropertyContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-09 13:49:47 $
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
package complex.tdoc.interfaces;

import com.sun.star.beans.XPropertyContainer;
import share.LogWriter;

/**
 *
 * @author  sg128468
 */
public class _XPropertyContainer {
    public XPropertyContainer oObj = null;
    public LogWriter log = null;

    public boolean _addProperty() {
        boolean result = true;
        // add illegal property
        try {
            oObj.addProperty("MyIllegalProperty", (short)0, null);
        }
        catch(com.sun.star.beans.PropertyExistException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            log.println("'IllegalArgument' Unexpected but correct.");
        }
        catch(com.sun.star.beans.IllegalTypeException e) {
            log.println("'IllegalType' Correctly thrown");
        }
        // add valid property
        try {
            oObj.addProperty("MyLegalProperty", (short)0, "Just a value");
        }
        catch(com.sun.star.beans.PropertyExistException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.beans.IllegalTypeException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        return result;
    }

    public boolean _removeProperty() {
        boolean result = true;
        try {
            oObj.removeProperty("MyIllegalProperty");
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("'UnknownProperty' Correctly thrown");
        }
        catch(com.sun.star.beans.NotRemoveableException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        try {
            oObj.removeProperty("MyLegalProperty");
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.beans.NotRemoveableException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        return result;
    }

}
