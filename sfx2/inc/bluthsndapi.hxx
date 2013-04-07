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

#ifndef INCLUDED_SFX_BLUETOOTHL_HXX
#define INCLUDED_SFX_BLUETOOTH_HXX

#include <vector>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "tools/link.hxx"
#include <tools/string.hxx>
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

#endif // INCLUDED_SFX_BLUETOOTH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
