/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/mailmodelapi.hxx>


// class SfxBluetoothModel_Impl -----------------------------------------------

class SfxBluetoothModel:public SfxMailModel
{
public:
    SendMailResult      SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame );
    SendMailResult      Send();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
