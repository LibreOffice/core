/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.awt;

import lib.MultiPropertyTest;
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;

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
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? new Integer(1234) : null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Integer(123) :
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Integer(123) :
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Integer(123) :
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
            protected Object getNewValue(String p, Object v) {
                Object newValue = null;
                try {
                    newValue = ((XMultiServiceFactory)tParam.getMSF()).createInstance(
                                    "com.sun.star.util.NumberFormatsSupplier");
                } catch (com.sun.star.uno.Exception e) {}
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Integer(0) :
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Boolean(true) :
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Boolean(true) :
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
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? "UnoControlFormattedFieldModel" :
                    super.getNewValue(p,v) ;
            }
        }) ;
    }
}
