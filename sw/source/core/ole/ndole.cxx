/*************************************************************************
 *
 *  $RCSfile: ndole.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:08:55 $
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

#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDMISC_HPP_
#include <com/sun/star/embed/EmbedMisc.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

#include <cppuhelper/implbase2.hxx>
#include <toolkit/helper/vclunohelper.hxx>

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

#include <vcl/graph.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star;


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
                    const svt::EmbeddedObjectRef& xObj,
                    SwGrfFmtColl *pGrfColl,
                    SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_OLENODE, pGrfColl, pAutoAttr ),
    aOLEObj( xObj ),
    pGraphic(0),
    nViewAspect( embed::Aspects::MSOLE_CONTENT ),
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
    pGraphic(0),
    nViewAspect( embed::Aspects::MSOLE_CONTENT ),
    bOLESizeInvalid( FALSE )
{
    aOLEObj.SetNode( this );
}

SwOLENode::~SwOLENode()
{
    delete pGraphic;
}

Graphic* SwOLENode::GetGraphic()
{
    if ( aOLEObj.GetOleRef().is() )
        return aOLEObj.xOLERef.GetGraphic();
    return pGraphic;
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
    DBG_ASSERT( aOLEObj.GetOleRef().is(), "No object to restore!" );
    if ( aOLEObj.xOLERef.is() )
    {
        // Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
        SfxObjectShell* p = GetDoc()->GetPersist();
        if( !p )
        {
            // TODO/LATER: reicht hier nicht ein EmbeddedObjectContainer? Was passiert mit
            // diesem Dokument?
            ASSERT( !this, "warum wird hier eine DocShell angelegt?" );
            p = new SwDocShell( GetDoc(), SFX_CREATE_MODE_INTERNAL );
            p->DoInitNew( NULL );
        }

        DBG_ASSERT( aOLEObj.aName.Len(), "No object name!" );
        ::rtl::OUString aObjName;
        if ( !p->GetEmbeddedObjectContainer().InsertEmbeddedObject( aOLEObj.xOLERef.GetObject(), aObjName ) )
        {
            DBG_ERROR( "InsertObject failed" );
        }
        else
        {
            aOLEObj.aName = aObjName;
            // TODO/LATER: allow to work in loaded state
            svt::EmbeddedObjectRef::TryRunningState( aOLEObj.xOLERef.GetObject() );
            aOLEObj.xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aObjName );
        }
    }

// muss das sein????
//  if( pOLELRU_Cache )
//      pOLELRU_Cache->RemovePtr( &aOLEObj );
    return TRUE;
}

// OLE object is transported into UNDO area
BOOL SwOLENode::SavePersistentData()
{
    if( aOLEObj.xOLERef.is() )
    {
        SfxObjectShell* p = GetDoc()->GetPersist();
        if( p )     // muss da sein
        {
            p->GetEmbeddedObjectContainer().RemoveEmbeddedObject( aOLEObj.aName);
            aOLEObj.xOLERef.AssignToContainer( 0, aOLEObj.aName );
            try
            {
                // "unload" object
                aOLEObj.xOLERef->changeState( embed::EmbedStates::LOADED );
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    if( SwOLEObj::pOLELRU_Cache )
        SwOLEObj::pOLELRU_Cache->RemovePtr( &aOLEObj );

    return TRUE;
}


SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
                    const svt::EmbeddedObjectRef& xObj,
                                    SwGrfFmtColl* pGrfColl,
                                    SwAttrSet* pAutoAttr )
{
    ASSERT( pGrfColl,"SwNodes::MakeOLENode: Formatpointer ist 0." );

    SwOLENode *pNode =
        new SwOLENode( rWhere, xObj, pGrfColl, pAutoAttr );

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
    uno::Reference < embed::XEmbeddedObject > xObj = ((SwOLENode*)this)->aOLEObj.GetOleRef();
    uno::Reference < embed::XVisualObject > xVis( xObj, uno::UNO_QUERY );
    awt::Size aSize = xVis->getVisualAreaSize( nViewAspect );
    Size aSz( aSize.Width, aSize.Height );
    const MapMode aDest( MAP_TWIP );
    const MapMode aSrc ( VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( nViewAspect ) ) );
    return OutputDevice::LogicToLogic( aSz, aSrc, aDest );
}

SwCntntNode* SwOLENode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
    SfxObjectShell* p = pDoc->GetPersist();
    if( !p )
    {
        // TODO/LATER: reicht hier nicht ein EmbeddedObjectContainer? Was passiert mit
        // diesem Dokument?
        ASSERT( pDoc->GetRefForDocShell(),
                        "wo ist die Ref-Klasse fuer die DocShell?")
        p = new SwDocShell( pDoc, SFX_CREATE_MODE_INTERNAL );
        *pDoc->GetRefForDocShell() = p;
        p->DoInitNew( NULL );
    }

    // Wir hauen das Ding auf SvPersist-Ebene rein
    // TODO/LATER: check if using the same naming scheme for all apps works here
    ::rtl::OUString aNewName/*( Sw3Io::UniqueName( p->GetStorage(), "Obj" ) )*/;
    SfxObjectShell* pSrc = GetDoc()->GetPersist();

    p->GetEmbeddedObjectContainer().CopyEmbeddedObject( pSrc->GetEmbeddedObjectContainer().GetEmbeddedObject( aOLEObj.aName ), aNewName );
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
    if( aOLEObj.xOLERef.is() )
    {
        SfxObjectShell* p = GetDoc()->GetPersist();
        if( p )     // muss da sein
        {
            return p->GetEmbeddedObjectContainer().HasEmbeddedObject( aOLEObj.aName );
            //SvInfoObjectRef aRef( p->Find( aOLEObj.aName ) );
            //if( aRef.Is() )
            //    bRet = aRef->IsDeleted();
        }
    }
    return bRet;
}


