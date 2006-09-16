/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fpinteraction.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:50:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#ifndef SVTOOLS_FILEPICKER_INTERACTION_HXX
#include "fpinteraction.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif

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

