/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EnhancedComplexTestCase.java,v $
 * $Revision: 1.7 $
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

package convwatch;

import complexlib.ComplexTestCase;
import helper.ProcessHandler;
import java.io.File;
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
            boolean bBackValue = false;
            // Process testshl = Runtime.getRuntime().exec(scriptFile);
            ProcessHandler aHandler = new ProcessHandler(_sScriptFile);
            bBackValue = aHandler.executeSynchronously();
            TimeHelper.waitInSeconds(1, "wait after ProcessHandler.executeSynchronously()");

            StringBuffer aBuffer = new StringBuffer();
            aBuffer.append(aHandler.getErrorText()).append(aHandler.getOutputText());
            String sText = aBuffer.toString();

            if (sText.length() == 0)
            {
                String sError = "Must quit. " + _sName + " may be not accessable.";
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
