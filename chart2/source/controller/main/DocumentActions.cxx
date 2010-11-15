/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_chart2.hxx"

#include "DocumentActions.hxx"
#include "ImplDocumentActions.hxx"
#include "DisposeHelper.hxx"
#include "MutexContainer.hxx"
#include "macros.hxx"
#include "ChartViewHelper.hxx"

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <unotools/configitem.hxx>
#include <tools/diagnose_ex.h>
#include <cppuhelper/compbase1.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <rtl/uuid.h>

#include <functional>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::document::XUndoManagerSupplier;
using ::com::sun::star::document::XUndoAction;
using ::rtl::OUString;


// --------------------------------------------------------------------------------

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper1<
        util::XModifyBroadcaster >
    ModifyBroadcaster_Base;

class ModifyBroadcaster :
        public ::chart::MutexContainer,
        public ModifyBroadcaster_Base
{
public:
    ModifyBroadcaster();

    void fireEvent();

public:
    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener( const Reference< util::XModifyListener >& xListener )
        throw (uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const Reference< util::XModifyListener >& xListener )
        throw (uno::RuntimeException);
};

ModifyBroadcaster::ModifyBroadcaster() :
        ModifyBroadcaster_Base( m_aMutex )
{}

void SAL_CALL ModifyBroadcaster::addModifyListener(
    const Reference< util::XModifyListener >& xListener )
    throw (uno::RuntimeException)
{
    rBHelper.addListener( ::getCppuType( & xListener ), xListener);
}

void SAL_CALL ModifyBroadcaster::removeModifyListener(
    const Reference< util::XModifyListener >& xListener )
    throw (uno::RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( & xListener ), xListener );
}

void ModifyBroadcaster::fireEvent()
{
    ::cppu::OInterfaceContainerHelper* pIC = rBHelper.getContainer(
        ::getCppuType((const uno::Reference< util::XModifyListener >*)0) );
    if( pIC )
    {
        lang::EventObject aEvent( static_cast< lang::XComponent* >( this ) );
        ::cppu::OInterfaceIteratorHelper aIt( *pIC );
        while( aIt.hasMoreElements() )
        {
            uno::Reference< util::XModifyListener > xListener( aIt.next(), uno::UNO_QUERY );
            if( xListener.is() )
                xListener->modified( aEvent );
        }
    }
}

} // namespace impl

DocumentActions::DocumentActions( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rModel )
    :impl::DocumentActions_Base( m_aMutex )
    ,m_pDocumentSnapshot()
    ,m_pModifyBroadcaster( NULL )
    ,m_aModel( rModel )
    ,m_xUndoManager()
{
    Reference< XUndoManagerSupplier > xSuppUndo( rModel, UNO_QUERY_THROW );
    m_xUndoManager.set( xSuppUndo->getUndoManager(), UNO_SET_THROW );
}

DocumentActions::~DocumentActions()
{
    if ( m_pModifyBroadcaster.is() )
    {
        m_pModifyBroadcaster->dispose();
        m_pModifyBroadcaster.clear();
    }
}

void DocumentActions::addShapeUndoAction( SdrUndoAction* pAction )
{
    if ( !pAction )
        return;
    const Reference< XUndoAction > xAction( new impl::ShapeUndoElement( *pAction ) );
    m_xUndoManager->addUndoAction( xAction );
    impl_fireModifyEvent();
}

Reference< XModel > DocumentActions::impl_getModel() const
{
    Reference< XModel > xModel( m_aModel );
    return xModel;
}

void DocumentActions::impl_fireModifyEvent()
{
    if ( m_pModifyBroadcaster.is() )
        m_pModifyBroadcaster->fireEvent();
}


// ____ XModifyBroadcaster ____
void SAL_CALL DocumentActions::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    if ( !m_pModifyBroadcaster.is() )
        m_pModifyBroadcaster.set( new impl::ModifyBroadcaster() );

    m_pModifyBroadcaster->addModifyListener( aListener );
}

void SAL_CALL DocumentActions::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    if ( m_pModifyBroadcaster.is() )
        m_pModifyBroadcaster->removeModifyListener( aListener );
}

// ____ chart2::XDocumentActions ____
void SAL_CALL DocumentActions::preAction(  )
    throw (uno::RuntimeException)
{
    ENSURE_OR_THROW( !m_pDocumentSnapshot, "DocumentActions::preAction: already started an action!" );
    m_pDocumentSnapshot.reset( new impl::ChartModelClone( impl_getModel(), impl::E_MODEL ) );
}

