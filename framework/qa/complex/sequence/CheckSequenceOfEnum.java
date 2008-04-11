/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckSequenceOfEnum.java,v $
 * $Revision: 1.4 $
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
package complex.sequence;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.enumexample.XEnumSequence;
import com.sun.star.beans.PropertyState;
import com.sun.star.uno.UnoRuntime;
import complexlib.ComplexTestCase;

/**
 * The test is for bug 111128. The mapping of sequence<enumeration> between
 * Java and C++ from the IDL definition was erroneous. This test checks, if
 * the mapping works.
 */
public class CheckSequenceOfEnum extends ComplexTestCase {

    /**
     * Return all test methods.
     * @return The test methods.
     */
    public String[] getTestMethodNames() {
        return new String[]{"checkSequence"};
    }

    /**
     * Check the sequence<enumeration> mapping between Java and C++.
     * Since the Office does
     * not use such a construct itself, a C++ component with an own defined
     * interface is used for testing.
     */
    public void checkSequence() {
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory) param.getMSF();
            Object oObj = xMSF.createInstance("com.sun.star.enumexample.ChangeSequenceOrder");
            assure("Build the shared library 'changeSequenceOrder' in directory 'enumexample' and\n"
                    + "register the created zip 'EnumSequenceComponent' before executing this test.", oObj != null);
            // build a first sequence
            PropertyState[] aOriginalSequence = new PropertyState[] {
                    PropertyState.DIRECT_VALUE,
                    PropertyState.DEFAULT_VALUE,
                    PropertyState.AMBIGUOUS_VALUE
            };
            XEnumSequence xSequence = (XEnumSequence)UnoRuntime.queryInterface(XEnumSequence.class, oObj);
            PropertyState[] aChangedSequence = xSequence.getSequenceInChangedOrder(aOriginalSequence);
            assure("Did not return a correct sequence.", checkSequence(aOriginalSequence, aChangedSequence));
        }
        catch(Exception e) {
            e.printStackTrace();
            failed("Exception!");
        }
    }

    private boolean checkSequence(PropertyState[] aOriginalSequence, PropertyState[] aChangedSequence) {
        boolean erg = true;
        int length = aOriginalSequence.length;
        for ( int i=0; i<length; i++ ) {
            if ( aOriginalSequence[i] != aChangedSequence[length -1 - i]) {
                log.println("Checking '" + aOriginalSequence[i] + "' == '" + aChangedSequence[length - 1 - i] + "'");
                erg = false;
            }
            if ( aChangedSequence[length - 1 - i].getValue() != PropertyState.fromInt(i).getValue() ) {
                log.println("Checking '" + aChangedSequence[length - 1 - i].getValue() + "' == '" + PropertyState.fromInt(i).getValue() + "'");
                erg = false;
            }
        }
        return erg;
    }
}


