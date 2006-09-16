/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpleioerrorrequest.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 17:24:44 $
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
#include "precompiled_ucbhelper.hxx"

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEAUGMENTEDIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#endif

#ifndef _UCBHELPER_SIMPLEIOERRORREQUEST_HXX
#include <ucbhelper/simpleioerrorrequest.hxx>
#endif

using namespace com::sun::star;
using namespace ucbhelper;

//=========================================================================
SimpleIOErrorRequest::SimpleIOErrorRequest(
                const ucb::IOErrorCode eError,
                const uno::Sequence< uno::Any > & rArgs,
                const rtl::OUString & rMessage,
                const uno::Reference< ucb::XCommandProcessor > & xContext )
{
    // Fill request...
    ucb::InteractiveAugmentedIOException aRequest;
    aRequest.Message         = rMessage;
    aRequest.Context         = xContext;
    aRequest.Classification  = task::InteractionClassification_ERROR;
    aRequest.Code            = eError;
    aRequest.Arguments       = rArgs;

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    uno::Sequence< uno::Reference<
            task::XInteractionContinuation > > aContinuations( 1 );
    aContinuations[ 0 ] = new InteractionAbort( this );

    setContinuations( aContinuations );
}