void SAL_CALL DocumentActions::preActionWithArguments( const Sequence< beans::PropertyValue >& aArguments ) throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    ENSURE_OR_THROW( !m_pDocumentSnapshot, "DocumentActions::preAction: already started an action!" );

    impl::ModelFacet eModelFacet( impl::E_MODEL );
    ::comphelper::NamedValueCollection aArgs( aArguments );

    const sal_Bool bWithData = aArgs.getOrDefault( "WithData", sal_False );
    if ( bWithData )
        eModelFacet = impl::E_MODEL_WITH_DATA;

    const sal_Bool bWithSelection = aArgs.getOrDefault( "WithSelection", sal_False );
    if ( bWithSelection )
        eModelFacet = impl::E_MODEL_WITH_SELECTION;

    const Reference< XModel > xModel( impl_getModel() );
    m_pDocumentSnapshot.reset( new impl::ChartModelClone( xModel, eModelFacet ) );
}

void SAL_CALL DocumentActions::postAction( const OUString& aUndoText )
    throw (uno::RuntimeException)
{
    ENSURE_OR_THROW( !!m_pDocumentSnapshot.get(), "no current action" );

    const Reference< XUndoAction > xAction( new impl::UndoElement( aUndoText, impl_getModel(), m_pDocumentSnapshot ) );
    m_pDocumentSnapshot.reset();

    m_xUndoManager->addUndoAction( xAction );

    impl_fireModifyEvent();
}

void SAL_CALL DocumentActions::cancelAction()
    throw (uno::RuntimeException)
{
    ENSURE_OR_THROW( !!m_pDocumentSnapshot.get(), "no current action" );

    m_pDocumentSnapshot->dispose();
    m_pDocumentSnapshot.reset();
}

void SAL_CALL DocumentActions::cancelActionWithUndo(  )
    throw (uno::RuntimeException)
{
    ENSURE_OR_THROW( !!m_pDocumentSnapshot.get(), "no current action" );

    m_pDocumentSnapshot->applyToModel( impl_getModel() );
    m_pDocumentSnapshot->dispose();
    m_pDocumentSnapshot.reset();
}

void SAL_CALL DocumentActions::undo(  )
    throw (uno::RuntimeException)
{
    try
    {
        m_xUndoManager->undo();
    }
    catch ( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SAL_CALL DocumentActions::redo(  )
    throw (uno::RuntimeException)
{
    try
    {
        m_xUndoManager->redo();
    }
    catch ( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

::sal_Bool SAL_CALL DocumentActions::undoPossible()
    throw (uno::RuntimeException)
{
    return m_xUndoManager->isUndoPossible();
}

::sal_Bool SAL_CALL DocumentActions::redoPossible()
    throw (uno::RuntimeException)
{
    return m_xUndoManager->isRedoPossible();
}

OUString SAL_CALL DocumentActions::getCurrentUndoString()
    throw (uno::RuntimeException)
{
    OUString sTitle;
    try
    {
        sTitle = m_xUndoManager->getCurrentUndoActionTitle();
    }
    catch ( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return sTitle;
}

OUString SAL_CALL DocumentActions::getCurrentRedoString()
    throw (uno::RuntimeException)
{
    OUString sTitle;
    try
    {
        sTitle = m_xUndoManager->getCurrentRedoActionTitle();
    }
    catch ( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return sTitle;
}

Sequence< OUString > SAL_CALL DocumentActions::getAllUndoStrings()
    throw (uno::RuntimeException)
{
    Sequence< OUString > aStrings;
    try
    {
        aStrings = m_xUndoManager->getAllUndoActionTitles();
    }
    catch ( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aStrings;
}

Sequence< OUString > SAL_CALL DocumentActions::getAllRedoStrings()
    throw (uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aStrings;
    try
    {
        aStrings = m_xUndoManager->getAllRedoActionTitles();
    }
    catch ( const RuntimeException& ) { throw; }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aStrings;
}

// ____ XUnoTunnel ____
sal_Int64 DocumentActions::getSomething( const Sequence< sal_Int8 >& rId )
    throw (uno::RuntimeException)
{
    if ( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ) );
    }
    return 0;
}

// static
const Sequence< sal_Int8 >& DocumentActions::getUnoTunnelId()
{
    static Sequence< sal_Int8 >* pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
DocumentActions* DocumentActions::getImplementation( const Reference< uno::XInterface > xObj )
{
    DocumentActions* pRet = NULL;
    Reference< lang::XUnoTunnel > xUT( xObj, uno::UNO_QUERY );
    if ( xUT.is() )
    {
        pRet = reinterpret_cast< DocumentActions* >( sal::static_int_cast< sal_IntPtr >( xUT->getSomething( getUnoTunnelId() ) ) );
    }
    return pRet;
}

} //  namespace chart
