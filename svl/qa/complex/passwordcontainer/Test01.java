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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XPasswordContainer;
import com.sun.star.task.UrlRecord;
import com.sun.star.task.UserRecord;
import com.sun.star.uno.UnoRuntime;

// import share.LogWriter;

public class Test01 implements PasswordContainerTest {
    XMultiServiceFactory m_xMSF = null;
    XPasswordContainer m_xPasswordContainer = null;
    TestHelper m_aTestHelper = null;

    public Test01 ( XMultiServiceFactory xMSF )
    {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper ( "Test01: ");
    }

    public boolean test() {
        final String sURL = "http://www.openoffice.org";
        final String sUserPre = "OOoUser";
        final String sPwdPre = "Password";
        final int iUserNum1 = 10;
        final int iUserNum2 = 5;

        UserRecord aInputUserList1[] = new UserRecord[iUserNum1];
        for(int i = 0; i < iUserNum1; i++) {
            String sTemp[] = {sPwdPre + "_1_" + i};     // currently one password for one user
            aInputUserList1[i] = new UserRecord(sUserPre + "_1_" + i, sTemp);
        }
        UserRecord aInputUserList2[] = new UserRecord[iUserNum2];
        for(int i = 0; i < iUserNum2; i++) {
            String sTemp[] = {sPwdPre + "_2_" + i};
            aInputUserList2[i] = new UserRecord(sUserPre + "_2_" + i, sTemp);
        }
        try {
            Object oPasswordContainer = m_xMSF.createInstance( "com.sun.star.task.PasswordContainer" );
            XPasswordContainer xContainer = UnoRuntime.queryInterface(XPasswordContainer.class, oPasswordContainer);
            Object oHandler = m_xMSF.createInstance( "com.sun.star.task.InteractionHandler" );
            XInteractionHandler xHandler = UnoRuntime.queryInterface(XInteractionHandler.class, oHandler);
            MasterPasswdHandler aMHandler = new MasterPasswdHandler( xHandler );

            // add a set of users and passwords for the same URL for runtime
            for(int i = 0; i < iUserNum1; i++) {
                xContainer.add(sURL, aInputUserList1[i].UserName, aInputUserList1[i].Passwords, aMHandler);
            }
            for (int i = 0; i < iUserNum2; i++) {
                xContainer.add(sURL, aInputUserList2[i].UserName, aInputUserList2[i].Passwords, aMHandler);
            }

            // remove some of the passwords
            for (int i = 0; i < iUserNum1; i++) {
                xContainer.remove(sURL, aInputUserList1[i].UserName);
            }

            // get the result and check it with the expected one
            UrlRecord aRecord = xContainer.find(sURL, aMHandler);
            if(!aRecord.Url.equals(sURL)) {
                m_aTestHelper.Error("URL mismatch. Got " + aRecord.Url + "; should be " + sURL);
                return false;
            }
            if(!m_aTestHelper.sameLists(aRecord.UserList, aInputUserList2)) {
                m_aTestHelper.Error("User list is not the expected");
                return false;
            }

            // remove the runtime passwords
            aRecord = xContainer.find(sURL, aMHandler);
            for(int i = 0; i < aRecord.UserList.length; i++) {
                xContainer.remove(sURL, aRecord.UserList[i].UserName);
            }
        } catch(Exception e) {
            m_aTestHelper.Error("Exception: " + e);
            return false;
        }
        return true;
    }
}
