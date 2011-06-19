/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/interaction.hxx>
#include <osl/diagnose.h>

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;

    //=========================================================================
    //= OInteractionPassword
    //=========================================================================
    //--------------------------------------------------------------------
    void SAL_CALL OInteractionPassword::setPassword( const ::rtl::OUString& _Password ) throw (RuntimeException)
    {
        m_sPassword = _Password;;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OInteractionPassword::getPassword(  ) throw (RuntimeException)
    {
        return m_sPassword;
    }

    //=========================================================================
    //= OInteractionRequest
    //=========================================================================
    //-------------------------------------------------------------------------
    OInteractionRequest::OInteractionRequest(const Any& _rRequestDescription)
        :m_aRequest(_rRequestDescription)
    {
    }

    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    void OInteractionRequest::clearContinuations()
    {
        m_aContinuations.realloc(0);
    }

    //-------------------------------------------------------------------------
    Any SAL_CALL OInteractionRequest::getRequest(  ) throw(RuntimeException)
    {
        return m_aRequest;
    }

    //-------------------------------------------------------------------------
    Sequence< Reference< XInteractionContinuation > > SAL_CALL OInteractionRequest::getContinuations(  ) throw(RuntimeException)
    {
        return m_aContinuations;
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
