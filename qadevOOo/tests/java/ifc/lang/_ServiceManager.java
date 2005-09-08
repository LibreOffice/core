/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _ServiceManager.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:14:21 $
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

package ifc.lang;

import lib.MultiPropertyTest;

import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.XComponentContext;

public class _ServiceManager extends MultiPropertyTest {

    public void _DefaultContext() {
        XComponentContext get = null;
        boolean res = false;
        try {
            get = (XComponentContext)AnyConverter.toObject(new Type(XComponentContext.class), oObj.getPropertyValue("DefaultContext"));
            res = get != null;
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Illegal Argument Exception");
            res = false;
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Property is optional and not supported");
            res = true;
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("Unexpected Exception");
            wte.printStackTrace(log);
            res = false;
        }
        tRes.tested("DefaultContext", res);

    }

} // EOF _ServiceManager

