/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _GenericDrawPage.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:33:16 $
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

package ifc.drawing;

import lib.MultiPropertyTest;


/**
* Testing <code>com.sun.star.drawing.GenericDrawPage</code>
* service properties :
* <ul>
*  <li><code> BorderBottom</code></li>
*  <li><code> BorderLeft</code></li>
*  <li><code> BorderRight</code></li>
*  <li><code> BorderTop</code></li>
*  <li><code> Height</code></li>
*  <li><code> Width</code></li>
*  <li><code> Number</code></li>
*  <li><code> Orientation</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.GenericDrawPage
*/
public class _GenericDrawPage extends MultiPropertyTest {

    public void _Number() {
        log.println("Number started");
        boolean res = false;
        log.println("trying to get the value");
        try {
            Short wat = (Short) oObj.getPropertyValue("Number");
            if (wat == null) {
                log.println("it is null");
            } else {
                log.println("it isn't null");
                res=true;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("an UnknownPropertyException occured");

        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("an WrappedTargetException occured");
        }

        tRes.tested("Number",res);
    }

    public void _UserDefinedAttributes() {
        log.println("Userdefined started");
        boolean res = false;
        log.println("trying to get the value");
        try {
            Object wat = oObj.getPropertyValue("UserDefinedAttributes");
            if (wat == null) {
                log.println("it is null");
            } else {
                log.println("it isn't null");
                res=true;
            }
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("an UnknownPropertyException occured");

        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("an WrappedTargetException occured");
        }

        tRes.tested("UserDefinedAttributes",res);
    }

} // end of GenericDrawPage

