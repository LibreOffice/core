/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            @Override
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? Integer.valueOf(6543) : null ;
            }
        }) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BorderColor() {
        testProperty("BorderColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? Integer.valueOf(1234) : null ;
            }
        }) ;
    }


    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Date() {
        testProperty("Date", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? Integer.valueOf(6543) :
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
            @Override
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? Boolean.TRUE : null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String prop, Object old) {
                return utils.isVoid(old) ? Integer.valueOf(123) : null ;
            }
        }) ;
    }

    /**
    * Redefined method returns value, that differs from property value.
    */
    protected PropertyTester ColorTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            if (util.ValueComparer.equalValue(oldValue, Integer.valueOf(17)))
                return Integer.valueOf(25);
            else
                return Integer.valueOf(17);
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
                dsc = Boolean.TRUE;
            } else {
                dsc = (Boolean) oDsc;
                dsc = Boolean.valueOf(!dsc.booleanValue());
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

