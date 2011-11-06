/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


