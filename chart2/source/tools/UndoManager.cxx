/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:06:21 $
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
            (static_cast< util::XModifyListener*>(aIt.next()))->modified( aEvent );
    }
}

}

UndoManager::UndoManager() :
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

void UndoManager::impl_undoRedo(
    const Reference< frame::XModel > & xCurrentModel,
    impl::UndoStack * pStackToRemoveFrom,
    impl::UndoStack * pStackToAddTo )
{
    if( pStackToRemoveFrom && ! pStackToRemoveFrom->empty() )
    {
        // get model from undo/redo
        impl::UndoElement * pTop( pStackToRemoveFrom->top());
        if( pTop )
        {
            // put a clone of current model into redo/undo stack with the same
            // action string as the undo/redo
            pStackToAddTo->push( pTop->createFromModel( xCurrentModel ));
            // change current model by properties of the model from undo
            pTop->applyToModel( xCurrentModel );
            // remove the top undo element
            pStackToRemoveFrom->pop(), pTop = 0;
            ChartViewHelper::setViewToDirtyState( xCurrentModel );
            fireModifyEvent();
        }
    }
    else
    {
        OSL_ENSURE( false, "Can't Undo/Redo" );
    }
}

void UndoManager::undo( const Reference< frame::XModel > & xCurrentModel )
{
    OSL_ASSERT( m_apUndoStack.get() && m_apRedoStack.get());
    impl_undoRedo( xCurrentModel, m_apUndoStack.get(), m_apRedoStack.get());
}

void UndoManager::redo( const Reference< frame::XModel > & xCurrentModel )
{
    OSL_ASSERT( m_apUndoStack.get() && m_apRedoStack.get());
    impl_undoRedo( xCurrentModel, m_apRedoStack.get(), m_apUndoStack.get());
}

bool UndoManager::canUndo() const
{
    return ! m_apUndoStack->empty();
}

bool UndoManager::canRedo() const
{
    return ! m_apRedoStack->empty();
}

OUString UndoManager::getCurrentUndoString() const
{
    return m_apUndoStack->topUndoString();
}

OUString UndoManager::getCurrentRedoString() const
{
    return m_apRedoStack->topUndoString();
}

Sequence< OUString > UndoManager::getUndoStrings() const
{
    return m_apUndoStack->getUndoStrings();
}

Sequence< OUString > UndoManager::getRedoStrings() const
{
    return m_apRedoStack->getUndoStrings();
}

void UndoManager::preAction( const Reference< frame::XModel > & xCurrentModel )
{
    OSL_ENSURE( ! m_pLastRemeberedUndoElement, "Looks like postAction or cancelAction call was missing" );
    m_pLastRemeberedUndoElement = new impl::UndoElement( xCurrentModel );
}

void UndoManager::preActionWithData( const Reference< frame::XModel > & xCurrentModel )
{
    OSL_ENSURE( ! m_pLastRemeberedUndoElement, "Looks like postAction or cancelAction call was missing" );
    m_pLastRemeberedUndoElement = new impl::UndoElementWithData( xCurrentModel );
}

void UndoManager::postAction( const OUString & rUndoString )
{
    OSL_ENSURE( m_pLastRemeberedUndoElement, "Looks like preAction call was missing" );
    if( m_pLastRemeberedUndoElement )
    {
        m_pLastRemeberedUndoElement->setActionString( rUndoString );
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

void UndoManager::cancelAction()
{
    delete m_pLastRemeberedUndoElement;
    m_pLastRemeberedUndoElement = 0;
}

void UndoManager::cancelActionUndo( const Reference< frame::XModel > & xModelToRestore )
{
    if( m_pLastRemeberedUndoElement )
    {
        m_pLastRemeberedUndoElement->applyToModel( xModelToRestore );
        cancelAction();
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
    retrieveConfigUndoSteps();
}

sal_Int32 UndoManager::getMaximumNumberOfUndoSteps()
{
    return m_nMaxNumberOfUndos;
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

void UndoManager::addModifyListener(
    const Reference< util::XModifyListener >& xListener )
{
    if( ! m_xModifyBroadcaster.is())
    {
        m_pModifyBroadcaster = new impl::ModifyBroadcaster();
        m_xModifyBroadcaster.set( static_cast< cppu::OWeakObject* >( m_pModifyBroadcaster ), uno::UNO_QUERY );
    }
    m_xModifyBroadcaster->addModifyListener( xListener );
}

void UndoManager::removeModifyListener(
    const Reference< util::XModifyListener >& xListener )
{
    if( ! m_xModifyBroadcaster.is())
    {
        m_pModifyBroadcaster = new impl::ModifyBroadcaster();
        m_xModifyBroadcaster.set( static_cast< cppu::OWeakObject* >( m_pModifyBroadcaster ), uno::UNO_QUERY );
    }
    m_xModifyBroadcaster->removeModifyListener( xListener );
}


} //  namespace chart
