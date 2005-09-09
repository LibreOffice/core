/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPropertyContainer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:08:45 $
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
