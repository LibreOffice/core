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
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <ucbhelper/simpleioerrorrequest.hxx>

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

