/*************************************************************************
 *
 *  $RCSfile: ndole.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:10:58 $
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

#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
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

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
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
#ifndef _SWDOCSH_HXX
#include <docsh.hxx>
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

#include <comphelper/classids.hxx>
#include <vcl/graph.hxx>
#include <sot/formats.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/filter.hxx>
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

SwOLELRUCache* pOLELRU_Cache = 0;

class SwOLEListener_Impl : public ::cppu::WeakImplHelper1< embed::XStateChangeListener >
{
    SwOLEObj* mpObj;
public:
    SwOLEListener_Impl( SwOLEObj* pObj );
    void Release();
    virtual void SAL_CALL changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);
};

SwOLEListener_Impl::SwOLEListener_Impl( SwOLEObj* pObj )
: mpObj( pObj )
{
    if ( mpObj->IsOleRef() && mpObj->GetOleRef()->getCurrentState() == embed::EmbedStates::RUNNING )
    {
        pOLELRU_Cache->Insert( *mpObj );
    }
}

void SAL_CALL SwOLEListener_Impl::changingState( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::embed::WrongStateException, ::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SwOLEListener_Impl::stateChanged( const ::com::sun::star::lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpObj && nOldState == embed::EmbedStates::LOADED && nNewState == embed::EmbedStates::RUNNING )
    {
        if( !pOLELRU_Cache )
            pOLELRU_Cache = new SwOLELRUCache;
        pOLELRU_Cache->Insert( *mpObj );
    }
    else if ( mpObj && nNewState == embed::EmbedStates::LOADED && nOldState == embed::EmbedStates::RUNNING )
    {
        if ( pOLELRU_Cache )
            pOLELRU_Cache->Remove( *mpObj );
    }
}

void SwOLEListener_Impl::Release()
{
    if ( mpObj && pOLELRU_Cache )
        pOLELRU_Cache->Remove( *mpObj );
    mpObj=0;
    release();
}

void SAL_CALL SwOLEListener_Impl::disposing( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpObj && pOLELRU_Cache )
        pOLELRU_Cache->Remove( *mpObj );
}

// --------------------
// SwEmbedObjectLink
// --------------------
// TODO/LATER: actually SwEmbedObjectLink should be used here, but because different objects are used to control
//             embedded object different link objects with the same functionality had to be implemented

class SwEmbedObjectLink : public sfx2::SvBaseLink
{
    SwOLENode*          pOleNode;

public:
                        SwEmbedObjectLink(SwOLENode* pNode);
    virtual             ~SwEmbedObjectLink();

    virtual void        Closed();
    virtual void        DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    sal_Bool            Connect() { return GetRealObject() != NULL; }
};

// -----------------------------------------------------------------------------

SwEmbedObjectLink::SwEmbedObjectLink(SwOLENode* pNode):
    ::sfx2::SvBaseLink( ::sfx2::LINKUPDATE_ONCALL, SOT_FORMATSTR_ID_SVXB ),
    pOleNode(pNode)
{
    SetSynchron( FALSE );
}

// -----------------------------------------------------------------------------

SwEmbedObjectLink::~SwEmbedObjectLink()
{
}

// -----------------------------------------------------------------------------

void SwEmbedObjectLink::DataChanged( const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue )
{
    if ( !pOleNode->UpdateLinkURL_Impl() )
    {
        // the link URL was not changed
        uno::Reference< embed::XEmbeddedObject > xObject = pOleNode->GetOLEObj().GetOleRef();
        OSL_ENSURE( xObject.is(), "The object must exist always!\n" );
        if ( xObject.is() )
        {
            // let the object reload the link
            // TODO/LATER: reload call could be used for this case

            try
            {
                sal_Int32 nState = xObject->getCurrentState();
                if ( nState == embed::EmbedStates::LOADED )
                    xObject->changeState( embed::EmbedStates::RUNNING );
                else
                {
                    // in some cases the linked file probably is not locked so it could be changed
                    xObject->changeState( embed::EmbedStates::LOADED );
                    xObject->changeState( nState );
                }
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    pOleNode->GetNewReplacement();
    // Initiate repainting
    // pObj->SetChanged();
}

// -----------------------------------------------------------------------------

void SwEmbedObjectLink::Closed()
{
    pOleNode->BreakFileLink_Impl();
    SvBaseLink::Closed();
}


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
    bOLESizeInvalid( FALSE ),
    mpObjectLink( NULL )
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
    bOLESizeInvalid( FALSE ),
    mpObjectLink( NULL )
{
    aOLEObj.SetNode( this );
}

SwOLENode::~SwOLENode()
{
    DisconnectFileLink_Impl();
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

        uno::Reference < container::XChild > xChild( aOLEObj.xOLERef.GetObject(), uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( p->GetModel() );

        DBG_ASSERT( aOLEObj.aName.Len(), "No object name!" );
        ::rtl::OUString aObjName;
        if ( !p->GetEmbeddedObjectContainer().InsertEmbeddedObject( aOLEObj.xOLERef.GetObject(), aObjName ) )
        {
            if ( xChild.is() )
                xChild->setParent( 0 );
            DBG_ERROR( "InsertObject failed" );
        }
        else
        {
            aOLEObj.aName = aObjName;
            aOLEObj.xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aObjName );
            CheckFileLink_Impl();
        }
    }

    return TRUE;
}

// OLE object is transported into UNDO area
BOOL SwOLENode::SavePersistentData()
{
    if( aOLEObj.xOLERef.is() )
    {
        comphelper::EmbeddedObjectContainer* pCnt = aOLEObj.xOLERef.GetContainer();

#if OSL_DEBUG_LEVEL > 0
        SfxObjectShell* p = GetDoc()->GetPersist();
        DBG_ASSERT( p, "No document!" );
        if( p )
        {
            comphelper::EmbeddedObjectContainer& rCnt = p->GetEmbeddedObjectContainer();
            OSL_ENSURE( !pCnt || &rCnt == pCnt, "The helper is assigned to unexpected container!\n" );
        }
#endif

        if ( pCnt && pCnt->HasEmbeddedObject( aOLEObj.aName ) )
        {
            uno::Reference < container::XChild > xChild( aOLEObj.xOLERef.GetObject(), uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( 0 );

            pCnt->RemoveEmbeddedObject( aOLEObj.aName );

            // TODO/LATER: aOLEObj.aName has no meaning here, since the undo container contains the object
            // by different name, in future it might makes sence that the name is transported here.
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

    DisconnectFileLink_Impl();

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

    // set parent if XChild is supported
    //!! needed to supply Math objects with a valid reference device
    Reference< container::XChild > xChild( pNode->GetOLEObj().GetObject().GetObject(), UNO_QUERY );
    if (xChild.is())
    {
        SwDocShell *pDocSh = GetDoc()->GetDocShell();
        if (pDocSh)
            xChild->setParent( pDocSh->GetModel() );
    }

    return pNode;
}


SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
    const String &rName, SwGrfFmtColl* pGrfColl, SwAttrSet* pAutoAttr )
{
    ASSERT( pGrfColl,"SwNodes::MakeOLENode: Formatpointer ist 0." );

    SwOLENode *pNode =
        new SwOLENode( rWhere, rName, pGrfColl, pAutoAttr );

    // set parent if XChild is supported
    //!! needed to supply Math objects with a valid reference device
    Reference< container::XChild > xChild( pNode->GetOLEObj().GetObject().GetObject(), UNO_QUERY );
    if (xChild.is())
    {
        SwDocShell *pDocSh= GetDoc()->GetDocShell();
        if (pDocSh)
            xChild->setParent( pDocSh->GetModel() );
    }

    return pNode;
}

Size SwOLENode::GetTwipSize() const
{
    uno::Reference < embed::XEmbeddedObject > xObj = ((SwOLENode*)this)->aOLEObj.GetOleRef();

    // TODO/LEAN: getMapUnit may switch object to running state
    awt::Size aSize;
    try
    {
        aSize = xObj->getVisualAreaSize( nViewAspect );
    }
    catch( embed::NoVisualAreaSizeException& )
    {
        DBG_ASSERT( sal_False, "Can't get visual area size!" );
        aSize.Width = 5000;
        aSize.Height = 5000;
    }

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

    p->GetEmbeddedObjectContainer().CopyAndGetEmbeddedObject(
        pSrc->GetEmbeddedObjectContainer(),
        pSrc->GetEmbeddedObjectContainer().GetEmbeddedObject( aOLEObj.aName ),
        aNewName );

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

void SwOLENode::GetNewReplacement()
{
    if ( aOLEObj.xOLERef.is() )
        aOLEObj.xOLERef.UpdateReplacement();
}

sal_Bool SwOLENode::UpdateLinkURL_Impl()
{
    sal_Bool bResult = sal_False;

    if ( mpObjectLink )
    {
        String aNewLinkURL;
        GetDoc()->GetLinkManager().GetDisplayNames( mpObjectLink, 0, &aNewLinkURL, 0, 0 );
        if ( !aNewLinkURL.EqualsIgnoreCaseAscii( maLinkURL ) )
        {
            if ( !aOLEObj.xOLERef.is() )
                aOLEObj.GetOleRef();

            uno::Reference< embed::XEmbeddedObject > xObj = aOLEObj.xOLERef.GetObject();
            uno::Reference< embed::XCommonEmbedPersist > xPersObj( xObj, uno::UNO_QUERY );
            OSL_ENSURE( xPersObj.is(), "The object must exist!\n" );
            if ( xPersObj.is() )
            {
                try
                {
                    sal_Int32 nCurState = xObj->getCurrentState();
                    if ( nCurState != embed::EmbedStates::LOADED )
                        xObj->changeState( embed::EmbedStates::LOADED );

                    // TODO/LATER: there should be possible to get current mediadescriptor settings from the object
                    uno::Sequence< beans::PropertyValue > aArgs( 1 );
                    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                    aArgs[0].Value <<= ::rtl::OUString( aNewLinkURL );
                    xPersObj->reload( aArgs, uno::Sequence< beans::PropertyValue >() );

                    maLinkURL = aNewLinkURL;
                    bResult = sal_True;

                    if ( nCurState != embed::EmbedStates::LOADED )
                        xObj->changeState( nCurState );
                }
                catch( uno::Exception& )
                {}
            }

            if ( !bResult )
            {
                // TODO/LATER: return the old name to the link manager, is it possible?
            }
        }
    }

    return bResult;
}

void SwOLENode::BreakFileLink_Impl()
{
    SfxObjectShell* pPers = GetDoc()->GetPersist();

    if ( pPers )
    {
        uno::Reference< embed::XStorage > xStorage = pPers->GetStorage();
        if ( xStorage.is() )
        {
            try
            {
                uno::Reference< embed::XLinkageSupport > xLinkSupport( aOLEObj.GetOleRef(), uno::UNO_QUERY_THROW );
                xLinkSupport->breakLink( xStorage, aOLEObj.GetCurrentPersistName() );
                DisconnectFileLink_Impl();
                maLinkURL = String();
            }
            catch( uno::Exception& )
            {
            }
        }
    }
}

void SwOLENode::DisconnectFileLink_Impl()
{
    if ( mpObjectLink )
    {
        GetDoc()->GetLinkManager().Remove( mpObjectLink );
        mpObjectLink = NULL;
    }
}

void SwOLENode::CheckFileLink_Impl()
{
    if ( aOLEObj.xOLERef.GetObject().is() && !mpObjectLink )
    {
        try
        {
            uno::Reference< embed::XLinkageSupport > xLinkSupport( aOLEObj.xOLERef.GetObject(), uno::UNO_QUERY_THROW );
            if ( xLinkSupport->isLink() )
            {
                String aLinkURL = xLinkSupport->getLinkURL();
                if ( aLinkURL.Len() )
                {
                    // this is a file link so the model link manager should handle it
                    mpObjectLink = new SwEmbedObjectLink( this );
                    maLinkURL = aLinkURL;
                    GetDoc()->GetLinkManager().InsertFileLink( *mpObjectLink, OBJECT_CLIENT_OLE, aLinkURL, NULL, NULL );
                    mpObjectLink->Connect();
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }
}

SwOLEObj::SwOLEObj( const svt::EmbeddedObjectRef& xObj ) :
    xOLERef( xObj ),
    pOLENd( 0 ),
    pListener( 0 )
{
    xOLERef.Lock( TRUE );
    if ( xObj.is() )
    {
        pListener = new SwOLEListener_Impl( this );
        pListener->acquire();
        xObj->addStateChangeListener( pListener );
    }
}


SwOLEObj::SwOLEObj( const String &rString ) :
    pOLENd( 0 ),
    aName( rString ),
    pListener( 0 )
{
    xOLERef.Lock( TRUE );
}


SwOLEObj::~SwOLEObj()
{
    if( pListener )
    {
        if ( xOLERef.is() )
            xOLERef->removeStateChangeListener( pListener );
        pListener->Release();
    }

    if( pOLENd && !pOLENd->GetDoc()->IsInDtor() )
    {
        // if the model is not currently in destruction it means that this object should be removed from the model
        comphelper::EmbeddedObjectContainer* pCnt = xOLERef.GetContainer();

#if OSL_DEBUG_LEVEL > 0
        SfxObjectShell* p = pOLENd->GetDoc()->GetPersist();
        DBG_ASSERT( p, "No document!" );
        if( p )
        {
            comphelper::EmbeddedObjectContainer& rCnt = p->GetEmbeddedObjectContainer();
            OSL_ENSURE( !pCnt || &rCnt == pCnt, "The helper is assigned to unexpected container!\n" );
        }
#endif

        if ( pCnt && pCnt->HasEmbeddedObject( aName ) )
        {
            uno::Reference < container::XChild > xChild( xOLERef.GetObject(), uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( 0 );

            // not already removed by deleting the object
            xOLERef.AssignToContainer( 0, aName );

            // unlock object so that object can be closed in RemoveEmbeddedObject
            // successful closing of the object will automatically clear the reference then
            xOLERef.Lock(FALSE);

            // Always remove object from conteiner it is connected to
            pCnt->RemoveEmbeddedObject( aName );
        }

    }

    if ( xOLERef.is() )
        // in case the object wasn't closed: release it
        // in case the object was not in the container: it's still locked, try to close
        xOLERef.Clear();
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
        uno::Reference < container::XChild > xChild( xOLERef.GetObject(), uno::UNO_QUERY );
        if ( xChild.is() && xChild->getParent() != p->GetModel() )
            // it is possible that the parent was set already
            xChild->setParent( p->GetModel() );
        if (!p->GetEmbeddedObjectContainer().InsertEmbeddedObject( xOLERef.GetObject(), aObjName ) )
        {
            DBG_ERROR( "InsertObject failed" );
        if ( xChild.is() )
            xChild->setParent( 0 );
        }
        else
            xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aObjName );

        ( (SwOLENode*)pOLENd )->CheckFileLink_Impl(); // for this notification nonconst access is required

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

        uno::Reference < embed::XEmbeddedObject > xObj = p->GetEmbeddedObjectContainer().GetEmbeddedObject( aName );
        ASSERT( !xOLERef.is(), "rekursiver Aufruf von GetOleRef() ist nicht erlaubt" )

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
            ::rtl::OUString aName;
            xObj = p->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_DUMMY_CLASSID ).GetByteSequence(), aName );
        }
        // else
        {
            xOLERef.Assign( xObj, xOLERef.GetViewAspect() );
            xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aName );
            pListener = new SwOLEListener_Impl( this );
            pListener->acquire();
            xObj->addStateChangeListener( pListener );
        }

        ( (SwOLENode*)pOLENd )->CheckFileLink_Impl(); // for this notification nonconst access is required
    }
    else if ( xOLERef->getCurrentState() == embed::EmbedStates::RUNNING )
    {
        // move object to first position in cache
        if( !pOLELRU_Cache )
            pOLELRU_Cache = new SwOLELRUCache;
        pOLELRU_Cache->Insert( *this );
    }

    return xOLERef.GetObject();
}

svt::EmbeddedObjectRef& SwOLEObj::GetObject()
{
    GetOleRef();
    return xOLERef;
}

BOOL SwOLEObj::UnloadObject()
{
    BOOL bRet = TRUE;
    //Nicht notwendig im Doc DTor (MM)
    //ASSERT( pOLERef && pOLERef->Is() && 1 < (*pOLERef)->GetRefCount(),
    //        "Falscher RefCount fuers Unload" );
    const SwDoc* pDoc;

    sal_Int32 nState = xOLERef.is() ? xOLERef->getCurrentState() : embed::EmbedStates::LOADED;
    DBG_ASSERT( nState != embed::EmbedStates::LOADED, "Strange state of cached object!" );
    BOOL bIsActive = ( nState != embed::EmbedStates::LOADED && nState != embed::EmbedStates::RUNNING );
    if( pOLENd && nState != embed::EmbedStates::LOADED &&
        !( pDoc = pOLENd->GetDoc())->IsInDtor() && !bIsActive &&
        embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY != xOLERef->getStatus( pOLENd->GetAspect() ) )
    {
        SfxObjectShell* p = pDoc->GetPersist();
        if( p )
        {
            if( pDoc->IsPurgeOLE() )
            {
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

                    // setting object to loaded state will remove it from cache
                    xOLERef->changeState( embed::EmbedStates::LOADED );
                }
                catch ( uno::Exception& )
                {
                    bRet = FALSE;
                }
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

void SwOLELRUCache::Notify( const com::sun::star::uno::Sequence< rtl::OUString>& rPropertyNames )
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
    if( aValues.getLength() == aNames.getLength() && pValues->hasValue() )
    {
        sal_Int32 nVal;
        *pValues >>= nVal;
        //if( 20 > nVal )
        //    nVal = 20;

        {
            if( nVal < nLRU_InitSize )
            {
                // size of cache has been changed
                USHORT nCount = SvPtrarr::Count();
                USHORT nPos = nCount;

                // try to remove the last entries until new maximum size is reached
                while( nCount > nVal )
                {
                    SwOLEObj* pObj = (SwOLEObj*) SvPtrarr::GetObject( --nPos );
                    if ( pObj->UnloadObject() )
                        nCount--;
                    if ( !nPos )
                        break;
                }
            }
        }

        nLRU_InitSize = (USHORT)nVal;
    }
}

void SwOLELRUCache::Insert( SwOLEObj& rObj )
{
    SwOLEObj* pObj = &rObj;
    USHORT nPos = SvPtrarr::GetPos( pObj );
    if( nPos )
    {
        // object is currently not the first in cache
        if( USHRT_MAX != nPos )
            SvPtrarr::Remove( nPos );

        SvPtrarr::Insert( pObj, 0 );

        // try to remove objects if necessary (of course not the freshly inserted one at nPos=0)
        USHORT nCount = SvPtrarr::Count();
        nPos = nCount-1;
        while( nPos && nCount > nLRU_InitSize )
        {
            pObj = (SwOLEObj*) SvPtrarr::GetObject( nPos-- );
            if ( pObj->UnloadObject() )
                nCount--;
        }
    }
}

void SwOLELRUCache::Remove( SwOLEObj& rObj )
{
    USHORT nPos = SvPtrarr::GetPos( &rObj );
    if ( nPos != 0xFFFF )
        SvPtrarr::Remove( nPos );
    if( !Count() )
        DELETEZ( pOLELRU_Cache );
}
