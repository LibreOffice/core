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

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <cppuhelper/implbase.hxx>

#include <tools/globname.hxx>
#include <hintids.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <sfx2/linkmgr.hxx>
#include <unotools/configitem.hxx>
#include <vcl/outdev.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <section.hxx>
#include <cntfrm.hxx>
#include <frmatr.hxx>
#include <ndole.hxx>
#include <viewsh.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <comphelper/classids.hxx>
#include <vcl/graph.hxx>
#include <sot/formats.hxx>
#include <vcl/graphicfilter.hxx>
#include <strings.hrc>
#include <svx/charthelper.hxx>
#include <comphelper/threadpool.hxx>
#include <atomic>
#include <deque>

using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star;

class SwOLELRUCache
    : private utl::ConfigItem
{
private:
    typedef std::deque<SwOLEObj *> OleObjects_t;
    OleObjects_t m_OleObjects;
    sal_Int32 m_nLRU_InitSize;
    static uno::Sequence< OUString > GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwOLELRUCache();

    virtual void Notify( const uno::Sequence<
                                OUString>& aPropertyNames ) override;
    void Load();

    void InsertObj( SwOLEObj& rObj );
    void RemoveObj( SwOLEObj& rObj );
};

static std::shared_ptr<SwOLELRUCache> g_pOLELRU_Cache;

class SwOLEListener_Impl : public ::cppu::WeakImplHelper< embed::XStateChangeListener >
{
    SwOLEObj* mpObj;
public:
    explicit SwOLEListener_Impl( SwOLEObj* pObj );
    void dispose();
    virtual void SAL_CALL changingState( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
    virtual void SAL_CALL stateChanged( const lang::EventObject& aEvent, ::sal_Int32 nOldState, ::sal_Int32 nNewState ) override;
    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) override;
};

SwOLEListener_Impl::SwOLEListener_Impl( SwOLEObj* pObj )
: mpObj( pObj )
{
    if ( mpObj->IsOleRef() && mpObj->GetOleRef()->getCurrentState() == embed::EmbedStates::RUNNING )
    {
        g_pOLELRU_Cache->InsertObj( *mpObj );
    }
}

void SAL_CALL SwOLEListener_Impl::changingState( const lang::EventObject&, ::sal_Int32 , ::sal_Int32 )
{
}

void SAL_CALL SwOLEListener_Impl::stateChanged( const lang::EventObject&, ::sal_Int32 nOldState, ::sal_Int32 nNewState )
{
    if ( mpObj && nOldState == embed::EmbedStates::LOADED && nNewState == embed::EmbedStates::RUNNING )
    {
        if (!g_pOLELRU_Cache)
            g_pOLELRU_Cache.reset(new SwOLELRUCache);
        g_pOLELRU_Cache->InsertObj( *mpObj );
    }
    else if ( mpObj && nNewState == embed::EmbedStates::LOADED && nOldState == embed::EmbedStates::RUNNING )
    {
        if (g_pOLELRU_Cache)
            g_pOLELRU_Cache->RemoveObj( *mpObj );
    }
    else if(mpObj && nNewState == embed::EmbedStates::RUNNING)
    {
        mpObj->resetBufferedData();
    }
}

void SwOLEListener_Impl::dispose()
{
    if (mpObj && g_pOLELRU_Cache)
        g_pOLELRU_Cache->RemoveObj( *mpObj );
    mpObj = nullptr;
}

void SAL_CALL SwOLEListener_Impl::disposing( const lang::EventObject& )
{
    if (mpObj && g_pOLELRU_Cache)
        g_pOLELRU_Cache->RemoveObj( *mpObj );
}

// TODO/LATER: actually SwEmbedObjectLink should be used here, but because different objects are used to control
//             embedded object different link objects with the same functionality had to be implemented

class SwEmbedObjectLink : public sfx2::SvBaseLink
{
    SwOLENode*          pOleNode;

public:
    explicit            SwEmbedObjectLink(SwOLENode* pNode);

    virtual void        Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;

    void            Connect() { GetRealObject(); }
};

SwEmbedObjectLink::SwEmbedObjectLink(SwOLENode* pNode):
    ::sfx2::SvBaseLink( ::SfxLinkUpdateMode::ONCALL, SotClipboardFormatId::SVXB ),
    pOleNode(pNode)
{
    SetSynchron( false );
}

::sfx2::SvBaseLink::UpdateResult SwEmbedObjectLink::DataChanged(
    const OUString&, const uno::Any& )
{
    if ( !pOleNode->UpdateLinkURL_Impl() )
    {
        // the link URL was not changed
        uno::Reference< embed::XEmbeddedObject > xObject = pOleNode->GetOLEObj().GetOleRef();
        OSL_ENSURE( xObject.is(), "The object must exist always!" );
        if ( xObject.is() )
        {
            // let the object reload the link
            // TODO/LATER: reload call could be used for this case

            try
            {
                sal_Int32 nState = xObject->getCurrentState();
                if ( nState != embed::EmbedStates::LOADED )
                {
                    // in some cases the linked file probably is not locked so it could be changed
                    xObject->changeState( embed::EmbedStates::LOADED );
                    xObject->changeState( nState );
                }
            }
            catch (const uno::Exception&)
            {
            }
        }
    }

    pOleNode->GetNewReplacement();
    pOleNode->SetChanged();

    return SUCCESS;
}

