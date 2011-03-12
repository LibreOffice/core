/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "UndoManager.hxx"
#include "ImplUndoManager.hxx"
#include "DisposeHelper.hxx"
#include "MutexContainer.hxx"
#include "macros.hxx"
#include "ChartViewHelper.hxx"

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

#include <unotools/configitem.hxx>
#include <cppuhelper/compbase1.hxx>
#include <rtl/uuid.h>
#include <svx/svdundo.hxx>

#include <functional>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
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

protected:
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

UndoManager::UndoManager() :
        impl::UndoManager_Base( m_aMutex ),
    m_apUndoStack( new impl::UndoStack()),
    m_apRedoStack( new impl::UndoStack()),
    m_pLastRemeberedUndoElement( 0 ),
    m_nMaxNumberOfUndos( 100 ),
    m_pModifyBroadcaster( 0 )
{}

UndoManager::~UndoManager()
{
    DisposeHelper::Dispose( m_xModifyBroadcaster );
    m_apUndoStack->disposeAndClear();
    m_apRedoStack->disposeAndClear();

    delete m_pLastRemeberedUndoElement;
    m_pLastRemeberedUndoElement = 0;
}

void UndoManager::addShapeUndoAction( SdrUndoAction* pAction )
{
    if ( !pAction )
    {
        return;
    }

    impl::ShapeUndoElement* pShapeUndoElement = new impl::ShapeUndoElement( pAction->GetComment(), pAction );
    if ( pShapeUndoElement )
    {
        m_apUndoStack->push( pShapeUndoElement );
        m_apRedoStack->disposeAndClear();
        if ( !m_apUndoStepsConfigItem.get() )
        {
            retrieveConfigUndoSteps();
        }
        fireModifyEvent();
    }
}

void UndoManager::impl_undoRedo(
    Reference< frame::XModel > & xCurrentModel,
    impl::UndoStack * pStackToRemoveFrom,
    impl::UndoStack * pStackToAddTo,
    bool bUndo )
{
    if( pStackToRemoveFrom && ! pStackToRemoveFrom->empty() )
    {
        // get model from undo/redo
        impl::UndoElement * pTop( pStackToRemoveFrom->top());
        if( pTop )
        {
            impl::ShapeUndoElement* pShapeUndoElement = dynamic_cast< impl::ShapeUndoElement* >( pTop );
            if ( pShapeUndoElement )
            {
                impl::ShapeUndoElement* pNewShapeUndoElement = new impl::ShapeUndoElement( *pShapeUndoElement );
                pStackToAddTo->push( pNewShapeUndoElement );
                SdrUndoAction* pAction = pNewShapeUndoElement->getSdrUndoAction();
                if ( pAction )
                {
                    if ( bUndo )
                    {
                        pAction->Undo();
                    }
                    else
                    {
                        pAction->Redo();
                    }
                }
            }
            else
            {
                // put a clone of current model into redo/undo stack with the same
                // action string as the undo/redo
                pStackToAddTo->push( pTop->createFromModel( xCurrentModel ));
                // change current model by properties of the model from undo
                pTop->applyToModel( xCurrentModel );
            }
            // remove the top undo element
            pStackToRemoveFrom->pop(), pTop = 0;
            ChartViewHelper::setViewToDirtyState( xCurrentModel );
            fireModifyEvent();
        }
    }
    else
    {
        OSL_FAIL( "Can't Undo/Redo" );
    }
}

void UndoManager::fireModifyEvent()
{
    if( m_xModifyBroadcaster.is())
        m_pModifyBroadcaster->fireEvent();
}


// ____ ConfigItemListener ____
void UndoManager::notify( const ::rtl::OUString & rPropertyName )
{
    OSL_ENSURE( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Steps" )),
                "Unwanted config property change Notified" );
    if( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Steps" )))
        retrieveConfigUndoSteps();
}

void UndoManager::retrieveConfigUndoSteps()
{
    if( ! m_apUndoStepsConfigItem.get())
        m_apUndoStepsConfigItem.reset( new impl::UndoStepsConfigItem( *this ));
    m_nMaxNumberOfUndos = m_apUndoStepsConfigItem->getUndoSteps();
    m_apUndoStack->limitSize( m_nMaxNumberOfUndos );
    m_apRedoStack->limitSize( m_nMaxNumberOfUndos );

    // a list of available undo steps could shrink here
    fireModifyEvent();
}

// ____ XModifyBroadcaster ____
void SAL_CALL UndoManager::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    if( ! m_xModifyBroadcaster.is())
    {
        m_pModifyBroadcaster = new impl::ModifyBroadcaster();
        m_xModifyBroadcaster.set( static_cast< cppu::OWeakObject* >( m_pModifyBroadcaster ), uno::UNO_QUERY );
    }
    m_xModifyBroadcaster->addModifyListener( aListener );
}

void SAL_CALL UndoManager::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    if( ! m_xModifyBroadcaster.is())
    {
        m_pModifyBroadcaster = new impl::ModifyBroadcaster();
        m_xModifyBroadcaster.set( static_cast< cppu::OWeakObject* >( m_pModifyBroadcaster ), uno::UNO_QUERY );
    }
    m_xModifyBroadcaster->removeModifyListener( aListener );
}

// ____ chart2::XUndoManager ____
void SAL_CALL UndoManager::preAction( const Reference< frame::XModel >& xModelBeforeChange )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( ! m_pLastRemeberedUndoElement, "Looks like postAction or cancelAction call was missing" );
    m_pLastRemeberedUndoElement = new impl::UndoElement( xModelBeforeChange );
}

