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
            ArrayList<Parameters> list = new ArrayList<Parameters>(11);

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
                    list = new ArrayList<Parameters>(11);
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
