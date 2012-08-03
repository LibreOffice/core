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
 * Copyright (C) 2012 Collabora Ltd.
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
#include <utility>
#include <vector>

#include <telepathy-glib/telepathy-glib.h>

#include <tubes/constants.h>
#include <tubes/contact-list.hxx>
#include <tubes/manager.hxx>

ContactList::ContactList(TpAccountManager *pAccountManager)
    : mpAccountManager(pAccountManager)
{
    SAL_WARN_IF( !mpAccountManager, "tubes",
        "ContactList::ContactList: passed a null account manager");
    g_object_ref( mpAccountManager);

    /* Tell the client factory (which creates and prepares proxy objects) to
     * get the features we need ready before giving us any objects.
     */
    TpSimpleClientFactory *factory = tp_proxy_get_factory (mpAccountManager);
    /* We need every online account's connection object to be available... */
    tp_simple_client_factory_add_account_features_varargs (factory,
        TP_ACCOUNT_FEATURE_CONNECTION,
        0);
    /* ...and we want those connection objects to have the contact list
     * available... */
    tp_simple_client_factory_add_connection_features_varargs (factory,
        TP_CONNECTION_FEATURE_CONTACT_LIST,
        0);
    /* ...and those contacts should have their alias and their capabilities
     * available.
     */
    tp_simple_client_factory_add_contact_features_varargs (factory,
        TP_CONTACT_FEATURE_ALIAS,
        TP_CONTACT_FEATURE_AVATAR_DATA,
        TP_CONTACT_FEATURE_CAPABILITIES,
        TP_CONTACT_FEATURE_PRESENCE,
        TP_CONTACT_FEATURE_INVALID);
}

ContactList::~ContactList()
{
    g_object_unref(mpAccountManager);
    mpAccountManager = NULL;
}

bool tb_contact_is_online( TpContact *contact )
{
    switch (tp_contact_get_presence_type (contact))
    {
        case TP_CONNECTION_PRESENCE_TYPE_UNSET:
        case TP_CONNECTION_PRESENCE_TYPE_OFFLINE:
            return false;
        case TP_CONNECTION_PRESENCE_TYPE_AVAILABLE:
        case TP_CONNECTION_PRESENCE_TYPE_AWAY:
        case TP_CONNECTION_PRESENCE_TYPE_EXTENDED_AWAY:
        case TP_CONNECTION_PRESENCE_TYPE_HIDDEN:
        case TP_CONNECTION_PRESENCE_TYPE_BUSY:
            return true;
        case TP_CONNECTION_PRESENCE_TYPE_UNKNOWN:
        case TP_CONNECTION_PRESENCE_TYPE_ERROR:
        default:
            return false;
    }
}

namespace tubes {
    void reDrawAllContacts();
}
static void presence_changed_cb( TpContact* /* contact */,
                                 guint      /* type */,
                                 gchar*     /* status */,
                                 gchar*     /* message */,
                                 gpointer   /* pContactList*/ )
{
    tubes::reDrawAllContacts();
}

AccountContactPairV ContactList::getContacts()
{
  GList *accounts;
  AccountContactPairV pairs;

  for (accounts = tp_account_manager_get_valid_accounts (mpAccountManager);
       accounts != NULL;
       accounts = g_list_delete_link (accounts, accounts))
    {
      TpAccount *account = reinterpret_cast<TpAccount *>(accounts->data);
      TpConnection *connection = tp_account_get_connection (account);
      TpContact *self;
      GPtrArray *contacts;
      guint i;

      /* Verify account is online and received its contact list. If state is not
       * SUCCESS this means we didn't received the roster from server yet and
       * we would have to wait for the "notify:contact-list-state" signal. */
      if (connection == NULL ||
          tp_connection_get_contact_list_state (connection) !=
              TP_CONTACT_LIST_STATE_SUCCESS)
        continue;

      self = tp_connection_get_self_contact (connection);
      contacts = tp_connection_dup_contact_list (connection);
      for (i = 0; i < contacts->len; i++)
        {
          TpContact *contact =
              reinterpret_cast<TpContact *>(g_ptr_array_index (contacts, i));
          if (maRegistered.find (contact) == maRegistered.end())
          {
              maRegistered.insert (contact);
              g_signal_connect (contact, "presence-changed",
                      G_CALLBACK (presence_changed_cb), this );
          }

          if (contact != self &&
              tb_contact_is_online (contact))
            {
              g_object_ref (account);
              g_object_ref (contact);

              AccountContactPair pair(account, contact);
              pairs.push_back(pair);
            }
        }
      g_ptr_array_unref (contacts);
    }

  return pairs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
