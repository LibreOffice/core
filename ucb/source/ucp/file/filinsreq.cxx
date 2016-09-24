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
#include "shell.hxx"
#include "filglob.hxx"

#include <comphelper/interaction.hxx>

#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>


using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;
using namespace fileaccess;


XInteractionRequestImpl::XInteractionRequestImpl(
    const OUString& aClashingName,
    const Reference<XInterface>& xOrigin,
    shell *pShell,sal_Int32 CommandId)
    : p1( new XInteractionSupplyNameImpl ),
      p2( new XInteractionAbortImpl ),
      m_nErrorCode(0),
      m_nMinorError(0),
      m_aClashingName(aClashingName),
      m_xOrigin(xOrigin)
{
    if( pShell )
        pShell->retrieveError(CommandId,m_nErrorCode,m_nMinorError);
    std::vector<uno::Reference<task::XInteractionContinuation>> continuations{
        Reference<XInteractionContinuation>(p1),
        Reference<XInteractionContinuation>(p2) };
    Any aAny;
    if(m_nErrorCode == TASKHANDLING_FOLDER_EXISTS_MKDIR)
    {
        NameClashException excep;
        excep.Name = m_aClashingName;
        excep.Classification = InteractionClassification_ERROR;
        excep.Context = m_xOrigin;
        excep.Message = "folder exists and overwrite forbidden";
        aAny <<= excep;
    }
    else if(m_nErrorCode == TASKHANDLING_INVALID_NAME_MKDIR)
    {
        InteractiveAugmentedIOException excep;
        excep.Code = IOErrorCode_INVALID_CHARACTER;
        PropertyValue prop;
        prop.Name = "ResourceName";
        prop.Handle = -1;
        prop.Value <<= m_aClashingName;
        Sequence<Any> seq(1);
        seq[0] <<= prop;
        excep.Arguments = seq;
        excep.Classification = InteractionClassification_ERROR;
        excep.Context = m_xOrigin;
        excep.Message = "the name contained invalid characters";
        aAny <<= excep;

    }
    m_xRequest.set(new ::comphelper::OInteractionRequest(aAny, continuations));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
