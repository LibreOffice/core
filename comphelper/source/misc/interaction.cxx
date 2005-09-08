/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interaction.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:50:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;

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


