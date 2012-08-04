/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Red Hat, Inc., Eike Rathke <erack@redhat.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/types.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tubes/conference.hxx>
#include <tubes/contact-list.hxx>
#include <tubes/manager.hxx>
#include <unotools/localfilehelper.hxx>

#include <telepathy-glib/telepathy-glib.h>

namespace {

class TestTeleTubes: public CppUnit::TestFixture
{
public:

    TestTeleTubes() {}
    ~TestTeleTubes() {}
    // This could happen in costructor wasn't there TestTeleTubes instance for each test:
    void testInitialize();
    void testCreateAccountManager();
    void testRegisterClients();
    void testContactList();
    void testPrepareAccountManager();
    void testStartBuddySession();
    void testSendPacket();
    void testReceivePacket();
    void testSendFile();
    void testDestroyTeleTubes();
    void testFailAlways();

    static void FileSent( bool success, void *user_data);

    // Order is significant.
    CPPUNIT_TEST_SUITE( TestTeleTubes );
    CPPUNIT_TEST( testInitialize );
    CPPUNIT_TEST( testCreateAccountManager );
    CPPUNIT_TEST( testRegisterClients );
    CPPUNIT_TEST( testPrepareAccountManager );
    CPPUNIT_TEST( testContactList );
    CPPUNIT_TEST( testStartBuddySession );
    CPPUNIT_TEST( testSendPacket );
    CPPUNIT_TEST( testReceivePacket );
    CPPUNIT_TEST( testSendFile );
    CPPUNIT_TEST( testDestroyTeleTubes );
#if 0
    CPPUNIT_TEST( testFailAlways );     // test failure displays SAL_LOG, uncomment for debugging
#endif
    CPPUNIT_TEST_SUITE_END();
};

// static, not members, so they actually survive cppunit test iteration
static TeleConference*      mpConference1 = NULL;
static TeleManager*         mpManager = NULL;
static TpContact*           mpAccepterContact = NULL;
static GMainLoop*           mpMainLoop = NULL;
static bool                 maFileSentSuccess = false;
static sal_uInt32           mnSentPackets = 0;
static OUString             maTestConfigIniURL;
static OString              maOffererIdentifier;
static OString              maAccepterIdentifier;

static gboolean
timed_out (void *user_data)
{
    CPPUNIT_ASSERT_MESSAGE( "Test took longer than ten seconds!", false);

    GMainLoop *loop = reinterpret_cast<GMainLoop *>(user_data);

    g_main_loop_quit (loop);
    return FALSE;
}

void TestTeleTubes::testInitialize()
{
    utl::LocalFileHelper::ConvertPhysicalNameToURL(
            OUString::createFromAscii( getenv("SRCDIR") ) + "/tubes/qa/test-config.ini",
            maTestConfigIniURL );
    rtl::Bootstrap aTestConfig( maTestConfigIniURL );

    TeleManager::addSuffixToNames( "TeleTest");

    rtl::OUString aOffererIdentifier;
    CPPUNIT_ASSERT_MESSAGE( "See README for how to set up test-config.ini",
        aTestConfig.getFrom("offerer", aOffererIdentifier));
    maOffererIdentifier = OUStringToOString( aOffererIdentifier, RTL_TEXTENCODING_UTF8);

    rtl::OUString aAccepterIdentifier;
    CPPUNIT_ASSERT_MESSAGE( "See README for how to set up test-config.ini",
        aTestConfig.getFrom("accepter", aAccepterIdentifier));
    maAccepterIdentifier = OUStringToOString( aAccepterIdentifier, RTL_TEXTENCODING_UTF8);

    mpMainLoop = g_main_loop_new (NULL, FALSE);
    g_timeout_add_seconds (10, timed_out, mpMainLoop);

    mpManager = new TeleManager();
}

void TestTeleTubes::testContactList()
{
    CPPUNIT_ASSERT( mpManager);
    CPPUNIT_ASSERT( mpManager->getAccountManagerStatus() == TeleManager::AMS_PREPARED);

    ContactList *cl = mpManager->getContactList();

    AccountContactPairV pairs;

    pairs = cl->getContacts();
    guint i;

    /* FIXME: this is racy, because we can't be 100% sure that MC has finished
     * discovering what we support and passing that on to the connection
     * manager...
     */

    /* Both our accounts are meant to be signed in, and they both should be
     * capable of LibreOffice tubes because this test runs after we register
     * our handler. */
    CPPUNIT_ASSERT_MESSAGE(
        "Make sure both your test accounts are signed in "
        "and are on each other's contact lists",
        pairs.size() > 0 );
    CPPUNIT_ASSERT(!mpAccepterContact);

    for (i = 0; i < pairs.size(); i++)
    {
        AccountContactPair pair = pairs[i];

        /* FIXME: verify that pair.first is the offerer account */
        if (tp_contact_get_identifier(pair.second) == maAccepterIdentifier) {
            mpAccepterContact = pair.second;
            g_object_ref(mpAccepterContact);
        }
        g_object_unref (pair.first);
        g_object_unref (pair.second);
    }

    CPPUNIT_ASSERT_MESSAGE(
        "Couldn't find accepter contact. "
        "Make sure both your test accounts are signed in "
        "and are on each other's contact lists",
        mpAccepterContact);
}

void TestTeleTubes::testPrepareAccountManager()
{
    mpManager->prepareAccountManager();
    TeleManager::AccountManagerStatus eStatus = mpManager->getAccountManagerStatus();
    CPPUNIT_ASSERT( eStatus == TeleManager::AMS_PREPARED);
}

void TestTeleTubes::testStartBuddySession()
{
    TpAccount *pAcc1 = mpManager->getAccount(maOffererIdentifier);
    CPPUNIT_ASSERT( pAcc1 != 0);
    /* This has to run after testContactList has run successfully. */
    CPPUNIT_ASSERT( mpAccepterContact != 0);
    mpConference1 = mpManager->startBuddySession( pAcc1, mpAccepterContact);
    CPPUNIT_ASSERT( mpConference1 != NULL);
}

void TestTeleTubes::testCreateAccountManager()
{
    bool bConnected = mpManager->createAccountManager();
    CPPUNIT_ASSERT( bConnected == true);
}

void TestTeleTubes::testRegisterClients()
{
    bool bRegistered = mpManager->registerClients();
    CPPUNIT_ASSERT( bRegistered == true);
}

void TestTeleTubes::testSendPacket()
{
    OString aPacket( "from 1 to 2" );

    bool bSentPacket = mpConference1->sendPacket( aPacket );
    CPPUNIT_ASSERT( bSentPacket );
    mnSentPackets++;
}

void TestTeleTubes::testReceivePacket()
{
    /* We can't get to the TeleConference accepting our packets.
     * It's stored in TeleManager but available only after receiving file
     * and extracting UUID from the name.
     */

    sal_uInt32 nReceivedPackets = 0;
    /* We expect to get every packet we send pushed
     * onto the queue to be echoed locally.
     */
    bool bOk;
    do
    {
        OString aReceived;
        bOk = mpConference1->popPacket( aReceived );
        if (bOk)
        {
            ++nReceivedPackets;
            CPPUNIT_ASSERT( "from 1 to 2" == aReceived );
        }
    } while (bOk);
    CPPUNIT_ASSERT( nReceivedPackets == mnSentPackets );
}

void TestTeleTubes::FileSent( bool success, void * )
{
    maFileSentSuccess = success;
    g_main_loop_quit (mpMainLoop);
}

void TestTeleTubes::testSendFile()
{
    /* This has to run after testContactList has run successfully. */
    CPPUNIT_ASSERT( mpAccepterContact != 0);

    mpConference1->sendFile( mpAccepterContact, maTestConfigIniURL,
        &TestTeleTubes::FileSent, NULL);
    /* Waiting for event: FileSent quits the mainloop */
    g_main_loop_run( mpMainLoop);

    CPPUNIT_ASSERT( maFileSentSuccess);
    // Currently there is no way to check that the file was received !
}

void TestTeleTubes::testDestroyTeleTubes()
{
    if (mpAccepterContact) {
        g_object_unref(mpAccepterContact);
        mpAccepterContact = NULL;
    }
    g_main_loop_unref( mpMainLoop );
    if (mpConference1)
        mpConference1->close();
    delete mpConference1;
    delete mpManager;
}

void TestTeleTubes::testFailAlways()
{
    CPPUNIT_ASSERT( false);
}


CPPUNIT_TEST_SUITE_REGISTRATION( TestTeleTubes);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
