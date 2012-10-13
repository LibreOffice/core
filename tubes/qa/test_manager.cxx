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
#include <tubes/collaboration.hxx>
#include <tubes/manager.hxx>
#include <unotools/localfilehelper.hxx>

#include <telepathy-glib/telepathy-glib.h>

class TeleConference;

namespace {

class TestTeleTubes: public CppUnit::TestFixture
{
public:

    virtual void setUp();
    virtual void tearDown();

    void testSession();

    // There is only one method because the code in setUp
    // and tearDown is expected to be executed only once.
    CPPUNIT_TEST_SUITE( TestTeleTubes );
    CPPUNIT_TEST( testSession );
    CPPUNIT_TEST_SUITE_END();
};

class TestCollaboration;
// static, not members, so they actually survive cppunit test iteration
static TestCollaboration*   mpCollaboration1 = NULL;
static TestCollaboration*   mpCollaboration2 = NULL;
//static bool                 mbFileSentSuccess = false;
static bool                 mbPacketReceived = false;
static OUString             maTestConfigIniURL;
static OString              maOffererIdentifier;
static OString              maAccepterIdentifier;

class TestCollaboration : public Collaboration
{
    virtual void EndCollaboration() const {}
    virtual void PacketReceived( const OString& rPacket ) const
    {
        CPPUNIT_ASSERT( rPacket == "from 1 to 2");
        mbPacketReceived = true;
    }
    virtual void SaveAndSendFile( TpContact* ) const {}
    virtual void StartCollaboration( TeleConference* ) {}
};

static gboolean timed_out( void * )
{
    CPPUNIT_ASSERT_MESSAGE( "Test took longer than ten seconds!", false);

    return FALSE;
}

void TestTeleTubes::setUp()
{
    g_timeout_add_seconds (10, timed_out, NULL);
    maTestConfigIniURL = OUString( "file://" +
            OUString::createFromAscii( getenv("SRCDIR") ) + "/tubes/qa/test-config.ini");
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

    mpCollaboration1 = new TestCollaboration();
    mpCollaboration2 = new TestCollaboration();

    CPPUNIT_ASSERT( TeleManager::init( true));
}

/* FIXME: do we need the possibility to pass function to Collaboration::SendFile() ?
static void lcl_FileSent( bool success, void * )
{
    mbFileSentSuccess = success;
}
*/

void TestTeleTubes::testSession()
{
    // First try to get account and contact
    AccountContactPairV pairs = TeleManager::getContacts();
    /* Both our accounts are meant to be signed in, and they both should be
     * capable of LibreOffice tubes because this test runs after we register
     * our handler. */
    CPPUNIT_ASSERT_MESSAGE(
        "Make sure both your test accounts are signed in "
        "and are on each other's contact lists",
        pairs.size() > 0 );

    TpAccount* mpOffererAccount = NULL;
    TpContact* mpAccepterContact = NULL;

    for (guint i = 0; i < pairs.size(); i++)
    {
        AccountContactPair pair = pairs[i];

        if (tp_account_get_normalized_name (pair.first) == maOffererIdentifier &&
            tp_contact_get_identifier (pair.second) == maAccepterIdentifier)
        {
            mpOffererAccount = pair.first;
            g_object_ref (mpOffererAccount);
            mpAccepterContact = pair.second;
            g_object_ref (mpAccepterContact);
        }
        g_object_unref (pair.first);
        g_object_unref (pair.second);
    }

    CPPUNIT_ASSERT_MESSAGE(
        "Couldn't find offerer account. "
        "Make sure both your test accounts are signed in "
        "and are on each other's contact lists",
        mpOffererAccount);
    CPPUNIT_ASSERT_MESSAGE(
        "Couldn't find accepter contact. "
        "Make sure both your test accounts are signed in "
        "and are on each other's contact lists",
        mpAccepterContact);

    // Now we can start session
    TeleConference* pConference = NULL;
    pConference = TeleManager::startBuddySession( mpOffererAccount, mpAccepterContact);
    CPPUNIT_ASSERT( pConference != NULL);
    mpCollaboration1->SetConference( pConference );
    mpCollaboration1->SendFile( mpAccepterContact, maTestConfigIniURL );

    g_object_unref(mpOffererAccount);
    mpOffererAccount = NULL;
    g_object_unref(mpAccepterContact);
    mpAccepterContact = NULL;

    //while (!mbFileSentSuccess)
    //    g_main_context_iteration( NULL, TRUE);

    // This checks that the file was received and msCurrentUUID set (see manager.cxx)
    while (!TeleManager::hasWaitingConference())
        g_main_context_iteration( NULL, TRUE);

    pConference = TeleManager::getConference();
    CPPUNIT_ASSERT( pConference != NULL);
    mpCollaboration2->SetConference( pConference );

    mpCollaboration1->SendPacket( "from 1 to 2");

    while (!mbPacketReceived)
        g_main_context_iteration( NULL, TRUE);
}

void TestTeleTubes::tearDown()
{
    // Closes the TeleConference in destructor:
    delete mpCollaboration1;
    delete mpCollaboration2;

    TeleManager::finalize();
}

CPPUNIT_TEST_SUITE_REGISTRATION( TestTeleTubes);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
