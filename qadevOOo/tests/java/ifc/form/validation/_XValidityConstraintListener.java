/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XValidityConstraintListener.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:58:28 $
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
package ifc.form.validation;

import com.sun.star.form.validation.XValidatable;
import com.sun.star.form.validation.XValidator;
import com.sun.star.form.validation.XValidityConstraintListener;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;


public class _XValidityConstraintListener extends MultiMethodTest {
    public XValidityConstraintListener oObj;
    protected boolean ValidatorCalled = false;

    public void _validityConstraintChanged() {
        boolean res = false;
        try {
            XValidatable xValidatable = (XValidatable) UnoRuntime.queryInterface(
                                                XValidatable.class,
                                                tEnv.getTestObject());

            log.println("adding Validator");
            XValidator xValidator = new MyValidator();
            xValidatable.setValidator(xValidator);
            ValidatorCalled = false;

            log.println("calling validityConstraintChanged()");
            oObj.validityConstraintChanged(
                    new com.sun.star.lang.EventObject());
            res = ValidatorCalled;
        } catch (com.sun.star.util.VetoException e) {
            e.printStackTrace();
        }
        tRes.tested("validityConstraintChanged()",res);
    }

    /*
     * The validator to add this Listener implementation
     *
     */
    public class MyValidator implements XValidator {
        public void addValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
            throws com.sun.star.lang.NullPointerException {
            log.println("\t Validator::addValidityConstraintListener called");
        }

        public String explainInvalid(Object obj) {
            log.println("\t Validator::explainInvalid() called");

            return "explainInvalid";
        }

        public boolean isValid(Object obj) {
            log.println("\t Validator::isValid() called");
            ValidatorCalled = true;
            return false;
        }

        public void removeValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
            throws com.sun.star.lang.NullPointerException {
            log.println("\t Validator::removeValidityConstraintListener called");
        }
    }
}