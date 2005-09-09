/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplenameclashresolverequest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:42:29 $
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

#ifndef _COM_SUN_STAR_UCB_NAMECLASHRESOLVEREQUEST_HPP_
#include <com/sun/star/ucb/NameClashResolveRequest.hpp>
#endif

#ifndef _UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX
#include <ucbhelper/simplenameclashresolverequest.hxx>
#endif

using namespace com::sun::star;
using namespace ucbhelper;

//=========================================================================
SimpleNameClashResolveRequest::SimpleNameClashResolveRequest(
                                    const rtl::OUString & rTargetFolderURL,
                                    const rtl::OUString & rClashingName,
                                    const rtl::OUString & rProposedNewName,
                                    sal_Bool bSupportsOverwriteData )
{
    // Fill request...
    ucb::NameClashResolveRequest aRequest;
//    aRequest.Message        = // OUString
//    aRequest.Context        = // XInterface
    aRequest.Classification  = task::InteractionClassification_QUERY;
    aRequest.TargetFolderURL = rTargetFolderURL;
    aRequest.ClashingName    = rClashingName;
    aRequest.ProposedNewName = rProposedNewName;

    setRequest( uno::makeAny( aRequest ) );

    // Fill continuations...
    m_xNameSupplier = new InteractionSupplyName( this );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
                            aContinuations( bSupportsOverwriteData ? 3 : 2 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = m_xNameSupplier.get();

    if ( bSupportsOverwriteData )
        aContinuations[ 2 ] = new InteractionReplaceExistingData( this );

    setContinuations( aContinuations );
}

