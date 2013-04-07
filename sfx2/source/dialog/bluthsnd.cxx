/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *                       SUSE.
 *
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Muthu Subramanian <sumuthu@suse.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <stdio.h>

#include "bluthsndapi.hxx"

SfxBluetoothModel::SendMailResult SfxBluetoothModel::SaveAndSend( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                            const OUString& rType )
{
    SaveResult		eSaveResult;
    SendMailResult  eResult = SEND_MAIL_ERROR;
    OUString   aFileName;

    eSaveResult  = SaveDocumentAsFormat( OUString(), xFrame, rType, aFileName );
    if( eSaveResult == SAVE_SUCCESSFULL )
    {
        maAttachedDocuments.push_back( aFileName );
        return Send( xFrame );
    }
    else if( eSaveResult == SAVE_CANCELLED )
        eResult = SEND_MAIL_CANCELLED;

    return eResult;
}

SfxBluetoothModel::SendMailResult SfxBluetoothModel::Send( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& /*xFrame*/ )
{
    char bthsend[300];
    SendMailResult eResult = SEND_MAIL_OK;
    OUString aFileName = maAttachedDocuments[0];
    snprintf(bthsend,300,"bluetooth-sendto %s",OUStringToOString( aFileName, RTL_TEXTENCODING_UTF8).getStr() );
    if( !system( bthsend ) )
        eResult = SEND_MAIL_ERROR;
    return eResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
