/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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











