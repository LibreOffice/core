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
#include "precompiled_svx.hxx"

#include "fmundo.hxx"
#include "fmdocumentclassification.hxx"
#include "fmcontrollayout.hxx"

#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/svdobj.hxx>
#include <tools/debug.hxx>

#ifndef SVX_LIGHT
#include <sfx2/objsh.hxx>
#else
class SfxObjectShell;
#endif

#include <boost/optional.hpp>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XNameContainer;
using namespace svxform;

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
FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SfxObjectShell* pPers, bool bUseExtColorTable)
:   SdrModel(rPath, pPool, pPers, bUseExtColorTable)
            ,m_pImpl( NULL )
            ,m_pObjShell(0)
            ,m_bOpenInDesignMode(sal_False)
            ,m_bAutoControlFocus(sal_False)
{
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
#endif
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
FmFormModel::~FmFormModel()
{
#ifndef SVX_LIGHT
    if (m_pObjShell && m_pImpl->pUndoEnv->IsListening(*m_pObjShell))
        SetObjectShell(NULL);

    ClearUndoBuffer();
    // minimale grenze fuer undos
    SetMaxUndoActionCount(1);

    m_pImpl->pUndoEnv->release();
    delete m_pImpl;

#endif
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
void FmFormModel::InsertPage(SdrPage* pPage, sal_uInt32 nPos)
{
#ifndef SVX_LIGHT
    // hack solange Methode intern
    if (m_pObjShell && !m_pImpl->pUndoEnv->IsListening( *m_pObjShell ))
        SetObjectShell(m_pObjShell);
#endif

    SdrModel::InsertPage( pPage, nPos );
}

/*************************************************************************
|*
|* MovePage
|*
\************************************************************************/
void FmFormModel::MovePage( sal_uInt32 nPgNum, sal_uInt32 nNewPos )
{
#ifndef SVX_LIGHT
    m_pImpl->bMovingPage = sal_True;
        // see InsertPage for this
#endif

    SdrModel::MovePage( nPgNum, nNewPos );

#ifndef SVX_LIGHT
    m_pImpl->bMovingPage = sal_False;
#endif
}

/*************************************************************************
|*
|* RemovePage
|*
\************************************************************************/
SdrPage* FmFormModel::RemovePage(sal_uInt32 nPgNum)
{
    FmFormPage* pToBeRemovedPage = dynamic_cast< FmFormPage* >( GetPage( nPgNum ) );
    OSL_ENSURE( pToBeRemovedPage, "FmFormModel::RemovePage: *which page*?" );

#ifndef SVX_LIGHT
    if ( pToBeRemovedPage )
    {
        Reference< XNameContainer > xForms( pToBeRemovedPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->pUndoEnv->RemoveForms( xForms );
    }
#endif

    FmFormPage* pRemovedPage = (FmFormPage*)SdrModel::RemovePage(nPgNum);
    OSL_ENSURE( pRemovedPage == pToBeRemovedPage, "FmFormModel::RemovePage: inconsistency!" );
    return pRemovedPage;
}

/*************************************************************************
|*
|* InsertMasterPage
|*
\************************************************************************/
void FmFormModel::InsertMasterPage(SdrPage* pPage, sal_uInt32 nPos)
{
#ifndef SVX_LIGHT
    // hack solange Methode intern
    if (m_pObjShell && !m_pImpl->pUndoEnv->IsListening( *m_pObjShell ))
        SetObjectShell(m_pObjShell);
#endif

    SdrModel::InsertMasterPage(pPage, nPos);
}

/*************************************************************************
|*
|* RemoveMasterPage
|*
\************************************************************************/
SdrPage* FmFormModel::RemoveMasterPage(sal_uInt32 nPgNum)
{
    FmFormPage* pPage = (FmFormPage*)SdrModel::RemoveMasterPage(nPgNum);

#ifndef SVX_LIGHT
    if ( pPage )
    {
        Reference< XNameContainer > xForms( pPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->pUndoEnv->RemoveForms( xForms );
    }
#endif

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
#ifndef SVX_LIGHT
    implSetOpenInDesignMode( bOpenDesignMode, sal_False );
#endif
}

#ifndef SVX_LIGHT
//------------------------------------------------------------------------
sal_Bool FmFormModel::OpenInDesignModeIsDefaulted( )
{
    return m_pImpl->bOpenInDesignIsDefaulted;
}
#endif

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
#ifndef SVX_LIGHT
    if( _bAutoControlFocus != m_bAutoControlFocus )
    {
        m_bAutoControlFocus = _bAutoControlFocus;
        m_pObjShell->SetModified( sal_True );
    }
#endif
}

//------------------------------------------------------------------------
void FmFormModel::SetObjectShell( SfxObjectShell* pShell )
{
#ifndef SVX_LIGHT
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
#endif
}

//------------------------------------------------------------------------
FmXUndoEnvironment& FmFormModel::GetUndoEnv()
{
    return *m_pImpl->pUndoEnv;
}
