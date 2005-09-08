/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _UnoControlImageControlModel.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:52:48 $
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

package ifc.awt;

import lib.MultiPropertyTest;
import util.utils;

/**
* Testing <code>com.sun.star.awt.UnoControlImageControlModel</code>
* service properties :
* <ul>
*  <li><code> BackgroundColor </code></li>
*  <li><code> Border </code></li>
*  <li><code> DefaultControl </code></li>
*  <li><code> Enabled </code></li>
*  <li><code> ImageURL </code></li>
*  <li><code> Printable </code></li>
* </ul>
* Almost all properties testing is automated by
* <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.awt.UnoControlImageControlModel
*/
public class _UnoControlImageControlModel extends MultiPropertyTest {

    /**
    * Custom <code>PropertyTester</code> used in <code>ImageURL</code> test.
    */
    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg");
            else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    /**
    * Test changes one image URL onto another.
    */
    public void _ImageURL() {
        testProperty("ImageURL", URLTester) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BackgroundColor() {
        testProperty("BackgroundColor", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                return utils.isVoid(oldValue) ? new Integer(8945) :
                    super.getNewValue(propName, oldValue) ;
            }
        }) ;
    }

   /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BorderColor() {
        testProperty("BorderColor", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? new Integer(1234) : null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Tabstop() {
        testProperty("Tabstop", new PropertyTester() {
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Boolean(true) :
                    null ;
            }
        }) ;
    }

}

