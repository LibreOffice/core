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
