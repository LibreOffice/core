/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnhancedComplexTestCase.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:17:26 $
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
