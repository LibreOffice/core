/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "UndoCommandDispatch.hxx"

#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/document/UndoFailedException.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>

#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

UndoCommandDispatch::UndoCommandDispatch(
    const Reference< uno::XComponentContext > & xContext,
    const Reference< frame::XModel > & xModel ) :
        CommandDispatch( xContext ),
        m_xModel( xModel )
{
    uno::Reference< document::XUndoManagerSupplier > xSuppUndo( m_xModel, uno::UNO_QUERY_THROW );
    m_xUndoManager.set( xSuppUndo->getUndoManager(), uno::UNO_SET_THROW );
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
    if( !m_xUndoManager.is() )
        return;

    const bool bFireAll = rURL.isEmpty();
    uno::Any aUndoState, aRedoState, aUndoStrings, aRedoStrings;
    if( m_xUndoManager->isUndoPossible())
        aUndoState <<= SvtResId( STR_UNDO ) + m_xUndoManager->getCurrentUndoActionTitle();
    if( m_xUndoManager->isRedoPossible())
        aRedoState <<= SvtResId( STR_REDO ) + m_xUndoManager->getCurrentRedoActionTitle();

    aUndoStrings <<= m_xUndoManager->getAllUndoActionTitles();
    aRedoStrings <<= m_xUndoManager->getAllRedoActionTitles();

    if( bFireAll || rURL == ".uno:Undo" )
        fireStatusEventForURL( ".uno:Undo", aUndoState, m_xUndoManager->isUndoPossible(), xSingleListener );
    if( bFireAll || rURL == ".uno:Redo" )
        fireStatusEventForURL( ".uno:Redo", aRedoState, m_xUndoManager->isRedoPossible(), xSingleListener );
    if( bFireAll || rURL == ".uno:GetUndoStrings" )
        fireStatusEventForURL( ".uno:GetUndoStrings", aUndoStrings, true, xSingleListener );
    if( bFireAll || rURL == ".uno:GetRedoStrings" )
        fireStatusEventForURL( ".uno:GetRedoStrings", aRedoStrings, true, xSingleListener );
}

// ____ XDispatch ____
void SAL_CALL UndoCommandDispatch::dispatch(
    const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
{
    if( !m_xUndoManager.is() )
        return;

    // why is it necessary to lock the solar mutex here?
    SolarMutexGuard aSolarGuard;
    try
    {
        sal_Int16 nCount( 1 );
        if ( Arguments.hasElements() && Arguments[0].Name == URL.Path )
            Arguments[0].Value >>= nCount;

        while ( nCount-- )
        {
            if ( URL.Path == "Undo" )
                m_xUndoManager->undo();
            else
                m_xUndoManager->redo();
        }
    }
    catch( const document::UndoFailedException& )
    {
        // silently ignore
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    // \--
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
{
    m_xUndoManager.clear();
    m_xModel.clear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
