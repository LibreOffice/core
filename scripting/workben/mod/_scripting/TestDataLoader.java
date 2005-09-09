/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestDataLoader.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:48:07 $
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
