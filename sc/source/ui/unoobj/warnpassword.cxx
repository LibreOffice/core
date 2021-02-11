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

#include <warnpassword.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <sfx2/docfile.hxx>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <svx/svxerr.hxx>
#include <rtl/ref.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::task::InteractionClassification_QUERY;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::ucb::InteractiveAppException;

bool ScWarnPassword::WarningOnPassword( SfxMedium& rMedium )
{
    bool bReturn = true;
    Reference< XInteractionHandler > xHandler( rMedium.GetInteractionHandler());
    if( xHandler.is() )
    {
        Any aException( InteractiveAppException("",
                Reference <XInterface> (),
                InteractionClassification_QUERY,
                sal_uInt32(ERRCODE_SVX_EXPORT_FILTER_CRYPT)));

        rtl::Reference< ucbhelper::SimpleInteractionRequest > xRequest
                    = new ucbhelper::SimpleInteractionRequest(
                        aException,
                        ContinuationFlags::Approve | ContinuationFlags::Disapprove );

        xHandler->handle( xRequest );

        const ContinuationFlags nResp = xRequest->getResponse();

        if ( nResp == ContinuationFlags::Disapprove )
            bReturn = false;
    }
    return bReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
