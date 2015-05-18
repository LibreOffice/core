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

#include <comphelper/interaction.hxx>
#include <osl/diagnose.h>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;

    void SAL_CALL OInteractionPassword::setPassword( const OUString& _Password ) throw (RuntimeException, std::exception)
    {
        m_sPassword = _Password;
    }


    OUString SAL_CALL OInteractionPassword::getPassword(  ) throw (RuntimeException, std::exception)
    {
        return m_sPassword;
    }

    OInteractionRequest::OInteractionRequest(const Any& _rRequestDescription)
        :m_aRequest(_rRequestDescription)
    {
    }


    void OInteractionRequest::addContinuation(const Reference< XInteractionContinuation >& _rxContinuation)
    {
        OSL_ENSURE(_rxContinuation.is(), "OInteractionRequest::addContinuation: invalid argument!");
        if (_rxContinuation.is())
        {
            sal_Int32 nOldLen = m_aContinuations.getLength();
            m_aContinuations.realloc(nOldLen + 1);
            m_aContinuations[nOldLen] = _rxContinuation;
        }
    }


    Any SAL_CALL OInteractionRequest::getRequest(  ) throw(RuntimeException, std::exception)
    {
        return m_aRequest;
    }


    Sequence< Reference< XInteractionContinuation > > SAL_CALL OInteractionRequest::getContinuations(  ) throw(RuntimeException, std::exception)
    {
        return m_aContinuations;
    }


}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
