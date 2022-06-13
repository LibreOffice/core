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

#include "UndoActions.hxx"
#include "ChartModelClone.hxx"
#include <ChartModel.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

#include <svx/svdundo.hxx>

#include <memory>
#include <utility>

using namespace ::com::sun::star;

namespace chart::impl
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::lang::DisposedException;

UndoElement::UndoElement( OUString i_actionString, rtl::Reference<::chart::ChartModel> i_documentModel, std::shared_ptr< ChartModelClone > i_modelClone )
    :m_sActionString(std::move( i_actionString ))
    ,m_xDocumentModel(std::move( i_documentModel ))
    ,m_pModelClone(std::move( i_modelClone ))
{
}

UndoElement::~UndoElement()
{
}

void UndoElement::disposing(std::unique_lock<std::mutex>&)
{
    if ( m_pModelClone )
        m_pModelClone->dispose();
    m_pModelClone.reset();
    m_xDocumentModel.clear();
}

OUString SAL_CALL UndoElement::getTitle()
{
    return m_sActionString;
}

void UndoElement::impl_toggleModelState()
{
    // get a snapshot of the current state of our model
    auto pNewClone = std::make_shared<ChartModelClone>( m_xDocumentModel, m_pModelClone->getFacet() );
    // apply the previous snapshot to our model
    m_pModelClone->applyToModel( m_xDocumentModel );
    // remember the new snapshot, for the next toggle
    m_pModelClone = pNewClone;
}

void SAL_CALL UndoElement::undo(  )
{
    impl_toggleModelState();
}

void SAL_CALL UndoElement::redo(  )
{
    impl_toggleModelState();
}

// = ShapeUndoElement

ShapeUndoElement::ShapeUndoElement( std::unique_ptr<SdrUndoAction> xSdrUndoAction )
    :m_xAction( std::move(xSdrUndoAction) )
{
}

ShapeUndoElement::~ShapeUndoElement()
{
}

OUString SAL_CALL ShapeUndoElement::getTitle()
{
    if ( !m_xAction )
        throw DisposedException( OUString(), *this );
    return m_xAction->GetComment();
}

void SAL_CALL ShapeUndoElement::undo(  )
{
    if ( !m_xAction )
        throw DisposedException( OUString(), *this );
    m_xAction->Undo();
}

void SAL_CALL ShapeUndoElement::redo(  )
{
    if ( !m_xAction )
        throw DisposedException( OUString(), *this );
    m_xAction->Redo();
}

} //  namespace chart::impl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
