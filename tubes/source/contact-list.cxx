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
}

ContactList::~ContactList()
{
    g_object_unref(mpAccountManager);
    mpAccountManager = NULL;
}

static bool tb_presence_is_online( const TpConnectionPresenceType& presence )
{
    switch (presence)
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

static bool tb_contact_is_online( TpContact *contact )
{
    return tb_presence_is_online (tp_contact_get_presence_type (contact));
}

static void presence_changed_cb( TpContact* /* contact */,
                                 guint      /* type */,
                                 gchar*     /* status */,
                                 gchar*     /* message */,
                                 gpointer   /* pContactList*/ )
{
    TeleManager::displayAllContacts();
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