void SwEmbedObjectLink::Closed()
{
    pOleNode->BreakFileLink_Impl();
    SvBaseLink::Closed();
}

SwOLENode::SwOLENode( const SwNodeIndex &rWhere,
                    const svt::EmbeddedObjectRef& xObj,
                    SwGrfFormatColl *pGrfColl,
                    SwAttrSet const * pAutoAttr ) :
    SwNoTextNode( rWhere, SwNodeType::Ole, pGrfColl, pAutoAttr ),
    maOLEObj( xObj ),
    mbOLESizeInvalid( false ),
    mpObjectLink( nullptr )
{
    maOLEObj.SetNode( this );
}

SwOLENode::SwOLENode( const SwNodeIndex &rWhere,
                    const OUString &rString,
                    sal_Int64 nAspect,
                    SwGrfFormatColl *pGrfColl,
                    SwAttrSet const * pAutoAttr ) :
    SwNoTextNode( rWhere, SwNodeType::Ole, pGrfColl, pAutoAttr ),
    maOLEObj( rString, nAspect ),
    mbOLESizeInvalid( false ),
    mpObjectLink( nullptr )
{
    maOLEObj.SetNode( this );
}

SwOLENode::~SwOLENode()
{
    DisconnectFileLink_Impl();
}

const Graphic* SwOLENode::GetGraphic()
{
    if ( maOLEObj.GetOleRef().is() )
        return maOLEObj.m_xOLERef.GetGraphic();
    return nullptr;
}

/**
 * Loading a OLE object that has been moved to the Undo Area
 */
bool SwOLENode::RestorePersistentData()
{
    OSL_ENSURE( maOLEObj.GetOleRef().is(), "No object to restore!" );
    if ( maOLEObj.m_xOLERef.is() )
    {
        // If a SvPersist instance already exists, we use it
        SfxObjectShell* p = GetDoc()->GetPersist();
        if( !p )
        {
            // TODO/LATER: Isn't a EmbeddedObjectContainer sufficient here?
            // What happens to this document?
            OSL_ENSURE( false, "Why are we creating a DocShell here?" );
            p = new SwDocShell( GetDoc(), SfxObjectCreateMode::INTERNAL );
            p->DoInitNew();
        }

        uno::Reference < container::XChild > xChild( maOLEObj.m_xOLERef.GetObject(), uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( p->GetModel() );

        OSL_ENSURE( !maOLEObj.m_aName.isEmpty(), "No object name!" );
        OUString aObjName;
        if ( !p->GetEmbeddedObjectContainer().InsertEmbeddedObject( maOLEObj.m_xOLERef.GetObject(), aObjName ) )
        {
            if ( xChild.is() )
                xChild->setParent( nullptr );
            OSL_FAIL( "InsertObject failed" );
        }
        else
        {
            maOLEObj.m_aName = aObjName;
            maOLEObj.m_xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aObjName );
            CheckFileLink_Impl();
        }
    }

    return true;
}

/**
 * OLE object is transported into UNDO area
 */
