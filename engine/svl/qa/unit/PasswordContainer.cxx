/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/task/UserRecord.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/MasterPasswordRequest.hpp>
#include <com/sun/star/task/XPasswordContainer.hpp>
#include <com/sun/star/task/XMasterPasswordHandling.hpp>
#include <com/sun/star/ucb/XInteractionSupplyAuthentication.hpp>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Reference.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <span>

using namespace css;
using namespace css::awt;
using namespace css::container;
using namespace css::lang;
using namespace css::script;
using namespace css::task;
using namespace css::ucb;
using namespace cpo::uno;

namespace
{
class PasswordContainerTest : public test::BootstrapFixture
{
    void test1();
    void test2();
    void test3();

private:
    CPPUNIT_TEST_SUITE(PasswordContainerTest);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST(test2);
    CPPUNIT_TEST(test3);
    CPPUNIT_TEST_SUITE_END();
};

void assertSameLists(const Sequence<UserRecord>& aUserList1, std::span<UserRecord> aUserList2)
{
    // only works when every name is unique within the list containing it

    CPPUNIT_ASSERT_EQUAL(sal_Int32(aUserList1.size()), sal_Int32(aUserList2.size()));

    for (size_t i = 0; i < aUserList1.size(); i++)
    {
        size_t j;
        for (j = 0; j < aUserList2.size(); j++)
        {
            if (!aUserList1[i].UserName.equals(aUserList2[j].UserName))
            {
                continue;
            }
            if (aUserList1[i].Passwords[0].equals(aUserList2[j].Passwords[0]))
            {
                break;
            }
        }
        if (j == aUserList2.size())
        {
            CPPUNIT_FAIL("mismatch");
        }
    }
}

class MasterPasswdHandler : public cppu::WeakImplHelper<XInteractionHandler>
{
    Reference<XInteractionHandler> m_xHandler;

public:
    MasterPasswdHandler(const Reference<XInteractionHandler>& xHandler) { m_xHandler = xHandler; }

