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

#include "UndoCommandDispatch.hxx"
#include "macros.hxx"

#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

// for ressource strings STR_UNDO and STR_REDO
#include <sfx2/sfx.hrc>

#include "ResId.hxx"

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
    m_xDocumentActions.set( xModel, uno::UNO_QUERY );
}

UndoCommandDispatch::~UndoCommandDispatch()
{}

void UndoCommandDispatch::initialize()
{
    Reference< util::XModifyBroadcaster > xBroadcaster( m_xDocumentActions, uno::UNO_QUERY );
    if( xBroadcaster.is() )
    {
        xBroadcaster->addModifyListener( this );
    }
}

void UndoCommandDispatch::fireStatusEvent(
    const OUString & rURL,
    const Reference< frame::XStatusListener > & xSingleListener /* = 0 */ )
{
    if( m_xDocumentActions.is() )
    {
        bool bFireAll = (rURL.getLength() == 0);
        uno::Any aUndoState, aRedoState;
        if( m_xDocumentActions->undoPossible())
        {
            // using assignment for broken gcc 3.3
            OUString aUndo = OUString( String( SchResId( STR_UNDO )));
            aUndoState <<= ( aUndo + m_xDocumentActions->getCurrentUndoString());
        }
        if( m_xDocumentActions->redoPossible())
        {
            // using assignment for broken gcc 3.3
            OUString aRedo = OUString( String( SchResId( STR_REDO )));
            aRedoState <<= ( aRedo + m_xDocumentActions->getCurrentRedoString());
        }

        if( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Undo")))
            fireStatusEventForURL( C2U(".uno:Undo"), aUndoState, m_xDocumentActions->undoPossible(), xSingleListener );
        if( bFireAll || rURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(".uno:Redo")))
            fireStatusEventForURL( C2U(".uno:Redo"), aRedoState, m_xDocumentActions->redoPossible(), xSingleListener );
    }
}

// ____ XDispatch ____
void SAL_CALL UndoCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& /* Arguments */ )
    throw (uno::RuntimeException)
{
    if( m_xDocumentActions.is() )
    {
        // why is it necessary to lock the solar mutex here?
        // /--
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        if( URL.Path.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Undo" )))
            m_xDocumentActions->undo();
        else
            m_xDocumentActions->redo();
        // \--
    }
}

// ____ WeakComponentImplHelperBase ____
/// is called when this is disposed
void SAL_CALL UndoCommandDispatch::disposing()
{
    Reference< util::XModifyBroadcaster > xBroadcaster( m_xDocumentActions, uno::UNO_QUERY );
    if( xBroadcaster.is() )
    {
        xBroadcaster->removeModifyListener( this );
    }

    m_xDocumentActions.clear();
    m_xModel.clear();
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL UndoCommandDispatch::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    m_xDocumentActions.clear();
    m_xModel.clear();
}

} //  namespace chart