bool SwOLENode::SavePersistentData()
{
    if( maOLEObj.m_xOLERef.is() )
    {
        comphelper::EmbeddedObjectContainer* pCnt = maOLEObj.m_xOLERef.GetContainer();

#if OSL_DEBUG_LEVEL > 0
        SfxObjectShell* p = GetDoc()->GetPersist();
        OSL_ENSURE( p, "No document!" );
        if( p )
        {
            comphelper::EmbeddedObjectContainer& rCnt = p->GetEmbeddedObjectContainer();
            OSL_ENSURE( !pCnt || &rCnt == pCnt, "The helper is assigned to unexpected container!" );
        }
#endif

        if ( pCnt && pCnt->HasEmbeddedObject( maOLEObj.m_aName ) )
        {
            uno::Reference < container::XChild > xChild( maOLEObj.m_xOLERef.GetObject(), uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( nullptr );

            /*
              #i119941
              When cut or move the chart, SwUndoFlyBase::DelFly will call SaveSection
              to store the content to storage. In this step, chart filter functions
              will be called. And chart filter will call chart core functions to create
              the chart again. Then chart core function will call the class
              ExplicitCategoryProvider to create data source. In this step, when SW data
              source provider create the data source, an UnoActionRemoveContext
              will mess with the layout and create a new SwFlyFrame.
              But later in SwUndoFlyBase::DelFly, it will clear anchor related attributes
              of SwFlyFrame. Then finally null pointer occur.
              Resolution:
              In pCnt->RemoveEmbeddedObject in SaveSection process of table chart,
              only remove the object from the object container, without removing it's
              storage and graphic stream. The chart already removed from formatter.
            */
            bool bKeepObjectToTempStorage = true;
            uno::Reference < embed::XEmbeddedObject > xIP = GetOLEObj().GetOleRef();
            if (IsChart() && !msChartTableName.isEmpty()
                && svt::EmbeddedObjectRef::TryRunningState(xIP))
            {
                uno::Reference< chart2::XChartDocument > xChart( xIP->getComponent(), UNO_QUERY );
                if (xChart.is() && !xChart->hasInternalDataProvider())
                {
                    bKeepObjectToTempStorage = false;
                }
            }

            pCnt->RemoveEmbeddedObject( maOLEObj.m_aName, bKeepObjectToTempStorage );

            // TODO/LATER: aOLEObj.aName has no meaning here, since the undo container contains the object
            // by different name, in future it might makes sense that the name is transported here.
            maOLEObj.m_xOLERef.AssignToContainer( nullptr, maOLEObj.m_aName );
            try
            {
                // "unload" object
                maOLEObj.m_xOLERef->changeState( embed::EmbedStates::LOADED );
            }
            catch (const uno::Exception&)
            {
            }
        }
    }

    DisconnectFileLink_Impl();

    return true;
}

SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
                    const svt::EmbeddedObjectRef& xObj,
                                    SwGrfFormatColl* pGrfColl )
{
    OSL_ENSURE( pGrfColl,"SwNodes::MakeOLENode: Formatpointer is 0." );

    SwOLENode *pNode =
        new SwOLENode( rWhere, xObj, pGrfColl, nullptr );

    // set parent if XChild is supported
    //!! needed to supply Math objects with a valid reference device
    uno::Reference< container::XChild > xChild( pNode->GetOLEObj().GetObject().GetObject(), UNO_QUERY );
    if (xChild.is())
    {
        SwDocShell *pDocSh = GetDoc()->GetDocShell();
        if (pDocSh)
            xChild->setParent( pDocSh->GetModel() );
    }

    return pNode;
}

SwOLENode * SwNodes::MakeOLENode( const SwNodeIndex & rWhere,
    const OUString &rName, sal_Int64 nAspect, SwGrfFormatColl* pGrfColl, SwAttrSet const * pAutoAttr )
{
    OSL_ENSURE( pGrfColl,"SwNodes::MakeOLENode: Formatpointer is 0." );

    SwOLENode *pNode =
        new SwOLENode( rWhere, rName, nAspect, pGrfColl, pAutoAttr );

    // set parent if XChild is supported
    //!! needed to supply Math objects with a valid reference device
    uno::Reference< container::XChild > xChild( pNode->GetOLEObj().GetObject().GetObject(), UNO_QUERY );
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
    MapMode aMapMode( MapUnit::MapTwip );
    return const_cast<SwOLENode*>(this)->maOLEObj.GetObject().GetSize( &aMapMode );
}

SwContentNode* SwOLENode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx, bool) const
{
    // If there's already a SvPersist instance, we use it
    SfxObjectShell* pPersistShell = pDoc->GetPersist();
    if( !pPersistShell )
    {
        // TODO/LATER: is EmbeddedObjectContainer not enough?
        // the created document will be closed by pDoc ( should use SfxObjectShellLock )
        pPersistShell = new SwDocShell( pDoc, SfxObjectCreateMode::INTERNAL );
        pDoc->SetTmpDocShell( pPersistShell );
        pPersistShell->DoInitNew();
    }

    // We insert it at SvPersist level
    // TODO/LATER: check if using the same naming scheme for all apps works here
    OUString aNewName/*( Sw3Io::UniqueName( p->GetStorage(), "Obj" ) )*/;
    SfxObjectShell* pSrc = GetDoc()->GetPersist();

    pPersistShell->GetEmbeddedObjectContainer().CopyAndGetEmbeddedObject(
        pSrc->GetEmbeddedObjectContainer(),
        pSrc->GetEmbeddedObjectContainer().GetEmbeddedObject( maOLEObj.m_aName ),
        aNewName,
        pSrc->getDocumentBaseURL(),
        pPersistShell->getDocumentBaseURL());

    SwOLENode* pOLENd = pDoc->GetNodes().MakeOLENode( rIdx, aNewName, GetAspect(),
                                    pDoc->GetDfltGrfFormatColl(),
                                    GetpSwAttrSet() );

    pOLENd->SetChartTableName( GetChartTableName() );
    pOLENd->SetTitle( GetTitle() );
    pOLENd->SetDescription( GetDescription() );
    pOLENd->SetContour( HasContour(), HasAutomaticContour() );
    pOLENd->SetAspect( GetAspect() ); // the replacement image must be already copied

    pOLENd->SetOLESizeInvalid( true );
    pDoc->SetOLEPrtNotifyPending();

    return pOLENd;
}

