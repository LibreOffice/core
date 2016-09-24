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
* Testing <code>com.sun.star.awt.UnoControlComboBoxModel</code>
* service properties :
* <ul>
*  <li><code> Autocomplete</code></li>
*  <li><code> BackgroundColor</code></li>
*  <li><code> Border</code></li>
*  <li><code> DefaultControl</code></li>
*  <li><code> Dropdown</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> FontDescriptor</code></li>
*  <li><code> LineCount</code></li>
*  <li><code> MaxTextLen</code></li>
*  <li><code> Printable</code></li>
*  <li><code> ReadOnly</code></li>
*  <li><code> StringItemList</code></li>
*  <li><code> Tabstop</code></li>
*  <li><code> Text</code></li>
*  <li><code> TextColor</code></li>
* </ul>
* @see com.sun.star.awt.UnoControlComboBoxModel
*/
public class _UnoControlComboBoxModel extends MultiPropertyTest {

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BackgroundColor() {
        testProperty("BackgroundColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? Integer.valueOf(1234) : null ;
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
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? Integer.valueOf(4321) : null ;
            }
        }) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _Tabstop() {
        testProperty("Tabstop", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? Boolean.TRUE : null ;
            }
        }) ;
    }

    public void _Text() {
        try {
            oObj.setPropertyValue("MaxTextLen",Short.valueOf((short)0));
        } catch (com.sun.star.beans.UnknownPropertyException ue) {
        } catch (com.sun.star.beans.PropertyVetoException pe) {
        } catch (com.sun.star.lang.IllegalArgumentException ie) {
        } catch (com.sun.star.lang.WrappedTargetException we) {
        }
        testProperty("Text");
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

} //EOF UnoControlComboBoxModel

