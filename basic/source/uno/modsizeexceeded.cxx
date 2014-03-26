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

#include <basic/modsizeexceeded.hxx>

#include <comphelper/interaction.hxx>
#include <com/sun/star/script/ModuleSizeExceededRequest.hpp>

using namespace com::sun::star;
using namespace cppu;
using namespace osl;

ModuleSizeExceeded::ModuleSizeExceeded( const uno::Sequence< OUString >& sModules )
{
    script::ModuleSizeExceededRequest aReq;
    aReq.Names = sModules;

    m_aRequest <<= aReq;

    m_xAbort.set( uno::Reference< task::XInteractionAbort >(new comphelper::OInteractionAbort), uno::UNO_QUERY );
    m_xApprove.set( uno::Reference< task::XInteractionApprove >(new comphelper::OInteractionApprove ), uno::UNO_QUERY );
    m_lContinuations.realloc( 2 );
    m_lContinuations[0] =  m_xApprove;
    m_lContinuations[1] = m_xAbort;
}

bool
ModuleSizeExceeded::isAbort() const
{
    comphelper::OInteractionAbort* pBase = static_cast< comphelper::OInteractionAbort* >( m_xAbort.get() );
    return pBase->wasSelected();
}

bool
ModuleSizeExceeded::isApprove() const
{
    comphelper::OInteractionApprove* pBase = static_cast< comphelper::OInteractionApprove* >( m_xApprove.get() );
    return pBase->wasSelected();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