bool SwOLENode::IsInGlobalDocSection() const
{
    // Find the "Body Anchor"
    sal_uLong nEndExtraIdx = GetNodes().GetEndOfExtras().GetIndex();
    const SwNode* pAnchorNd = this;
    do {
        SwFrameFormat* pFlyFormat = pAnchorNd->GetFlyFormat();
        if( !pFlyFormat )
            return false;

        const SwFormatAnchor& rAnchor = pFlyFormat->GetAnchor();
        if( !rAnchor.GetContentAnchor() )
            return false;

        pAnchorNd = &rAnchor.GetContentAnchor()->nNode.GetNode();
    } while( pAnchorNd->GetIndex() < nEndExtraIdx );

    const SwSectionNode* pSectNd = pAnchorNd->FindSectionNode();
    if( !pSectNd )
        return false;

    while( pSectNd )
    {
        pAnchorNd = pSectNd;
        pSectNd = pAnchorNd->StartOfSectionNode()->FindSectionNode();
    }

    // pAnchorNd contains the most recently found Section Node, which
    // now must fulfill the prerequisites for the GlobalDoc
    pSectNd = static_cast<const SwSectionNode*>(pAnchorNd);
    return FILE_LINK_SECTION == pSectNd->GetSection().GetType() &&
            pSectNd->GetIndex() > nEndExtraIdx;
}

bool SwOLENode::IsOLEObjectDeleted() const
{
    if( maOLEObj.m_xOLERef.is() )
    {
        SfxObjectShell* p = GetDoc()->GetPersist();
        if( p ) // Must be there
        {
            return !p->GetEmbeddedObjectContainer().HasEmbeddedObject( maOLEObj.m_aName );
        }
    }
    return false;
}

void SwOLENode::GetNewReplacement()
{
    if ( maOLEObj.m_xOLERef.is() )
        maOLEObj.m_xOLERef.UpdateReplacement();
}

