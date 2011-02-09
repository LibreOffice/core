/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package complex.path_substitution;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XStringSubstitution;

import java.util.Vector;

// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------

/**
 *
 */
public class PathSubstitutionTest
{

    private static XMultiServiceFactory xMSF;
    // all substitution variables
    private VariableContainer substVars = null;

    /**
     * A function to tell the framework, which test functions are available.
     * Right now, it's only 'checkXStringSubstitution'.
     * @return All test methods.
     */
//    public String[] getTestMethodNames() {
//        return new String[]{"checkXStringSubstitution"};
//    }
    /**
     * Create an array with all substitution variables
     */
    @Before private void initialize()
    {
        substVars = new VariableContainer();
        substVars.add("$(prog)", true, true);
        substVars.add("$(inst)", true, true);
        substVars.add("$(user)", true, true);
        substVars.add("$(work)", true, true);
        substVars.add("$(home)", true, true);
        substVars.add("$(temp)", true, true);
        substVars.add("$(lang)", false, false);
        substVars.add("$(langid)", false, false);
        substVars.add("$(vlang)", false, false);
        // path won't resubstitute
        substVars.add("$(path)", true, false);
    }

    /**
     * One actual test: as the method 'getTestMethodNames()' tells.
     */
    @Test public void checkXStringSubstitution()
    {
        xMSF = getMSF();
        System.out.println("---- Testing the XStringSubstitution interface ----");
        System.out.println("Create intance of test object.\n");
        XStringSubstitution oObj = null;
        try
        {
            Object x = xMSF.createInstance(
                    "com.sun.star.util.PathSubstitution");
            oObj = UnoRuntime.queryInterface(XStringSubstitution.class, x);
            if (oObj == null)
            {
                throw new com.sun.star.uno.Exception();
            }
        }
        catch (com.sun.star.uno.Exception e)
        {
            System.out.println(e.getClass().getName());
            System.out.println("Message: " + e.getMessage());
            fail("Could not create an instance of the test object.");
            return;
        }

        for (int i = 0; i < substVars.size(); i++)
        {
            String var = substVars.getVariable(i);
            System.out.println("Testing var '" + var + "'");
            try
            {
                String substVal = oObj.getSubstituteVariableValue(var);
                System.out.println("\tvalue '" + substVal + "'");
                substVars.putValue(i, substVal);

                // simple check: let path in a string replace
                String substString = var + "/additional/path";

                System.out.println("Substitute '" + substString + "'");
                String newValue = oObj.substituteVariables(substString, true);
                System.out.println("Return value '" + newValue + "'");
                // 2do: better check for correct substitution
                assertTrue("Did not substitute '"
                        + substString + "' to '" + newValue
                        + "' correctly:", newValue.startsWith(substVal));

                // simple check part two:
                //make substitution backwards if possible
                if (substVars.canReSubstitute(i))
                {
                    substString = substVal + "/additional/path";

                    System.out.println("Substitute backwards '" + substString + "'");
                    newValue = oObj.reSubstituteVariables(substString);
                    System.out.println("Return value '" + newValue + "'");
                    // 2do: better check for correct substitution
                    assertTrue("Did not reSubstitute '"
                            + substString + "' to '" + newValue
                            + "' correctly:", checkResubstitute(newValue, var));
                }

                // simple check part three: look if replace
                //in middle of text works
                substString = "file:///starting/" + var + "/path";

                String sCanSubstAllPos;
                if (substVars.onlySubstituteAtBegin(i))
                    sCanSubstAllPos = "NO";
                else
                    sCanSubstAllPos = "YES";
                System.out.println("Variable can substitute within string: "+sCanSubstAllPos);
                System.out.println("Substitute '" + substString + "'");
                newValue = oObj.substituteVariables(substString, false);
                System.out.println("Return value '" + newValue + "'");
                boolean erg = true;
                if (substVars.onlySubstituteAtBegin(i))
                {
                    // in this case it should not have worked
                    erg = newValue.indexOf(substVal) == -1;
                }
                else
                {
                    erg = newValue.indexOf(substVal) != -1;
                }
                assertTrue("Did not substitute '"
                        + substString + "' to '" + newValue
                        + "' correctly:", erg);

            }
            catch (com.sun.star.uno.Exception e)
            {
                System.out.println(e.getClass().getName());
                System.out.println("Message: " + e.getMessage());
                fail("Could not create an instance of the test object.");
                return;
            }
            System.out.println("Finish testing '" + var + "'\n");
        }

        // check of greedy resubstitution
        String prog = "$(prog)";
        String inst = "$(inst)";
        String instPth = substVars.getValue(inst);
        String progPth = substVars.getValue(prog);

        if (progPth.startsWith(instPth) && instPth.startsWith(progPth))
        {
            System.out.println("Greedy ReSubstitute");
            String substString = progPth + "/additional/path";
            String newVal = oObj.reSubstituteVariables(substString);
            System.out.println("String '" + substString
                    + "' should be resubstituted with");
            System.out.println("Variable '" + prog + "' instead of Variable '"
                    + inst + "'");
            assertTrue("Did not reSubstitute '" + substString
                    + "' to '" + newVal + "' correctly:",
                    newVal.startsWith(prog));
        }

        System.out.println(
                "---- Finish testing the XStringSubstitution interface ----");
    }

    /**
     * test the resubstitution
     * @return true, if resubstitution is correct.
     */
    private boolean checkResubstitute(String subst, String original)
    {
        // simple: subst starts with original
        if (subst.startsWith(original))
        {
            return true;
        }
        else
        {                             // hard: been resubstituted with a differernt variable.
            for (int i = 0; i < substVars.size(); i++)
            {
                String var = substVars.getVariable(i);
                if (subst.startsWith(var) && original.startsWith(original))
                {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Class for containing the substitution variables with their
     * values and some information.
     */
    private class VariableContainer
    {

        public Vector varName;
        public Vector varValue;
        public Vector substAtBegin;
        public Vector resubst;

        public VariableContainer()
        {
            varName = new Vector();
            varValue = new Vector();
            substAtBegin = new Vector();
            resubst = new Vector();
        }

        public void add(String var)
        {
            varName.add(var);
            substAtBegin.add(Boolean.TRUE);
            resubst.add(Boolean.TRUE);
        }

        public void add(String var, boolean onlySubstAtBegin,
                boolean canResubst)
        {
            varName.add(var);
            this.substAtBegin.add(new Boolean(onlySubstAtBegin));
            this.resubst.add(new Boolean(canResubst));
        }

        public void putValue(int i, String val)
        {
            varValue.add(i, val);
        }

        public int size()
        {
            return varName.size();
        }

        public String getVariable(int i)
        {
            return (String) varName.get(i);
        }

        public String getValue(int i)
        {
            return (String) varName.get(i);
        }

        public String getValue(String var)
        {
            return (String) varValue.get(varName.indexOf(var));
        }

        public boolean onlySubstituteAtBegin(int i)
        {
            return ((Boolean) substAtBegin.get(i)).booleanValue();
        }

        public boolean canReSubstitute(int i)
        {
            return ((Boolean) resubst.get(i)).booleanValue();
        }
    }

    private XMultiServiceFactory getMSF()
    {
        final XMultiServiceFactory xMSF1 = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        return xMSF1;
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
