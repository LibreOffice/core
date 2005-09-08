/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _UnoControlProgressBarModel.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:54:05 $
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
* Testing <code>com.sun.star.awt.UnoControlProgressBarModel</code>
* service properties :
* <ul>
*  <li><code> BackgroundColor</code></li>
*  <li><code> Border</code></li>
*  <li><code> DefaultControl</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> FillColor</code></li>
*  <li><code> HelpText</code></li>
*  <li><code> HelpURL</code></li>
*  <li><code> Printable</code></li>
*  <li><code> ProgressValue</code></li>
*  <li><code> ProgressValueMax</code></li>
*  <li><code> ProgressValueMin</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.awt.UnoControlProgressBarModel
*/
public class _UnoControlProgressBarModel extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BackgroundColor() {
        testProperty("BackgroundColor", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? new Integer(1234) : null ;
            }
        }) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _FillColor() {
        testProperty("FillColor", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? new Integer(4321) : null ;
            }
        }) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _ProgressValue() {
        testProperty("ProgressValue", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? new Integer(10) : null ;
            }
        }) ;
    }
}