bool SwOLENode::UpdateLinkURL_Impl()
{
    bool bResult = false;

    if ( mpObjectLink )
    {
        OUString aNewLinkURL;
        sfx2::LinkManager::GetDisplayNames( mpObjectLink, nullptr, &aNewLinkURL );
        if ( !aNewLinkURL.equalsIgnoreAsciiCase( maLinkURL ) )
        {
            if ( !maOLEObj.m_xOLERef.is() )
                maOLEObj.GetOleRef();

            uno::Reference< embed::XEmbeddedObject > xObj = maOLEObj.m_xOLERef.GetObject();
            uno::Reference< embed::XCommonEmbedPersist > xPersObj( xObj, uno::UNO_QUERY );
            OSL_ENSURE( xPersObj.is(), "The object must exist!" );
            if ( xPersObj.is() )
            {
                try
                {
                    sal_Int32 nCurState = xObj->getCurrentState();
                    if ( nCurState != embed::EmbedStates::LOADED )
                        xObj->changeState( embed::EmbedStates::LOADED );

                    // TODO/LATER: there should be possible to get current mediadescriptor settings from the object
                    uno::Sequence< beans::PropertyValue > aArgs( 1 );
                    aArgs[0].Name = "URL";
                    aArgs[0].Value <<= aNewLinkURL;
                    xPersObj->reload( aArgs, uno::Sequence< beans::PropertyValue >() );

                    maLinkURL = aNewLinkURL;
                    bResult = true;

                    if ( nCurState != embed::EmbedStates::LOADED )
                        xObj->changeState( nCurState );
                }
                catch (const uno::Exception&)
                {
                }
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
                uno::Reference< embed::XLinkageSupport > xLinkSupport( maOLEObj.GetOleRef(), uno::UNO_QUERY_THROW );
                xLinkSupport->breakLink( xStorage, maOLEObj.GetCurrentPersistName() );
                DisconnectFileLink_Impl();
                maLinkURL.clear();
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
        GetDoc()->getIDocumentLinksAdministration().GetLinkManager().Remove( mpObjectLink );
        mpObjectLink = nullptr;
    }
}

void SwOLENode::CheckFileLink_Impl()
{
    if ( maOLEObj.m_xOLERef.GetObject().is() && !mpObjectLink )
    {
        try
        {
            uno::Reference< embed::XLinkageSupport > xLinkSupport( maOLEObj.m_xOLERef.GetObject(), uno::UNO_QUERY_THROW );
            if ( xLinkSupport->isLink() )
            {
                const OUString aLinkURL = xLinkSupport->getLinkURL();
                if ( !aLinkURL.isEmpty() )
                {
                    // this is a file link so the model link manager should handle it
                    mpObjectLink = new SwEmbedObjectLink( this );
                    maLinkURL = aLinkURL;
                    GetDoc()->getIDocumentLinksAdministration().GetLinkManager().InsertFileLink( *mpObjectLink, OBJECT_CLIENT_OLE, aLinkURL );
                    mpObjectLink->Connect();
                }
            }
        }
        catch( uno::Exception& )
        {
        }
    }
}

// #i99665#
bool SwOLENode::IsChart() const
{
    bool bIsChart( false );

    const uno::Reference< embed::XEmbeddedObject > xEmbObj =
                            const_cast<SwOLEObj&>(GetOLEObj()).GetOleRef();
    if ( xEmbObj.is() )
    {
        SvGlobalName aClassID( xEmbObj->getClassID() );
        bIsChart = SotExchange::IsChart( aClassID );
    }

    return bIsChart;
}

// react on visual change (invalidate)
void SwOLENode::SetChanged()
{
    SwFrame* pFrame(getLayoutFrame(nullptr));

    if(nullptr == pFrame)
    {
        return;
    }

    const SwRect aFrameArea(pFrame->getFrameArea());
    SwViewShell* pVSh(GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell());

    if(nullptr == pVSh)
    {
        return;
    }

    for(SwViewShell& rShell : pVSh->GetRingContainer())
    {
        SET_CURR_SHELL(&rShell);

        if(rShell.VisArea().IsOver(aFrameArea) && OUTDEV_WINDOW == rShell.GetOut()->GetOutDevType())
        {
            // invalidate instead of painting
            rShell.GetWin()->Invalidate(aFrameArea.SVRect());
        }
    }
}

namespace { class DeflateThread; }

/// Holder for local data for a parallel-executed task to load a chart model
class DeflateData
{
private:
    friend DeflateThread;
    friend class SwOLEObj;

    uno::Reference< frame::XModel >                     maXModel;
    drawinglayer::primitive2d::Primitive2DContainer     maPrimitive2DSequence;
    basegfx::B2DRange                                   maRange;

    // evtl.set from the SwOLEObj destructor when a WorkerThread is still active
    // since it is not possible to kill it - let it terminate and delete the
    // data working on itself
    std::atomic< bool>                                  mbKilled;

    std::shared_ptr<comphelper::ThreadTaskTag>          mpTag;

public:
    explicit DeflateData(const uno::Reference< frame::XModel >& rXModel)
    :   maXModel(rXModel),
        maPrimitive2DSequence(),
        maRange(),
        mbKilled(false),
        mpTag( comphelper::ThreadPool::createThreadTaskTag() )
    {
    }

    const drawinglayer::primitive2d::Primitive2DContainer& getSequence() const
    {
        return maPrimitive2DSequence;
    }

    const basegfx::B2DRange& getRange() const
    {
        return maRange;
    }

    bool isFinished() const
    {
        return comphelper::ThreadPool::isTaskTagDone(mpTag);
    }

    void waitFinished()
    {
        // need to wait until the load in progress is finished.
        // WorkerThreads need the SolarMutex to be able to continue
        // and finish the running import.
        SolarMutexReleaser aReleaser;
        comphelper::ThreadPool::getSharedOptimalPool().waitUntilDone(mpTag);
    }
};

namespace {

/// Task for parallelly-executed task to load a chart model
class DeflateThread : public comphelper::ThreadTask
{
    // the data to work on
    DeflateData&            mrDeflateData;

public:
    explicit DeflateThread(DeflateData& rDeflateData)
    :   comphelper::ThreadTask(rDeflateData.mpTag), mrDeflateData(rDeflateData)
    {
    }

private:
    virtual void doWork() override
    {
        try
        {
            // load the chart data and get the primitives
            mrDeflateData.maPrimitive2DSequence = ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
                mrDeflateData.maXModel,
                mrDeflateData.maRange);

            // model no longer needed and done
            mrDeflateData.maXModel.clear();
        }
        catch (const uno::Exception&)
        {
        }

        if(mrDeflateData.mbKilled)
        {
            // need to cleanup myself - data will not be used
            delete &mrDeflateData;
        }
    }
};

}

//////////////////////////////////////////////////////////////////////////////

SwOLEObj::SwOLEObj( const svt::EmbeddedObjectRef& xObj ) :
    m_pOLENode( nullptr ),
    m_xOLERef( xObj ),
    m_aPrimitive2DSequence(),
    m_aRange()
{
    m_xOLERef.Lock();
    if ( xObj.is() )
    {
        m_xListener = new SwOLEListener_Impl( this );
        xObj->addStateChangeListener( m_xListener.get() );
    }
}

SwOLEObj::SwOLEObj( const OUString &rString, sal_Int64 nAspect ) :
    m_pOLENode( nullptr ),
    m_aName( rString ),
    m_aPrimitive2DSequence(),
    m_aRange()
{
    m_xOLERef.Lock();
    m_xOLERef.SetViewAspect( nAspect );
}

SwOLEObj::~SwOLEObj() COVERITY_NOEXCEPT_FALSE
{
    if(m_pDeflateData)
    {
        // set flag so that the worker thread will delete m_pDeflateData
        // when finished and forget about it
        m_pDeflateData->mbKilled = true;
        m_pDeflateData = nullptr;
    }

    if( m_xListener )
    {
        if ( m_xOLERef.is() )
            m_xOLERef->removeStateChangeListener( m_xListener.get() );
        m_xListener->dispose();
        m_xListener.clear();
    }

    if( m_pOLENode && !m_pOLENode->GetDoc()->IsInDtor() )
    {
        // if the model is not currently in destruction it means that this object should be removed from the model
        comphelper::EmbeddedObjectContainer* pCnt = m_xOLERef.GetContainer();

#if OSL_DEBUG_LEVEL > 0
        SfxObjectShell* p = m_pOLENode->GetDoc()->GetPersist();
        OSL_ENSURE( p, "No document!" );
        if( p )
        {
            comphelper::EmbeddedObjectContainer& rCnt = p->GetEmbeddedObjectContainer();
            OSL_ENSURE( !pCnt || &rCnt == pCnt, "The helper is assigned to unexpected container!" );
        }
#endif

        if ( pCnt && pCnt->HasEmbeddedObject( m_aName ) )
        {
            uno::Reference < container::XChild > xChild( m_xOLERef.GetObject(), uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( nullptr );

            // not already removed by deleting the object
            m_xOLERef.AssignToContainer( nullptr, m_aName );

            // unlock object so that object can be closed in RemoveEmbeddedObject
            // successful closing of the object will automatically clear the reference then
            m_xOLERef.Lock(false);

            // Always remove object from container it is connected to
            try
            {
                // remove object from container but don't close it
                pCnt->RemoveEmbeddedObject( m_aName );
            }
            catch ( uno::Exception& )
            {
            }
        }

    }

    if ( m_xOLERef.is() )
        // in case the object wasn't closed: release it
        // in case the object was not in the container: it's still locked, try to close
        m_xOLERef.Clear();
}

void SwOLEObj::SetNode( SwOLENode* pNode )
{
    m_pOLENode = pNode;
    if ( m_aName.isEmpty() )
    {
        SwDoc* pDoc = pNode->GetDoc();

        // If there's already a SvPersist instance, we use it
        SfxObjectShell* p = pDoc->GetPersist();
        if( !p )
        {
            // TODO/LATER: Isn't a EmbeddedObjectContainer sufficient here?
            // What happens to the document?
            OSL_ENSURE( false, "Why are we creating a DocShell here??" );
            p = new SwDocShell( pDoc, SfxObjectCreateMode::INTERNAL );
            p->DoInitNew();
        }

        OUString aObjName;
        uno::Reference < container::XChild > xChild( m_xOLERef.GetObject(), uno::UNO_QUERY );
        if ( xChild.is() && xChild->getParent() != p->GetModel() )
            // it is possible that the parent was set already
            xChild->setParent( p->GetModel() );
        if (!p->GetEmbeddedObjectContainer().InsertEmbeddedObject( m_xOLERef.GetObject(), aObjName ) )
        {
            OSL_FAIL( "InsertObject failed" );
            if ( xChild.is() )
                xChild->setParent( nullptr );
        }
        else
            m_xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), aObjName );

        const_cast<SwOLENode*>(m_pOLENode)->CheckFileLink_Impl(); // for this notification nonconst access is required

        m_aName = aObjName;
    }
}

