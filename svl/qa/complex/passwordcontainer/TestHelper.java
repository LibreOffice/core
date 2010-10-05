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

package complex.passwordcontainer;

import com.sun.star.task.UserRecord;

// import share.LogWriter;

public class TestHelper {
    // LogWriter m_aLogWriter;
    String m_sTestPrefix;

    public TestHelper(  String sTestPrefix ) {
        // m_aLogWriter = aLogWriter;
        m_sTestPrefix = sTestPrefix;
    }

    public void Error( String sError ) {
        System.out.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage ) {
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











