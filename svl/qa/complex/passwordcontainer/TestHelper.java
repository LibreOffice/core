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

package complex.passwordcontainer;

import com.sun.star.task.UserRecord;


public class TestHelper {
    private String m_sTestPrefix;

    public TestHelper(  String sTestPrefix ) {
        m_sTestPrefix = sTestPrefix;
    }

    public void Error( String sError ) {
        System.out.println( m_sTestPrefix + "Error: " + sError );
    }

    private void Message( String sMessage ) {
        System.out.println( m_sTestPrefix + sMessage );
    }

    public boolean sameLists(UserRecord aUserList1[], UserRecord aUserList2[]) {
        // only works when every name is unique within the list containing it

        if(aUserList1.length != aUserList2.length) {
            Message("User list lengths: " + aUserList1.length + " <--> " + aUserList2.length + " respectively ");
            return false;
        }

        for(int i = 0; i < aUserList1.length; i++) {
            int j;
            for(j = 0; j < aUserList2.length; j++) {
                if(!aUserList1[i].UserName.equals(aUserList2[j].UserName))
                {
                    continue;
                }
                if(aUserList1[i].Passwords[0].equals(aUserList2[j].Passwords[0])) {
                    break;
                }
            }
            if(j == aUserList2.length) {
                for(int k = 0; k < aUserList1.length; k++) {
                    Message(aUserList1[k].UserName + " <--> " + aUserList2[i].UserName);
                }
                return false;
            }
        }
        return true;
    }
}