SwOLEObj::SwOLEObj( const svt::EmbeddedObjectRef& xObj ) :
    xOLERef( xObj ),
    pOLENd( 0 )
{
    xOLERef.Lock( TRUE );
}


SwOLEObj::SwOLEObj( const String &rString ) :
    pOLENd( 0 ),
    aName( rString )
{
    xOLERef.Lock( TRUE );
}


SwOLEObj::~SwOLEObj()
{
    xOLERef.Clear();

    // Object aus dem Storage removen!!
    if( pOLENd && !pOLENd->GetDoc()->IsInDtor() )   //NIcht notwendig im DTor (MM)
    {
        SfxObjectShell* p = pOLENd->GetDoc()->GetPersist();
        DBG_ASSERT( p, "No document!" );
        if( p )     // muss er existieren ?
        {
            comphelper::EmbeddedObjectContainer& rCnt = p->GetEmbeddedObjectContainer();
            if ( rCnt.HasEmbeddedObject( aName ) )
            {
                // not already removed by deleting the object
                xOLERef.AssignToContainer( 0, aName );
                rCnt.RemoveEmbeddedObject( aName );
            }
        }
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
    if ( !aName.Len() )
    {
        SwDoc* pDoc = pNode->GetDoc();

        // Falls bereits eine SvPersist-Instanz existiert, nehmen wir diese
        SfxObjectShell* p = pDoc->GetPersist();
        if( !p )
        {
            // TODO/LATER: reicht hier nicht ein EmbeddedObjectContainer? Was passiert mit
            // diesem Dokument?
            ASSERT( !this, "warum wird hier eine DocShell angelegt?" );
            p = new SwDocShell( pDoc, SFX_CREATE_MODE_INTERNAL );
            p->DoInitNew( NULL );
        }

        ::rtl::OUString aObjName;
        if (!p->GetEmbeddedObjectContainer().InsertEmbeddedObject( xOLERef.GetObject(), aObjName ) )
        {
            DBG_ERROR( "InsertObject failed" );
        }
        else
            xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aObjName );

        aName = aObjName;
    }
}

BOOL SwOLEObj::IsOleRef() const
{
    return xOLERef.is();
}

