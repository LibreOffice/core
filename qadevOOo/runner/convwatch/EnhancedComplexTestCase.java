/*************************************************************************
 *
 *  $RCSfile: EnhancedComplexTestCase.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:09:05 $
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

            for (int i=0;i<_aList.length;i++)
            {
                String sCommand = (String)_aList[i];
// TODO: nice to have, a pair object
                checkExistance(sCommand, sCommand);
            }
        }

    // -----------------------------------------------------------------------------

    protected abstract Object[] mustInstalledSoftware();

    // -----------------------------------------------------------------------------

    public GraphicalTestArguments getGraphicalTestArguments()
        {
            return new GraphicalTestArguments(param);
        }


}
