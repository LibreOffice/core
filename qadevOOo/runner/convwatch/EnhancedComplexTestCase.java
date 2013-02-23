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

package convwatch;

import complexlib.ComplexTestCase;
import helper.ProcessHandler;
import convwatch.GraphicalTestArguments;

/**
 * Some Helperfunctions which are nice in ReferenceBuilder and ConvWatchTest
 */

public abstract class EnhancedComplexTestCase extends ComplexTestCase
{
    // public void before()
    //     {
    //         // System.out.println("before()");
    //     }
    //
    // public void after()
    //     {
    //         // System.out.println("after()");
    //     }

    void checkExistance(String _sScriptFile, String _sName)
        {
            // Process testshl = Runtime.getRuntime().exec(scriptFile);
            ProcessHandler aHandler = new ProcessHandler(_sScriptFile);
            aHandler.executeSynchronously();
            TimeHelper.waitInSeconds(1, "wait after ProcessHandler.executeSynchronously()");

            StringBuffer aBuffer = new StringBuffer();
            aBuffer.append(aHandler.getErrorText()).append(aHandler.getOutputText());
            String sText = aBuffer.toString();

            if (sText.length() == 0)
            {
                String sError = "Must quit. " + _sName + " may be not accessible.";
                assure(sError, false);
                // System.exit(1);
            }
            else
            {
                // System.out.println("Output from script:");
                // System.out.println(sText);
            }
        }

    // -----------------------------------------------------------------------------

    protected void checkEnvironment(Object[] _aList)
        {
            // checks if some packages already installed,
            // this function will not return if packages are not installed,
            // it will call System.exit(1)!

            if (needCheckForInstalledSoftware())
            {
                for (int i=0;i<_aList.length;i++)
                {
                    String sCommand = (String)_aList[i];
                    // TODO: nice to have, a pair object
                    checkExistance(sCommand, sCommand);
                }
            }
        }

    // -----------------------------------------------------------------------------

    protected abstract Object[] mustInstalledSoftware();
    public boolean needCheckForInstalledSoftware()
        {
            String sNEEDCHECK = (String)param.get( PropertyName.CHECK_NEED_TOOLS );
// TODO: I need to get the boolean value with get("name") because, if it is not given getBool() returns
//       with a default of 'false' which is not very helpful if the default should be 'true'
//       maybe a getBoolean("name", true) could be a better choise.
            if (sNEEDCHECK == null)
            {
                sNEEDCHECK = "false";
            }
            if (sNEEDCHECK.toLowerCase().equals("yes") ||
                sNEEDCHECK.toLowerCase().equals("true"))
            {
                return true;
            }
            return false;
        }

    // -----------------------------------------------------------------------------

    public GraphicalTestArguments getGraphicalTestArguments()
        {
            GraphicalTestArguments aGTA = new GraphicalTestArguments(param);
            if (aGTA.getImportFilterName() != null && aGTA.getImportFilterName().toLowerCase().equals("help"))
            {
                aGTA = null;
            }
            if (aGTA.getExportFilterName() != null && aGTA.getExportFilterName().toLowerCase().equals("help"))
            {
                aGTA = null;
            }
            return aGTA;
        }
}
