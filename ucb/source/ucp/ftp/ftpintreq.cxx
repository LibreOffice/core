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


#include "ftpintreq.hxx"

#include <comphelper/interaction.hxx>

#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>

using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::task;
using namespace ftp;


XInteractionApproveImpl::XInteractionApproveImpl()
    : m_bSelected(false)
{
}

void SAL_CALL XInteractionApproveImpl::select()
    throw (RuntimeException,
           std::exception)
{
    m_bSelected = true;
}


// XInteractionDisapproveImpl

XInteractionDisapproveImpl::XInteractionDisapproveImpl()
    : m_bSelected(false)
{
}

void SAL_CALL XInteractionDisapproveImpl::select()
    throw (RuntimeException,
           std::exception)
{
    m_bSelected = true;
}

// XInteractionRequestImpl

XInteractionRequestImpl::XInteractionRequestImpl()
    : p1( new XInteractionApproveImpl )
    , p2( new XInteractionDisapproveImpl )
{
    uno::Sequence<uno::Reference<task::XInteractionContinuation>> continuations{
        Reference<XInteractionContinuation>(p1),
        Reference<XInteractionContinuation>(p2) };
    UnsupportedNameClashException excep;
    excep.NameClash = NameClash::ERROR;
    m_xRequest.set(new ::comphelper::OInteractionRequest(Any(excep), continuations));
}

bool XInteractionRequestImpl::approved() const
{
    return p1->isSelected();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
