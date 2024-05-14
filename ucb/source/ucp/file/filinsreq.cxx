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

#include "filinsreq.hxx"
#include "filtask.hxx"

#include <comphelper/interaction.hxx>

#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>


using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace fileaccess;


XInteractionRequestImpl::XInteractionRequestImpl(
    const OUString& aClashingName,
    const Reference<XInterface>& xOrigin,
    TaskManager *pShell,sal_Int32 CommandId)
    : p1( new XInteractionSupplyNameImpl ),
      p2( new XInteractionAbortImpl ),
      m_xOrigin(xOrigin)
{
    sal_Int32 nErrorCode(0), nMinorError(0);
    if( pShell )
        pShell->retrieveError(CommandId,nErrorCode,nMinorError);
    std::vector<uno::Reference<task::XInteractionContinuation>> continuations{
        Reference<XInteractionContinuation>(p1),
        Reference<XInteractionContinuation>(p2) };
    Any aAny;
    if(nErrorCode == TASKHANDLING_FOLDER_EXISTS_MKDIR)
    {
        NameClashException excep(u"folder exists and overwrite forbidden"_ustr, m_xOrigin,
                                 InteractionClassification_ERROR, aClashingName);
        aAny <<= excep;
    }
    else if(nErrorCode == TASKHANDLING_INVALID_NAME_MKDIR)
    {
        PropertyValue prop;
        prop.Name = "ResourceName";
        prop.Handle = -1;
        prop.Value <<= aClashingName;
        InteractiveAugmentedIOException excep(u"the name contained invalid characters"_ustr, m_xOrigin,
                                              InteractionClassification_ERROR,
                                              IOErrorCode_INVALID_CHARACTER, { Any(prop) });
        aAny <<= excep;

    }
    m_xRequest.set(new ::comphelper::OInteractionRequest(aAny, std::move(continuations)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
