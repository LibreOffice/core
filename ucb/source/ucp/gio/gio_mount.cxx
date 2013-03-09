/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "gio_mount.hxx"
#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <stdio.h>
#include <string.h>

G_DEFINE_TYPE (OOoMountOperation, ooo_mount_operation, G_TYPE_MOUNT_OPERATION);

static void ooo_mount_operation_ask_password (GMountOperation   *op,
    const char *message, const char *default_user, const char *default_domain,
    GAskPasswordFlags flags);

static void ooo_mount_operation_init (OOoMountOperation *op)
{
    op->m_pPrevPassword = NULL;
    op->m_pPrevUsername = NULL;
}

static void ooo_mount_operation_finalize (GObject *object)
{
    OOoMountOperation *mount_op = OOO_MOUNT_OPERATION (object);
    if (mount_op->m_pPrevUsername)
        free(mount_op->m_pPrevUsername);
    if (mount_op->m_pPrevPassword)
        free(mount_op->m_pPrevPassword);

    G_OBJECT_CLASS (ooo_mount_operation_parent_class)->finalize (object);
}

static void ooo_mount_operation_class_init (OOoMountOperationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = ooo_mount_operation_finalize;

    GMountOperationClass *mount_op_class = G_MOUNT_OPERATION_CLASS (klass);
    mount_op_class->ask_password = ooo_mount_operation_ask_password;
}

using namespace com::sun::star;

static void ooo_mount_operation_ask_password (GMountOperation *op,
    const char * /*message*/, const char *default_user,
    const char *default_domain, GAskPasswordFlags flags)
{
    uno::Reference< task::XInteractionHandler > xIH;

    OOoMountOperation *pThis = (OOoMountOperation*)op;

    const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment > &xEnv = *(pThis->pEnv);

    if (xEnv.is())
      xIH = xEnv->getInteractionHandler();

    if (!xIH.is())
    {
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    ::rtl::OUString aHostName, aDomain, aUserName, aPassword;

    ucbhelper::SimpleAuthenticationRequest::EntityType eUserName =
        (flags & G_ASK_PASSWORD_NEED_USERNAME)
          ? ucbhelper::SimpleAuthenticationRequest::ENTITY_MODIFY
          : ucbhelper::SimpleAuthenticationRequest::ENTITY_NA;

    if (default_user)
        aUserName = rtl::OUString(default_user, strlen(default_user), RTL_TEXTENCODING_UTF8);

    ucbhelper::SimpleAuthenticationRequest::EntityType ePassword =
        (flags & G_ASK_PASSWORD_NEED_PASSWORD)
          ? ucbhelper::SimpleAuthenticationRequest::ENTITY_MODIFY
          : ucbhelper::SimpleAuthenticationRequest::ENTITY_NA;

    rtl::OUString aPrevPassword, aPrevUsername;
    if (pThis->m_pPrevUsername)
        aPrevUsername = rtl::OUString(pThis->m_pPrevUsername, strlen(pThis->m_pPrevUsername), RTL_TEXTENCODING_UTF8);
    if (pThis->m_pPrevPassword)
        aPrevPassword = rtl::OUString(pThis->m_pPrevPassword, strlen(pThis->m_pPrevPassword), RTL_TEXTENCODING_UTF8);

    //The damn dialog is stupidly broken, so do like webdav, i.e. "#102871#"
    if ( aUserName.isEmpty() )
        aUserName = aPrevUsername;

    if ( aPassword.isEmpty() )
        aPassword = aPrevPassword;

    ucbhelper::SimpleAuthenticationRequest::EntityType eDomain =
        (flags & G_ASK_PASSWORD_NEED_DOMAIN)
          ? ucbhelper::SimpleAuthenticationRequest::ENTITY_MODIFY
          : ucbhelper::SimpleAuthenticationRequest::ENTITY_NA;

    if (default_domain)
        aDomain = rtl::OUString(default_domain, strlen(default_domain), RTL_TEXTENCODING_UTF8);

    uno::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
        = new ucbhelper::SimpleAuthenticationRequest (rtl::OUString() /* FIXME: provide URL here */, aHostName, eDomain, aDomain, eUserName, aUserName, ePassword, aPassword);

    xIH->handle( xRequest.get() );

    rtl::Reference< ucbhelper::InteractionContinuation > xSelection = xRequest->getSelection();

    if ( !xSelection.is() )
    {
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    uno::Reference< task::XInteractionAbort > xAbort(xSelection.get(), uno::UNO_QUERY );
    if ( xAbort.is() )
    {
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    const rtl::Reference< ucbhelper::InteractionSupplyAuthentication > & xSupp = xRequest->getAuthenticationSupplier();
    aUserName = xSupp->getUserName();
    aPassword = xSupp->getPassword();

    if (flags & G_ASK_PASSWORD_NEED_USERNAME)
        g_mount_operation_set_username(op, rtl::OUStringToOString(aUserName, RTL_TEXTENCODING_UTF8).getStr());

    if (flags & G_ASK_PASSWORD_NEED_PASSWORD)
        g_mount_operation_set_password(op, rtl::OUStringToOString(aPassword, RTL_TEXTENCODING_UTF8).getStr());

    if (flags & G_ASK_PASSWORD_NEED_DOMAIN)
        g_mount_operation_set_domain(op, rtl::OUStringToOString(xSupp->getRealm(), RTL_TEXTENCODING_UTF8).getStr());

    switch (xSupp->getRememberPasswordMode())
    {
    default:
        case ucb::RememberAuthentication_NO:
            g_mount_operation_set_password_save(op, G_PASSWORD_SAVE_NEVER);
            break;
        case ucb::RememberAuthentication_SESSION:
            g_mount_operation_set_password_save(op, G_PASSWORD_SAVE_FOR_SESSION);
            break;
        case ucb::RememberAuthentication_PERSISTENT:
            g_mount_operation_set_password_save(op, G_PASSWORD_SAVE_PERMANENTLY);
            break;
    }

    if (pThis->m_pPrevPassword)
        free(pThis->m_pPrevPassword);
    pThis->m_pPrevPassword = strdup(rtl::OUStringToOString(aPassword, RTL_TEXTENCODING_UTF8).getStr());
    if (pThis->m_pPrevUsername)
        free(pThis->m_pPrevUsername);
    pThis->m_pPrevUsername = strdup(rtl::OUStringToOString(aUserName, RTL_TEXTENCODING_UTF8).getStr());
    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

GMountOperation *ooo_mount_operation_new(const uno::Reference< ucb::XCommandEnvironment >& rEnv)
{
    OOoMountOperation *pRet = (OOoMountOperation*)g_object_new (OOO_TYPE_MOUNT_OPERATION, NULL);
    pRet->pEnv = &rEnv;
    return (GMountOperation*)pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
