/*************************************************************************
 *
 *  $RCSfile: TestDataLoader.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2002-12-10 14:12:42 $
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

package mod._scripting;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.StringTokenizer;

import lib.TestEnvironment;
import lib.Parameters;

public class TestDataLoader {

    private TestDataLoader() {
    }

    public static void setupData(TestEnvironment tEnv, String name) {
        String filename =
            util.utils.getFullTestDocName("testdata");
        File testdatadir = new File(filename);
        File f = new File(testdatadir, name + ".csv");

        if (!f.exists())
            return;

        BufferedReader in;

        try {
            in = new BufferedReader(new FileReader(f));

            String s, previous, current;
            ArrayList list = new ArrayList(11);

            if ((s = in.readLine()) != null) {
                StringTokenizer st = new StringTokenizer(s, ";");

                current = previous = st.nextToken();
                list.add(getParameters(st));
            }
            else {
                return;
            }

            while ((s = in.readLine()) != null) {
                StringTokenizer st = new StringTokenizer(s, ";");

                current = st.nextToken();

                if (!current.equals(previous)) {
                    tEnv.addObjRelation(previous, list);
                    previous = current;
                    list = new ArrayList(11);
                }

                list.add(getParameters(st));
            }

            tEnv.addObjRelation(previous, list);
        }
        catch (IOException ioe) {
        }
    }

    private static Parameters getParameters(StringTokenizer st) {
        String separator = "=";
        HashMap map = new HashMap(5);

        while (st.hasMoreTokens()) {
            String pair = st.nextToken();
            StringTokenizer tokens = new StringTokenizer(pair, separator);

            String name;
            String value;

            if (tokens.countTokens() < 2)
                continue;

            name = tokens.nextToken();
            if (tokens.countTokens() == 1)
                value = tokens.nextToken();
            else {
                StringBuffer buf = new StringBuffer(tokens.nextToken());
                while (tokens.hasMoreTokens())
                    buf.append(separator).append(tokens.nextToken());
                value = buf.toString();
            }

            map.put(name, value);
        }

        return new Parameters(map);
    }
}
