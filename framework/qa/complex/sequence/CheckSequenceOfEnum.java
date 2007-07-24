/*************************************************************************
 *
 *  $RCSfile: CheckSequenceOfEnum.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2007-07-24 13:28:36 $
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


