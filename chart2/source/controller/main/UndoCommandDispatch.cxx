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

#include "UndoCommandDispatch.hxx"
#include "ResId.hxx"
#include "macros.hxx"

#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

// for ressource strings STR_UNDO and STR_REDO
#include <svtools/svtools.hrc>
#include <svtools/svtdata.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

UndoCommandDispatch::UndoCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< frame::XModel > & xModel ) :
        CommandDispatch( xContext ),
        m_xModel( xModel )
{
    uno::Reference< document::XUndoManagerSupplier > xSuppUndo( m_xModel, uno::UNO_QUERY_THROW );
    m_xUndoManager.set( xSuppUndo->getUndoManager(), uno::UNO_QUERY_THROW );
}

UndoCommandDispatch::~UndoCommandDispatch()
{}

void UndoCommandDispatch::initialize()
{
    Reference< util::XModifyBroadcaster > xBroadcaster( m_xUndoManager, uno::UNO_QUERY );
    ENSURE_OR_RETURN_VOID( xBroadcaster.is(), "UndoCommandDispatch::initialize: missing modification broadcaster interface!" );
    xBroadcaster->addModifyListener( this );
}

void UndoCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    if( m_xUndoManager.is() )
    {
        const bool bFireAll = rURL.isEmpty();
        uno::Any aUndoState, aRedoState;
        if( m_xUndoManager->isUndoPossible())
        {
            // using assignment for broken gcc 3.3
            OUString aUndo = OUString( String( SvtResId( STR_UNDO )));
            aUndoState <<= ( aUndo + m_xUndoManager->getCurrentUndoActionTitle());
        }
        if( m_xUndoManager->isRedoPossible())
        {
            // using assignment for broken gcc 3.3
            OUString aRedo = OUString( String( SvtResId( STR_REDO )));
            aRedoState <<= ( aRedo + m_xUndoManager->getCurrentRedoActionTitle());
        }

        if( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Undo")))
            fireStatusEventForURL( C2U(".uno:Undo"), aUndoState, m_xUndoManager->isUndoPossible(), xSingleListener );
        if( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Redo")))
            fireStatusEventForURL( C2U(".uno:Redo"), aRedoState, m_xUndoManager->isRedoPossible(), xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL UndoCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& /* Arguments */ )
    throw (uno::RuntimeException)
{
    if( m_xUndoManager.is() )
    {
        // why is it necessary to lock the solar mutex here?
        // /--
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        try
        {
            if( URL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Undo" )))
                m_xUndoManager->undo();
            else
                m_xUndoManager->redo();
        }
        catch( const document::UndoFailedException& )
        {
            // silently ignore
        }
        catch( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        // \--
    }
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL UndoCommandDispatch::disposing()
{
    Reference< util::XModifyBroadcaster > xBroadcaster( m_xUndoManager, uno::UNO_QUERY );
    OSL_ENSURE( xBroadcaster.is(), "UndoCommandDispatch::initialize: missing modification broadcaster interface!" );
    if( xBroadcaster.is() )
    {
        xBroadcaster->removeModifyListener( this );
    }

    m_xUndoManager.clear();
    m_xModel.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL UndoCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    m_xUndoManager.clear();
    m_xModel.clear();
}

} //  namespace chart
