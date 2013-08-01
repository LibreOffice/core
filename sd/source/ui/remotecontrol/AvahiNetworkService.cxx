/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <time.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-common/thread-watch.h>

#include "AvahiNetworkService.hxx"
#include "ZeroconfService.hxx"

using namespace sd;

static AvahiClient *client = NULL;
static AvahiThreadedPoll *threaded_poll = NULL;
static AvahiEntryGroup *group = NULL;
static AvahiNetworkService *avahiService = NULL;

static void create_services(AvahiClient *c);

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
    assert(g == group || group == NULL);
    group = g;

    /* Called whenever the entry group state changes */

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            std::cerr << "Service " << avahiService->getName().c_str() << " successfully established.\n";
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : {
            char *n;

            /* A service name collision with a remote service
             * happened. Let's pick a new name */
            n = avahi_alternative_service_name(avahiService->getName().c_str());
            avahiService->setName(n);

            std::cerr << "Service name collision, renaming service to " << avahiService->getName() << std::endl;

            /* And recreate the services */
            create_services(avahi_entry_group_get_client(g));
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

            /* Some kind of failure happened while we were registering our services */
            avahi_threaded_poll_quit(threaded_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static void create_services(AvahiClient *c) {
    char *n, r[128];
    int ret;
    assert(c);

    /* If this is the first time we're called, let's create a new
     * entry group if necessary */

    if (!group)
        if (!(group = avahi_entry_group_new(c, entry_group_callback, NULL))) {
            fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(c)));
            avahiService->clear();
        }

    /* If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries.  */

    if (avahi_entry_group_is_empty(group)) {
        fprintf(stderr, "Adding service '%s'\n", avahiService->getName().c_str());
        snprintf(r, sizeof(r), "random=%i", rand());
        if ((ret = avahi_entry_group_add_service(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, static_cast<AvahiPublishFlags>(0), avahiService->getName().c_str(), "_impressremote._tcp", NULL, NULL, 1599, "local", r, NULL)) < 0) {

            if (ret == AVAHI_ERR_COLLISION){
                /* A service name collision with a local service happened. Let's
                 * pick a new name */
                n = avahi_alternative_service_name(avahiService->getName().c_str());
                avahiService->setName(n);

                fprintf(stderr, "Service name collision, renaming service to '%s'\n", avahiService->getName().c_str());

                avahi_entry_group_reset(group);

                create_services(c);
                return;
            }

            fprintf(stderr, "Failed to add _impressremote._tcp service: %s\n", avahi_strerror(ret));
            avahiService->clear();
        }

        /* Tell the server to register the service */
        if ((ret = avahi_entry_group_commit(group)) < 0) {
            fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
            avahiService->clear();
        }
    }

    return;
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    assert(c);

    /* Called whenever the client or server state changes */

    switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
            create_services(c);
            break;
        case AVAHI_CLIENT_FAILURE:
            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
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
   int error = 0;
   avahiService = this;
   if (!(threaded_poll = avahi_threaded_poll_new())) {
     fprintf(stderr, "avahi_threaded_poll_new '%s' failed.\n", avahi_strdup( avahiService->getName().c_str() ));
     return;
   }

   if (!(client = avahi_client_new(avahi_threaded_poll_get(threaded_poll), static_cast<AvahiClientFlags>(0), client_callback, NULL, &error))) {
     fprintf(stderr, "avahi_client_new failed.\n");
     return;
   }

   create_services(client);

   /* Finally, start the event loop thread */
   if (avahi_threaded_poll_start(threaded_poll) < 0) {
     fprintf(stderr, "avahi_threaded_poll_start failed.\n");
     return;
   }
}

void AvahiNetworkService::clear() {
  /* Call this when the app shuts down */
  avahi_threaded_poll_stop(threaded_poll);
  avahi_client_free(client);
  avahi_threaded_poll_free(threaded_poll);
}
