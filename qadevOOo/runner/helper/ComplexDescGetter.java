/*************************************************************************
 *
 *  $RCSfile: ComplexDescGetter.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-01-27 16:27:31 $
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
package helper;

import complexlib.ComplexTestCase;
import share.DescEntry;
import share.DescGetter;
import share.ComplexTest;

/**
 *
 */
public class ComplexDescGetter implements DescGetter {

    ComplexTest testClass;
    /** Creates new ComplexDescGetter */
    public ComplexDescGetter(ComplexTest tClass) {

        testClass = tClass;
    }

    public DescEntry[] getDescriptionFor(String entry, String DescPath,
                                                            boolean debug) {

        if (debug) {
            System.out.println("Searching Class: "+((Object)testClass).toString());
            System.out.println("Test Name: "+entry);
        }

        String testObjectName = testClass.getTestObjectName();
        if (testObjectName != null) {
            if (testObjectName.equals(""))
                testObjectName = entry;
        }
        else
            testObjectName = entry;

        String[] testMethodName = testClass.getTestMethodNames();
        DescEntry dEntry = new DescEntry();

        dEntry.entryName = testObjectName;
        dEntry.longName = testObjectName;
        dEntry.isOptional = false;
        dEntry.EntryType = "unit";
        dEntry.isToTest = true;
        dEntry.SubEntryCount = testMethodName.length;
        dEntry.SubEntries = new DescEntry[dEntry.SubEntryCount];
        for (int i=0; i<dEntry.SubEntryCount; i++) {
            DescEntry aEntry = new DescEntry();
            aEntry.entryName = testMethodName[i];
            aEntry.longName = aEntry.entryName;
            aEntry.isOptional = false;
            aEntry.EntryType = "method";
            aEntry.isToTest = true;
            dEntry.SubEntries[i] = aEntry;
        }

        return new DescEntry[]{dEntry};
    }
}
