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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
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

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
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
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

using namespace ::utl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;


class SwOLELRUCache : private SvPtrarr, private ::utl::ConfigItem
{
    sal_uInt16 nLRU_InitSize;
    sal_Bool bInUnload;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetPropertyNames();

public:
    SwOLELRUCache();

    void Load();

    SvPtrarr::Count;

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
 
    void Insert( SwOLEObj& rObj );
    void Remove( SwOLEObj& rObj );
 
    void RemovePtr( SwOLEObj* pObj )
    {
        USHORT nPos = SvPtrarr::GetPos( pObj );
        if( USHRT_MAX != nPos )
            SvPtrarr::Remove( nPos );
    }
};

void SwOLELRUCache::Commit() {}
void SwOLELRUCache::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames ) {}

SwOLELRUCache* SwOLEObj::pOLELRU_Cache = 0;

// --------------------
// SwOLENode
// --------------------

/*N*/ SwOLENode::SwOLENode( const SwNodeIndex &rWhere,
/*N*/ 					SvInPlaceObject *pObj,
/*N*/ 					SwGrfFmtColl *pGrfColl,
/*N*/ 					SwAttrSet* pAutoAttr ) :
/*N*/ 	SwNoTxtNode( rWhere, ND_OLENODE, pGrfColl, pAutoAttr ),
/*N*/ 	aOLEObj( pObj ),
/*N*/ 	bOLESizeInvalid( FALSE )
/*N*/ {
/*N*/ 	aOLEObj.SetNode( this );
/*N*/ }

/*N*/ SwOLENode::SwOLENode( const SwNodeIndex &rWhere,
/*N*/ 					const String &rString,
/*N*/ 					SwGrfFmtColl *pGrfColl,
/*N*/ 					SwAttrSet* pAutoAttr ) :
/*N*/ 	SwNoTxtNode( rWhere, ND_OLENODE, pGrfColl, pAutoAttr ),
/*N*/ 	aOLEObj( rString ),
/*N*/ 	bOLESizeInvalid( FALSE )
/*N*/ {
/*N*/ 	aOLEObj.SetNode( this );
/*N*/ }

/*N*/ SwCntntNode *SwOLENode::SplitNode( const SwPosition & )
/*N*/ {
/*N*/ 	// OLE-Objecte vervielfaeltigen ??
/*N*/ 	ASSERT( FALSE, "OleNode: can't split." );
/*N*/ 	return this;
/*N*/ }

// Laden eines in den Undo-Bereich verschobenen OLE-Objekts


// Sichern eines in den Undo-Bereich zu verschiebenden OLE-Objekts



