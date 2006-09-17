/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmmodel.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:07:17 $
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
#include "precompiled_svx.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef SVX_LIGHT
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#else
class SfxObjectShell;
#endif

#ifndef _FM_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _FM_PAGE_HXX
#include "fmpage.hxx"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif
#ifndef _SVX_SVDOBJ_HXX
#include "svdobj.hxx"
#endif

using ::com::sun::star::uno::Reference;
using ::com::sun::star::container::XNameContainer;

TYPEINIT1(FmFormModel, SdrModel);

struct FmFormModelImplData
{
    FmXUndoEnvironment*     pUndoEnv;
    sal_Bool                bOpenInDesignIsDefaulted;
    sal_Bool                bMovingPage;

    FmFormModelImplData()
        :pUndoEnv( NULL )
        ,bOpenInDesignIsDefaulted( sal_True )
        ,bMovingPage( sal_False )
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
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
#endif
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
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
#endif
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(SfxItemPool* pPool, SfxObjectShell* pPers,
                         FASTBOOL bUseExtColorTable
                         )
            :SdrModel(pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
            ,m_pImpl(NULL)
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
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SfxObjectShell* pPers,
                         FASTBOOL bUseExtColorTable)
            :SdrModel(rPath, pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
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
SdrPage* FmFormModel::AllocPage(FASTBOOL bMasterPage)
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
void FmFormModel::MovePage( USHORT nPgNum, USHORT nNewPos )
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
SdrPage* FmFormModel::RemovePage(sal_uInt16 nPgNum)
{
    FmFormPage* pPage = (FmFormPage*)SdrModel::RemovePage(nPgNum);

#ifndef SVX_LIGHT
    if (pPage)
    {
        Reference< XNameContainer > xForms( pPage->GetForms( false ) );
        if ( xForms.is() )
            m_pImpl->pUndoEnv->RemoveForms( xForms );
    }
#endif

    return pPage;
}

/*************************************************************************
|*
|* InsertMasterPage
|*
\************************************************************************/
void FmFormModel::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos)
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
SdrPage* FmFormModel::RemoveMasterPage(sal_uInt16 nPgNum)
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
