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

#include <sal/precppunit.hxx>

#include <tubes/contact-list.hxx>
#include <tubes/manager.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <unotest/bootstrapfixturebase.hxx>

namespace {

class TestTeleTubes: public test::BootstrapFixtureBase
{
public:

    TestTeleTubes();
    ~TestTeleTubes();
    void testSetupManager1();
    void testSetupManager2();
    void testConnect1();
    void testConnect2();
    void testContactList();
    void testPrepareAccountManager1();
    void testPrepareAccountManager2();
    void testStartBuddySession1();
    void testStartBuddySession2();
    void testSendPacket();
    void testReceivePacket();
    void testFlushLoops();
    void testDestroyManager1();
    void testDestroyManager2();
    void testFailAlways();

    GMainLoop*                  mpMainLoop;

    // Order is significant.
    CPPUNIT_TEST_SUITE( TestTeleTubes );
    CPPUNIT_TEST( testSetupManager1 );
    CPPUNIT_TEST( testSetupManager2 );
    CPPUNIT_TEST( testConnect1 );
    CPPUNIT_TEST( testConnect2 );
    CPPUNIT_TEST( testContactList );
    CPPUNIT_TEST( testPrepareAccountManager1 );
    CPPUNIT_TEST( testPrepareAccountManager2 );
    CPPUNIT_TEST( testStartBuddySession1 );
    CPPUNIT_TEST( testStartBuddySession2 );
    CPPUNIT_TEST( testSendPacket );
    CPPUNIT_TEST( testReceivePacket );
    CPPUNIT_TEST( testFlushLoops );
    CPPUNIT_TEST( testDestroyManager1 );
    CPPUNIT_TEST( testDestroyManager2 );
    CPPUNIT_TEST( testFailAlways );     // need failure to display SAL_LOG, comment out for real builds
    CPPUNIT_TEST_SUITE_END();

private:
// XXX The Jabber accounts specified in test-config.ini need to be setup in
// Empathy, enabled, connected, and on each other's rosters.
    rtl::OUString             maTestConfigIniURL;
    rtl::Bootstrap            maTestConfig;

