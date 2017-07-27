/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <stdio.h>

#include "bluthsndapi.hxx"

SfxBluetoothModel::SendMailResult SfxBluetoothModel::SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    SaveResult      eSaveResult;
    SendMailResult  eResult = SEND_MAIL_ERROR;
    OUString   aFileName;

    eSaveResult  = SaveDocumentAsFormat( OUString(), xFrame, OUString(), aFileName );
    if( eSaveResult == SAVE_SUCCESSFULL )
    {
        maAttachedDocuments.push_back( aFileName );
        return Send();
    }
    else if( eSaveResult == SAVE_CANCELLED )
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

SfxBluetoothModel::SendMailResult SfxBluetoothModel::Send()
{
#ifndef LINUX
    (void) this; // avoid loplugin:staticmethods
    return SEND_MAIL_ERROR;
#else
    char bthsend[300];
    SendMailResult eResult = SEND_MAIL_OK;
    OUString aFileName = maAttachedDocuments[0];
    snprintf(bthsend,300,"bluetooth-sendto %s",OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8).getStr() );
    if( !system( bthsend ) )
        eResult = SEND_MAIL_ERROR;
    return eResult;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
