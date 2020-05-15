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

import java.util.*;

/** This class prints out the system properties.

    We cannot print the strings directly because of encoding issues. Since
    about 1.3.1 one can start java with the option -Dfile.encoding=UTF-8, but
    unfortunately this works only with later update - versions (for example,
    1.3.1_07). Therefore we use this scheme. The property string has this form:
    name=value

    Every character is cast to an integer which value is printed, followed by a
    space. If all characters of the string are printed, then a new line is printed.
*/
public class JREProperties
{
    public static void main(String[] args)
    {
         try
        {
            Properties p = System.getProperties();
            Enumeration e = p.propertyNames();
            while (e.hasMoreElements()) {
                String sProp = (String) e.nextElement();
                String sCompleteProp = sProp + "=" + p.getProperty(sProp);
                char[] arChars = new char[sCompleteProp.length()];
                sCompleteProp.getChars(0, sCompleteProp.length(), arChars, 0);
                for (int c = 0; c < arChars.length; c++) {
                    System.out.print(String.valueOf((int) arChars[c]));
                    System.out.print(" ");
                }
                System.out.print("\n");
            }
        }
        catch(Exception e)
        {
            System.err.println(e);
        }

        System.exit(0);
    }



}
