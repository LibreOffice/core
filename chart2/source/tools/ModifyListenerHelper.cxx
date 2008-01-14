/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ModifyListenerHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:04:09 $
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
#include "precompiled_chart2.hxx"

#include "ModifyListenerHelper.hxx"
#include "WeakListenerAdapter.hxx"
#include "macros.hxx"

#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/frame/XModel.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{

void lcl_fireModifyEvent(
    ::cppu::OBroadcastHelper & rBroadcastHelper,
    const Reference< uno::XWeak > & xEventSource,
    const lang::EventObject * pEvent )
{
    ::cppu::OInterfaceContainerHelper * pCntHlp = rBroadcastHelper.getContainer(
        ::getCppuType( reinterpret_cast< Reference< util::XModifyListener > * >(0)));
    if( pCntHlp )
    {
        lang::EventObject aEventToSend;
        if( pEvent )
            aEventToSend = *pEvent;
        else
            aEventToSend.Source.set( xEventSource );
        OSL_ENSURE( aEventToSend.Source.is(), "Sending event without source" );

        ::cppu::OInterfaceIteratorHelper aIt( *pCntHlp );

        while( aIt.hasMoreElements())
        {
            Reference< util::XModifyListener > xModListener( aIt.next(), uno::UNO_QUERY );
            if( xModListener.is())
                xModListener->modified( aEventToSend );
        }
    }
}

struct lcl_weakReferenceToSame : public ::std::unary_function<
        ::std::pair<
            ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XModifyListener >,
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > >,
        bool >
{
    lcl_weakReferenceToSame( const Reference< util::XModifyListener > & xModListener ) :
            m_xHardRef( xModListener )
    {}

    bool operator() ( const argument_type & xElem )
    {
        Reference< util::XModifyListener > xWeakAsHard( xElem.first );
        if( xWeakAsHard.is())
            return (xWeakAsHard == m_xHardRef);
        return false;
    }

private:
    Reference< util::XModifyListener > m_xHardRef;
};

} //  anonymous namespace

// ================================================================================

namespace chart
{
namespace ModifyListenerHelper
{

ModifyEventForwarder::ModifyEventForwarder() :
        ::cppu::WeakComponentImplHelper2<
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >( m_aMutex ),
        m_aModifyListeners( m_aMutex )
{
}

void ModifyEventForwarder::FireEvent( const Reference< uno::XWeak > & xSource )
{
    lcl_fireModifyEvent( m_aModifyListeners, xSource, 0 );
}

void ModifyEventForwarder::FireEvent( const lang::EventObject & rEvent )
{
    lcl_fireModifyEvent( m_aModifyListeners, Reference< uno::XWeak >(), & rEvent );
}

void ModifyEventForwarder::AddListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyListener > xListenerToAdd( aListener );

        Reference< uno::XWeak > xWeak( aListener, uno::UNO_QUERY );
        if( xWeak.is())
        {
            // remember the helper class for later remove
            uno::WeakReference< util::XModifyListener > xWeakRef( aListener );
            xListenerToAdd.set( new WeakModifyListenerAdapter( xWeakRef ));
            m_aListenerMap.push_back( tListenerMap::value_type( xWeakRef, xListenerToAdd ));
        }

        m_aModifyListeners.addListener( ::getCppuType( &xListenerToAdd ), xListenerToAdd );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ModifyEventForwarder::RemoveListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        // look up fitting helper class that has been added
        Reference< util::XModifyListener > xListenerToRemove( aListener );
        tListenerMap::iterator aIt(
            ::std::find_if( m_aListenerMap.begin(), m_aListenerMap.end(), lcl_weakReferenceToSame( aListener )));
        if( aIt != m_aListenerMap.end())
        {
            xListenerToRemove.set( (*aIt).second );
            // map entry is no longer needed
            m_aListenerMap.erase( aIt );
        }

        m_aModifyListeners.removeListener( ::getCppuType( &aListener ), xListenerToRemove );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ModifyEventForwarder::DisposeAndClear( const Reference< uno::XWeak > & xSource )
{
    ::cppu::OInterfaceContainerHelper * pCntHlp = m_aModifyListeners.getContainer(
        ::getCppuType( reinterpret_cast< Reference< util::XModifyListener > * >(0)));
    if( pCntHlp )
        pCntHlp->disposeAndClear( lang::EventObject( xSource ) );
}

// ____ XModifyBroadcaster ____
void SAL_CALL ModifyEventForwarder::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    AddListener( aListener );
}

void SAL_CALL ModifyEventForwarder::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    RemoveListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL ModifyEventForwarder::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    FireEvent( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ModifyEventForwarder::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ WeakComponentImplHelperBase ____
void SAL_CALL ModifyEventForwarder::disposing()
{
    // dispose was called at this
    DisposeAndClear( this );
}

// ================================================================================

ModifyListenerOnDemmandRefresh::ModifyListenerOnDemmandRefresh( ::osl::Mutex & rMutex )
        : m_rMutex( rMutex ),
          m_bNeedsUpdate( true )
{
}

void ModifyListenerOnDemmandRefresh::listenAtDocument(
    const Reference< chart2::XChartDocument > & xModel )
{
    Reference< util::XModifyBroadcaster > xBroadcaster( xModel, uno::UNO_QUERY );
    if( xBroadcaster.is())
    {
        try
        {
            xBroadcaster->addModifyListener( Reference< util::XModifyListener >( this ));
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

void ModifyListenerOnDemmandRefresh::update()
{
    m_bNeedsUpdate = false;
}

bool ModifyListenerOnDemmandRefresh::needsUpdate() const
{
    return m_bNeedsUpdate;
}

// ____ XModifyListener ____
void SAL_CALL ModifyListenerOnDemmandRefresh::modified(
    const lang::EventObject& /* aEvent */ )
    throw (uno::RuntimeException)
{
    m_bNeedsUpdate = true;
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ModifyListenerOnDemmandRefresh::disposing(
    const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    m_bNeedsUpdate = true;
}

} //  namespace ModifyListenerHelper
} //  namespace chart
