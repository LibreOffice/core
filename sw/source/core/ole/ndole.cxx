/*************************************************************************
 *
 *  $RCSfile: ndole.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:36:33 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;


class SwOLELRUCache : private SvPtrarr, private utl::ConfigItem
{
    sal_uInt16 nLRU_InitSize;
    sal_Bool bInUnload;
    com::sun::star::uno::Sequence< rtl::OUString > GetPropertyNames();

public:
    SwOLELRUCache();

    virtual void Notify( const com::sun::star::uno::Sequence<
                                rtl::OUString>& aPropertyNames );
    virtual void Commit();
    void Load();

    void SetInUnload( BOOL bFlag )  { bInUnload = bFlag; }
    SvPtrarr::Count;

    void Insert( SwOLEObj& rObj );
    void Remove( SwOLEObj& rObj );

    void RemovePtr( SwOLEObj* pObj )
    {
        USHORT nPos = SvPtrarr::GetPos( pObj );
        if( USHRT_MAX != nPos )
            SvPtrarr::Remove( nPos );
    }
};

SwOLELRUCache* SwOLEObj::pOLELRU_Cache = 0;

// --------------------
// SwOLENode
// --------------------

SwOLENode::SwOLENode( const SwNodeIndex &rWhere,
                    SvInPlaceObject *pObj,
                    SwGrfFmtColl *pGrfColl,
                    SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_OLENODE, pGrfColl, pAutoAttr ),
    aOLEObj( pObj ),
    bOLESizeInvalid( FALSE )
{
    aOLEObj.SetNode( this );
}

SwOLENode::SwOLENode( const SwNodeIndex &rWhere,
                    const String &rString,
                    SwGrfFmtColl *pGrfColl,
                    SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_OLENODE, pGrfColl, pAutoAttr ),
    aOLEObj( rString ),
    bOLESizeInvalid( FALSE )
{
    aOLEObj.SetNode( this );
}

SwCntntNode *SwOLENode::SplitNode( const SwPosition & )
{
    // OLE-Objecte vervielfaeltigen ??
    ASSERT( FALSE, "OleNode: can't split." );
    return this;
}

// Laden eines in den Undo-Bereich verschobenen OLE-Objekts

BOOL SwOLENode::RestorePersistentData()
{
    aOLEObj.GetOleRef();
    if( aOLEObj.pOLERef && aOLEObj.pOLERef->Is() )
    {
        SvPersist* p = GetDoc()->GetPersist();
        if( p )     // muss da sein
        {
            SvInfoObjectRef aRef( p->Find( aOLEObj.aName ) );
            if( aRef.Is() )
                aRef->SetDeleted( FALSE );
        }
    }

// muss das sein????
//  if( pOLELRU_Cache )
//      pOLELRU_Cache->RemovePtr( &aOLEObj );
    return TRUE;
}

// Sichern eines in den Undo-Bereich zu verschiebenden OLE-Objekts

BOOL SwOLENode::SavePersistentData()
{
    if( aOLEObj.pOLERef && aOLEObj.pOLERef->Is() )
    {
        SvPersist* p = GetDoc()->GetPersist();
        if( p )     // muss da sein
        {
            SvInfoObjectRef aRef( p->Find( aOLEObj.aName ) );
            if( aRef.Is() )
            {
                aRef->SetDeleted( TRUE );
                aRef->SetObj(0);
            }
        }

        (*aOLEObj.pOLERef)->DoClose();
    }

    if( SwOLEObj::pOLELRU_Cache )
        SwOLEObj::pOLELRU_Cache->RemovePtr( &aOLEObj );

    if( aOLEObj.pOLERef && aOLEObj.pOLERef->Is() )
        (*aOLEObj.pOLERef).Clear();

    return TRUE;
}


SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
                                    SvInPlaceObject *pObj,
                                    SwGrfFmtColl* pGrfColl,
                                    SwAttrSet* pAutoAttr )
{
    ASSERT( pGrfColl,"SwNodes::MakeOLENode: Formatpointer ist 0." );

    SwOLENode *pNode =
        new SwOLENode( rWhere, pObj, pGrfColl, pAutoAttr );

#if 0
JP 02.10.97 - OLE Objecte stehen immer alleine im Rahmen, also hat es
                keinen Sinn, nach einem vorherigen/nachfolgenden
                ContentNode zu suchen!

    SwCntntNode *pCntntNd;
    SwIndex aIdx( rWhere, -1 );
    if ( (pCntntNd=(*this)[ rWhere ]->GetCntntNode()) != 0 )
        pCntntNd->MakeFrms( rWhere, aIdx );
    else
    {
        aIdx--;
        if ( (pCntntNd=(*this)[aIdx]->GetCntntNode()) != 0 )
        {
            SwIndex aTmp( aIdx );
            aIdx++;
            pCntntNd->MakeFrms( aTmp, aIdx );
        }
    }
#endif
    return pNode;
}


SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
    const String &rName, SwGrfFmtColl* pGrfColl, SwAttrSet* pAutoAttr )
{
    ASSERT( pGrfColl,"SwNodes::MakeOLENode: Formatpointer ist 0." );

    SwOLENode *pNode =
        new SwOLENode( rWhere, rName, pGrfColl, pAutoAttr );

#if 0
JP 02.10.97 - OLE Objecte stehen immer alleine im Rahmen, also hat es
                keinen Sinn, nach einem vorherigen/nachfolgenden
                ContentNode zu suchen!
    SwCntntNode *pCntntNd;
    SwIndex aIdx( rWhere, -1 );
    if ( (pCntntNd=(*this)[ rWhere ]->GetCntntNode()) != 0 )
        pCntntNd->MakeFrms( rWhere, aIdx );
    else
    {
        aIdx--;
        if ( (pCntntNd=(*this)[aIdx]->GetCntntNode()) != 0 )
        {
            SwIndex aTmp( aIdx );
            aIdx++;
            pCntntNd->MakeFrms( aTmp, aIdx );
        }
    }
#endif
    return pNode;
}


Size SwOLENode::GetTwipSize() const
{
    SvInPlaceObjectRef xRef( ((SwOLENode*)this)->aOLEObj.GetOleRef() );
    Size aSz( xRef->GetVisArea().GetSize() );
    const MapMode aDest( MAP_TWIP );
    const MapMode aSrc ( xRef->GetMapUnit() );
    return OutputDevice::LogicToLogic( aSz, aSrc, aDest );
}


SwCntntNode* SwOLENode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
    SvPersist* p = pDoc->GetPersist();
    if( !p )
    {
        ASSERT( pDoc->GetRefForDocShell(),
                        "wo ist die Ref-Klasse fuer die DocShell?")
        p = new SwDocShell( pDoc, SFX_CREATE_MODE_INTERNAL );
        *pDoc->GetRefForDocShell() = p;
        p->DoInitNew( NULL );
    }

    // Wir hauen das Ding auf SvPersist-Ebene rein
    String aNewName( Sw3Io::UniqueName( p->GetStorage(), "Obj" ) );
    SvPersist* pSrc = GetDoc()->GetPersist();

    p->CopyObject( aOLEObj.aName, aNewName, pSrc );
    SwOLENode* pOLENd = pDoc->GetNodes().MakeOLENode( rIdx, aNewName,
                                    (SwGrfFmtColl*)pDoc->GetDfltGrfFmtColl(),
                                    (SwAttrSet*)GetpSwAttrSet() );

    pOLENd->SetChartTblName( GetChartTblName() );
    pOLENd->SetAlternateText( GetAlternateText() );
    pOLENd->SetContour( HasContour(), HasAutomaticContour() );

    pOLENd->SetOLESizeInvalid( TRUE );
    pDoc->SetOLEPrtNotifyPending();

    return pOLENd;
}


BOOL SwOLENode::IsInGlobalDocSection() const
{
    // suche den "Body Anchor"
    ULONG nEndExtraIdx = GetNodes().GetEndOfExtras().GetIndex();
    const SwNode* pAnchorNd = this;
    do {
        SwFrmFmt* pFlyFmt = pAnchorNd->GetFlyFmt();
        if( !pFlyFmt )
            return FALSE;

        const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
        if( !rAnchor.GetCntntAnchor() )
            return FALSE;

        pAnchorNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
    } while( pAnchorNd->GetIndex() < nEndExtraIdx );

    const SwSectionNode* pSectNd = pAnchorNd->FindSectionNode();
    if( !pSectNd )
        return FALSE;

    while( pSectNd )
    {
        pAnchorNd = pSectNd;
        pSectNd = pAnchorNd->FindStartNode()->FindSectionNode();
    }

    // in pAnchorNd steht der zuletzt gefundene Section Node. Der muss
    // jetzt die Bedingung fuers GlobalDoc erfuellen.
    pSectNd = (SwSectionNode*)pAnchorNd;
    return FILE_LINK_SECTION == pSectNd->GetSection().GetType() &&
            pSectNd->GetIndex() > nEndExtraIdx;
}


BOOL SwOLENode::IsOLEObjectDeleted() const
{
    BOOL bRet = FALSE;
    if( aOLEObj.pOLERef && aOLEObj.pOLERef->Is() )
    {
        SvPersist* p = GetDoc()->GetPersist();
        if( p )     // muss da sein
        {
            SvInfoObjectRef aRef( p->Find( aOLEObj.aName ) );
            if( aRef.Is() )
                bRet = aRef->IsDeleted();
        }
    }
    return bRet;
}


SwOLEObj::SwOLEObj( SvInPlaceObject *pObj ) :
    pOLERef( new SvInPlaceObjectRef( pObj ) ),
    pOLENd( 0 )
{
}


SwOLEObj::SwOLEObj( const String &rString ) :
    pOLERef( 0 ),
    pOLENd( 0 ),
    aName( rString )
{
}


SwOLEObj::~SwOLEObj()
{
    if( pOLERef && pOLERef->Is() )
        //#41499# Kein DoClose(). Beim Beenden ruft der Sfx ein DoClose auf
        //die offenen Objekte. Dadurch wird ggf. eine temp. OLE-Grafik wieder
        //in eine Grafik gewandelt. Der OLE-Node wird zerstoert. Das DoClose
        //wueder in das leere laufen, weil das Objekt bereits im DoClose steht.
        //Durch das remove unten waere das DoClose aber nicht vollstaendig.
        (*pOLERef)->GetProtocol().Reset();
    delete pOLERef;
    // Object aus dem Storage removen!!
    if( pOLENd && !pOLENd->GetDoc()->IsInDtor() )   //NIcht notwendig im DTor (MM)
    {
        SvPersist* p = pOLENd->GetDoc()->GetPersist();
        if( p )     // muss er existieren ?
            p->Remove( aName );
    }

    if( pOLELRU_Cache )
    {
        pOLELRU_Cache->RemovePtr( this );
        if( !pOLELRU_Cache->Count() )
            // der letzte macht die Tuer zu
            delete pOLELRU_Cache, pOLELRU_Cache = 0;
    }
}


void SwOLEObj::SetNode( SwOLENode* pNode )
{
    pOLENd = pNode;
    if ( pOLERef && !aName.Len() )
    {
        SwDoc* pDoc = pNode->GetDoc();

        // Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
        SvPersist* p = pDoc->GetPersist();
        if( !p )
        {
            ASSERT( !this, "warum wird hier eine DocShell angelegt?" );
            p = new SwDocShell( pDoc, SFX_CREATE_MODE_INTERNAL );
            p->DoInitNew( NULL );
        }
        // Wir hauen das Ding auf SvPersist-Ebene rein
        aName = Sw3Io::UniqueName( p->GetStorage(), "Obj" );
        SvInfoObjectRef refObj = new SvEmbeddedInfoObject( *pOLERef, aName );

        ULONG nLstLen = p->GetObjectList() ? p->GetObjectList()->Count() : 0;
        if ( !p->Move( refObj, aName ) ) // Eigentuemer Uebergang!
            refObj.Clear();
        else if( nLstLen == p->GetObjectList()->Count() )
        {
            // Task 91051: Info-Object not insertet, so it exist another
            //              InfoObject to the same Object and the Objects
            //              is stored in the persist. This InfoObject we must
            //              found.
            p->Insert( refObj );
        }
        ASSERT( refObj.Is(), "InsertObject failed" );
    }
}

BOOL SwOLEObj::IsOleRef() const
{
    return pOLERef && pOLERef->Is();
}

SvInPlaceObjectRef SwOLEObj::GetOleRef()
{
    if( !pOLERef || !pOLERef->Is() )
    {
        SvPersist* p = pOLENd->GetDoc()->GetPersist();
        ASSERT( p, "kein SvPersist vorhanden" );

        // MIB 18.5.97: DIe Base-URL wird jetzt gesetzt, damit Plugins
        // nach dem Laden und vor dem Aktivieren des Frames korrekt
        // geladen werden koennen
        String sBaseURL( INetURLObject::GetBaseURL() );
        const SwDocShell *pDocSh = pOLENd->GetDoc()->GetDocShell();
        const SfxMedium *pMedium;
        if( pDocSh && 0 != (pMedium = pDocSh->GetMedium()) &&
            pMedium->GetName() != sBaseURL )
                INetURLObject::SetBaseURL( pMedium->GetName() );

        SvPersistRef xObj = p->GetObject( aName );
        ASSERT( !pOLERef || !pOLERef->Is(),
                "rekursiver Aufruf von GetOleRef() ist nicht erlaubt" )

        INetURLObject::SetBaseURL( sBaseURL );

        if ( !xObj.Is() )
        {
            //Das Teil konnte nicht geladen werden (wahrsch. Kaputt).
            Rectangle aArea;
            SwFrm *pFrm = pOLENd->GetFrm();
            if ( pFrm )
            {
                Size aSz( pFrm->Frm().SSize() );
                const MapMode aSrc ( MAP_TWIP );
                const MapMode aDest( MAP_100TH_MM );
                aSz = OutputDevice::LogicToLogic( aSz, aSrc, aDest );
                aArea.SetSize( aSz );
            }
            else
                aArea.SetSize( Size( 5000,  5000 ) );
            xObj = new SvDeathObject( aArea );
        }

        if( pOLERef )
            *pOLERef = &xObj;
        else
            pOLERef = new SvInPlaceObjectRef( xObj );
    }

    if( !pOLELRU_Cache )
        pOLELRU_Cache = new SwOLELRUCache;

    pOLELRU_Cache->Insert( *this );

    return *pOLERef;
}


void SwOLEObj::Unload()
{
    if( pOLERef && pOLELRU_Cache )
        pOLELRU_Cache->Remove( *this );
}

BOOL SwOLEObj::RemovedFromLRU()
{
    BOOL bRet = TRUE;
    //Nicht notwendig im Doc DTor (MM)
    ASSERT( pOLERef && pOLERef->Is() && 1 < (*pOLERef)->GetRefCount(),
            "Falscher RefCount fuers Unload" );
    const SwDoc* pDoc;
    if( pOLERef && pOLERef->Is() && pOLENd &&
        !( pDoc = pOLENd->GetDoc())->IsInDtor() &&
        SVOBJ_MISCSTATUS_ALWAYSACTIVATE != (*pOLERef)->GetMiscStatus() &&
        1 < (*pOLERef)->GetRefCount() &&
        !(*pOLERef)->GetProtocol().IsConnect() &&
        !(*pOLERef)->GetProtocol().IsInPlaceActive() )
    {
        SvPersist* p = pDoc->GetPersist();
        if( p )
        {
            if( pDoc->IsPurgeOLE() )
            {
                pOLELRU_Cache->SetInUnload( TRUE );
                SvPersist* pO = *pOLERef;

                if( pO->IsModified() && !pO->IsHandsOff() )
                {
                    pO->DoSave();
                    pO->DoSaveCompleted();
                }

                pOLERef->Clear();
                if( !p->Unload( pO ) )
                    *pOLERef = pO;

                pOLELRU_Cache->SetInUnload( FALSE );
            }
            else
                bRet = FALSE;
        }
    }
    return bRet;
}

SwOLELRUCache::SwOLELRUCache()
    : SvPtrarr( 64, 16 ),
    utl::ConfigItem( OUString::createFromAscii( "Office.Common/Cache" )),
    bInUnload( sal_False ),
    nLRU_InitSize( 20 )
{
    EnableNotification( GetPropertyNames() );
    Load();
}

com::sun::star::uno::Sequence< rtl::OUString > SwOLELRUCache::GetPropertyNames()
{
    Sequence< OUString > aNames( 1 );
    OUString* pNames = aNames.getArray();
    pNames[0] = OUString::createFromAscii( "Writer/OLE_Objects" );
    return aNames;
}

void SwOLELRUCache::Notify( const com::sun::star::uno::Sequence<
                                rtl::OUString>& rPropertyNames )
{
    Load();
}

void SwOLELRUCache::Commit()
{
}

void SwOLELRUCache::Load()
{
    Sequence< OUString > aNames( GetPropertyNames() );
    Sequence< Any > aValues = GetProperties( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" )
    if( aValues.getLength() == aNames.getLength() &&
        pValues->hasValue() )
    {
        sal_Int32 nVal;
        *pValues >>= nVal;
        if( 20 > nVal )
            nVal = 20;

        if( !bInUnload )
        {
            USHORT nPos = SvPtrarr::Count();
            if( nVal < nLRU_InitSize && nPos > nVal )
            {
                // remove the last entries
                while( nPos > nVal )
                {
                    SwOLEObj* pObj = (SwOLEObj*) SvPtrarr::GetObject( --nPos );
                    if( pObj->RemovedFromLRU() )
                        SvPtrarr::Remove( nPos );
                }
            }
        }
        nLRU_InitSize = (USHORT)nVal;
    }
}

void SwOLELRUCache::Insert( SwOLEObj& rObj )
{
    if( !bInUnload )
    {
        SwOLEObj* pObj = &rObj;
        USHORT nPos = SvPtrarr::GetPos( pObj );
        if( nPos )  // der auf der 0. Pos muss nicht verschoben werden!
        {
            if( USHRT_MAX != nPos )
                SvPtrarr::Remove( nPos );

            SvPtrarr::Insert( pObj, 0 );

            nPos = SvPtrarr::Count();
            while( nPos > nLRU_InitSize )
            {
                pObj = (SwOLEObj*) SvPtrarr::GetObject( --nPos );
                if( pObj->RemovedFromLRU() )
                    SvPtrarr::Remove( nPos );
            }
        }
    }
#ifndef PRODUCT
    else
    {
        SwOLEObj* pObj = &rObj;
        USHORT nPos = SvPtrarr::GetPos( pObj );
        ASSERT( USHRT_MAX != nPos, "Insert a new OLE object into a looked cache" );
    }
#endif
}

void SwOLELRUCache::Remove( SwOLEObj& rObj )
{
    if( !bInUnload )
    {
        USHORT nPos = SvPtrarr::GetPos( &rObj );
        if( USHRT_MAX != nPos && rObj.RemovedFromLRU() )
            SvPtrarr::Remove( nPos );
    }
}



