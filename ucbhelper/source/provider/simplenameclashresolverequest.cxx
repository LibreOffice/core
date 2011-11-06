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
#include "precompiled_ucbhelper.hxx"
#include <com/sun/star/ucb/NameClashResolveRequest.hpp>
#include <ucbhelper/simplenameclashresolverequest.hxx>

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

