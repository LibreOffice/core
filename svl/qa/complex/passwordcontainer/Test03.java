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

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.UrlRecord;
import com.sun.star.task.UserRecord;
import com.sun.star.task.XPasswordContainer;
import com.sun.star.task.XMasterPasswordHandling;
import com.sun.star.task.XInteractionHandler;


import com.sun.star.uno.UnoRuntime;


public class Test03 implements PasswordContainerTest {
    XMultiServiceFactory m_xMSF = null;
    XPasswordContainer m_xPasswordContainer = null;
    TestHelper m_aTestHelper = null;

    public Test03 ( XMultiServiceFactory xMSF )
    {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper ( "Test03: ");
    }

    public boolean test() {
        final String sURL = "http://www.openoffice.org";
        final String sUserPre = "OOoUser";
        final String sPwdPre = "Password";
        final int iPersistentUserNum = 10;
        final int iRuntimeUserNum = 5;

        UserRecord aInputUserList[] = new UserRecord[iPersistentUserNum+iRuntimeUserNum];
        for(int i = 0; i < iPersistentUserNum; i++) {
            String sTemp[] = {sPwdPre + "_1_" + i};     // currently one password for one user
            aInputUserList[i] = new UserRecord(sUserPre + "_1_" + i, sTemp);
        }
        for(int i = 0; i < iRuntimeUserNum; i++) {
            String sTemp[] = {sPwdPre + "_2_" + i};
            aInputUserList[i+iPersistentUserNum] = new UserRecord(sUserPre + "_2_" + i, sTemp);
        }

        try {
            Object oPasswordContainer = m_xMSF.createInstance("com.sun.star.task.PasswordContainer");
            XPasswordContainer xContainer = UnoRuntime.queryInterface(XPasswordContainer.class, oPasswordContainer);
            Object oHandler = m_xMSF.createInstance("com.sun.star.task.InteractionHandler");
            XInteractionHandler xHandler = UnoRuntime.queryInterface(XInteractionHandler.class, oHandler);
            MasterPasswdHandler aMHandler = new MasterPasswdHandler(xHandler);
            XMasterPasswordHandling xMHandling = UnoRuntime.queryInterface(XMasterPasswordHandling.class, oPasswordContainer);

            // allow the storing of the passwords
            xMHandling.allowPersistentStoring(true);

            // add a set of users and passwords for the same URL persistently
            for(int i = 0; i < iPersistentUserNum; i++) {
                xContainer.addPersistent(sURL, aInputUserList[i].UserName, aInputUserList[i].Passwords, aMHandler);
            }

            // add a set of users and passwords for the same URL for runtime
            for(int i = 0; i < iRuntimeUserNum; i++) {
                xContainer.add(sURL, aInputUserList[i+iPersistentUserNum].UserName, aInputUserList[i+iPersistentUserNum].Passwords, aMHandler);
            }

            // get the result for the URL and check that it contains persistent and runtime passwords
            UrlRecord aRecord = xContainer.find(sURL, aMHandler);
            if(!aRecord.Url.equals(sURL)) {
                m_aTestHelper.Error("URL mismatch. Got " + aRecord.Url + "; should be " + sURL);
                return false;
            }
            if(!m_aTestHelper.sameLists(aRecord.UserList, aInputUserList)) {
                m_aTestHelper.Error("User list is not the expected");
                return false;
            }

            // remove all the persistent passwords
            xContainer.removeAllPersistent();

             // remove the runtime passwords
            aRecord = xContainer.find(sURL, aMHandler);
            for(int i = 0; i < aRecord.UserList.length; i++) {
                xContainer.remove(sURL, aRecord.UserList[i].UserName);
            }

            // disallow the storing of the passwords
            xMHandling.allowPersistentStoring(false);
        }catch(Exception e){
            m_aTestHelper.Error("Exception: " + e);
            return false;
        }
        return true;
    }
}