void SAL_CALL UndoManager::preActionWithArguments(
    const Reference< frame::XModel >& xModelBeforeChange,
    const Sequence< beans::PropertyValue >& aArguments )
    throw (uno::RuntimeException)
{
    bool bActionHandled( false );
    OSL_ENSURE( ! m_pLastRemeberedUndoElement, "Looks like postAction or cancelAction call was missing" );
    if( aArguments.getLength() > 0 )
    {
        OSL_ENSURE( aArguments.getLength() == 1, "More than one argument is not supported yet" );
        if( aArguments[0].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("WithData")))
        {
            m_pLastRemeberedUndoElement = new impl::UndoElementWithData( xModelBeforeChange );
            bActionHandled = true;
        }
        else if( aArguments[0].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("WithSelection")))
        {
            m_pLastRemeberedUndoElement = new impl::UndoElementWithSelection( xModelBeforeChange );
            bActionHandled = true;
        }
    }

    if( !bActionHandled )
        preAction( xModelBeforeChange );
}

void SAL_CALL UndoManager::postAction( const OUString& aUndoText )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( m_pLastRemeberedUndoElement, "Looks like preAction call was missing" );
    if( m_pLastRemeberedUndoElement )
    {
        m_pLastRemeberedUndoElement->setActionString( aUndoText );
        m_apUndoStack->push( m_pLastRemeberedUndoElement );
        m_pLastRemeberedUndoElement = 0;

        // redo no longer possible
        m_apRedoStack->disposeAndClear();

        // it suffices to get the number of undo steps from config after the
        // first time postAction has been called
        if( ! m_apUndoStepsConfigItem.get())
            retrieveConfigUndoSteps();

        fireModifyEvent();
    }
}

void SAL_CALL UndoManager::cancelAction()
    throw (uno::RuntimeException)
{
    delete m_pLastRemeberedUndoElement;
    m_pLastRemeberedUndoElement = 0;
}

void SAL_CALL UndoManager::cancelActionWithUndo( Reference< frame::XModel >& xModelToRestore )
    throw (uno::RuntimeException)
{
    if( m_pLastRemeberedUndoElement )
    {
        m_pLastRemeberedUndoElement->applyToModel( xModelToRestore );
        cancelAction();
    }
}

void SAL_CALL UndoManager::undo( Reference< frame::XModel >& xCurrentModel )
    throw (uno::RuntimeException)
{
    OSL_ASSERT( m_apUndoStack.get() && m_apRedoStack.get());
    impl_undoRedo( xCurrentModel, m_apUndoStack.get(), m_apRedoStack.get(), true );
}

void SAL_CALL UndoManager::redo( Reference< frame::XModel >& xCurrentModel )
    throw (uno::RuntimeException)
{
    OSL_ASSERT( m_apUndoStack.get() && m_apRedoStack.get());
    impl_undoRedo( xCurrentModel, m_apRedoStack.get(), m_apUndoStack.get(), false );
}

::sal_Bool SAL_CALL UndoManager::undoPossible()
    throw (uno::RuntimeException)
{
    return ! m_apUndoStack->empty();
}

::sal_Bool SAL_CALL UndoManager::redoPossible()
    throw (uno::RuntimeException)
{
    return ! m_apRedoStack->empty();
}

OUString SAL_CALL UndoManager::getCurrentUndoString()
    throw (uno::RuntimeException)
{
    return m_apUndoStack->topUndoString();
}

OUString SAL_CALL UndoManager::getCurrentRedoString()
    throw (uno::RuntimeException)
{
    return m_apRedoStack->topUndoString();
}

Sequence< OUString > SAL_CALL UndoManager::getAllUndoStrings()
    throw (uno::RuntimeException)
{
    return m_apUndoStack->getUndoStrings();
}

Sequence< OUString > SAL_CALL UndoManager::getAllRedoStrings()
    throw (uno::RuntimeException)
{
    return m_apRedoStack->getUndoStrings();
}

// ____ XUndoHelper ____
Reference< frame::XModel > SAL_CALL UndoManager::getModelCloneForUndo(
    const Reference< frame::XModel >& xModelBeforeChange )
    throw (uno::RuntimeException)
{
    return impl::UndoElement::cloneModel( xModelBeforeChange );
}

void SAL_CALL UndoManager::applyModelContent(
    Reference< frame::XModel >& xModelToChange,
    const Reference< frame::XModel >& xModelToCopyFrom )
    throw (uno::RuntimeException)
{
    impl::UndoElement::applyModelContentToModel( xModelToChange, xModelToCopyFrom );
}

// ____ XUnoTunnel ____
sal_Int64 UndoManager::getSomething( const Sequence< sal_Int8 >& rId )
    throw (uno::RuntimeException)
{
    if ( rId.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ) );
    }
    return 0;
}

const Sequence< sal_Int8 >& UndoManager::getUnoTunnelId()
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

UndoManager* UndoManager::getImplementation( const Reference< uno::XInterface > xObj )
{
    UndoManager* pRet = NULL;
    Reference< lang::XUnoTunnel > xUT( xObj, uno::UNO_QUERY );
    if ( xUT.is() )
    {
        pRet = reinterpret_cast< UndoManager* >( sal::static_int_cast< sal_IntPtr >( xUT->getSomething( getUnoTunnelId() ) ) );
    }
    return pRet;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
