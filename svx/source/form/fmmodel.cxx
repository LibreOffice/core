/*************************************************************************
 *
 *  $RCSfile: fmmodel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#pragma hdrstop

#ifndef _FM_FMMODEL_HXX
#include "fmmodel.hxx"
#endif

#ifndef _SVDIO_HXX
#include "svdio.hxx"
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

TYPEINIT1(FmFormModel, SdrModel);

struct FmFormModelImplData
{
    FmXUndoEnvironment*     pUndoEnv;
    XubString               sNextPageId;
};

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(SfxItemPool* pPool, SvPersist* pPers)
            :SdrModel(pPool, pPers, LOADREFCOUNTS)
            ,pObjShell(0)
            ,bStreamingOldVersion(sal_False)
            ,m_pImpl(NULL)
            ,bOpenInDesignMode(sal_True)
{
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
    m_pImpl->sNextPageId = '0';
#endif
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SvPersist* pPers)
            :SdrModel(rPath, pPool, pPers)
            ,pObjShell(0)
            ,bStreamingOldVersion(sal_False)
            ,m_pImpl(NULL)
            ,bOpenInDesignMode(sal_True)
{
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
    m_pImpl->sNextPageId = '0';
#endif
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(SfxItemPool* pPool, SvPersist* pPers,
                         FASTBOOL bUseExtColorTable
                         )
            :SdrModel(pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
            ,pObjShell(0)
            ,bStreamingOldVersion(sal_False)
            ,m_pImpl(NULL)
{
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
    m_pImpl->sNextPageId = '0';
#endif
}

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(const XubString& rPath, SfxItemPool* pPool, SvPersist* pPers,
                         FASTBOOL bUseExtColorTable)
            :SdrModel(rPath, pPool, pPers, bUseExtColorTable, LOADREFCOUNTS)
            ,pObjShell(0)
            ,bStreamingOldVersion(sal_False)
            ,bOpenInDesignMode(sal_True)
{
#ifndef SVX_LIGHT
    m_pImpl = new FmFormModelImplData;
    m_pImpl->pUndoEnv = new FmXUndoEnvironment(*this);
    m_pImpl->pUndoEnv->acquire();
    m_pImpl->sNextPageId = '0';
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
    if (pObjShell && m_pImpl->pUndoEnv->IsListening(*pObjShell))
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
|* Copy-Ctor
|*
\************************************************************************/
FmFormModel::FmFormModel(const FmFormModel&)
{
    DBG_ERROR("FmFormModel: CopyCtor not implemented");
}

/*************************************************************************
|*
|* Operator=
|*
\************************************************************************/
void FmFormModel::operator=(const FmFormModel&)
{
    DBG_ERROR("FmFormModel: operator= not implemented");
}

/*************************************************************************
|*
|* Operator==
|*
\************************************************************************/
FASTBOOL FmFormModel::operator==(const FmFormModel&) const
{
    DBG_ERROR("FmFormModel: operator== not implemented");
    return sal_False;
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
|* WriteData
|*
\************************************************************************/
void FmFormModel::WriteData(SvStream& rOut) const
{
#ifndef SVX_LIGHT

    if( rOut.GetVersion() < SOFFICE_FILEFORMAT_50 )
        ((FmFormModel*)this)->bStreamingOldVersion = sal_True;

    SdrModel::WriteData( rOut );

    //////////////////////////////////////////////////////////////////////
    // Speichern der Option OpenInDesignMode
    if (!bStreamingOldVersion)
    {
        SdrDownCompat aModelFormatCompat(rOut,STREAM_WRITE);
        rOut << bOpenInDesignMode;
    }

    ((FmFormModel*)this)->bStreamingOldVersion = sal_False;

#endif
}


/*************************************************************************
|*
|* ReadData
|*
\************************************************************************/
void FmFormModel::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    if( rIn.GetVersion() < SOFFICE_FILEFORMAT_50 )
        ((FmFormModel*)this)->bStreamingOldVersion = sal_True;

    SdrModel::ReadData( rHead, rIn );

    //////////////////////////////////////////////////////////////////////
    // Lesen der Option OpenInDesignMode
    if (!bStreamingOldVersion)
    {
        SdrDownCompat aCompat(rIn,STREAM_READ);
        rIn>> bOpenInDesignMode;
    }

    ((FmFormModel*)this)->bStreamingOldVersion = sal_False;
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
    if (pObjShell && !m_pImpl->pUndoEnv->IsListening( *pObjShell ))
        SetObjectShell(pObjShell);
#endif

    SdrModel::InsertPage( pPage, nPos );

#ifndef SVX_LIGHT
    if (pPage)
        m_pImpl->pUndoEnv->AddForms(((FmFormPage*)pPage)->GetForms());
#endif
}

