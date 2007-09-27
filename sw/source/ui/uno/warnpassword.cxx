/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: warnpassword.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:44:10 $
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
+ ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
// ============================================================================

#ifndef SW_WARNPASSWORD_HXX
#include "warnpassword.hxx"
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#include <ucbhelper/simpleinteractionrequest.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAPPEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#endif
#ifndef _SVXERR_HXX
#include <svx/svxerr.hxx>
#endif

using ::rtl::OUString;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;

bool SwWarnPassword::WarningOnPassword( SfxMedium& rMedium )
{
    bool bReturn = true;
    uno::Reference< task::XInteractionHandler > xHandler( rMedium.GetInteractionHandler());
    if( xHandler.is() )
    {

        OUString empty;
        uno::Any xException( makeAny(InteractiveAppException(empty,
                uno::Reference <XInterface> (),
            InteractionClassification_QUERY,
                 ERRCODE_SVX_EXPORT_FILTER_CRYPT)));

        uno::Reference< ucbhelper::SimpleInteractionRequest > xRequest
                    = new ucbhelper::SimpleInteractionRequest(
                        xException,
                 ucbhelper::CONTINUATION_APPROVE
                           | ucbhelper::CONTINUATION_DISAPPROVE );

        xHandler->handle( xRequest.get() );

        const sal_Int32 nResp = xRequest->getResponse();

        switch ( nResp )
        {
        case ucbhelper::CONTINUATION_UNKNOWN:
                break;

        case ucbhelper::CONTINUATION_APPROVE:
                // Continue
                break;

        case ucbhelper::CONTINUATION_DISAPPROVE:
                bReturn = false;
                break;
        }
    }
    return bReturn;
}