uno::Reference < embed::XEmbeddedObject > SwOLEObj::GetOleRef()
{
    if( !xOLERef.is() )
    {
        SfxObjectShell* p = pOLENd->GetDoc()->GetPersist();
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

        uno::Reference < embed::XEmbeddedObject > xObj = p->GetEmbeddedObjectContainer().GetEmbeddedObject( aName );
        ASSERT( !xOLERef.is(), "rekursiver Aufruf von GetOleRef() ist nicht erlaubt" )

        INetURLObject::SetBaseURL( sBaseURL );

        if ( !xObj.is() )
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
            // TODO/LATER: set replacement graphic for dead object
            // It looks as if it should work even without the object, because the replace will be generated automatically
            // The only possible problem might be that we will try to create the object again and again
            //xObj = new SvDeathObject( aArea );
            // It would be better to make the code fit for the fact that no object is available!
            // Of course we also must prevent permanent calls to this function
        }
        else
        {
            xOLERef.Assign( xObj, xOLERef.GetViewAspect() );
            xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aName );
        }
    }

    // TODO/LATER: allow loaded state
    svt::EmbeddedObjectRef::TryRunningState( xOLERef.GetObject() );

    if( !pOLELRU_Cache )
        pOLELRU_Cache = new SwOLELRUCache;

    pOLELRU_Cache->Insert( *this );

    return xOLERef.GetObject();
}

svt::EmbeddedObjectRef& SwOLEObj::GetObject()
{
    GetOleRef();
    return xOLERef;
}

// void SwOLEObj::Unload()
// {
//     if( pOLELRU_Cache )
//      pOLELRU_Cache->Remove( *this );
// }

BOOL SwOLEObj::RemovedFromLRU()
{
    BOOL bRet = TRUE;
    //Nicht notwendig im Doc DTor (MM)
    //ASSERT( pOLERef && pOLERef->Is() && 1 < (*pOLERef)->GetRefCount(),
    //        "Falscher RefCount fuers Unload" );
    const SwDoc* pDoc;

    sal_Int32 nState = xOLERef.is() ? xOLERef->getCurrentState() : embed::EmbedStates::LOADED;
    BOOL bIsActive = ( nState != embed::EmbedStates::LOADED && nState != embed::EmbedStates::RUNNING );
    if( xOLERef.is() && pOLENd &&
        !( pDoc = pOLENd->GetDoc())->IsInDtor() &&
        embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY != xOLERef->getStatus( pOLENd->GetAspect() ) &&
        //1 < (*pOLERef)->GetRefCount() &&
        !bIsActive )
        //!(*pOLERef)->GetProtocol().IsConnect() &&
        //!(*pOLERef)->GetProtocol().IsInPlaceActive() )
    {
        SfxObjectShell* p = pDoc->GetPersist();
        if( p )
        {
            if( pDoc->IsPurgeOLE() )
            {
                if ( xOLERef->getCurrentState() != embed::EmbedStates::LOADED )
                {
                    pOLELRU_Cache->SetInUnload( TRUE );

                    try
                    {
                        uno::Reference < util::XModifiable > xMod( xOLERef->getComponent(), uno::UNO_QUERY );
                        if( xMod.is() && xMod->isModified() )
                        {
                            uno::Reference < embed::XEmbedPersist > xPers( xOLERef.GetObject(), uno::UNO_QUERY );
                            if ( xPers.is() )
                                xPers->storeOwn();
                            else
                                DBG_ERROR("Modified object without persistance in cache!");
                        }

                        xOLERef->changeState( embed::EmbedStates::LOADED );
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }

                pOLELRU_Cache->SetInUnload( FALSE );
            }
            else
                bRet = FALSE;
        }
    }
    return bRet;
}


String SwOLEObj::GetDescription()
{
    String aResult;
    uno::Reference< embed::XEmbeddedObject > xEmbObj = GetOleRef();

    if ( xEmbObj.is() )
    {
        SvGlobalName aClassID( xEmbObj->getClassID() );
        if ( SotExchange::IsMath( aClassID ) )
            aResult = SW_RES(STR_FORMULA);
        else if ( SotExchange::IsChart( aClassID ) )
            aResult = SW_RES(STR_CHART);
        else
            aResult = SW_RES(STR_OLE);
    }

    return aResult;
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
