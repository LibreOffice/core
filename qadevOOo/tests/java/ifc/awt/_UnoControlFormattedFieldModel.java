/*************************************************************************
 *
 *  $RCSfile: _UnoControlFormattedFieldModel.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:08:16 $
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
