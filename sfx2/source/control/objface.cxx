/*************************************************************************
 *
 *  $RCSfile: objface.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
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

#include <stdlib.h>

#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif
#pragma hdrstop

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include "module.hxx"
#include "objface.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "cfgitem.hxx"
#include "sfxresid.hxx"
#include "minarray.hxx"
#include "cfgmgr.hxx"
#include "tbxmgr.hxx"
#ifdef _PROXYSHELL_VERWENDEN
#include "bindings.hxx" // fuer ProxyInterface
#include "dispatch.hxx" // fuer ProxyInterface
#endif

DBG_NAME(SfxInterface);

static const USHORT nVersion = 5;

//====================================================================

EXTERN_C
#if defined( PM2 ) && (!defined( CSET ) && !defined ( MTW ) && !defined( WTC ))
int _stdcall
#else
#ifdef WNT
int _cdecl
#else
int
#endif
#endif

SfxCompareSlots_Impl( const void* pSmaller, const void* pBigger )
{
    DBG_MEMTEST();
    return ( (int) ((SfxSlot*)pSmaller)->GetSlotId() ) -
           ( (int) ((SfxSlot*)pBigger)->GetSlotId() );
}

//=========================================================================

struct SfxObjectUI_Impl
{
    USHORT  nPos;
    USHORT  nInterfaceId;
    ResId   aResId;
    BOOL    bVisible;
    BOOL    bContext;
    String* pName;
    ULONG   nFeature;

    SfxObjectUI_Impl(USHORT n, const ResId& rResId, BOOL bVis, ULONG nFeat,
                USHORT nClassId=0xFFFF) :
        nPos(n),
        aResId(rResId.GetId(), rResId.GetResMgr()),
        nInterfaceId(nClassId),
        bVisible(bVis),
        pName(0),
        bContext(FALSE),
        nFeature(nFeat)
    {
        aResId.SetRT(rResId.GetRT());
    }

    ~SfxObjectUI_Impl()
    {
        delete pName;
    }
};

DECL_PTRARRAY(SfxObjectUIArr_Impl, SfxObjectUI_Impl*, 2, 2);

struct SfxInterface_Impl
{
    SfxObjectUIArr_Impl*    pObjectBars;    // registered ObjectBars
    SfxObjectUIArr_Impl*    pObjectMenues;  // registered ObjectMenues
    SfxObjectUIArr_Impl*    pChildWindows;  // registered ChildWindows
    ResId                   aPopupRes;      // registered PopupMenu
    ResId                   aStatBarRes;    // registered StatusBar
    SfxModule*              pModule;

    SfxInterface_Impl() :
        aPopupRes((USHORT)0),
        aStatBarRes((USHORT)0)
    {
        pObjectBars   = new SfxObjectUIArr_Impl;
        pObjectMenues = new SfxObjectUIArr_Impl;
        pChildWindows = new SfxObjectUIArr_Impl;
    }

    ~SfxInterface_Impl()
    {
        USHORT n;
        for (n=0; n<pObjectBars->Count(); n++)
            delete (*pObjectBars)[n];
        delete pObjectBars;

        for (n=0; n<pObjectMenues->Count(); n++)
            delete (*pObjectMenues)[n];
        delete pObjectMenues;

        for (n=0; n<pChildWindows->Count(); n++)
            delete (*pChildWindows)[n];
        delete pChildWindows;
    }
};

//====================================================================

class SfxIFConfig_Impl : public SfxConfigItem
{
friend class SfxInterface;
    USHORT                  nCount;
    SfxInterface*           pIFace;
    SfxObjectUIArr_Impl*    pObjectBars;

public:
                    SfxIFConfig_Impl(USHORT nClassId, SfxInterface *pIF);
    virtual         ~SfxIFConfig_Impl();
    virtual int     Load(SvStream&);
    virtual BOOL    Store(SvStream&);
    virtual String  GetName() const
                    { if (pIFace->HasName()) return pIFace->GetName();
                        else return String(); }
    virtual void    UseDefault();
    void            SaveDefaults(USHORT);
};

//-------------------------------------------------------------------------

SfxIFConfig_Impl::SfxIFConfig_Impl(USHORT nClassId, SfxInterface *pIF) :
    SfxConfigItem(SFX_ITEMTYPE_INTERFACE_START + nClassId),
    nCount(0),
    pObjectBars(0),
    pIFace(pIF)
{
    SetInternal(TRUE);
}

//-------------------------------------------------------------------------

void SfxIFConfig_Impl::SaveDefaults(USHORT nClassId)
{
    if ( pObjectBars )
        return;

    pObjectBars = new SfxObjectUIArr_Impl;

    for (nCount=0; nCount < pIFace->pImpData->pObjectBars->Count(); nCount++)
    {
        SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(
            pIFace->GetObjectBarPos(nCount),
            pIFace->GetObjectBarResId(nCount),
            pIFace->IsObjectBarVisible(nCount),
            pIFace->GetObjectBarFeature(nCount),
            nClassId);

        pObjectBars->Append(pUI);
        pUI->pName = new String(*pIFace->GetObjectBarName(nCount));
    }
}

//-------------------------------------------------------------------------

SfxIFConfig_Impl::~SfxIFConfig_Impl()
{
    if( pObjectBars )
    {
        for (USHORT n=0; n<pObjectBars->Count(); n++)
            delete (*pObjectBars)[n];
        delete pObjectBars;
    }
}

//-------------------------------------------------------------------------

int SfxIFConfig_Impl::Load(SvStream& rStream)
{
    for ( ;pIFace->pImpData->pObjectBars->Count(); )
        pIFace->ReleaseObjectBar(pIFace->GetObjectBarResId(0).GetId());

    USHORT n;
    rStream >> n;

    if ( n != nVersion)
        return SfxConfigItem::WARNING_VERSION;

    rStream >> n;
    for (USHORT i=0; i<n; i++)
    {
        pIFace->LoadObjectBar(i, rStream);
    }

    return SfxConfigItem::ERR_OK;
}

//-------------------------------------------------------------------------

BOOL SfxIFConfig_Impl::Store(SvStream& rStream)
{
    rStream << nVersion;

    rStream << pIFace->pImpData->pObjectBars->Count();
    USHORT nIFaceId = GetType() - SFX_ITEMTYPE_INTERFACE_START;

    for (USHORT i=0; i<pIFace->pImpData->pObjectBars->Count(); i++)
    {
        pIFace->StoreObjectBar(i, rStream);
    }

    return TRUE;
}

//-------------------------------------------------------------------------

void SfxIFConfig_Impl::UseDefault()
{
    pIFace->UseDefault();
}

//====================================================================
// ctor, registeres a new unit

SfxInterface::SfxInterface( SfxModule *pMod,
                            const char *pClassName,
                            const ResId& rNameResId,
                            USHORT nId,
                            const SfxInterface* pParent,
                            const SfxTypeLibImpl* pLibInfo ):
    pName(pClassName),
    nClassId(nId),
    aNameResId(rNameResId.GetId()),
    pGenoType(pParent),
    pTypeLibInfo(pLibInfo),
    pConfig(NULL),
    pImpData(0),
    nCount(0)
{
    aNameResId.SetResMgr(rNameResId.GetResMgr());
    Init( );
    pImpData->pModule = pMod;
}

SfxInterface::SfxInterface( SfxModule *pMod,
                            const char *pClassName,
                            const ResId& rNameResId,
                            USHORT nId,
                            const SfxInterface* pParent,
                            const SfxTypeLibImpl* pLibInfo,
                            SfxSlot &rSlotMap, USHORT nSlotCount ):
    pName(pClassName),
    nClassId(nId),
    aNameResId(rNameResId.GetId()),
    pGenoType(pParent),
    pTypeLibInfo(pLibInfo),
    pConfig(NULL),
    pImpData(0)
{
    aNameResId.SetResMgr(rNameResId.GetResMgr());
    Init();
    pImpData->pModule = pMod;
    SetSlotMap( rSlotMap, nSlotCount );
    // register the functions at the SfxMessagePool
    if ( pMod )
        pMod->GetSlotPool()->RegisterInterface(*this);
    else
        SFX_APP()->GetAppSlotPool_Impl().RegisterInterface(*this);
}

void SfxInterface::Init()
{
    DBG_MEMTEST();
    DBG_CTOR(SfxInterface, 0);
//    DBG_ASSERT(nCount, "Anzahl der Slot == NULL");

    if ( nClassId )
        pConfig = new SfxIFConfig_Impl(nClassId, this);
    pImpData = new SfxInterface_Impl;
}


void SfxInterface::SetSlotMap( SfxSlot& rSlotMap, USHORT nSlotCount )
{
    pSlots = &rSlotMap;
    nCount = nSlotCount;
    SfxSlot* pIter = pSlots;
    if ( 1 == nCount && !pIter->pNextSlot )
        pIter->pNextSlot = pIter;

    if ( !pIter->pNextSlot )
    {
        // sort the SfxSlots by id
        qsort( pSlots, nCount, sizeof(SfxSlot), SfxCompareSlots_Impl );

        // link masters and slaves
        USHORT nIter = 1;
        for ( pIter = pSlots; nIter <= nCount; ++pIter, ++nIter )
        {
            //! hier bitte sinnvoll pruefen
            //! DBG_ASSERT(!(pIter->IsMode(SFX_SLOT_CACHABLE) &&
            //!                 pIter->IsMode(SFX_SLOT_VOLATILE)),
            //!             "invalid Flags" );
            DBG_ASSERT( nIter == nCount ||
                        pIter->GetSlotId() != (pIter+1)->GetSlotId(),
                        "doppelte SID" );

            // jeder Master verweist auf seinen ersten Slave (ENUM), alle
            // Slaves auf ihren Master.
            // Slaves verweisen im Ring auf die anderen mit gleichem Master
            if ( pIter->GetKind() == SFX_KIND_ENUM )
            {
                pIter->pLinkedSlot = GetSlot( pIter->nMasterSlotId );
                DBG_ASSERT( pIter->pLinkedSlot, "slave without master" );
                if ( !pIter->pLinkedSlot->pLinkedSlot )
                    ( (SfxSlot*) pIter->pLinkedSlot)->pLinkedSlot = pIter;

                if ( 0 == pIter->GetNextSlot() )
                {
                    SfxSlot *pLastSlot = pIter;
                    for ( USHORT n = nIter; n < Count(); ++n )
                    {
                        SfxSlot *pCurSlot = (pSlots+n);
                        if ( pCurSlot->nMasterSlotId == pIter->nMasterSlotId )
                        {
                            pLastSlot->pNextSlot = pCurSlot;
                            pLastSlot = pCurSlot;
                        }
                    }
                    pLastSlot->pNextSlot = pIter;
                }
            }
            else if ( 0 == pIter->GetNextSlot() )
            {
                // Slots verweisen im Ring auf den n"achten mit derselben Statusmethode
                SfxSlot *pLastSlot = pIter;
                for ( USHORT n = nIter; n < Count(); ++n )
                {
                    SfxSlot *pCurSlot = (pSlots+n);
                    if ( pCurSlot->GetStateFnc() == pIter->GetStateFnc() )
                    {
                        pLastSlot->pNextSlot = pCurSlot;
                        pLastSlot = pCurSlot;
                    }
                }
                pLastSlot->pNextSlot = pIter;
            }
        }
    }
#ifdef DBG_UTIL
    else
    {
        USHORT nIter = 1;
        for ( SfxSlot *pNext = pIter+1; nIter < nCount; ++pNext, ++nIter )
        {

            if ( pNext->GetSlotId() <= pIter->GetSlotId() )
                DBG_ERROR ("Falsche Reihenfolge!");

            if ( pIter->GetKind() == SFX_KIND_ENUM )
            {
                const SfxSlot *pMasterSlot = GetSlot(pIter->nMasterSlotId);
                const SfxSlot *pFirstSlave = pMasterSlot->pLinkedSlot;
                const SfxSlot *pSlave = pFirstSlave;
                do
                {
                    if ( pSlave->pLinkedSlot != pMasterSlot )
                    {
                        ByteString aStr("Falsche Master/Slave-Verkettung : ");
                        aStr += ByteString::CreateFromInt32(pMasterSlot->GetSlotId());
                        aStr += " , ";
                        aStr += ByteString::CreateFromInt32(pSlave->GetSlotId());
                        DBG_ERROR(aStr.GetBuffer());
                    }

                    if ( pSlave->nMasterSlotId != pMasterSlot->GetSlotId() )
                    {
                        ByteString aStr("Falsche Master/Slave-Ids : ");
                        aStr += ByteString::CreateFromInt32(pMasterSlot->GetSlotId());
                        aStr += " , ";
                        aStr += ByteString::CreateFromInt32(pSlave->GetSlotId());
                        DBG_ERROR(aStr.GetBuffer());
                    }

                    pSlave = pSlave->pNextSlot;
                }
                while ( pSlave != pFirstSlave );
            }
            else
            {
                if ( pIter->pLinkedSlot )
                {
                    if ( pIter->pLinkedSlot->GetKind() != SFX_KIND_ENUM )
                    {
                        ByteString aStr("Slave ist kein enum : ");
                        aStr += ByteString::CreateFromInt32(pIter->GetSlotId());
                        aStr += " , ";
                        aStr += ByteString::CreateFromInt32(pIter->pLinkedSlot->GetSlotId());
                        DBG_ERROR(aStr.GetBuffer());
                    }
                }

                const SfxSlot *pLastSlot = pIter;
                const SfxSlot *pCurSlot = pIter;
                do
                {
                    pCurSlot = pCurSlot->pNextSlot;
                    if ( pCurSlot->GetStateFnc() != pIter->GetStateFnc() )
                    {
                        ByteString aStr("Verkettete Slots mit verschiedenen StateMethods : ");
                        aStr += ByteString::CreateFromInt32(pCurSlot->GetSlotId());
                        aStr += " , ";
                        aStr += ByteString::CreateFromInt32(pIter->GetSlotId());
                        DBG_ERROR(aStr.GetBuffer());
                    }
                }
                while ( pCurSlot != pIter );
            }

            pIter = pNext;
        }
    }
#endif
}


//--------------------------------------------------------------------



SfxInterface::~SfxInterface()
{
    if (pConfig)
        delete pConfig;

    SfxModule *pMod = pImpData->pModule;
    delete pImpData;
    if ( nClassId )
    {
        if ( pMod )
            pMod->GetSlotPool()->ReleaseInterface(*this);
        else
            SFX_APP()->GetAppSlotPool_Impl().ReleaseInterface(*this);
    }
}

//--------------------------------------------------------------------

// searches for the specified func


const SfxSlot* SfxInterface::GetSlot( USHORT nFuncId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxInterface, 0);
    DBG_ASSERT( this && pSlots && nCount, "" );

    // find the id using binary search
    void* p = bsearch( &nFuncId, pSlots, nCount, sizeof(SfxSlot),
                       SfxCompareSlots_Impl );
    if ( !p && pGenoType )
        return pGenoType->GetSlot( nFuncId );

    return p ? (const SfxSlot*)p : 0;
}

const SfxSlot* SfxInterface::GetSlot( const String& rCommand ) const
{
    SfxSlotPool& rPool = SFX_SLOTPOOL();
    for ( USHORT n=0; n<nCount; n++ )
    {
        USHORT nId = (pSlots+n)->GetSlotId();
        if ( rCommand.CompareIgnoreCaseToAscii( rPool.GetSlotName_Impl( nId ) ) == COMPARE_EQUAL )
            return pSlots+n;
    }

    return pGenoType ? pGenoType->GetSlot( rCommand ) : NULL;
}

//--------------------------------------------------------------------


const SfxSlot* SfxInterface::GetRealSlot( const SfxSlot *pSlot ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxInterface, 0);
    DBG_ASSERT( this && pSlots && nCount, "" );

    if ( !ContainsSlot_Impl(pSlot) )
    {
        if(pGenoType)
            return pGenoType->GetRealSlot(pSlot);
        DBG_ERROR("fremder Slot");
        return 0;
    }

    return pSlot->pLinkedSlot;
}

//--------------------------------------------------------------------


const SfxSlot* SfxInterface::GetRealSlot( USHORT nSlotId ) const
{
    DBG_MEMTEST();
    DBG_CHKTHIS(SfxInterface, 0);
    DBG_ASSERT( this && pSlots && nCount, "" );

    const SfxSlot *pSlot = GetSlot(nSlotId);
    if ( !pSlot )
    {
        if(pGenoType)
            return pGenoType->GetRealSlot(nSlotId);
        DBG_ERROR("fremder Slot");
        return 0;
    }

    return pSlot->pLinkedSlot;
}

//--------------------------------------------------------------------


void SfxInterface::RegisterPopupMenu( const ResId& rResId )
{
    DBG_CHKTHIS(SfxInterface, 0);
    pImpData->aPopupRes = rResId;
}

//--------------------------------------------------------------------


void SfxInterface::LoadConfig()
{
    if ( pConfig )
    {
        pConfig->SaveDefaults(nClassId);

        // Wenn die Application schon l"auft, wurde das Interface nachtr"aglich
        // erzeugt und mu\s daher explizit initialisiert werden.
        if ( Application::IsInExecute() )
            pConfig->Initialize();
    }
}


BOOL SfxInterface::IsDefault()
{
    if (pConfig)
        return pConfig->IsDefault();
     else
        return TRUE;
}


void SfxInterface::UseDefault()
{
    if ( !pConfig )
    {
        for (USHORT n=0; n<pImpData->pObjectBars->Count(); n++)
            delete (*pImpData->pObjectBars)[n];
        pImpData->pObjectBars->Remove(0, pImpData->pObjectBars->Count());
        return;
    }
    else if ( !pConfig->IsDefault() )
    {
        USHORT n;
        for (n=0; n<pImpData->pObjectBars->Count(); n++)
            delete (*pImpData->pObjectBars)[n];
        pImpData->pObjectBars->Remove(0, pImpData->pObjectBars->Count());

        for (n=0; n<pConfig->nCount; n++)
        {
            SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(
                (*pConfig->pObjectBars)[n]->nPos,
                (*pConfig->pObjectBars)[n]->aResId,
                (*pConfig->pObjectBars)[n]->bVisible,
                (*pConfig->pObjectBars)[n]->nFeature,
                nClassId);

            pImpData->pObjectBars->Append(pUI);
            pUI->pName = new String(*((*pConfig->pObjectBars)[n]->pName));
        }
    }
}

//--------------------------------------------------------------------


void SfxInterface::SetObjectBarName(const String& rName, USHORT nId)
{
    // Objectbar im eigenen Array suchen
    USHORT nCount = pImpData->pObjectBars->Count();
    USHORT n;
    for ( n=0; n<nCount; n++ )
        if ((*pImpData->pObjectBars)[n]->aResId.GetId() == nId) break;

    if ( n >= nCount )
    {
        // Nicht gefunden, in der Superklasse versuchen
        BOOL bGenoType = ( pGenoType != 0 && !pGenoType->HasName() );
        if (bGenoType)
            ((SfxInterface*)pGenoType)->SetObjectBarName ( rName, nId );
        else
            DBG_ERROR("Objectbar ist unbekannt!");
    }
    else
    {
        SfxObjectUI_Impl *pUI = (*pImpData->pObjectBars)[n];
        delete pUI->pName;
        pUI->pName = new String( rName );
        if ( pConfig )
            pConfig->SetDefault(FALSE);
    }
}


void SfxInterface::SetObjectBarPos(USHORT nPos, USHORT nId)
{
    // Objectbar im eigenen Array suchen
    USHORT nCount = pImpData->pObjectBars->Count();
    USHORT n;
    for ( n=0; n<nCount; n++ )
        if ((*pImpData->pObjectBars)[n]->aResId.GetId() == nId) break;

    if ( n >= nCount )
    {
        // Nicht gefunden, in der Superklasse versuchen
        BOOL bGenoType = ( pGenoType != 0 && !pGenoType->HasName() );
        if (bGenoType)
            ((SfxInterface*)pGenoType)->SetObjectBarPos ( nPos, nId );
        else
            DBG_ERROR("Objectbar ist unbekannt!");
    }
    else
    {
        (*pImpData->pObjectBars)[n]->nPos = nPos;
        if (pConfig)
            pConfig->SetDefault(FALSE);
    }
}

//-------------------------------------------------------------------------

void SfxInterface::RegisterObjectBar( USHORT nPos, const ResId& rResId,
        const String *pStr )
{
    RegisterObjectBar( nPos, rResId, 0UL, pStr );
}


void SfxInterface::RegisterObjectBar( USHORT nPos, const ResId& rResId,
        ULONG nFeature, const String *pStr )
{
    if ((nPos & SFX_VISIBILITY_MASK) == 0)
    {
        if ((nPos & SFX_POSITION_MASK) == 0 )
            nPos |= SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_STANDARD;
        else
            nPos |= (SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_SERVER);
    }

    // App-Leiste?
    if ( SFX_OBJECTBAR_APPLICATION == ( nPos & SFX_POSITION_MASK ) )
        // je nach Desktop oder Einzelapp nur die richtige registrieren
        if ( !( SFX_VISIBILITY_DESKTOP == ( nPos & SFX_VISIBILITY_DESKTOP ) ) )
            return;
    nPos &= ~(USHORT)SFX_VISIBILITY_DESKTOP;

    if( nPos & SFX_VISIBILITY_SERVER )
        nPos |= SFX_VISIBILITY_PLUGSERVER;
    if( nPos & SFX_VISIBILITY_CLIENT )
        nPos |= SFX_VISIBILITY_PLUGCLIENT;

    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(nPos, rResId, TRUE, nFeature, nClassId);

    pImpData->pObjectBars->Append(pUI);

    if (pStr == 0)
    {
        ResId aResId(rResId);
        aResId.SetRT(RSC_STRING);
        aResId.SetResMgr(rResId.GetResMgr());
        if ( !Resource::GetResManager()->IsAvailable(aResId) )
        {
            DBG_ERROR("Der Objectbar-Name fehlt in der Resource!");
            pUI->pName = new String (DEFINE_CONST_UNICODE("NoName"));
        }
        else
            pUI->pName = new String(aResId);
    }
    else
        pUI->pName = new String(*pStr);
}


void SfxInterface::TransferObjectBar( USHORT nPos, USHORT nId, SfxInterface *pIFace,
        const String *pStr)
{
    if ( !pIFace )
    {
        RegisterObjectBar( nPos, nId, pStr );
        if (pConfig)
            pConfig->SetDefault(FALSE);
        return;
    }

    // Suche den ObjectBar im Quellen-Interface ( GenoType ?? )
    USHORT n;
    for (n=0; n<pIFace->pImpData->pObjectBars->Count(); n++)
        if ((*pIFace->pImpData->pObjectBars)[n]->aResId.GetId() == nId) break;

    DBG_ASSERT(n<pIFace->pImpData->pObjectBars->Count(),"Objectbar ist unbekannt!");

    // Uebernimm ResId und Interface-Id
    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(nPos,
                             (*pIFace->pImpData->pObjectBars)[n]->aResId,
                             (*pIFace->pImpData->pObjectBars)[n]->bVisible,
                             (*pIFace->pImpData->pObjectBars)[n]->nFeature,
                             (*pIFace->pImpData->pObjectBars)[n]->nInterfaceId);
    pImpData->pObjectBars->Append(pUI);

    if (pStr == 0)
    {
        pUI->pName = new String( *(pIFace->GetObjectBarName(n)));
    }
    else
        pUI->pName = new String(*pStr);

    if (pConfig)
        pConfig->SetDefault(FALSE);

    pIFace->ReleaseObjectBar(nId);
}



void SfxInterface::ReleaseObjectBar( USHORT nId )
{
    // Objectbar im eigenen Array suchen
    USHORT nCount = pImpData->pObjectBars->Count();
    USHORT n;
    for ( n=0; n<nCount; n++ )
        if ((*pImpData->pObjectBars)[n]->aResId.GetId() == nId) break;

    if ( n >= nCount )
    {
        // Nicht gefunden, in der Superklasse versuchen
        BOOL bGenoType = ( pGenoType != 0 && !pGenoType->HasName() );
        if (bGenoType)
            ((SfxInterface*)pGenoType)->ReleaseObjectBar( nId );
        else
            DBG_ERROR("Objectbar ist unbekannt!");
    }
    else
    {
        delete (*pImpData->pObjectBars)[n];
        pImpData->pObjectBars->Remove(n);
        if (pConfig)
            pConfig->SetDefault(FALSE);
    }
}

//--------------------------------------------------------------------


const ResId& SfxInterface::GetObjectBarResId( USHORT nNo ) const
{
    BOOL bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Gibt es Toolbars in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->GetObjectBarResId( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pObjectBars->Count();
    DBG_ASSERT( nNo<nCount,"Objectbar ist unbekannt!" );
    return (*pImpData->pObjectBars)[nNo]->aResId;
}

//--------------------------------------------------------------------


USHORT SfxInterface::GetObjectBarPos( USHORT nNo ) const
{
    BOOL bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Gibt es Toolbars in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->GetObjectBarPos( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pObjectBars->Count();
    DBG_ASSERT( nNo<nCount,"Objectbar ist unbekannt!" );
    return (*pImpData->pObjectBars)[nNo]->nPos;
}

//--------------------------------------------------------------------


USHORT SfxInterface::GetObjectBarCount() const
{
    if (pGenoType && ! pGenoType->HasName())
        return pImpData->pObjectBars->Count() + pGenoType->GetObjectBarCount();
    else
        return pImpData->pObjectBars->Count();
}

//--------------------------------------------------------------------


const ResId& SfxInterface::GetObjectMenuResId( USHORT nNo ) const
{
    DBG_ASSERT(nNo<pImpData->pObjectMenues->Count(),"ObjectMenue ist unbekannt!");
    return (*pImpData->pObjectMenues)[nNo]->aResId;
}

//--------------------------------------------------------------------


USHORT SfxInterface::GetObjectMenuPos( USHORT nNo ) const
{
    DBG_ASSERT(nNo<pImpData->pObjectMenues->Count(),"ObjectMenue ist unbekannt!");
    return (*pImpData->pObjectMenues)[nNo]->nPos;
}

//--------------------------------------------------------------------


USHORT SfxInterface::GetObjectMenuCount() const
{
    return pImpData->pObjectMenues->Count();
}

void SfxInterface::ClearObjectMenus()
{
    for (USHORT n = pImpData->pObjectMenues->Count(); n; )
    {
        delete pImpData->pObjectMenues->GetObject( --n );
        pImpData->pObjectMenues->Remove( n );
    }
}

void SfxInterface::RemoveObjectMenu( USHORT nPos )
{
    for (USHORT n = pImpData->pObjectMenues->Count(); n; )
        if( pImpData->pObjectMenues->GetObject(--n)->nPos == nPos )
        {
            delete pImpData->pObjectMenues->GetObject( n );
            pImpData->pObjectMenues->Remove( n );
        }
}
//--------------------------------------------------------------------


void SfxInterface::RegisterObjectMenu( USHORT nPos, const ResId& rResId )
{
    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(nPos, rResId, TRUE, 0, nClassId);
    pImpData->pObjectMenues->Append(pUI);
}

//--------------------------------------------------------------------


BOOL SfxInterface::StoreObjectBar(USHORT n, SvStream& rStream)
{
    rStream << ((*pImpData->pObjectBars)[n]->nPos)
            << (*pImpData->pObjectBars)[n]->aResId.GetId()
            << (*pImpData->pObjectBars)[n]->nInterfaceId
            << (USHORT) (*pImpData->pObjectBars)[n]->bVisible;
    rStream.WriteByteString(*(*pImpData->pObjectBars)[n]->pName);
    rStream << (*pImpData->pObjectBars)[n]->nFeature;
    return TRUE;
}

//--------------------------------------------------------------------


int SfxInterface::LoadObjectBar(USHORT n, SvStream& rStream)
{
    DBG_ASSERT( pConfig, "LoadObjectBar ohne Config!" );

    USHORT nId, nPos, nIFaceId, nVisible;
    ULONG nFeature;
    String aName;

    rStream >> nPos >> nId >> nIFaceId >> nVisible;
    rStream.ReadByteString(aName);

    if ( nVersion > 4 )
        rStream >> nFeature;

    ResId aResId(nId);
    RegisterObjectBar(nPos, aResId, nFeature, &aName );
    (*pImpData->pObjectBars)[n]->nInterfaceId = nIFaceId;
    (*pImpData->pObjectBars)[n]->bVisible = (BOOL) nVisible;

    SfxIFConfig_Impl *pCfg = pConfig;

    if (nIFaceId != nClassId)           // pConfig von anderem Interface holen
    {
        SfxInterface *pIFace =
            SFX_APP()->GetInterfaceByIdImpl(SfxInterfaceId(nIFaceId));
        if (pIFace)
            pCfg = pIFace->pConfig;
    }

    ResMgr *pResMgr=0;                  // falls selbstdefiniert!
    if (pCfg->pObjectBars->Count())
        pResMgr = (*pCfg->pObjectBars)[0]->aResId.GetResMgr();

    (*pImpData->pObjectBars)[n]->aResId.SetResMgr(pResMgr);

    return SfxConfigItem::ERR_OK;
}


void SfxInterface::RegisterChildWindow(USHORT nId, BOOL bContext, const String* pName)
{
    RegisterChildWindow( nId, bContext, 0UL, pName );
}

void SfxInterface::RegisterChildWindow(USHORT nId, BOOL bContext, ULONG nFeature, const String*)
{
    SfxObjectUI_Impl* pUI = new SfxObjectUI_Impl(0, nId, TRUE, nFeature, 0);
    pUI->bContext = bContext;
    pImpData->pChildWindows->Append(pUI);
}

void SfxInterface::ReleaseChildWindow( USHORT nId )
{
    USHORT nCount = pImpData->pChildWindows->Count();
    USHORT n;
    for ( n=0; n<nCount; n++ )
        if ((*pImpData->pChildWindows)[n]->aResId.GetId() == nId) break;

    if ( n >= nCount )
    {
        // Nicht gefunden, in der Superklasse versuchen
        BOOL bGenoType = ( pGenoType != 0 && !pGenoType->HasName() );
        if (bGenoType)
            ((SfxInterface*)pGenoType)->ReleaseChildWindow( nId );
        else
            DBG_ERROR("ChildWindow ist unbekannt!");
    }
    else
    {
        delete (*pImpData->pChildWindows)[n];
        pImpData->pChildWindows->Remove(n);
        if (pConfig)
            pConfig->SetDefault(FALSE);
    }
}

void SfxInterface::RegisterStatusBar(const ResId& rResId)
{
    pImpData->aStatBarRes = rResId;
}


ULONG SfxInterface::GetChildWindowId (USHORT nNo) const
{
    if ( pGenoType )
    {
        // Gibt es ChildWindows in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetChildWindowCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->GetChildWindowId( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pChildWindows->Count();
    DBG_ASSERT( nNo<nCount,"ChildWindow ist unbekannt!" );
    ULONG nRet = (ULONG) (*pImpData->pChildWindows)[nNo]->aResId.GetId();
    if ( (*pImpData->pChildWindows)[nNo]->bContext )
        nRet += ( (ULONG) nClassId ) << 16;
    return nRet;
}

ULONG SfxInterface::GetChildWindowFeature (USHORT nNo) const
{
    if ( pGenoType )
    {
        // Gibt es ChildWindows in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetChildWindowCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->GetChildWindowFeature( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pChildWindows->Count();
    DBG_ASSERT( nNo<nCount,"ChildWindow ist unbekannt!" );
    ULONG nRet = (ULONG) (*pImpData->pChildWindows)[nNo]->nFeature;
    return nRet;
}

//--------------------------------------------------------------------


USHORT SfxInterface::GetChildWindowCount() const
{
    if (pGenoType)
        return pImpData->pChildWindows->Count() + pGenoType->GetChildWindowCount();
    else
        return pImpData->pChildWindows->Count();
}


const ResId& SfxInterface::GetPopupMenuResId() const
{
    return pImpData->aPopupRes;
}


const ResId& SfxInterface::GetStatusBarResId() const
{
    if (pImpData->aStatBarRes.GetId() == 0 && pGenoType)
        return pGenoType->GetStatusBarResId();
    else
        return pImpData->aStatBarRes;
}



const String* SfxInterface::GetObjectBarName ( USHORT nNo ) const
{
    BOOL bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Gibt es Toolbars in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->GetObjectBarName( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pObjectBars->Count();
    DBG_ASSERT( nNo<nCount,"Objectbar ist unbekannt!" );
    return (*pImpData->pObjectBars)[nNo]->pName;
}

ULONG SfxInterface::GetObjectBarFeature ( USHORT nNo ) const
{
    BOOL bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Gibt es Toolbars in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->GetObjectBarFeature( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pObjectBars->Count();
    DBG_ASSERT( nNo<nCount,"Objectbar ist unbekannt!" );
    return (*pImpData->pObjectBars)[nNo]->nFeature;
}


BOOL SfxInterface::IsObjectBarVisible(USHORT nNo) const
{
    BOOL bGenoType = (pGenoType != 0 && !pGenoType->HasName());
    if ( bGenoType )
    {
        // Gibt es Toolbars in der Superklasse ?
        USHORT nBaseCount = pGenoType->GetObjectBarCount();
        if ( nNo < nBaseCount )
            // Die der Superklasse kommen zuerst
            return pGenoType->IsObjectBarVisible( nNo );
        else
            nNo -= nBaseCount;
    }

    USHORT nCount = pImpData->pObjectBars->Count();
    DBG_ASSERT( nNo<nCount,"Objectbar ist unbekannt!" );
    return (*pImpData->pObjectBars)[nNo]->bVisible;
}


void SfxInterface::SetObjectBarVisible(BOOL bVis, USHORT nId)
{
    // Objectbar im eigenen Array suchen
    USHORT nCount = pImpData->pObjectBars->Count();
    USHORT n;
    for ( n=0; n<nCount; n++ )
        if ( (*pImpData->pObjectBars)[n]->aResId.GetId() == nId ) break;

    if ( n >= nCount )
    {
        // Nicht gefunden, in der Superklasse versuchen
        BOOL bGenoType = (pGenoType != 0 && !pGenoType->HasName());
        if (bGenoType)
            ((SfxInterface*)pGenoType)->SetObjectBarVisible(bVis,nId);
        else
            DBG_ERROR("Objectbar ist unbekannt!");
    }
    else
    {
        (*pImpData->pObjectBars)[n]->bVisible = bVis;
        if (pConfig)
            pConfig->SetDefault(FALSE);
    }
}


void SfxInterface::ReInitialize(SfxConfigManager *pCfgMgr)
{
    DBG_ASSERT( pConfig, "ReInitialize ohne Config!" );

    SfxConfigManager *pOld = pConfig->GetConfigManager_Impl();
    if (pCfgMgr == pOld)
        return;
    pConfig->ReInitialize(pCfgMgr);

    pCfgMgr->RemoveConfigItem(pConfig);
    pOld->AddConfigItem(pConfig);

    pConfig->SetDefault(FALSE);

    for (USHORT n=0; n<pImpData->pObjectBars->Count(); n++)
    {
        USHORT nId = (*pImpData->pObjectBars)[n]->aResId.GetId();
        if ( SfxToolBoxManager::IsUserDefToolBox_Impl(nId))
        {
            if (!pOld->HasConfigItem(nId))
                pOld->InsertConfigItem(nId);
        }
    }
}


void SfxInterface::SaveConfig()
{
    if ( pConfig )
        pConfig->StoreConfig();
}


USHORT SfxInterface::GetConfigId() const
{
    return SFX_ITEMTYPE_INTERFACE_START + nClassId;
}


SfxObjectUIArr_Impl* SfxInterface::GetObjectBarArr_Impl() const
{
    return pImpData->pObjectBars;
}


USHORT SfxInterface::RegisterUserDefToolBox(USHORT nId, const String *pName,
                SfxConfigManager *pCfgMgr)
{
    // Zuerst "uber den Namen die ToolBox suchen
    SfxInterface *pIFace;
    for (pIFace = SFX_SLOTPOOL().FirstInterface(); pIFace != 0;
            pIFace = SFX_SLOTPOOL().NextInterface())
    {
        // Nur Interfaces mit Namen d"urfen welche bekommen
        if (pIFace->HasName())
        {
            // Zugriff auf ObjectBars
            SfxObjectUIArr_Impl *pArr = pIFace->GetObjectBarArr_Impl();
            for (USHORT nNo=0; nNo<pArr->Count(); nNo++)
            {
                if ( *pName == *(*pArr)[nNo]->pName)
                    return (*pArr)[nNo]->aResId.GetId();
            }
        }
    }

    // Ansonsten n"achste freie Id besorgen
    USHORT nFreeId = SfxToolBoxManager::GetUserDefToolBoxId_Impl();
    BOOL bFound = FALSE;

    // Alle Interfaces durchprobieren, ob sie noch eine UserDefToolBox
    // aufnehmen k"onnen
    USHORT nPos;
    for (pIFace = SFX_SLOTPOOL().FirstInterface(); pIFace != 0;
            pIFace = SFX_SLOTPOOL().NextInterface())
    {
        BOOL bFound = FALSE;

        // Nur Interfaces mit Namen d"urfen welche bekommen
        if (pIFace->HasName())
        {
            // Zugriff auf ObjectBars
            SfxObjectUIArr_Impl *pArr = pIFace->GetObjectBarArr_Impl();

            for ( nPos=SFX_OBJECTBAR_USERDEF1; nPos<=SFX_OBJECTBAR_USERDEF3; nPos++ )
            {
                // Nach freier Position suchen
                bFound = TRUE;
                for (USHORT nNo=0; nNo<pArr->Count(); nNo++)
                {
                    if ( nPos == (*pArr)[nNo]->nPos)
                    {
                        // Diese Position ist schon besetzt, n"achste versuchen
                        bFound = FALSE;
                        break;
                    }
                }

                // Wurde eine noch freie Position gefunden ?
                if ( bFound )
                    break;
            }
        }

        // Wurde am Interface etwas gefunden ?
        if ( bFound )
            break;
    }

    if ( pIFace )
    {
        // Am gefundenen Interface mu\s die Konfiguration ver"andert werden
        SfxConfigItem *pCfgItem = pIFace->GetConfig_Impl();
        SfxConfigManager *pOldCfgMgr = pCfgItem->GetConfigManager_Impl();

        if ( pOldCfgMgr != pCfgMgr )
        {
            // Wenn das Interface bisher von einem anderen ConfigManager
            // bedient wird, wird dessen Config gesichert
            pCfgItem->StoreConfig();

            // Der Ziel-Configmanager mu\s die Konfiguration aufnehmen k"onnen
            if ( !pCfgMgr->HasConfigItem(pIFace->GetConfigId()) )
                pCfgMgr->InsertConfigItem(pIFace->GetConfigId());

            // Verbindung mit dem ConfigItem herstellen und Config "ubernehmen
            pCfgMgr->AddConfigItem(pCfgItem);
            pCfgItem->Initialize();
        }

        // Jetzt wird das Interface umkonfiguriert
        pIFace->RegisterObjectBar(nPos, nFreeId, pName);
        pIFace->SetObjectBarVisible(TRUE, nFreeId);

        // Konfiguration sichern und aktuellen Stand der Config restaurieren
        if ( pCfgMgr != pOldCfgMgr && pCfgMgr != SFX_CFGMANAGER() )
        {
            pCfgItem->StoreConfig();
            pCfgMgr->RemoveConfigItem(pCfgItem);
            pCfgItem->Connect(pOldCfgMgr);
            pCfgItem->Initialize();
        }
    }

    return nFreeId;
}


void SfxInterface::ReleaseUserDefToolBox(USHORT nId, SfxConfigManager *pCfgMgr)

{
    BOOL bDone = FALSE;

    // Alle Interfaces durchprobieren, ob sie diese UserDefToolBox haben
    for (SfxInterface *pIFace = SFX_SLOTPOOL().FirstInterface(); pIFace != 0;
            pIFace = SFX_SLOTPOOL().NextInterface())
    {
        // Nur Interfaces mit Namen k"onnen welche haben
        if (pIFace->HasName())
        {
            // Die Konfiguration vom "ubergebenen Manager interessiert
            SfxConfigItem *pCfgItem = pIFace->GetConfig_Impl();
            SfxConfigManager *pOldCfgMgr = pCfgItem->GetConfigManager_Impl();

            if ( pOldCfgMgr != pCfgMgr )
            {
                // Wenn das Interface bisher von einem anderen ConfigManager
                // bedient wird, wird dessen Config gesichert
                pCfgItem->StoreConfig();

                // Verbindung mit dem ConfigItem herstellen und Config "ubernehmen
                pCfgMgr->AddConfigItem(pCfgItem);
                pCfgItem->Initialize();
            }

            if ( pIFace->HasObjectBar(nId) )
            {
                pIFace->ReleaseObjectBar(nId);
                BOOL bDone = TRUE;
            }

            // Konfiguration sichern und aktuellen Stand der Config restaurieren
            if ( pCfgMgr != pOldCfgMgr )
            {
                pCfgItem->StoreConfig();
                pCfgMgr->RemoveConfigItem(pCfgItem);
                pCfgItem->Connect(pOldCfgMgr);
                pCfgItem->Initialize();
            }
        }

        if ( bDone )
        {
            break;
        }
    }
}


BOOL SfxInterface::HasObjectBar( USHORT nId ) const
{
    for (USHORT n=0; n<pImpData->pObjectBars->Count(); n++)
        if ((*pImpData->pObjectBars)[n]->aResId.GetId() == nId) return TRUE;
    return FALSE;
}

SfxModule* SfxInterface::GetModule() const
{
    return pImpData->pModule;
}

const SfxInterface* SfxInterface::GetRealInterfaceForSlot( const SfxSlot *pRealSlot ) const
{
    const SfxInterface* pInterface = this;

    // Der Slot k"onnte auch aus dem Interface einer Shell-Basisklasse stammen
    do
    {
        const SfxSlot *pLastSlot  = (*pInterface)[pInterface->Count()-1];
        const SfxSlot *pFirstSlot = (*pInterface)[0];

        // Ist pInterface der Owner von pRealSlot ?
        if ( pFirstSlot <= pRealSlot && pRealSlot <= pLastSlot )
            break;

        // Sonst Interface der Superklasse probieren
        pInterface = pInterface->pGenoType;
    }
    while ( pInterface );

    return pInterface;
}