    void handle(const Reference<XInteractionRequest>& xRequest) override
    {
        try
        {
            MasterPasswordRequest aMasterPasswordRequest;
            if (xRequest->getRequest() >>= aMasterPasswordRequest)
            {
                Sequence<Reference<XInteractionContinuation>> xContinuations
                    = xRequest->getContinuations();
                Reference<XInteractionSupplyAuthentication> xAuthentication;

                for (int i = 0; i < xContinuations.getLength(); ++i)
                {
                    xAuthentication
                        = Reference<XInteractionSupplyAuthentication>(xContinuations[i], UNO_QUERY);
                    if (xAuthentication != nullptr)
                    {
                        break;
                    }
                }
                if (xAuthentication->canSetPassword())
                {
                    xAuthentication->setPassword(u"abcdefghijklmnopqrstuvwxyz123456"_ustr);
                }
                xAuthentication->select();
            }
            else
            {
                m_xHandler->handle(xRequest);
            }
        }
        catch (Exception e)
        {
            SAL_WARN("svl", "MasterPasswordHandler Error: " + e.Message);
        }
    }
};

void PasswordContainerTest::test1()
{
    Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    Reference<lang::XMultiServiceFactory> xMSF(xFactory, UNO_QUERY_THROW);

    const OUString sURL = u"http://www.openoffice.org"_ustr;
    const OUString sUserPre = u"OOoUser"_ustr;
    const OUString sPwdPre = u"Password"_ustr;
    const int iUserNum1 = 10;
    const int iUserNum2 = 5;

    std::vector<UserRecord> aInputUserList1(iUserNum1);
    for (int i = 0; i < iUserNum1; i++)
    {
        Sequence<OUString> sTemp{ sPwdPre + "_1_"
                                  + OUString::number(i) }; // currently one password for one user
        aInputUserList1[i] = UserRecord(sUserPre + "_1_" + OUString::number(i), sTemp);
    }
    std::vector<UserRecord> aInputUserList2(iUserNum2);
    for (int i = 0; i < iUserNum2; i++)
    {
        Sequence<OUString> sTemp{ sPwdPre + "_2_" + OUString::number(i) };
        aInputUserList2[i] = UserRecord(sUserPre + "_2_" + OUString::number(i), sTemp);
    }

    Reference<XInterface> oPasswordContainer
        = xMSF->createInstance(u"com.sun.star.task.PasswordContainer"_ustr);
    Reference<XPasswordContainer> xContainer(oPasswordContainer, UNO_QUERY);
    Reference<XInterface> oHandler
        = xMSF->createInstance(u"com.sun.star.task.InteractionHandler"_ustr);
    Reference<XInteractionHandler> xHandler(oHandler, UNO_QUERY);
    rtl::Reference<MasterPasswdHandler> aMHandler = new MasterPasswdHandler(xHandler);

    // add a set of users and passwords for the same URL for runtime
    for (int i = 0; i < iUserNum1; i++)
    {
        xContainer->add(sURL, aInputUserList1[i].UserName, aInputUserList1[i].Passwords, aMHandler);
    }
    for (int i = 0; i < iUserNum2; i++)
    {
        xContainer->add(sURL, aInputUserList2[i].UserName, aInputUserList2[i].Passwords, aMHandler);
    }

    // remove some of the passwords
    for (int i = 0; i < iUserNum1; i++)
    {
        xContainer->remove(sURL, aInputUserList1[i].UserName);
    }

    // get the result and check it with the expected one
    UrlRecord aRecord = xContainer->find(sURL, aMHandler);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URL mismatch", aRecord.Url, sURL);
    assertSameLists(aRecord.UserList, aInputUserList2);

    // remove the runtime passwords
    aRecord = xContainer->find(sURL, aMHandler);
    for (int i = 0; i < aRecord.UserList.getLength(); i++)
    {
        xContainer->remove(sURL, aRecord.UserList[i].UserName);
    }
}

void PasswordContainerTest::test2()
{
    Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    Reference<lang::XMultiServiceFactory> xMSF(xFactory, UNO_QUERY_THROW);

    const OUString sURL = u"http://www.openoffice.org"_ustr;
    const OUString sUserPre = u"OOoUser"_ustr;
    const OUString sPwdPre = u"Password"_ustr;
    const int iUserNum1 = 10;
    const int iUserNum2 = 5;

    std::vector<UserRecord> aInputUserList1(iUserNum1);
    for (int i = 0; i < iUserNum1; i++)
    {
        Sequence<OUString> sTemp{ sPwdPre + "_1_"
                                  + OUString::number(i) }; // currently one password for one user
        aInputUserList1[i] = UserRecord(sUserPre + "_1_" + OUString::number(i), sTemp);
    }
    std::vector<UserRecord> aInputUserList2(iUserNum2);
    for (int i = 0; i < iUserNum2; i++)
    {
        Sequence<OUString> sTemp{ sPwdPre + "_2_" + OUString::number(i) };
        aInputUserList2[i] = UserRecord(sUserPre + "_2_" + OUString::number(i), sTemp);
    }

    Reference<XInterface> oPasswordContainer
        = xMSF->createInstance(u"com.sun.star.task.PasswordContainer"_ustr);
    Reference<XPasswordContainer> xContainer(oPasswordContainer, UNO_QUERY);
    Reference<XInterface> oHandler
        = xMSF->createInstance(u"com.sun.star.task.InteractionHandler"_ustr);
    Reference<XInteractionHandler> xHandler(oHandler, UNO_QUERY);
    rtl::Reference<MasterPasswdHandler> aMHandler = new MasterPasswdHandler(xHandler);
    Reference<XMasterPasswordHandling> xMHandling(oPasswordContainer, UNO_QUERY);

    // allow the storing of the passwords
    xMHandling->allowPersistentStoring(true);

    // add a set of users and passwords for the same URL persistently
    for (int i = 0; i < iUserNum1; ++i)
    {
        xContainer->addPersistent(sURL, aInputUserList1[i].UserName, aInputUserList1[i].Passwords,
                                  aMHandler);
    }
    for (int i = 0; i < iUserNum2; ++i)
    {
        xContainer->addPersistent(sURL, aInputUserList2[i].UserName, aInputUserList2[i].Passwords,
                                  aMHandler);
    }

    // remove some of the passwords
    for (int i = 0; i < iUserNum1; ++i)
    {
        xContainer->remove(sURL, aInputUserList1[i].UserName);
    }

    // get the result with find() and check it with the expected one
    UrlRecord aRecord = xContainer->find(sURL, aMHandler);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URL mismatch", aRecord.Url, sURL);
    assertSameLists(aRecord.UserList, aInputUserList2);

    // get the result with getAllPersistent() and check
    Sequence<UrlRecord> aRecords = xContainer->getAllPersistent(aMHandler);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URL mismatch", aRecords[0].Url, sURL);
    assertSameLists(aRecords[0].UserList, aInputUserList2);

    // remove all the persistent passwords
    xContainer->removeAllPersistent();

    // remove the runtime passwords
    for (int i = 0; i < aRecords[0].UserList.getLength(); ++i)
    {
        xContainer->remove(sURL, aRecords[0].UserList[i].UserName);
    }

    // disallow the storing of the passwords
    xMHandling->allowPersistentStoring(false);
}

void PasswordContainerTest::test3()
{
    Reference<lang::XMultiComponentFactory> xFactory(m_xContext->getServiceManager());
    Reference<lang::XMultiServiceFactory> xMSF(xFactory, UNO_QUERY_THROW);

    const OUString sURL = u"http://www.openoffice.org"_ustr;
    const OUString sUserPre = u"OOoUser"_ustr;
    const OUString sPwdPre = u"Password"_ustr;
    const int iPersistentUserNum = 10;
    const int iRuntimeUserNum = 5;

    std::vector<UserRecord> aInputUserList(iPersistentUserNum + iRuntimeUserNum);
    for (int i = 0; i < iPersistentUserNum; i++)
    {
        Sequence<OUString> sTemp{ sPwdPre + "_1_"
                                  + OUString::number(i) }; // currently one password for one user
        aInputUserList[i] = UserRecord(sUserPre + "_1_" + OUString::number(i), sTemp);
    }
    for (int i = 0; i < iRuntimeUserNum; i++)
    {
        Sequence<OUString> sTemp{ sPwdPre + "_2_" + OUString::number(i) };
        aInputUserList[i + iPersistentUserNum]
            = UserRecord(sUserPre + "_2_" + OUString::number(i), sTemp);
    }

    Reference<XInterface> oPasswordContainer
        = xMSF->createInstance(u"com.sun.star.task.PasswordContainer"_ustr);
    Reference<XPasswordContainer> xContainer(oPasswordContainer, UNO_QUERY);
    Reference<XInterface> oHandler
        = xMSF->createInstance(u"com.sun.star.task.InteractionHandler"_ustr);
    Reference<XInteractionHandler> xHandler(oHandler, UNO_QUERY);
    rtl::Reference<MasterPasswdHandler> aMHandler = new MasterPasswdHandler(xHandler);
    Reference<XMasterPasswordHandling> xMHandling(oPasswordContainer, UNO_QUERY);

    // allow the storing of the passwords
    xMHandling->allowPersistentStoring(true);

    // add a set of users and passwords for the same URL persistently
    for (int i = 0; i < iPersistentUserNum; i++)
    {
        xContainer->addPersistent(sURL, aInputUserList[i].UserName, aInputUserList[i].Passwords,
                                  aMHandler);
    }

    // add a set of users and passwords for the same URL for runtime
    for (int i = 0; i < iRuntimeUserNum; i++)
    {
        xContainer->add(sURL, aInputUserList[i + iPersistentUserNum].UserName,
                        aInputUserList[i + iPersistentUserNum].Passwords, aMHandler);
    }

    // get the result for the URL and check that it contains persistent and runtime passwords
    UrlRecord aRecord = xContainer->find(sURL, aMHandler);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("URL mismatch", aRecord.Url, sURL);
    assertSameLists(aRecord.UserList, aInputUserList);

    // remove all the persistent passwords
    xContainer->removeAllPersistent();

    // remove the runtime passwords
    aRecord = xContainer->find(sURL, aMHandler);
    for (int i = 0; i < aRecord.UserList.getLength(); i++)
    {
        xContainer->remove(sURL, aRecord.UserList[i].UserName);
    }

    // disallow the storing of the passwords
    xMHandling->allowPersistentStoring(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PasswordContainerTest);
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