/*N*/ SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
/*N*/ 									SvInPlaceObject *pObj,
/*N*/ 									SwGrfFmtColl* pGrfColl,
/*N*/ 									SwAttrSet* pAutoAttr )
/*N*/ {
/*N*/ 	ASSERT( pGrfColl,"SwNodes::MakeOLENode: Formatpointer ist 0." );
/*N*/ 
/*N*/ 	SwOLENode *pNode =
/*N*/ 		new SwOLENode( rWhere, pObj, pGrfColl, pAutoAttr );
/*N*/ 
/*N*/ #if 0
/*N*/ JP 02.10.97 - OLE Objecte stehen immer alleine im Rahmen, also hat es
/*N*/ 				keinen Sinn, nach einem vorherigen/nachfolgenden
/*N*/ 				ContentNode zu suchen!
/*N*/ 
/*N*/ 	SwCntntNode *pCntntNd;
/*N*/ 	SwIndex aIdx( rWhere, -1 );
/*N*/ 	if ( (pCntntNd=(*this)[ rWhere ]->GetCntntNode()) != 0 )
/*N*/ 		pCntntNd->MakeFrms( rWhere, aIdx );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aIdx--;
/*N*/ 		if ( (pCntntNd=(*this)[aIdx]->GetCntntNode()) != 0 )
/*N*/ 		{
/*N*/ 			SwIndex aTmp( aIdx );
/*N*/ 			aIdx++;
/*N*/ 			pCntntNd->MakeFrms( aTmp, aIdx );
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return pNode;
/*N*/ }


/*N*/ SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
/*N*/ 									String &rName,
/*N*/ 									SwGrfFmtColl* pGrfColl,
/*N*/ 									SwAttrSet* pAutoAttr )
/*N*/ {
/*N*/ 	ASSERT( pGrfColl,"SwNodes::MakeOLENode: Formatpointer ist 0." );
/*N*/ 
/*N*/ 	SwOLENode *pNode =
/*N*/ 		new SwOLENode( rWhere, rName, pGrfColl, pAutoAttr );
/*N*/ 
/*N*/ #if 0
/*N*/ JP 02.10.97 - OLE Objecte stehen immer alleine im Rahmen, also hat es
/*N*/ 				keinen Sinn, nach einem vorherigen/nachfolgenden
/*N*/ 				ContentNode zu suchen!
/*N*/ 	SwCntntNode *pCntntNd;
/*N*/ 	SwIndex aIdx( rWhere, -1 );
/*N*/ 	if ( (pCntntNd=(*this)[ rWhere ]->GetCntntNode()) != 0 )
/*N*/ 		pCntntNd->MakeFrms( rWhere, aIdx );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aIdx--;
/*N*/ 		if ( (pCntntNd=(*this)[aIdx]->GetCntntNode()) != 0 )
/*N*/ 		{
/*N*/ 			SwIndex aTmp( aIdx );
/*N*/ 			aIdx++;
/*N*/ 			pCntntNd->MakeFrms( aTmp, aIdx );
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return pNode;
/*N*/ }


/*N*/ Size SwOLENode::GetTwipSize() const
/*N*/ {
/*N*/ 	SvInPlaceObjectRef xRef( ((SwOLENode*)this)->aOLEObj.GetOleRef() );
/*N*/ 	Size aSz( xRef->GetVisArea().GetSize() );
/*N*/ 	const MapMode aDest( MAP_TWIP );
/*N*/ 	const MapMode aSrc ( xRef->GetMapUnit() );
/*N*/ 	return OutputDevice::LogicToLogic( aSz, aSrc, aDest );
/*N*/ }


/*N*/ SwCntntNode* SwOLENode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
/*N*/ {
/*N*/ 	// Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
/*N*/ 	SvPersist* p = pDoc->GetPersist();
/*N*/ 	if( !p )
/*N*/ 	{
/*?*/ 		ASSERT( pDoc->GetRefForDocShell(),
/*?*/ 						"wo ist die Ref-Klasse fuer die DocShell?")
/*?*/ 		p = new SwDocShell( pDoc, SFX_CREATE_MODE_INTERNAL );
/*?*/ 		*pDoc->GetRefForDocShell() = p;
/*?*/ 		p->DoInitNew( NULL );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Wir hauen das Ding auf SvPersist-Ebene rein
/*N*/ 	String aNewName( Sw3Io::UniqueName( p->GetStorage(), "Obj" ) );
/*N*/ 	SvPersist* pSrc = GetDoc()->GetPersist();
/*N*/ 
/*N*/ 	p->CopyObject( aOLEObj.aName, aNewName, pSrc );
/*N*/ 	SwOLENode* pOLENd = pDoc->GetNodes().MakeOLENode( rIdx, aNewName,
/*N*/ 									(SwGrfFmtColl*)pDoc->GetDfltGrfFmtColl(),
/*N*/ 									(SwAttrSet*)GetpSwAttrSet() );
/*N*/ 
/*N*/ 	pOLENd->SetChartTblName( GetChartTblName() );
/*N*/ 	pOLENd->SetAlternateText( GetAlternateText() );
/*N*/     pOLENd->SetContour( HasContour(), HasAutomaticContour() );
/*N*/ 
/*N*/ 	pOLENd->SetOLESizeInvalid( TRUE );
/*N*/ 	pDoc->SetOLEPrtNotifyPending();
/*N*/ 
/*N*/ 	return pOLENd;
/*N*/ }


/*N*/ BOOL SwOLENode::IsInGlobalDocSection() const
/*N*/ {
/*N*/ 	// suche den "Body Anchor"
/*N*/ 	ULONG nEndExtraIdx = GetNodes().GetEndOfExtras().GetIndex();
/*N*/ 	const SwNode* pAnchorNd = this;
/*N*/ 	do {
/*N*/ 		SwFrmFmt* pFlyFmt = pAnchorNd->GetFlyFmt();
/*N*/ 		if( !pFlyFmt )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 		const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
/*N*/ 		if( !rAnchor.GetCntntAnchor() )
/*N*/ 			return FALSE;
/*N*/ 
/*N*/ 		pAnchorNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
/*N*/ 	} while( pAnchorNd->GetIndex() < nEndExtraIdx );
/*N*/ 
/*N*/ 	const SwSectionNode* pSectNd = pAnchorNd->FindSectionNode();
/*N*/ 	if( !pSectNd )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	while( pSectNd )
/*N*/ 	{
/*N*/ 		pAnchorNd = pSectNd;
/*N*/ 		pSectNd = pAnchorNd->FindStartNode()->FindSectionNode();
/*N*/ 	}
/*N*/ 
/*N*/ 	// in pAnchorNd steht der zuletzt gefundene Section Node. Der muss
/*N*/ 	// jetzt die Bedingung fuers GlobalDoc erfuellen.
/*N*/ 	pSectNd = (SwSectionNode*)pAnchorNd;
/*N*/ 	return FILE_LINK_SECTION == pSectNd->GetSection().GetType() &&
/*N*/ 			pSectNd->GetIndex() > nEndExtraIdx;
/*N*/ }


/*N*/ BOOL SwOLENode::IsOLEObjectDeleted() const
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if( aOLEObj.pOLERef && aOLEObj.pOLERef->Is() )
/*N*/ 	{
/*N*/ 		SvPersist* p = GetDoc()->GetPersist();
/*N*/ 		if( p )		// muss da sein
/*N*/ 		{
/*N*/ 			SvInfoObjectRef aRef( p->Find( aOLEObj.aName ) );
/*N*/ 			if( aRef.Is() )
/*N*/ 				bRet = aRef->IsDeleted();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ SwOLEObj::SwOLEObj( SvInPlaceObject *pObj ) :
/*N*/ 	pOLERef( new SvInPlaceObjectRef( pObj ) ),
/*N*/ 	pOLENd( 0 )
/*N*/ {
/*N*/ }


/*N*/ SwOLEObj::SwOLEObj( const String &rString ) :
/*N*/ 	pOLERef( 0 ),
/*N*/ 	pOLENd( 0 ),
/*N*/ 	aName( rString )
/*N*/ {
/*N*/ }


/*N*/ SwOLEObj::~SwOLEObj()
/*N*/ {
/*N*/ 	if( pOLERef && pOLERef->Is() )
/*N*/ 		//#41499# Kein DoClose(). Beim Beenden ruft der Sfx ein DoClose auf
/*N*/ 		//die offenen Objekte. Dadurch wird ggf. eine temp. OLE-Grafik wieder
/*N*/ 		//in eine Grafik gewandelt. Der OLE-Node wird zerstoert. Das DoClose
/*N*/ 		//wueder in das leere laufen, weil das Objekt bereits im DoClose steht.
/*N*/ 		//Durch das remove unten waere das DoClose aber nicht vollstaendig.
/*N*/ 		(*pOLERef)->GetProtocol().Reset();
/*N*/ 	delete pOLERef;
/*N*/ 	// Object aus dem Storage removen!!
/*N*/ 	if( pOLENd && !pOLENd->GetDoc()->IsInDtor() )	//NIcht notwendig im DTor (MM)
/*N*/ 	{
/*N*/ 		SvPersist* p = pOLENd->GetDoc()->GetPersist();
/*N*/ 		if( p )		// muss er existieren ?
/*N*/ 			p->Remove( aName );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pOLELRU_Cache )
/*N*/ 	{
/*N*/ 		pOLELRU_Cache->RemovePtr( this );
/*N*/ 		if( !pOLELRU_Cache->Count() )
/*N*/ 			// der letzte macht die Tuer zu
/*N*/ 			delete pOLELRU_Cache, pOLELRU_Cache = 0;
/*N*/ 	}
/*N*/ }


/*N*/ void SwOLEObj::SetNode( SwOLENode* pNode )
/*N*/ {
/*N*/ 	pOLENd = pNode;
/*N*/ 	if ( pOLERef && !aName.Len() )
/*N*/ 	{
/*N*/ 		SwDoc* pDoc = pNode->GetDoc();
/*N*/ 
/*N*/ 		// Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
/*N*/ 		SvPersist* p = pDoc->GetPersist();
/*N*/ 		if( !p )
/*N*/ 		{
/*?*/ 			ASSERT( !this, "warum wird hier eine DocShell angelegt?" );
/*?*/ 			p = new SwDocShell( pDoc, SFX_CREATE_MODE_INTERNAL );
/*?*/ 			p->DoInitNew( NULL );
/*N*/ 		}
/*N*/ 		// Wir hauen das Ding auf SvPersist-Ebene rein
/*N*/ 		aName = Sw3Io::UniqueName( p->GetStorage(), "Obj" );
/*N*/ 		SvInfoObjectRef refObj = new SvEmbeddedInfoObject( *pOLERef, aName );
/*N*/ 
/*N*/ 		ULONG nLstLen = p->GetObjectList() ? p->GetObjectList()->Count() : 0;
/*N*/ 		if ( !p->Move( refObj, aName ) ) // Eigentuemer Uebergang!
/*?*/ 			refObj.Clear();
/*N*/ 		else if( nLstLen == p->GetObjectList()->Count() )
/*N*/ 		{
/*N*/ 			// Task 91051: Info-Object not insertet, so it exist another
/*N*/ 			//				InfoObject to the same Object and the Objects
/*N*/ 			//				is stored in the persist. This InfoObject we must
/*N*/ 			//				found.
/*?*/ 			p->Insert( refObj );
/*N*/ 		}
/*N*/ 		ASSERT( refObj.Is(), "InsertObject failed" );
/*N*/ 	}
/*N*/ }

/*N*/ BOOL SwOLEObj::IsOleRef() const
/*N*/ {
/*N*/ 	return pOLERef && pOLERef->Is();
/*N*/ }

/*N*/ SvInPlaceObjectRef SwOLEObj::GetOleRef()
/*N*/ {
/*N*/ 	if( !pOLERef || !pOLERef->Is() )
/*N*/ 	{
/*N*/ 		SvPersist* p = pOLENd->GetDoc()->GetPersist();
/*N*/ 		ASSERT( p, "kein SvPersist vorhanden" );
/*N*/ 
/*N*/ 		// MIB 18.5.97: DIe Base-URL wird jetzt gesetzt, damit Plugins
/*N*/ 		// nach dem Laden und vor dem Aktivieren des Frames korrekt
/*N*/ 		// geladen werden koennen
/*N*/ 		String sBaseURL( ::binfilter::StaticBaseUrl::GetBaseURL() );
/*N*/ 		const SwDocShell *pDocSh = pOLENd->GetDoc()->GetDocShell();
/*N*/ 		const SfxMedium *pMedium;
/*N*/ 		if( pDocSh && 0 != (pMedium = pDocSh->GetMedium()) &&
/*N*/ 			pMedium->GetName() != sBaseURL )
/*N*/ 				::binfilter::StaticBaseUrl::SetBaseURL( pMedium->GetName() );
/*N*/ 
/*N*/ 		SvPersistRef xObj = p->GetObject( aName );
/*N*/ 		ASSERT( !pOLERef || !pOLERef->Is(),
/*N*/ 				"rekursiver Aufruf von GetOleRef() ist nicht erlaubt" )
/*N*/ 
/*N*/ 		::binfilter::StaticBaseUrl::SetBaseURL( sBaseURL );
/*N*/ 
/*N*/ 		if ( !xObj.Is() )
/*N*/ 		{
/*?*/ 			//Das Teil konnte nicht geladen werden (wahrsch. Kaputt).
/*?*/ 			Rectangle aArea;
/*?*/ 			SwFrm *pFrm = pOLENd->GetFrm();
/*?*/ 			if ( pFrm )
/*?*/ 			{
/*?*/ 				Size aSz( pFrm->Frm().SSize() );
/*?*/ 				const MapMode aSrc ( MAP_TWIP );
/*?*/ 				const MapMode aDest( MAP_100TH_MM );
/*?*/ 				aSz = OutputDevice::LogicToLogic( aSz, aSrc, aDest );
/*?*/ 				aArea.SetSize( aSz );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				aArea.SetSize( Size( 5000,  5000 ) );
/*?*/ 			xObj = new SvDeathObject( aArea );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pOLERef )
/*?*/ 			*pOLERef = &xObj;
/*N*/ 		else
/*N*/ 			pOLERef = new SvInPlaceObjectRef( xObj );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !pOLELRU_Cache )
/*N*/ 		pOLELRU_Cache = new SwOLELRUCache;
/*N*/ 
/*N*/ 	pOLELRU_Cache->Insert( *this );
/*N*/ 
/*N*/ 	return *pOLERef;
/*N*/ }


/*N*/ BOOL SwOLEObj::RemovedFromLRU()
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	return bRet;
/*N*/ }

/*N*/ SwOLELRUCache::SwOLELRUCache()
/*N*/ 	: SvPtrarr( 64, 16 ),
/*N*/ 	::utl::ConfigItem( OUString::createFromAscii( "Office.Common/Cache" )),
/*N*/ 	bInUnload( sal_False ),
/*N*/ 	nLRU_InitSize( 20 )
/*N*/ {
/*N*/ 	EnableNotification( GetPropertyNames() );
/*N*/ 	Load();
/*N*/ }

/*N*/ ::com::sun::star::uno::Sequence< ::rtl::OUString > SwOLELRUCache::GetPropertyNames()
/*N*/ {
/*N*/ 	Sequence< OUString > aNames( 1 );
/*N*/ 	OUString* pNames = aNames.getArray();
/*N*/ 	pNames[0] = OUString::createFromAscii( "Writer/OLE_Objects" );
/*N*/ 	return aNames;
/*N*/ }



/*N*/ void SwOLELRUCache::Load()
/*N*/ {
/*N*/ 	Sequence< OUString > aNames( GetPropertyNames() );
/*N*/ 	Sequence< Any > aValues = GetProperties( aNames );
/*N*/ 	const Any* pValues = aValues.getConstArray();
/*N*/ 	DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
/*N*/ 	if( aValues.getLength() == aNames.getLength() &&
/*N*/ 		pValues->hasValue() )
/*N*/ 	{
/*N*/ 		sal_Int32 nVal;
/*N*/ 		*pValues >>= nVal;
/*N*/ 		if( 20 > nVal )
/*N*/ 			nVal = 20;
/*N*/ 
/*N*/ 		if( !bInUnload )
/*N*/ 		{
/*N*/ 			USHORT nPos = SvPtrarr::Count();
/*N*/ 			if( nVal < nLRU_InitSize && nPos > nVal )
/*N*/ 			{
/*N*/ 				// remove the last entries
/*N*/ 				while( nPos > nVal )
/*N*/ 				{
/*?*/ 					SwOLEObj* pObj = (SwOLEObj*) SvPtrarr::GetObject( --nPos );
/*?*/ 					if( pObj->RemovedFromLRU() )
/*?*/ 						SvPtrarr::Remove( nPos );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		nLRU_InitSize = (USHORT)nVal;
/*N*/ 	}
/*N*/ }

/*N*/ void SwOLELRUCache::Insert( SwOLEObj& rObj )
/*N*/ {
/*N*/ 	if( !bInUnload )
/*N*/ 	{
/*N*/ 		SwOLEObj* pObj = &rObj;
/*N*/ 		USHORT nPos = SvPtrarr::GetPos( pObj );
/*N*/ 		if( nPos )	// der auf der 0. Pos muss nicht verschoben werden!
/*N*/ 		{
/*N*/ 			if( USHRT_MAX != nPos )
/*?*/ 				SvPtrarr::Remove( nPos );
/*N*/ 
/*N*/ 			SvPtrarr::Insert( pObj, 0 );
/*N*/ 
/*N*/ 			nPos = SvPtrarr::Count();
/*N*/ 			while( nPos > nLRU_InitSize )
/*N*/ 			{
/*?*/ 				pObj = (SwOLEObj*) SvPtrarr::GetObject( --nPos );
/*?*/ 				if( pObj->RemovedFromLRU() )
/*?*/ 					SvPtrarr::Remove( nPos );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef DBG_UTIL
/*N*/ 	else
/*N*/ 	{
/*?*/ 		SwOLEObj* pObj = &rObj;
/*?*/ 		USHORT nPos = SvPtrarr::GetPos( pObj );
/*?*/ 		ASSERT( USHRT_MAX != nPos, "Insert a new OLE object into a looked cache" );
/*?*/ 	}
/*N*/ #endif
/*N*/ }

/*N*/ void SwOLELRUCache::Remove( SwOLEObj& rObj )
/*N*/ {
/*N*/ 	if( !bInUnload )
/*N*/ 	{
/*N*/ 		USHORT nPos = SvPtrarr::GetPos( &rObj );
/*N*/ 		if( USHRT_MAX != nPos && rObj.RemovedFromLRU() )
/*N*/ 			SvPtrarr::Remove( nPos );
/*N*/ 	}
/*N*/ }



}