OUString SwOLEObj::GetStyleString()
{
    OUString strStyle;
    if (m_xOLERef.is() && m_xOLERef.IsChart())
        strStyle = m_xOLERef.GetChartType();
    return strStyle;
}

bool SwOLEObj::IsOleRef() const
{
    return m_xOLERef.is();
}

uno::Reference < embed::XEmbeddedObject > const & SwOLEObj::GetOleRef()
{
    if( !m_xOLERef.is() )
    {
        SfxObjectShell* p = m_pOLENode->GetDoc()->GetPersist();
        assert(p && "No SvPersist present");

        OUString sDocumentBaseURL = p->getDocumentBaseURL();
        uno::Reference < embed::XEmbeddedObject > xObj = p->GetEmbeddedObjectContainer().GetEmbeddedObject(m_aName, &sDocumentBaseURL);
        OSL_ENSURE( !m_xOLERef.is(), "Calling GetOleRef() recursively is not permitted" );

        if ( !xObj.is() )
        {
            // We could not load this part (probably broken)
            tools::Rectangle aArea;
            SwFrame *pFrame = m_pOLENode->getLayoutFrame(nullptr);
            if ( pFrame )
            {
                Size aSz( pFrame->getFrameArea().SSize() );
                const MapMode aSrc ( MapUnit::MapTwip );
                const MapMode aDest( MapUnit::Map100thMM );
                aSz = OutputDevice::LogicToLogic( aSz, aSrc, aDest );
                aArea.SetSize( aSz );
            }
            else
                aArea.SetSize( Size( 5000,  5000 ) );
            // TODO/LATER: set replacement graphic for dead object
            // It looks as if it should work even without the object, because the replace will be generated automatically
            OUString aTmpName;
            xObj = p->GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_DUMMY_CLASSID ).GetByteSequence(), aTmpName );
        }
        if (xObj.is())
        {
            m_xOLERef.Assign( xObj, m_xOLERef.GetViewAspect() );
            m_xOLERef.AssignToContainer( &p->GetEmbeddedObjectContainer(), m_aName );
            m_xListener = new SwOLEListener_Impl( this );
            xObj->addStateChangeListener( m_xListener.get() );
        }

        const_cast<SwOLENode*>(m_pOLENode)->CheckFileLink_Impl(); // for this notification nonconst access is required
    }
    else if ( m_xOLERef->getCurrentState() == embed::EmbedStates::RUNNING )
    {
        // move object to first position in cache
        if (!g_pOLELRU_Cache)
            g_pOLELRU_Cache.reset(new SwOLELRUCache);
        g_pOLELRU_Cache->InsertObj( *this );
    }

    return m_xOLERef.GetObject();
}

