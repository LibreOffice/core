/*************************************************************************
 *
 *  $RCSfile: _UnoControlDateFieldModel.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:07:18 $
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

package ifc.awt;

import lib.MultiPropertyTest;
import util.utils;

/*
* Testing <code>com.sun.star.awt.UnoControlDateFieldModel</code>
* service properties :
* <ul>
*  <li><code> BackgroundColor</code></li>
*  <li><code> Border</code></li>
*  <li><code> Date</code></li>
*  <li><code> DateMax</code></li>
*  <li><code> DateMin</code></li>
*  <li><code> DefaultControl</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> DateFormat</code></li>
*  <li><code> FontDescriptor</code></li>
*  <li><code> Printable</code></li>
*  <li><code> ReadOnly</code></li>
*  <li><code> Spin</code></li>
*  <li><code> StrictFormat</code></li>
*  <li><code> Tabstop</code></li>
*  <li><code> TextColor</code></li>
* </ul>
* Almost all properties testing is automated by
* <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.awt.UnoControlDateFieldModel
*/
public class _UnoControlDateFieldModel extends MultiPropertyTest {

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _BackgroundColor() {
        testProperty("BackgroundColor", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? new Integer(6543) : null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Date() {
        testProperty("Date", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? new Integer(6543) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Tabstop() {
        testProperty("Tabstop", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? new Boolean(true) : null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? new Integer(123) : null ;
            }
        }) ;
    }

    /**
    * Redefined method returns value, that differs from property value.
    */
    protected PropertyTester ColorTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (util.ValueComparer.equalValue(oldValue, new Integer(17)))
                return new Integer(25);
            else
                return new Integer(17);
        }
    };

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextLineColor() {
        log.println("Testing with custom Property tester") ;
        testProperty("TextLineColor", ColorTester) ;
    }

    public void _DateShowCentury() {
        boolean res = false;
        try {
            util.dbg.printPropertyInfo(oObj,"DateShowCentury",log);
            Object oDsc = oObj.getPropertyValue("DateShowCentury");
            Boolean dsc = null;
            if (util.utils.isVoid(oDsc)) {
                log.println("Property is void, set it to true");
                dsc = new Boolean(true);
            } else {
                dsc = (Boolean) oDsc;
                dsc = new Boolean(!dsc.booleanValue());
                log.println("Setting property to "+dsc);
            }
            oObj.setPropertyValue("DateShowCentury",dsc);
            Boolean getdsc = (Boolean) oObj.getPropertyValue("DateShowCentury");
            log.println("Getting value "+getdsc);
            res = dsc.equals(getdsc);
        } catch (com.sun.star.beans.UnknownPropertyException upe) {
            log.println("Don't know the Property 'DateShowCentury'");
        } catch (com.sun.star.lang.WrappedTargetException wte) {
            log.println("WrappedTargetException while getting Property 'DateShowCentury'");
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("IllegalArgumentException while getting Property 'DateShowCentury'");
        } catch (com.sun.star.beans.PropertyVetoException pve) {
            log.println("PropertyVetoException while getting Property 'DateShowCentury'");
        }
        tRes.tested("DateShowCentury",res);

    }
}

