/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_dbus.h>

#include <time.h>
#include <iostream>
#include <limits>
#include <new>
#include <stdlib.h>
#include <assert.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-common/thread-watch.h>
#include <comphelper/random.hxx>

#if ENABLE_DBUS
#include <dbus/dbus.h>
#endif

#include <sal/log.hxx>

#include "AvahiNetworkService.hxx"
#include "ZeroconfService.hxx"

using namespace sd;

static AvahiClient *client = nullptr;
static AvahiThreadedPoll *threaded_poll = nullptr;
static AvahiEntryGroup *group = nullptr;
static AvahiNetworkService *avahiService = nullptr;

static bool create_services(AvahiClient *c);

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
    assert(g == group || group == nullptr);
    group = g;

    /* Called whenever the entry group state changes */

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            SAL_INFO( "sdremote.wifi", "Service '" << avahiService->getName() << "' successfully established." );
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : {
            char *n;

            /* A service name collision with a remote service
             * happened. Let's pick a new name */
            n = avahi_alternative_service_name(avahiService->getName().c_str());
            avahiService->setName(n);

            SAL_INFO( "sdremote.wifi", "Service name collision, renaming service to '" << avahiService->getName() << "'");

            /* And recreate the services */
            create_services(avahi_entry_group_get_client(g));
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            SAL_WARN("sdremote.wifi", "Entry group failure: " << avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

            /* Some kind of failure happened while we were registering our services */
            avahi_threaded_poll_quit(threaded_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static bool create_services(AvahiClient *c) {
    assert(c);

    /* If this is the first time we're called, let's create a new
     * entry group if necessary */
    if(!client)
        return false;

    if (!group)
        if (!(group = avahi_entry_group_new(c, entry_group_callback, nullptr))) {
            SAL_WARN("sdremote.wifi", "avahi_entry_group_new() failed: " << avahi_strerror(avahi_client_errno(c)));
            avahiService->clear();
            return false;
        }

    /* If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries.  */

    if (avahi_entry_group_is_empty(group)) {
        SAL_INFO("sdremote.wifi", "Adding service '" << avahiService->getName() << "'");
        char r[128];
        int nRandom = comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max());
        snprintf(r, sizeof(r), "random=%i", nRandom);
        int ret = avahi_entry_group_add_service(
            group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, static_cast<AvahiPublishFlags>(0),
            avahiService->getName().c_str(), kREG_TYPE, nullptr, nullptr, 1599, "local", r, nullptr
        );
        if (ret < 0) {

            if (ret == AVAHI_ERR_COLLISION){
                /* A service name collision with a local service happened. Let's
                 * pick a new name */
                char *n = avahi_alternative_service_name(avahiService->getName().c_str());
                avahiService->setName(n);

                SAL_WARN("sdremote.wifi", "Service name collision, renaming service to '" << avahiService->getName() << "'");

                avahi_entry_group_reset(group);

                return create_services(c);
            }

            SAL_WARN("sdremote.wifi", "Failed to add _impressremote._tcp service: " << avahi_strerror(ret));
            avahiService->clear();
            return false;
        }

        /* Tell the server to register the service */
        if ((ret = avahi_entry_group_commit(group)) < 0) {
            SAL_WARN("sdremote.wifi", "Failed to commit entry group: " << avahi_strerror(ret));
            avahiService->clear();
            return false;
        }
    }

    return true; //Services we're already created
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    assert(c);

    /* Called whenever the client or server state changes */

    switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
            create_services(c);
            break;
        case AVAHI_CLIENT_FAILURE:
            SAL_WARN("sdremote.wifi", "Client failure: " << avahi_strerror(avahi_client_errno(c)));
            avahiService->clear();
            break;
        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:
            if (group)
                avahi_entry_group_reset(group);
            break;
        case AVAHI_CLIENT_CONNECTING:
            ;
    }
}

void AvahiNetworkService::setup() {
#if ENABLE_DBUS
    // Sure, without ENABLE_DBUS it probably makes no sense to try to use this Avahi stuff either,
    // but this is just a stop-gap measure to get this to even compile for now with the probably
    // pointless combination of configurable options --enable-avahi --enable-dbus --disable-gui.

    // Avahi internally uses D-Bus, which requires the following in order to be
    // thread-safe (and we potentially access D-Bus from different threads in
    // different places of the code base):
    if (!dbus_threads_init_default()) {
        throw std::bad_alloc();
    }
#endif

    int error = 0;
    avahiService = this;
    if (!(threaded_poll = avahi_threaded_poll_new())) {
       SAL_WARN("sdremote.wifi", "avahi_threaded_poll_new '" << avahiService->getName() << "' failed");
       return;
    }

    if (!(client = avahi_client_new(avahi_threaded_poll_get(threaded_poll), static_cast<AvahiClientFlags>(0), client_callback, nullptr, &error))) {
       SAL_WARN("sdremote.wifi", "avahi_client_new failed");
       return;
    }

    if(!create_services(client))
        return;

    /* Finally, start the event loop thread */
    if (avahi_threaded_poll_start(threaded_poll) < 0) {
       SAL_WARN("sdremote.wifi", "avahi_threaded_poll_start failed");
       return;
    }
}

void AvahiNetworkService::clear() {
    /* Call this when the app shuts down */
        if(threaded_poll)
            avahi_threaded_poll_stop(threaded_poll);
        if(client)
            avahi_client_free(client);
        if(threaded_poll)
            avahi_threaded_poll_free(threaded_poll);
}