    rtl::OString              maOffererIdentifier;
    rtl::OString              maAccepterIdentifier;
};

// static, not members, so they actually survive cppunit test iteration
static TeleManager* mpManager1 = NULL;
static TeleManager* mpManager2 = NULL;

static sal_uInt32 nSentPackets = 0;

TestTeleTubes::TestTeleTubes()
    : maTestConfigIniURL(getURLFromSrc("/tubes/qa/test-config.ini")),
      maTestConfig(maTestConfigIniURL)
{
    TeleManager::addSuffixToNames( "TeleTest");

    rtl::OUString aOffererIdentifier;
    CPPUNIT_ASSERT_MESSAGE( "See README for how to set up test-config.ini",
        maTestConfig.getFrom("offerer", aOffererIdentifier));
    maOffererIdentifier = OUStringToOString( aOffererIdentifier, RTL_TEXTENCODING_UTF8);

    rtl::OUString aAccepterIdentifier;
    CPPUNIT_ASSERT_MESSAGE( "See README for how to set up test-config.ini",
        maTestConfig.getFrom("accepter", aAccepterIdentifier));
    maAccepterIdentifier = OUStringToOString( aAccepterIdentifier, RTL_TEXTENCODING_UTF8);
}

TestTeleTubes::~TestTeleTubes()
{
}

static void TeleTestTubes_ContactListPrepared( GError *errorOr0, void *user_data )
{
    TestTeleTubes *self = reinterpret_cast<TestTeleTubes *>(user_data);

    CPPUNIT_ASSERT( errorOr0 == 0 );

    g_main_loop_quit (self->mpMainLoop);
}

static gboolean
timed_out (void *user_data)
{
    CPPUNIT_ASSERT( false);

    GMainLoop *loop = reinterpret_cast<GMainLoop *>(user_data);

    g_main_loop_quit (loop);
    return FALSE;
}

void TestTeleTubes::testContactList()
{
    mpMainLoop = g_main_loop_new (NULL, FALSE);

    ContactList cl;
    cl.prepare( TeleTestTubes_ContactListPrepared, this );
    g_timeout_add_seconds (5, timed_out, mpMainLoop);
    g_main_loop_run (mpMainLoop);
    g_main_loop_unref (mpMainLoop);
    mpMainLoop = NULL;

    /* Okay, now everything's prepared, we can get contacts synchronously. */
    AccountContactPairV pairs;

    pairs = cl.getContacts();
    guint i;

    CPPUNIT_ASSERT( pairs.size() > 0 );

    for (i = 0; i < pairs.size(); i++)
    {
        AccountContactPair pair = pairs[i];
        g_print ("Account %s; contact %s (%s)\n",
            tp_account_get_display_name (pair.first),
            tp_contact_get_alias (pair.second),
            tp_contact_get_identifier (pair.second));
        g_object_unref (pair.first);
        g_object_unref (pair.second);
    }
}

void TestTeleTubes::testSetupManager1()
{
    mpManager1 = new TeleManager( true);
}

void TestTeleTubes::testSetupManager2()
{
    mpManager2 = new TeleManager;
}

void TestTeleTubes::testPrepareAccountManager1()
{
    mpManager1->prepareAccountManager();
    TeleManager::AccountManagerStatus eStatus = mpManager1->getAccountManagerStatus();
    CPPUNIT_ASSERT( eStatus == TeleManager::AMS_PREPARED);
}

void TestTeleTubes::testPrepareAccountManager2()
{
    mpManager2->prepareAccountManager();
    TeleManager::AccountManagerStatus eStatus = mpManager2->getAccountManagerStatus();
    CPPUNIT_ASSERT( eStatus == TeleManager::AMS_PREPARED);
}

void TestTeleTubes::testStartBuddySession1()
{
    TpAccount *pAcc1 = mpManager1->getAccount(maOffererIdentifier);
    CPPUNIT_ASSERT ( pAcc1 != 0);
    bool bStarted = mpManager1->startBuddySession( pAcc1, maAccepterIdentifier);
    CPPUNIT_ASSERT( bStarted == true);
}

void TestTeleTubes::testStartBuddySession2()
{
    //bool bStarted = mpManager2->startBuddySession( sAcc2, sAcc1);
    //CPPUNIT_ASSERT( bStarted == true);
}

void TestTeleTubes::testConnect1()
{
    bool bConnected = mpManager1->connect();
    CPPUNIT_ASSERT( bConnected == true);
}

void TestTeleTubes::testConnect2()
{
    bool bConnected = mpManager2->connect();
    CPPUNIT_ASSERT( bConnected == true);
}

void TestTeleTubes::testSendPacket()
{
    TelePacket aPacket( "", RTL_CONSTASCII_STRINGPARAM( "from 1 to 2"));
    nSentPackets = mpManager1->sendPacket( aPacket);
    CPPUNIT_ASSERT( nSentPackets == 2); // expect out+in conference, as own instance accepted self
}

void TestTeleTubes::testReceivePacket()
{
    TelePacket aPacket( "", RTL_CONSTASCII_STRINGPARAM( "from 1 to 2"));
    TelePacket aReceived;
    sal_uInt32 nReceivedPackets = 0;
    bool bOk;
    do
    {
        do
        {
            bOk = mpManager1->popPacket( aReceived);
            if (bOk)
            {
                ++nReceivedPackets;
                CPPUNIT_ASSERT( aPacket == aReceived);
            }
        } while (bOk);
        if (nReceivedPackets < nSentPackets)
            mpManager1->iterateLoop();
    } while (nReceivedPackets < nSentPackets);
    CPPUNIT_ASSERT( nReceivedPackets == nSentPackets);
}

void TestTeleTubes::testFlushLoops()
{
    mpManager1->flushLoop();
    mpManager2->flushLoop();
}

void TestTeleTubes::testDestroyManager1()
{
    delete mpManager1;
    mpManager1 = NULL;
}

void TestTeleTubes::testDestroyManager2()
{
    delete mpManager2;
    mpManager2 = NULL;
}

void TestTeleTubes::testFailAlways()
{
    CPPUNIT_ASSERT( false);
}


CPPUNIT_TEST_SUITE_REGISTRATION( TestTeleTubes);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
