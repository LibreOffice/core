/*************************************************************************
 *
 *  $RCSfile: interaction.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-25 12:46:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    //= OInteractionApprove
    //=========================================================================
    void SAL_CALL OInteractionApprove::select(  ) throw(RuntimeException)
    {
        implSelected();
    }

    //=========================================================================
    //= OInteractionDisapprove
    //=========================================================================
    void SAL_CALL OInteractionDisapprove::select(  ) throw(RuntimeException)
    {
        implSelected();
    }

    //=========================================================================
    //= OInteractionAbort
    //=========================================================================
    void SAL_CALL OInteractionAbort::select(  ) throw(RuntimeException)
    {
        implSelected();
    }

    //=========================================================================
    //= OInteractionRetry
    //=========================================================================
    void SAL_CALL OInteractionRetry::select(  ) throw(RuntimeException)
    {
        implSelected();
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
        OSL_ENSHURE(_rxContinuation.is(), "OInteractionRequest::addContinuation: invalid argument!");
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 25.10.00 12:23:29  fs
 ************************************************************************/

