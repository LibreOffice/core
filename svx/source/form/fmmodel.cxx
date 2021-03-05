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


#include <fmundo.hxx>
#include <fmdocumentclassification.hxx>
#include <fmcontrollayout.hxx>

#include <com/sun/star/form/XForms.hpp>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>

#include <sfx2/objsh.hxx>

#include <optional>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XNameContainer;
using namespace svxform;


struct FmFormModelImplData
{
    rtl::Reference<FmXUndoEnvironment>  mxUndoEnv;
    bool                bOpenInDesignIsDefaulted;
    std::optional<bool> aControlsUseRefDevice;

    FmFormModelImplData()
        :bOpenInDesignIsDefaulted( true )
    {
    }
};

FmFormModel::FmFormModel(
    SfxItemPool* pPool,
    SfxObjectShell* pPers)
:   SdrModel(
        pPool,
        pPers)
    , m_pObjShell(nullptr)
    , m_bOpenInDesignMode(false)
    , m_bAutoControlFocus(false)
{
    m_pImpl.reset( new FmFormModelImplData );
    m_pImpl->mxUndoEnv = new FmXUndoEnvironment(*this);
}

FmFormModel::~FmFormModel()
{
    if (m_pObjShell && m_pImpl->mxUndoEnv->IsListening(*m_pObjShell))
        SetObjectShell(nullptr);

    ClearUndoBuffer();
    // minimum limit for undos
    SetMaxUndoActionCount(1);
}

rtl::Reference<SdrPage> FmFormModel::AllocPage(bool bMasterPage)
{
    return new FmFormPage(*this, bMasterPage);
}

void FmFormModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    // hack for as long as the method is internal
    if (m_pObjShell && !m_pImpl->mxUndoEnv->IsListening( *m_pObjShell ))
        SetObjectShell(m_pObjShell);

    SdrModel::InsertPage( pPage, nPos );
}

rtl::Reference<SdrPage> FmFormModel::RemovePage(sal_uInt16 nPgNum)
{
    FmFormPage* pToBeRemovedPage = dynamic_cast< FmFormPage* >( GetPage( nPgNum ) );
    OSL_ENSURE( pToBeRemovedPage, "FmFormModel::RemovePage: *which page*?" );

    if ( pToBeRemovedPage )
    {
        Reference< XNameContainer > xForms( pToBeRemovedPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->mxUndoEnv->RemoveForms( xForms );
    }

    rtl::Reference<FmFormPage> pRemovedPage = static_cast<FmFormPage*>(SdrModel::RemovePage(nPgNum).get());
    OSL_ENSURE( pRemovedPage == pToBeRemovedPage, "FmFormModel::RemovePage: inconsistency!" );
    return pRemovedPage;
}

void FmFormModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
{
    // hack for as long as the method is internal
    if (m_pObjShell && !m_pImpl->mxUndoEnv->IsListening( *m_pObjShell ))
        SetObjectShell(m_pObjShell);

    SdrModel::InsertMasterPage(pPage, nPos);
}

rtl::Reference<SdrPage> FmFormModel::RemoveMasterPage(sal_uInt16 nPgNum)
{
    rtl::Reference<FmFormPage> pPage = static_cast<FmFormPage*>(SdrModel::RemoveMasterPage(nPgNum).get());

    if ( pPage )
    {
        Reference< XNameContainer > xForms( pPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->mxUndoEnv->RemoveForms( xForms );
    }

    return pPage;
}


void FmFormModel::implSetOpenInDesignMode( bool _bOpenDesignMode )
{
    if( _bOpenDesignMode != m_bOpenInDesignMode )
    {
        m_bOpenInDesignMode = _bOpenDesignMode;

        if ( m_pObjShell )
            m_pObjShell->SetModified();
    }
    // no matter if we really did it or not - from now on, it does not count as defaulted anymore
    m_pImpl->bOpenInDesignIsDefaulted = false;
}


void FmFormModel::SetOpenInDesignMode( bool bOpenDesignMode )
{
    implSetOpenInDesignMode( bOpenDesignMode );
}


bool FmFormModel::OpenInDesignModeIsDefaulted( )
{
    return m_pImpl->bOpenInDesignIsDefaulted;
}


bool FmFormModel::ControlsUseRefDevice() const
{
    if ( !m_pImpl->aControlsUseRefDevice )
    {
        DocumentType eDocType = eUnknownDocumentType;
        if ( m_pObjShell )
            eDocType = DocumentClassification::classifyHostDocument( m_pObjShell->GetModel() );
        m_pImpl->aControlsUseRefDevice = ControlLayouter::useDocumentReferenceDevice(eDocType);
    }
    return *m_pImpl->aControlsUseRefDevice;
}


void FmFormModel::SetAutoControlFocus( bool _bAutoControlFocus )
{
    if( _bAutoControlFocus != m_bAutoControlFocus )
    {
        m_bAutoControlFocus = _bAutoControlFocus;
        m_pObjShell->SetModified();
    }
}


void FmFormModel::SetObjectShell( SfxObjectShell* pShell )
{
    if (pShell == m_pObjShell)
        return;

    if (m_pObjShell)
    {
        m_pImpl->mxUndoEnv->EndListening( *this );
        m_pImpl->mxUndoEnv->EndListening( *m_pObjShell );
    }

    m_pObjShell = pShell;

    if (m_pObjShell)
    {
        m_pImpl->mxUndoEnv->SetReadOnly( m_pObjShell->IsReadOnly() || m_pObjShell->IsReadOnlyUI(), FmXUndoEnvironment::Accessor() );

        if (!m_pImpl->mxUndoEnv->IsReadOnly())
             m_pImpl->mxUndoEnv->StartListening(*this);

        m_pImpl->mxUndoEnv->StartListening( *m_pObjShell );
    }
}


FmXUndoEnvironment& FmFormModel::GetUndoEnv()
{
    return *m_pImpl->mxUndoEnv;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