svt::EmbeddedObjectRef& SwOLEObj::GetObject()
{
    GetOleRef();
    return m_xOLERef;
}

bool SwOLEObj::UnloadObject()
{
    bool bRet = true;
    if ( m_pOLENode )
    {
        const SwDoc* pDoc = m_pOLENode->GetDoc();
        bRet = UnloadObject( m_xOLERef.GetObject(), pDoc, m_xOLERef.GetViewAspect() );
    }

    return bRet;
}

bool SwOLEObj::UnloadObject( uno::Reference< embed::XEmbeddedObject > const & xObj, const SwDoc* pDoc, sal_Int64 nAspect )
{
    if ( !pDoc )
        return false;

    bool bRet = true;
    sal_Int32 nState = xObj.is() ? xObj->getCurrentState() : embed::EmbedStates::LOADED;
    bool bIsActive = ( nState != embed::EmbedStates::LOADED && nState != embed::EmbedStates::RUNNING );
    sal_Int64 nMiscStatus = xObj->getStatus( nAspect );

    if( nState != embed::EmbedStates::LOADED && !pDoc->IsInDtor() && !bIsActive &&
        embed::EmbedMisc::MS_EMBED_ALWAYSRUN != ( nMiscStatus & embed::EmbedMisc::MS_EMBED_ALWAYSRUN ) &&
        embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY != ( nMiscStatus & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) )
    {
        SfxObjectShell* p = pDoc->GetPersist();
        if( p )
        {
            if( pDoc->GetDocumentSettingManager().get(DocumentSettingId::PURGE_OLE) )
            {
                try
                {
                    uno::Reference < util::XModifiable > xMod( xObj->getComponent(), uno::UNO_QUERY );
                    if( xMod.is() && xMod->isModified() )
                    {
                        uno::Reference < embed::XEmbedPersist > xPers( xObj, uno::UNO_QUERY );
                        assert(xPers.is() && "Modified object without persistence in cache!");
                        xPers->storeOwn();
                    }

                    // setting object to loaded state will remove it from cache
                    xObj->changeState( embed::EmbedStates::LOADED );
                }
                catch (const uno::Exception&)
                {
                    bRet = false;
                }
            }
            else
                bRet = false;
        }
    }

    return bRet;
}

OUString SwOLEObj::GetDescription()
{
    uno::Reference< embed::XEmbeddedObject > xEmbObj = GetOleRef();
    if ( !xEmbObj.is() )
        return OUString();

    SvGlobalName aClassID( xEmbObj->getClassID() );
    if ( SotExchange::IsMath( aClassID ) )
        return SwResId(STR_MATH_FORMULA);

    if ( SotExchange::IsChart( aClassID ) )
        return SwResId(STR_CHART);

    return SwResId(STR_OLE);
}

