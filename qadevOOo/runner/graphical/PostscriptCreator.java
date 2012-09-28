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

package graphical;

public class PostscriptCreator extends EnhancedComplexTestCase
{

    public String[] getTestMethodNames()
    {
        return new String[]{"DocumentToPostscript"};
    }

    /**
     * test function.
     */
    public void DocumentToPostscript()
    {
        GlobalLogWriter.set(log);
        ParameterHelper aParam = new ParameterHelper(param);

        param.put(util.PropertyName.OFFICE_CLOSE_TIME_OUT, 2000);
        // run through all documents found in Inputpath
        foreachDocumentinInputPath(aParam);
    }


    public void checkOneFile(String _sDocumentName, String _sResult, ParameterHelper _aParams) throws OfficeException
    {
        GlobalLogWriter.println("  Document: " + _sDocumentName);
        GlobalLogWriter.println("   results: " + _sResult);
        IOffice aOffice = new Office(_aParams, _sResult);

        PerformanceContainer a = new PerformanceContainer();
        a.startTime(PerformanceContainer.AllTime);

        a.startTime(PerformanceContainer.OfficeStart);
        aOffice.start();
        a.stopTime(PerformanceContainer.OfficeStart);

        try
        {
            a.startTime(PerformanceContainer.Load);
            aOffice.load(_sDocumentName);
            a.stopTime(PerformanceContainer.Load);

            a.startTime(PerformanceContainer.Print);
            aOffice.storeAsPostscript();
            a.stopTime(PerformanceContainer.Print);
        }
        finally
        {
            a.startTime(PerformanceContainer.OfficeStop);
            aOffice.close();
            a.stopTime(PerformanceContainer.OfficeStop);

            a.stopTime(PerformanceContainer.AllTime);

            a.print( System.out );
        }
    }
}