/*************************************************************************
|*
|* InsertPage
|*
\************************************************************************/
SdrPage* FmFormModel::RemovePage(sal_uInt16 nPgNum)
{
    FmFormPage* pPage = (FmFormPage*)SdrModel::RemovePage(nPgNum);

#ifndef SVX_LIGHT
    if (pPage)
        m_pImpl->pUndoEnv->RemoveForms(pPage->GetForms());
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
    if (pObjShell && !m_pImpl->pUndoEnv->IsListening( *pObjShell ))
        SetObjectShell(pObjShell);
#endif

    SdrModel::InsertMasterPage(pPage, nPos);

#ifndef SVX_LIGHT
    if (pPage)
        m_pImpl->pUndoEnv->AddForms(((FmFormPage*)pPage)->GetForms());
#endif
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
    if (pPage)
        m_pImpl->pUndoEnv->RemoveForms(pPage->GetForms());
#endif

    return pPage;
}

//------------------------------------------------------------------------
SdrLayerID FmFormModel::GetControlExportLayerId( const SdrObject& rObj ) const
{
    return rObj.GetLayer();
}

//------------------------------------------------------------------------
void FmFormModel::SetOpenInDesignMode( sal_Bool bOpenDesignMode )
{
#ifndef SVX_LIGHT
    if( bOpenDesignMode != bOpenInDesignMode )
    {
        bOpenInDesignMode = bOpenDesignMode;
        pObjShell->SetModified( sal_True );
    }
#endif
}

//------------------------------------------------------------------------
void FmFormModel::SetObjectShell( SfxObjectShell* pShell )
{
#ifndef SVX_LIGHT
    if (pShell == pObjShell)
        return;

    if (pObjShell)
    {
        m_pImpl->pUndoEnv->EndListening( *this );
        m_pImpl->pUndoEnv->EndListening( *pObjShell );
    }

    pObjShell = pShell;

    if (pObjShell)
    {
        m_pImpl->pUndoEnv->SetReadOnly(pObjShell->IsReadOnly() || pObjShell->IsReadOnlyUI());

        if (!m_pImpl->pUndoEnv->IsReadOnly())
             m_pImpl->pUndoEnv->StartListening(*this);

        m_pImpl->pUndoEnv->StartListening( *pObjShell );
    }
#endif
}

//------------------------------------------------------------------------
FmXUndoEnvironment& FmFormModel::GetUndoEnv()
{
    return *m_pImpl->pUndoEnv;
}

//------------------------------------------------------------------------
XubString FmFormModel::GetUniquePageId()
{
    XubString sReturn = m_pImpl->sNextPageId;

    xub_Unicode aNextChar = m_pImpl->sNextPageId.GetChar(m_pImpl->sNextPageId.Len() - 1);
    sal_Bool bNeedNewChar = sal_False;
    switch (aNextChar)
    {
        case '9' : aNextChar = 'A'; break;
        case 'Z' : aNextChar = 'a'; break;
        case 'z' : aNextChar = '0'; bNeedNewChar = sal_True; break;
        default: ++aNextChar; break;
    }
    m_pImpl->sNextPageId.SetChar(m_pImpl->sNextPageId.Len() - 1, aNextChar);
    if (bNeedNewChar)
        m_pImpl->sNextPageId += '0';

    return sReturn;
}


