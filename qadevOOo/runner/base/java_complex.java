/*************************************************************************
 *
 *  $RCSfile: java_complex.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-01-27 16:27:20 $
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

package base;

import com.sun.star.lang.XMultiServiceFactory;
import java.lang.reflect.Constructor;
import complexlib.ComplexTestCase;
import lib.DynamicClassLoader;
import share.DescGetter;
import helper.ComplexDescGetter;
import helper.OfficeProvider;
import share.DescEntry;
import share.LogWriter;
import stats.Summarizer;
import base.TestBase;

/**
 * Test base for executing a java complex test.
 * @see base.TestBase
 */
public class java_complex implements TestBase{

    /**
     * Execute a test.
     * @param param The test parameters.
     * @return True, if the test was executed.
     */
    public boolean executeTest(lib.TestParameters param) {
        // get the test job without leading "-o "
        String testJob = ((String)param.get("TestJob")).substring(3).trim();
        ComplexTestCase testClass = null;
        DynamicClassLoader dcl = new DynamicClassLoader();
        // create an instance
        try {
            testClass = (ComplexTestCase)dcl.getInstance(testJob);
        }
        catch(java.lang.Exception e) {
            e.printStackTrace();
            return false;
        }
        DescGetter descGetter = new ComplexDescGetter(testClass);
        DescEntry dEntry = descGetter.getDescriptionFor(testJob,null,true)[0];

        LogWriter log = (LogWriter)dcl.getInstance(
                                            (String)param.get("LogWriter"));
        log.initialize(dEntry,true);
        dEntry.Logger = log;

        if (!param.getBool("NoOffice")) {
            OfficeProvider office = new OfficeProvider();
            XMultiServiceFactory msf = (XMultiServiceFactory)
                                            office.getManager(param);
            if (msf == null) return false;
            param.put("ServiceFactory",msf);
        }

        testClass.executeMethods(dEntry, param);

        Summarizer sum = new Summarizer();
        sum.summarizeUp(dEntry);

        LogWriter out = (LogWriter)dcl.getInstance(
                                            (String)param.get("OutProducer"));
        out.summary(dEntry);
        return true;
    }


}
