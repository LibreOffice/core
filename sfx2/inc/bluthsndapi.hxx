/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX_BLUTHSNDAPI_HXX
#define INCLUDED_SFX_BLUTHSNDAPI_HXX

#include <vector>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include "sfx2/dllapi.h"
#include "mailmodelapi.hxx"


// class SfxBluetoothModel_Impl -----------------------------------------------

class SFX2_DLLPUBLIC SfxBluetoothModel:public SfxMailModel
{
public:
    SendMailResult      SaveAndSend( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                     const OUString& rType );
    SendMailResult      Send( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
