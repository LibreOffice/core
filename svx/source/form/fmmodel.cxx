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
#include "precompiled_svx.hxx"

#include "fmundo.hxx"
#include "fmdocumentclassification.hxx"
#include "fmcontrollayout.hxx"

#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/svdobj.hxx>
#include <tools/debug.hxx>

#include <sfx2/objsh.hxx>

#include <boost/optional.hpp>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XNameContainer;
using namespace svxform;

TYPEINIT1(FmFormModel, SdrModel);

struct FmFormModelImplData
{
    FmXUndoEnvironment*     pUndoEnv;
    sal_Bool                bOpenInDesignIsDefaulted;
    sal_Bool                bMovingPage;
    ::boost::optional< sal_Bool >
                            aControlsUseRefDevice;

    FmFormModelImplData()
        :pUndoEnv( NULL )
        ,bOpenInDesignIsDefaulted( sal_True )
        ,bMovingPage( sal_False )
        ,aControlsUseRefDevice()
    {
    }
};

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(SfxItemPool* pPool, SfxObjectShell* pPers)
            :SdrModel(pPool, pPers, LOADREFCOUNTS)
            ,m_pImpl(NULL)
            ,m_pObjShell(0)
            ,m_bOpenInDesignMode(sal_False)
            ,m_bAutoControlFocus(sal_False)
{
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SfxObjectShell* pPers)
            :SdrModel(rPath, pPool, pPers)
            ,m_pImpl(NULL)
            ,m_pObjShell(0)
            ,m_bOpenInDesignMode(sal_False)
            ,m_bAutoControlFocus(sal_False)
{
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(SfxItemPool* pPool, SfxObjectShell* pPers,
                         bool bUseExtColorTable
                         )
            :SdrModel(pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
            ,m_pImpl(NULL)
            ,m_pObjShell(0)
            ,m_bOpenInDesignMode(sal_False)
            ,m_bAutoControlFocus(sal_False)
{
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SfxObjectShell* pPers,
                         bool bUseExtColorTable)
            :SdrModel(rPath, pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
            ,m_pImpl( NULL )
            ,m_pObjShell(0)
            ,m_bOpenInDesignMode(sal_False)
            ,m_bAutoControlFocus(sal_False)
{
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
FmFormModel::~FmFormModel()
{
    if (m_pObjShell && m_pImpl->pUndoEnv->IsListening(*m_pObjShell))
        SetObjectShell(NULL);

    ClearUndoBuffer();
    // minimale grenze fuer undos
    SetMaxUndoActionCount(1);

    m_pImpl->pUndoEnv->release();
    delete m_pImpl;

}

/*************************************************************************
|*
|* Erzeugt eine neue Seite
|*
\************************************************************************/
SdrPage* FmFormModel::AllocPage(bool bMasterPage)
{
    return new FmFormPage(*this, NULL, bMasterPage);
}

/*************************************************************************
|*
|* InsertPage
|*
\************************************************************************/
void FmFormModel::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    // hack solange Methode intern
    if (m_pObjShell && !m_pImpl->pUndoEnv->IsListening( *m_pObjShell ))
        SetObjectShell(m_pObjShell);

    SdrModel::InsertPage( pPage, nPos );
}

/*************************************************************************
|*
|* MovePage
|*
\************************************************************************/
void FmFormModel::MovePage( sal_uInt16 nPgNum, sal_uInt16 nNewPos )
{
    m_pImpl->bMovingPage = sal_True;
        // see InsertPage for this

    SdrModel::MovePage( nPgNum, nNewPos );

    m_pImpl->bMovingPage = sal_False;
}

/*************************************************************************
|*
|* RemovePage
|*
\************************************************************************/
SdrPage* FmFormModel::RemovePage(sal_uInt16 nPgNum)
{
    FmFormPage* pToBeRemovedPage = dynamic_cast< FmFormPage* >( GetPage( nPgNum ) );
    OSL_ENSURE( pToBeRemovedPage, "FmFormModel::RemovePage: *which page*?" );

    if ( pToBeRemovedPage )
    {
        Reference< XNameContainer > xForms( pToBeRemovedPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->pUndoEnv->RemoveForms( xForms );
    }

    FmFormPage* pRemovedPage = (FmFormPage*)SdrModel::RemovePage(nPgNum);
    OSL_ENSURE( pRemovedPage == pToBeRemovedPage, "FmFormModel::RemovePage: inconsistency!" );
    return pRemovedPage;
}

/*************************************************************************
|*
|* InsertMasterPage
|*
\************************************************************************/
void FmFormModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
{
    // hack solange Methode intern
    if (m_pObjShell && !m_pImpl->pUndoEnv->IsListening( *m_pObjShell ))
        SetObjectShell(m_pObjShell);

    SdrModel::InsertMasterPage(pPage, nPos);
}

/*************************************************************************
|*
|* RemoveMasterPage
|*
\************************************************************************/
SdrPage* FmFormModel::RemoveMasterPage(sal_uInt16 nPgNum)
{
    FmFormPage* pPage = (FmFormPage*)SdrModel::RemoveMasterPage(nPgNum);

    if ( pPage )
    {
        Reference< XNameContainer > xForms( pPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->pUndoEnv->RemoveForms( xForms );
    }

    return pPage;
}

//------------------------------------------------------------------------
SdrLayerID FmFormModel::GetControlExportLayerId( const SdrObject& rObj ) const
{
    return rObj.GetLayer();
}

//------------------------------------------------------------------------
void FmFormModel::implSetOpenInDesignMode( sal_Bool _bOpenDesignMode, sal_Bool _bForce )
{
    if( ( _bOpenDesignMode != m_bOpenInDesignMode ) || _bForce )
    {
        m_bOpenInDesignMode = _bOpenDesignMode;

        if ( m_pObjShell )
            m_pObjShell->SetModified( sal_True );
    }
    // no matter if we really did it or not - from now on, it does not count as defaulted anymore
    m_pImpl->bOpenInDesignIsDefaulted = sal_False;
}

//------------------------------------------------------------------------
void FmFormModel::SetOpenInDesignMode( sal_Bool bOpenDesignMode )
{
    implSetOpenInDesignMode( bOpenDesignMode, sal_False );
}

//------------------------------------------------------------------------
sal_Bool FmFormModel::OpenInDesignModeIsDefaulted( )
{
    return m_pImpl->bOpenInDesignIsDefaulted;
}

//------------------------------------------------------------------------
sal_Bool FmFormModel::ControlsUseRefDevice() const
{
    if ( !m_pImpl->aControlsUseRefDevice )
    {
        DocumentType eDocType = eUnknownDocumentType;
        if ( m_pObjShell )
            eDocType = DocumentClassification::classifyHostDocument( m_pObjShell->GetModel() );
        m_pImpl->aControlsUseRefDevice.reset( ControlLayouter::useDocumentReferenceDevice( eDocType ) );
    }
    return *m_pImpl->aControlsUseRefDevice;
}

//------------------------------------------------------------------------
void FmFormModel::SetAutoControlFocus( sal_Bool _bAutoControlFocus )
{
    if( _bAutoControlFocus != m_bAutoControlFocus )
    {
        m_bAutoControlFocus = _bAutoControlFocus;
        m_pObjShell->SetModified( sal_True );
    }
}

//------------------------------------------------------------------------
void FmFormModel::SetObjectShell( SfxObjectShell* pShell )
{
    if (pShell == m_pObjShell)
        return;

    if (m_pObjShell)
    {
        m_pImpl->pUndoEnv->EndListening( *this );
        m_pImpl->pUndoEnv->EndListening( *m_pObjShell );
    }

    m_pObjShell = pShell;

    if (m_pObjShell)
    {
        m_pImpl->pUndoEnv->SetReadOnly( m_pObjShell->IsReadOnly() || m_pObjShell->IsReadOnlyUI(), FmXUndoEnvironment::Accessor() );

        if (!m_pImpl->pUndoEnv->IsReadOnly())
             m_pImpl->pUndoEnv->StartListening(*this);

        m_pImpl->pUndoEnv->StartListening( *m_pObjShell );
    }
}

//------------------------------------------------------------------------
FmXUndoEnvironment& FmFormModel::GetUndoEnv()
{
    return *m_pImpl->pUndoEnv;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
