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
#include "precompiled_chart2.hxx"

#include "RangeSelectionListener.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

RangeSelectionListener::RangeSelectionListener(
    RangeSelectionListenerParent & rParent,
    const ::rtl::OUString & rInitialRange,
    const Reference< frame::XModel >& xModelToLockController ) :
        m_rParent( rParent ),
        m_aRange( rInitialRange ),
        m_aControllerLockGuard( xModelToLockController )
{}

RangeSelectionListener::~RangeSelectionListener()
{}

// ____ XRangeSelectionListener ____
void SAL_CALL RangeSelectionListener::done( const sheet::RangeSelectionEvent& aEvent )
    throw (uno::RuntimeException)
{
    m_aRange = aEvent.RangeDescriptor;
    m_rParent.listeningFinished( m_aRange );
}

void SAL_CALL RangeSelectionListener::aborted( const sheet::RangeSelectionEvent& /*aEvent*/ )
    throw (uno::RuntimeException)
{
    m_rParent.listeningFinished( m_aRange );
}

// ____ XEventListener ____
void SAL_CALL RangeSelectionListener::disposing( const lang::EventObject& /*Source*/ )
    throw (uno::RuntimeException)
{
    m_rParent.disposingRangeSelection();
}

} //  namespace chart
