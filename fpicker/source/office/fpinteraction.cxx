/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fpinteraction.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_fpicker.hxx"
#include "fpinteraction.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>

//........................................................................
namespace svt
{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::ucb;

    //====================================================================
    //= OFilePickerInteractionHandler
    //====================================================================
    DBG_NAME( OFilePickerInteractionHandler )
    //--------------------------------------------------------------------
    OFilePickerInteractionHandler::OFilePickerInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxMaster )
        :m_xMaster( _rxMaster )
        ,m_bUsed( sal_False )
        ,m_eInterceptions( OFilePickerInteractionHandler::E_NOINTERCEPTION )
    {
        DBG_CTOR( OFilePickerInteractionHandler, NULL );
        DBG_ASSERT( m_xMaster.is(), "OFilePickerInteractionHandler::OFilePickerInteractionHandler: invalid master handler!" );
    }

    //--------------------------------------------------------------------
    OFilePickerInteractionHandler::~OFilePickerInteractionHandler( )
    {
        DBG_DTOR( OFilePickerInteractionHandler, NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OFilePickerInteractionHandler::handle( const Reference< XInteractionRequest >& _rxRequest ) throw (RuntimeException)
    {
        if (!_rxRequest.is())
            return;

        m_bUsed = sal_True;

        // extract some generic continuations ... might we need it later
        // if something goes wrong.
        Reference< XInteractionAbort >       xAbort;
        Reference< XInteractionApprove >     xApprove;
        Reference< XInteractionDisapprove >  xDisapprove;
        Reference< XInteractionRetry >       xRetry;

        const Sequence< Reference< XInteractionContinuation > > lConts = _rxRequest->getContinuations();
        const Reference< XInteractionContinuation >*            pConts = lConts.getConstArray();
        for (sal_Int32 i=0; i<lConts.getLength(); ++i)
        {
            if (!xAbort.is())
                xAbort = Reference< XInteractionAbort >(pConts[i], UNO_QUERY);
            if (!xApprove.is())
                xApprove = Reference< XInteractionApprove >(pConts[i], UNO_QUERY);
            if (!xDisapprove.is())
                xDisapprove = Reference< XInteractionDisapprove >(pConts[i], UNO_QUERY);
            if (!xRetry.is())
                xRetry = Reference< XInteractionRetry >(pConts[i], UNO_QUERY);
        }

        // safe the original request for later analyzing!
        m_aException = _rxRequest->getRequest();

        // intercept some interesting interactions

        // The "does not exist" interaction will be supressed here completly.
        if (m_eInterceptions & OFilePickerInteractionHandler::E_DOESNOTEXIST)
        {
            InteractiveIOException aIoException;
            if (
                (m_aException             >>= aIoException     ) &&
                (IOErrorCode_NOT_EXISTING  == aIoException.Code)
               )
            {
                if (xAbort.is())
                    xAbort->select();
                return;
            }
        }

        // no master => abort this operation ...
        if (!m_xMaster.is())
        {
            if (xAbort.is())
                xAbort->select();
            return;
        }

        // forward it to our master - so he can handle all
        // not interesting interactions :-)
        m_xMaster->handle(_rxRequest);
    }

    //--------------------------------------------------------------------
    void OFilePickerInteractionHandler::enableInterceptions( EInterceptedInteractions eInterceptions )
    {
        m_eInterceptions = eInterceptions;
    }

    //--------------------------------------------------------------------
    sal_Bool OFilePickerInteractionHandler::wasUsed() const
    {
        return m_bUsed;
    }

    //--------------------------------------------------------------------
    void OFilePickerInteractionHandler::resetUseState()
    {
        m_bUsed = sal_False;
    }

    //--------------------------------------------------------------------
    void OFilePickerInteractionHandler::forgetRequest()
    {
        m_aException = Any();
    }

    //--------------------------------------------------------------------
    sal_Bool OFilePickerInteractionHandler::wasAccessDenied() const
    {
        InteractiveIOException aIoException;
        if (
            (m_aException              >>= aIoException     ) &&
            (IOErrorCode_ACCESS_DENIED  == aIoException.Code)
           )
        {
            return sal_True;
        }
        return sal_False;
    }

//........................................................................
}   // namespace svt
//........................................................................

