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

/**
* Testing <code>com.sun.star.awt.UnoControlFormattedFieldModel</code>
* service properties :
* <ul>
*  <li><code> Align</code></li>
*  <li><code> BackgroundColor</code></li>
*  <li><code> Border</code></li>
*  <li><code> DefaultControl</code></li>
*  <li><code> EffectiveDefault</code></li>
*  <li><code> EffectiveMax</code></li>
*  <li><code> EffectiveMin</code></li>
*  <li><code> EffectiveValue</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> FontDescriptor</code></li>
*  <li><code> FormatKey</code></li>
*  <li><code> FormatsSupplier</code></li>
*  <li><code> MaxTextLen</code></li>
*  <li><code> Printable</code></li>
*  <li><code> ReadOnly</code></li>
*  <li><code> Spin</code></li>
*  <li><code> StrictFormat</code></li>
*  <li><code> Tabstop</code></li>
*  <li><code> Text</code></li>
*  <li><code> TextColor</code></li>
*  <li><code> HelpText</code></li>
*  <li><code> HelpURL</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.awt.UnoControlFormattedFieldModel
*/
public class _UnoControlFormattedFieldModel extends MultiPropertyTest {

    /**
    * Redefined method must return a valid service name, that differs from
    * name exists.
    *
    */
    protected PropertyTester ControlTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            if (util.ValueComparer.equalValue(oldValue,
                        "com.sun.star.form.control.FormattedField"))
                return "com.sun.star.awt.UnoControlFormattedField";
            else
                return "com.sun.star.form.control.FormattedField";
        }
    };

    /**
    * This property must contain a valid service name.
    */
    public void _DefaultControl() {
        log.println("Testing with custom Property tester") ;
        testProperty("DefaultControl", ControlTester) ;
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
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? Integer.valueOf(123) :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextLineColor() {
        testProperty("TextLineColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? Integer.valueOf(123) :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _BackgroundColor() {
        testProperty("BackgroundColor", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? Integer.valueOf(123) :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _EffectiveDefault() {
        testProperty("EffectiveDefault", new Double(5.8),new Double(2.3)) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _EffectiveValue() {
        testProperty("EffectiveValue", new Double(5.8),new Double(2.3)) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _EffectiveMax() {
        testProperty("EffectiveMax", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Double(123.8) :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _FormatsSupplier() {
        testProperty("FormatsSupplier", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                Object newValue = null;
                try {
                    newValue = tParam.getMSF().createInstance(
                                    "com.sun.star.util.NumberFormatsSupplier");
                } catch (com.sun.star.uno.Exception e) {
                    System.out.println("caught exception: " + e);
                }
                return newValue;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _EffectiveMin() {
        testProperty("EffectiveMin", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Double(0.1) :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _FormatKey() {
        testProperty("FormatKey", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? Integer.valueOf(0) :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _StrictFromat() {
        testProperty("StrictFromat", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? Boolean.TRUE :
                    super.getNewValue(p,v) ;
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? Boolean.TRUE :
                    null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Text() {
        testProperty("Text", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? "UnoControlFormattedFieldModel" :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }
}
