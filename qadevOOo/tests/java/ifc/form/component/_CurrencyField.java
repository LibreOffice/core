/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _CurrencyField.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:50:06 $
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

package ifc.form.component;

import lib.MultiPropertyTest;


/**
* Testing <code>com.sun.star.form.component.CurrencyField</code>
* service properties :
* <ul>
*  <li><code> DefaultValue</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.form.component.CurrencyField
*/
public class _CurrencyField extends MultiPropertyTest {

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _DefaultValue() {
        testProperty("DefaultValue", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return util.utils.isVoid(old) ? new Double(17.5) :
                    super.getNewValue(p, old) ;
            }
        }) ;
    }
}  // finish class _CurrencyField


