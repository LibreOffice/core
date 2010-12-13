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
import com.sun.star.task.XPasswordContainer;
import com.sun.star.task.XMasterPasswordHandling;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.UrlRecord;
import com.sun.star.task.UserRecord;

import com.sun.star.uno.UnoRuntime;


public class Test02 implements PasswordContainerTest {
    XMultiServiceFactory m_xMSF = null;
    XPasswordContainer m_xPasswordContainer = null;
    TestHelper m_aTestHelper = null;

    public Test02 ( XMultiServiceFactory xMSF )
    {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper ( "Test02: ");
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
            Object oPasswordContainer = m_xMSF.createInstance("com.sun.star.task.PasswordContainer");
            XPasswordContainer xContainer = UnoRuntime.queryInterface(XPasswordContainer.class, oPasswordContainer);
            Object oHandler = m_xMSF.createInstance("com.sun.star.task.InteractionHandler");
            XInteractionHandler xHandler = UnoRuntime.queryInterface(XInteractionHandler.class, oHandler);
            MasterPasswdHandler aMHandler = new MasterPasswdHandler(xHandler);
            XMasterPasswordHandling xMHandling = UnoRuntime.queryInterface(XMasterPasswordHandling.class, oPasswordContainer);

            // allow the storing of the passwords
            xMHandling.allowPersistentStoring(true);

            // add a set of users and passwords for the same URL persistently
            for(int i = 0; i < iUserNum1; ++i) {
                xContainer.addPersistent(sURL, aInputUserList1[i].UserName, aInputUserList1[i].Passwords, aMHandler);
            }
            for(int i = 0; i < iUserNum2; ++i) {
                xContainer.addPersistent(sURL, aInputUserList2[i].UserName, aInputUserList2[i].Passwords, aMHandler);
            }

            // remove some of the passwords
            for(int i = 0; i < iUserNum1; ++i) {
                xContainer.remove(sURL, aInputUserList1[i].UserName);
            }

            // get the result with find() and check it with the expected one
            UrlRecord aRecord = xContainer.find(sURL, aMHandler);
            if(!aRecord.Url.equals(sURL)) {
                m_aTestHelper.Error("URL mismatch. Got " + aRecord.Url + "; should be " + sURL);
                return false;
            }
            if(!m_aTestHelper.sameLists(aRecord.UserList, aInputUserList2)) {
                m_aTestHelper.Error("User list is not the expected");
                return false;
            }

            // get the result with getAllPersistent() and check
            UrlRecord aRecords[] = xContainer.getAllPersistent(aMHandler);
            if(!aRecords[0].Url.equals(sURL)) {
                m_aTestHelper.Error("URL mismatch");
                return false;
            }
            if(!m_aTestHelper.sameLists(aRecords[0].UserList, aInputUserList2)) {
                m_aTestHelper.Error("User list is not the expected");
                return false;
            }

            // remove all the persistent passwords
            xContainer.removeAllPersistent();

            // remove the runtime passwords
            for(int i = 0; i < aRecords[0].UserList.length; ++i) {
                xContainer.remove(sURL, aRecords[0].UserList[i].UserName);
            }

            // disallow the storing of the passwords
            xMHandling.allowPersistentStoring(false);
        } catch(Exception e) {
            m_aTestHelper.Error("Exception: " + e);
            return false;
        }
        return true;
    }
}























