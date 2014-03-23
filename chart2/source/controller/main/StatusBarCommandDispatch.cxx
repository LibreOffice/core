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

#include "StatusBarCommandDispatch.hxx"
#include "ObjectNameProvider.hxx"
#include "macros.hxx"
#include <com/sun/star/util/XModifyBroadcaster.hpp>

// #ifndef _VOS_MUTEX_HXX_
// #include <vos/mutex.hxx>
// #endif
// #ifndef _SV_SVAPP_HXX
// #include <vcl/svapp.hxx>
// #endif

#include "ResId.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

StatusBarCommandDispatch::StatusBarCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< frame::XModel > & xModel,
    const Reference< view::XSelectionSupplier > & xSelSupp ) :
        impl::StatusBarCommandDispatch_Base( xContext ),
        m_xModifiable( xModel, uno::UNO_QUERY  ),
        m_xSelectionSupplier( xSelSupp ),
        m_bIsModified( false )
{}

StatusBarCommandDispatch::~StatusBarCommandDispatch()
{}

void StatusBarCommandDispatch::initialize()
{
    if( m_xModifiable.is())
    {
        Reference< util::XModifyBroadcaster > xModifyBroadcaster( m_xModifiable, uno::UNO_QUERY );
        OSL_ASSERT( xModifyBroadcaster.is());
        if( xModifyBroadcaster.is())
            xModifyBroadcaster->addModifyListener( this );
    }

    if( m_xSelectionSupplier.is())
    {
        m_xSelectionSupplier->addSelectionChangeListener( this );
    }
}

void StatusBarCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    bool bFireAll( rURL.isEmpty() );
    bool bFireContext(  bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Context")));
    bool bFireModified( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:ModifiedStatus")));

    if( bFireContext )
    {
        uno::Any aArg;
        Reference< chart2::XChartDocument > xDoc( m_xModifiable, uno::UNO_QUERY );
        aArg <<= ObjectNameProvider::getSelectedObjectText( m_aSelectedOID.getObjectCID(), xDoc );
        fireStatusEventForURL( C2U(".uno:Context"), aArg, true, xSingleListener );
    }
    if( bFireModified )
    {
        uno::Any aArg;
        if( m_bIsModified )
            aArg <<= C2U("*");
        fireStatusEventForURL( C2U(".uno:ModifiedStatus"), aArg, true, xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL StatusBarCommandDispatch::dispatch(
    const util::URL& /* URL */,
    const Sequence< beans::PropertyValue >& /* Arguments */ )
    throw (uno::RuntimeException)
{
    // nothing to do here
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL StatusBarCommandDispatch::disposing()
{
    m_xModifiable.clear();
    m_xSelectionSupplier.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StatusBarCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    m_xModifiable.clear();
    m_xSelectionSupplier.clear();
}

// ____ XModifyListener ____
void SAL_CALL StatusBarCommandDispatch::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    if( m_xModifiable.is())
        m_bIsModified = m_xModifiable->isModified();

    CommandDispatch::modified( aEvent );
}

// ____ XSelectionChangeListener ____
void SAL_CALL StatusBarCommandDispatch::selectionChanged( const lang::EventObject& /* aEvent */ )
    throw (uno::RuntimeException)
{
    if( m_xSelectionSupplier.is())
        m_aSelectedOID = ObjectIdentifier( m_xSelectionSupplier->getSelection() );
    else
        m_aSelectedOID = ObjectIdentifier();
    fireAllStatusEvents( 0 );
}

} //  namespace chart