drawinglayer::primitive2d::Primitive2DContainer const & SwOLEObj::tryToGetChartContentAsPrimitive2DSequence(
    basegfx::B2DRange& rRange,
    bool bSynchron)
{
    if(m_pDeflateData)
    {
        if(bSynchron)
        {
            // data in high quality is requested, wait until the data is available
            // since a WorkerThread was already started to load it
            m_pDeflateData->waitFinished();
        }

        if(m_pDeflateData->isFinished())
        {
            // copy the result data and cleanup
            m_aPrimitive2DSequence = m_pDeflateData->getSequence();
            m_aRange = m_pDeflateData->getRange();
            m_pDeflateData.reset();
        }
    }

    if(m_aPrimitive2DSequence.empty() && m_aRange.isEmpty() && m_xOLERef.is() && m_xOLERef.IsChart())
    {
        const uno::Reference< frame::XModel > aXModel(m_xOLERef->getComponent(), uno::UNO_QUERY);

        if(aXModel.is())
        {
            // disabled for now, need to check deeper
            static bool bAsynchronousLoadingAllowed = false; // loplugin:constvars:ignore

            if(bSynchron ||
                !bAsynchronousLoadingAllowed)
            {
                // load chart synchron in this Thread
                m_aPrimitive2DSequence = ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
                    aXModel,
                    m_aRange);
            }
            else
            {
                // if not yet setup, initiate and start a WorkerThread to load the chart
                // and it's primitives asynchron. If it already works, returning nothing
                // is okay (preview will be reused)
                if(!m_pDeflateData)
                {
                    m_pDeflateData.reset( new DeflateData(aXModel) );
                    std::unique_ptr<DeflateThread> pNew( new DeflateThread(*m_pDeflateData) );
                    comphelper::ThreadPool::getSharedOptimalPool().pushTask(std::move(pNew));
                }
            }
        }
    }

    if(!m_aPrimitive2DSequence.empty() && !m_aRange.isEmpty())
    {
        // when we have data, also copy the buffered Range data as output
        rRange = m_aRange;
    }

    return m_aPrimitive2DSequence;
}

void SwOLEObj::resetBufferedData()
{
    m_aPrimitive2DSequence = drawinglayer::primitive2d::Primitive2DContainer();
    m_aRange.reset();

    if(m_pDeflateData)
    {
        // load is in progress, wait until finished and cleanup without using it
        m_pDeflateData->waitFinished();
        m_pDeflateData.reset();
    }
}

SwOLELRUCache::SwOLELRUCache()
    : utl::ConfigItem("Office.Common/Cache")
    , m_nLRU_InitSize( 20 )
{
    EnableNotification( GetPropertyNames() );
    Load();
}

uno::Sequence< OUString > SwOLELRUCache::GetPropertyNames()
{
    Sequence< OUString > aNames { "Writer/OLE_Objects" };
    return aNames;
}

void SwOLELRUCache::Notify( const uno::Sequence< OUString>&  )
{
    Load();
}

void SwOLELRUCache::ImplCommit()
{
}

void SwOLELRUCache::Load()
{
    Sequence< OUString > aNames( GetPropertyNames() );
    Sequence< Any > aValues = GetProperties( aNames );
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if (aValues.getLength() != aNames.getLength() || !pValues->hasValue())
        return;

    sal_Int32 nVal = 0;
    *pValues >>= nVal;

    if (nVal < m_nLRU_InitSize)
    {
        std::shared_ptr<SwOLELRUCache> tmp(g_pOLELRU_Cache); // prevent delete this
        // size of cache has been changed
        sal_Int32 nCount = m_OleObjects.size();
        sal_Int32 nPos = nCount;

        // try to remove the last entries until new maximum size is reached
        while( nCount > nVal )
        {
            SwOLEObj *const pObj = m_OleObjects[ --nPos ];
            if ( pObj->UnloadObject() )
                nCount--;
            if ( !nPos )
                break;
        }
    }

    m_nLRU_InitSize = nVal;
}

void SwOLELRUCache::InsertObj( SwOLEObj& rObj )
{
    SwOLEObj* pObj = &rObj;
    OleObjects_t::iterator it =
        std::find(m_OleObjects.begin(), m_OleObjects.end(), pObj);
    if (it != m_OleObjects.end() && it != m_OleObjects.begin())
    {
        // object in cache but is currently not the first in cache
        m_OleObjects.erase(it);
        it = m_OleObjects.end();
    }
    if (it == m_OleObjects.end())
    {
        std::shared_ptr<SwOLELRUCache> tmp(g_pOLELRU_Cache); // prevent delete this
        // try to remove objects if necessary
        sal_Int32 nCount = m_OleObjects.size();
        sal_Int32 nPos = nCount-1;
        while (nPos >= 0 && nCount >= m_nLRU_InitSize)
        {
            pObj = m_OleObjects[ nPos-- ];
            if ( pObj->UnloadObject() )
                nCount--;
        }
        m_OleObjects.push_front(&rObj);
    }
}

void SwOLELRUCache::RemoveObj( SwOLEObj& rObj )
{
    OleObjects_t::iterator const it =
        std::find(m_OleObjects.begin(), m_OleObjects.end(), &rObj);
    if (it != m_OleObjects.end())
    {
        m_OleObjects.erase(it);
    }
    if (m_OleObjects.empty())
    {
        if (g_pOLELRU_Cache.use_count() == 1) // test that we're not in InsertObj()
        {
            g_pOLELRU_Cache.reset();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
