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

#include <vcl/help.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metric.hxx>
#include "selector.hxx"
#include <dialmgr.hxx>
#include "selector.hrc"
#include <svx/fmresids.hrc> // for RID_SVXIMG_...
#include <svx/dialmgr.hxx>  // for RID_SVXIMG_...
#include <cuires.hrc>
#include <sfx2/app.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/dispatch.hxx>

#include <comphelper/documentinfo.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/XBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/DispatchInformation.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::container;

#include <svtools/imagemgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>

/*
 * The implementations of SvxConfigFunctionListBox_Impl and
 * SvxConfigGroupListBox_Impl are copied from sfx2/source/dialog/cfg.cxx
 */
SvxConfigFunctionListBox_Impl::SvxConfigFunctionListBox_Impl( Window* pParent, const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
    , pCurEntry( 0 )
    , m_pDraggingEntry( 0 )
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    GetModel()->SetSortMode( SortAscending );

    // Timer for the BallonHelp
    aTimer.SetTimeout( 200 );
    aTimer.SetTimeoutHdl(
        LINK( this, SvxConfigFunctionListBox_Impl, TimerHdl ) );
}

SvxConfigFunctionListBox_Impl::~SvxConfigFunctionListBox_Impl()
{
    ClearAll();
}

SvTreeListEntry* SvxConfigFunctionListBox_Impl::GetLastSelectedEntry()
{
    if ( m_pDraggingEntry != NULL )
    {
        return m_pDraggingEntry;
    }
    else
    {
        return FirstSelected();
    }
}

void SvxConfigFunctionListBox_Impl::MouseMove( const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();
    pCurEntry = GetCurEntry();

    if ( pCurEntry && GetEntry( aMousePos ) == pCurEntry )
        aTimer.Start();
    else
    {
        Help::ShowBalloon( this, aMousePos, String() );
        aTimer.Stop();
    }
}


IMPL_LINK_NOARG(SvxConfigFunctionListBox_Impl, TimerHdl)
{
    aTimer.Stop();
    Point aMousePos = GetPointerPosPixel();
    SvTreeListEntry *pEntry = GetCurEntry();
    if ( pEntry && GetEntry( aMousePos ) == pEntry && pCurEntry == pEntry )
        Help::ShowBalloon( this, OutputToScreenPixel( aMousePos ), GetHelpText( pEntry ) );
    return 0L;
}

void SvxConfigFunctionListBox_Impl::ClearAll()
{
    aArr.clear();
    Clear();
}

String SvxConfigFunctionListBox_Impl::GetHelpText( SvTreeListEntry *pEntry )
{
    SvxGroupInfo_Impl *pInfo =
        pEntry ? (SvxGroupInfo_Impl*) pEntry->GetUserData(): 0;

    if ( pInfo )
    {
        if ( pInfo->nKind == SVX_CFGFUNCTION_SLOT )
        {
            OUString aCmdURL( pInfo->sURL );

            OUString aHelpText = Application::GetHelp()->GetHelpText( aCmdURL, this );

            return aHelpText;
        }
        else if ( pInfo->nKind == SVX_CFGFUNCTION_SCRIPT )
        {
            return pInfo->sHelpText;
        }
    }

    return String();
}

void SvxConfigFunctionListBox_Impl::FunctionSelected()
{
    Help::ShowBalloon( this, Point(), String() );
}

// drag and drop support
DragDropMode SvxConfigFunctionListBox_Impl::NotifyStartDrag(
    TransferDataContainer& /*aTransferDataContainer*/, SvTreeListEntry* pEntry )
{
    m_pDraggingEntry = pEntry;
    return GetDragDropMode();
}

void SvxConfigFunctionListBox_Impl::DragFinished( sal_Int8 /*nDropAction*/ )
{
    m_pDraggingEntry = NULL;
}

sal_Int8
SvxConfigFunctionListBox_Impl::AcceptDrop( const AcceptDropEvent& /*rEvt*/ )
{
    return DND_ACTION_NONE;
}

SvxConfigGroupListBox_Impl::SvxConfigGroupListBox_Impl(
    Window* pParent, const ResId& rResId,
    bool _bShowSlots, const Reference< frame::XFrame >& xFrame )
        : SvTreeListBox( pParent, rResId )
        , m_bShowSlots( _bShowSlots ),
    m_hdImage(ResId(IMG_HARDDISK,*rResId.GetResMgr())),
    m_libImage(ResId(IMG_LIB,*rResId.GetResMgr())),
    m_macImage(ResId(IMG_MACRO,*rResId.GetResMgr())),
    m_docImage(ResId(IMG_DOC,*rResId.GetResMgr())),
    m_sMyMacros(String(ResId(STR_MYMACROS,*rResId.GetResMgr()))),
    m_sProdMacros(String(ResId(STR_PRODMACROS,*rResId.GetResMgr())))
{
    FreeResource();

    if ( xFrame != NULL )
    {
        m_xFrame.set( xFrame );
    }

    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT );

    ImageList aNavigatorImages( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );

    SetNodeBitmaps(
        aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
        aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE )
    );
}


SvxConfigGroupListBox_Impl::~SvxConfigGroupListBox_Impl()
{
    ClearAll();
}

void SvxConfigGroupListBox_Impl::ClearAll()
{
    aArr.clear();
    Clear();
}

//-----------------------------------------------
namespace
{
    //...........................................
    /** examines a component whether it supports XEmbeddedScripts, or provides access to such a
        component by implementing XScriptInvocationContext.
        @return
            the model which supports the embedded scripts, or <NULL/> if it cannot find such a
            model
    */
    static Reference< XModel > lcl_getDocumentWithScripts_throw( const Reference< XInterface >& _rxComponent )
    {
        Reference< XEmbeddedScripts > xScripts( _rxComponent, UNO_QUERY );
        if ( !xScripts.is() )
        {
            Reference< XScriptInvocationContext > xContext( _rxComponent, UNO_QUERY );
            if ( xContext.is() )
                xScripts.set( xContext->getScriptContainer(), UNO_QUERY );
        }

        return Reference< XModel >( xScripts, UNO_QUERY );
    }

    //...........................................
    static Reference< XModel > lcl_getScriptableDocument_nothrow( const Reference< XFrame >& _rxFrame )
    {
        Reference< XModel > xDocument;

        // examine our associated frame
        try
        {
            OSL_ENSURE( _rxFrame.is(), "lcl_getScriptableDocument_nothrow: you need to pass a frame to this dialog/tab page!" );
            if ( _rxFrame.is() )
            {
                // first try the model in the frame
                Reference< XController > xController( _rxFrame->getController(), UNO_SET_THROW );
                xDocument = lcl_getDocumentWithScripts_throw( xController->getModel() );

                if ( !xDocument.is() )
                {
                    // if there is no suitable document in the frame, try the controller
                    xDocument = lcl_getDocumentWithScripts_throw( _rxFrame->getController() );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return xDocument;
    }
}

void SvxConfigGroupListBox_Impl::fillScriptList( const Reference< browse::XBrowseNode >& _rxRootNode, SvTreeListEntry* _pParentEntry, bool _bCheapChildrenOnDemand )
{
    OSL_PRECOND( _rxRootNode.is(), "SvxConfigGroupListBox_Impl::fillScriptList: invalid root node!" );
    if ( !_rxRootNode.is() )
        return;

    try
    {
        if ( _rxRootNode->hasChildNodes() )
        {
            Sequence< Reference< browse::XBrowseNode > > children =
                _rxRootNode->getChildNodes();

            sal_Bool bIsRootNode = _rxRootNode->getName() == "Root";

            /* To mimic current starbasic behaviour we
            need to make sure that only the current document
            is displayed in the config tree. Tests below
            set the bDisplay flag to sal_False if the current
            node is a first level child of the Root and is NOT
            either the current document, user or share */
            OUString sCurrentDocTitle;
            Reference< XModel > xWorkingDocument = lcl_getScriptableDocument_nothrow( m_xFrame );
            if ( xWorkingDocument.is() )
            {
                sCurrentDocTitle = ::comphelper::DocumentInfo::getDocumentTitle( xWorkingDocument );
            }

            for ( long n = 0; n < children.getLength(); ++n )
            {
                Reference< browse::XBrowseNode >& theChild = children[n];
                //#139111# some crash reports show that it might be unset
                if ( !theChild.is() )
                    continue;
                ::rtl::OUString sUIName = theChild->getName();
                sal_Bool bDisplay = sal_True;

                if  (   bIsRootNode
                    ||  ( m_bShowSlots && _pParentEntry && ( GetModel()->GetDepth( _pParentEntry ) == 0 ) )
                        // if we show slots (as in the customize dialog)
                        // then the user & share are added at depth=1
                    )
                {
                    if ( sUIName == "user" )
                    {
                        sUIName = m_sMyMacros;
                        bIsRootNode = sal_True;
                    }
                    else if ( sUIName == "share" )
                    {
                        sUIName = m_sProdMacros;
                        bIsRootNode = sal_True;
                    }
                    else if ( !sUIName.equals( sCurrentDocTitle ) )
                    {
                        bDisplay = sal_False;
                    }
                }

                if ( !bDisplay )
                    continue;

                if ( children[n]->getType() == browse::BrowseNodeTypes::SCRIPT )
                    continue;

                SvTreeListEntry* pNewEntry = InsertEntry( sUIName, _pParentEntry );

                Image aImage = GetImage( theChild, comphelper::getProcessComponentContext(), bIsRootNode );
                SetExpandedEntryBmp( pNewEntry, aImage );
                SetCollapsedEntryBmp( pNewEntry, aImage );

                SvxGroupInfo_Impl* pInfo =
                    new SvxGroupInfo_Impl( SVX_CFGGROUP_SCRIPTCONTAINER, 0, theChild );
                pNewEntry->SetUserData( pInfo );
                aArr.push_back( pInfo );

                if ( _bCheapChildrenOnDemand )
                {
                    /* i30923 - Would be nice if there was a better
                    * way to determine if a basic lib had children
                    * without having to ask for them (which forces
                    * the library to be loaded */
                    pNewEntry->EnableChildrenOnDemand( sal_True );
                }
                else
                {
                    // if there are granchildren we're interested in, display the '+' before
                    // the entry, but do not yet expand
                    Sequence< Reference< browse::XBrowseNode > > grandchildren =
                        children[n]->getChildNodes();

                    for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                    {
                        if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                        {
                            pNewEntry->EnableChildrenOnDemand( sal_True );
                            break;
                        }
                    }
                }
            }
        }
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SvxConfigGroupListBox_Impl::Init()
{
    SetUpdateMode(sal_False);
    ClearAll();

    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );

    // are we showing builtin commands?
    if ( m_bShowSlots && m_xFrame.is() )
    {
        Reference< lang::XMultiComponentFactory > xMCF =
            xContext->getServiceManager();

        Reference< frame::XDispatchInformationProvider > xDIP(
            m_xFrame, UNO_QUERY );

        Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xContext) );

        OUString aModuleId;
        try{
            aModuleId = xModuleManager->identify( m_xFrame );
        }catch(const uno::Exception&)
            { aModuleId = ::rtl::OUString(); }

        Reference< container::XNameAccess > const xNameAccess(
                frame::UICommandDescription::create(xContext) );
        xNameAccess->getByName( aModuleId ) >>= m_xModuleCommands;

        Reference< container::XNameAccess > xAllCategories(
            xMCF->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.ui.UICategoryDescription" )),
                xContext ),
            UNO_QUERY );

        Reference< container::XNameAccess > xModuleCategories;
        if ( xAllCategories.is() )
        {
            if ( !aModuleId.isEmpty() )
            {
                try
                {
                    xModuleCategories = Reference< container::XNameAccess >(
                           xAllCategories->getByName( aModuleId ), UNO_QUERY );
                }
                catch ( container::NoSuchElementException& )
                {
                }
            }

            if ( !xModuleCategories.is() )
            {
                xModuleCategories = xAllCategories;
            }
        }

        if ( xModuleCategories.is() )
        {
            Sequence< sal_Int16 > gids =
                xDIP->getSupportedCommandGroups();

            for ( sal_Int32 i = 0; i < gids.getLength(); ++i )
            {
                Sequence< frame::DispatchInformation > commands;
                try
                {
                    commands =
                        xDIP->getConfigurableDispatchInformation( gids[i] );
                }
                catch ( container::NoSuchElementException& )
                {
                    continue;
                }

                if ( commands.getLength() == 0 )
                {
                    continue;
                }

                sal_Int32 gid = gids[i];
                OUString idx = OUString::valueOf( gid );
                OUString group = idx;
                try
                {
                    xModuleCategories->getByName( idx ) >>= group;
                }
                catch ( container::NoSuchElementException& )
                {
                }

                SvTreeListEntry *pEntry = InsertEntry( group, NULL );

                SvxGroupInfo_Impl *pInfo =
                    new SvxGroupInfo_Impl( SVX_CFGGROUP_FUNCTION, gids[i] );
                aArr.push_back( pInfo );

                pEntry->SetUserData( pInfo );
            }
        }
    }

    // Add Scripting Framework entries
    Reference< browse::XBrowseNode > rootNode;

    try
    {
        Reference< browse::XBrowseNodeFactory > xFac( xContext->getValueByName(
                                                          OUString(RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.script.browse.theBrowseNodeFactory")) ), UNO_QUERY_THROW );
        rootNode.set( xFac->createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( rootNode.is() )
    {
        if ( m_bShowSlots )
        {
            SvxGroupInfo_Impl *pInfo =
                new SvxGroupInfo_Impl( SVX_CFGGROUP_SCRIPTCONTAINER, 0, rootNode );

            String aTitle =
                String( CUI_RES( STR_SELECTOR_MACROS ) );

            SvTreeListEntry *pNewEntry = InsertEntry( aTitle, NULL );
            pNewEntry->SetUserData( pInfo );
            pNewEntry->EnableChildrenOnDemand( sal_True );
            aArr.push_back( pInfo );
        }
        else
        {
            fillScriptList( rootNode, NULL, false );
        }
    }
    MakeVisible( GetEntry( 0,0 ) );
    SetUpdateMode( sal_True );
}

Image SvxConfigGroupListBox_Impl::GetImage(
    Reference< browse::XBrowseNode > node,
    Reference< XComponentContext > xCtx,
    bool bIsRootNode
)
{
    Image aImage;
    if ( bIsRootNode )
    {
        if ( node->getName() == "user" || node->getName() == "share" )
        {
            aImage = m_hdImage;
        }
        else
        {
            OUString factoryURL;
            OUString nodeName = node->getName();
            Reference<XInterface> xDocumentModel = getDocumentModel(xCtx, nodeName );
            if ( xDocumentModel.is() )
            {
                Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xCtx) );
                // get the long name of the document:
                OUString appModule( xModuleManager->identify(
                                    xDocumentModel ) );
                Sequence<beans::PropertyValue> moduleDescr;
                Any aAny = xModuleManager->getByName(appModule);
                if( sal_True != ( aAny >>= moduleDescr ) )
                {
                    throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("SFTreeListBox::Init: failed to get PropertyValue")), Reference< XInterface >());
                }
                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if ( pmoduleDescr[ pos ].Name == "ooSetupFactoryEmptyDocumentURL" )
                    {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        break;
                    }
                }
            }
            if( !factoryURL.isEmpty() )
            {
                aImage = SvFileInformationManager::GetFileImage( INetURLObject(factoryURL), false );
            }
            else
            {
                aImage = m_docImage;
            }
        }
    }
    else
    {
        if( node->getType() == browse::BrowseNodeTypes::SCRIPT )
            aImage = m_macImage;
        else
            aImage = m_libImage;
    }
    return aImage;
}

Reference< XInterface  >
SvxConfigGroupListBox_Impl::getDocumentModel(
    Reference< XComponentContext >& xCtx, OUString& docName )
{
    Reference< XInterface > xModel;
    Reference< lang::XMultiComponentFactory > mcf =
            xCtx->getServiceManager();
    Reference< frame::XDesktop > desktop (
        mcf->createInstanceWithContext(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")),                 xCtx ),
            UNO_QUERY );

    Reference< container::XEnumerationAccess > componentsAccess =
        desktop->getComponents();
    Reference< container::XEnumeration > components =
        componentsAccess->createEnumeration();
    while (components->hasMoreElements())
    {
        Reference< frame::XModel > model(
            components->nextElement(), UNO_QUERY );
        if ( model.is() )
        {
            OUString sTdocUrl = ::comphelper::DocumentInfo::getDocumentTitle( model );
            if( sTdocUrl.equals( docName ) )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}

void SvxConfigGroupListBox_Impl::GroupSelected()
{
    SvTreeListEntry *pEntry = FirstSelected();
    SvxGroupInfo_Impl *pInfo = (SvxGroupInfo_Impl*) pEntry->GetUserData();
    pFunctionListBox->SetUpdateMode(sal_False);
    pFunctionListBox->ClearAll();
    if ( pInfo->nKind != SVX_CFGGROUP_FUNCTION &&
             pInfo->nKind != SVX_CFGGROUP_SCRIPTCONTAINER )
    {
        pFunctionListBox->SetUpdateMode(sal_True);
        return;
    }

    switch ( pInfo->nKind )
    {
        case SVX_CFGGROUP_FUNCTION :
        {
            SvTreeListEntry *_pEntry = FirstSelected();
            if ( _pEntry != NULL )
            {
                SvxGroupInfo_Impl *_pInfo =
                    (SvxGroupInfo_Impl*) _pEntry->GetUserData();

                Reference< frame::XDispatchInformationProvider > xDIP(
                    m_xFrame, UNO_QUERY );

                Sequence< frame::DispatchInformation > commands;
                try
                {
                    commands = xDIP->getConfigurableDispatchInformation(
                        _pInfo->nOrd );
                }
                catch ( container::NoSuchElementException& )
                {
                }

                for ( sal_Int32 i = 0; i < commands.getLength(); ++i )
                {
                    if ( commands[i].Command.isEmpty() )
                    {
                        continue;
                    }

                    Image aImage;

                    OUString aCmdURL( commands[i].Command );

                    if ( m_pImageProvider )
                    {
                        aImage = m_pImageProvider->GetImage( aCmdURL );
                    }

                    OUString aLabel;
                    try
                    {
                        Any a = m_xModuleCommands->getByName( aCmdURL );
                        Sequence< beans::PropertyValue > aPropSeq;

                        if ( a >>= aPropSeq )
                        {
                            for ( sal_Int32 k = 0; k < aPropSeq.getLength(); ++k )
                            {
                                if ( aPropSeq[k].Name == "Name" )
                                {
                                    aPropSeq[k].Value >>= aLabel;
                                    break;
                                }
                            }
                        }
                    }
                    catch ( container::NoSuchElementException& )
                    {
                    }

                    if ( aLabel.isEmpty() )
                    {
                        aLabel = commands[i].Command;
                    }

                    SvTreeListEntry* pFuncEntry = NULL;
                    if ( !!aImage )
                    {
                        pFuncEntry = pFunctionListBox->InsertEntry(
                            aLabel, aImage, aImage );
                    }
                    else
                    {
                        pFuncEntry = pFunctionListBox->InsertEntry(
                            aLabel, NULL );
                    }

                    SvxGroupInfo_Impl *_pGroupInfo = new SvxGroupInfo_Impl(
                        SVX_CFGFUNCTION_SLOT, 123, aCmdURL, ::rtl::OUString() );

                    pFunctionListBox->aArr.push_back( _pGroupInfo );

                    pFuncEntry->SetUserData( _pGroupInfo );
                }
            }
            break;
        }

        case SVX_CFGGROUP_SCRIPTCONTAINER:
        {
            Reference< browse::XBrowseNode > rootNode( pInfo->xBrowseNode );

            try {
                if ( rootNode->hasChildNodes() )
                {
                    Sequence< Reference< browse::XBrowseNode > > children =
                        rootNode->getChildNodes();

                    for ( sal_Int32 n = 0; n < children.getLength(); ++n )
                    {
                        if (!children[n].is())
                            continue;
                        if (children[n]->getType() == browse::BrowseNodeTypes::SCRIPT)
                        {
                            OUString uri;
                            OUString description;

                            Reference < beans::XPropertySet >xPropSet( children[n], UNO_QUERY );
                            if (!xPropSet.is())
                            {
                                continue;
                            }

                            Any value = xPropSet->getPropertyValue(
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URI")));
                            value >>= uri;

                            try
                            {
                                value = xPropSet->getPropertyValue(
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")));
                                value >>= description;
                            }
                            catch (Exception &) {
                                // do nothing, the description will be empty
                            }

                            SvxGroupInfo_Impl* _pGroupInfo =
                                new SvxGroupInfo_Impl(
                                    SVX_CFGFUNCTION_SCRIPT, 123, uri, description );

                            Image aImage = GetImage( children[n], Reference< XComponentContext >(), sal_False );
                            SvTreeListEntry* pNewEntry =
                                pFunctionListBox->InsertEntry( children[n]->getName(), NULL );
                            pFunctionListBox->SetExpandedEntryBmp( pNewEntry, aImage );
                            pFunctionListBox->SetCollapsedEntryBmp(pNewEntry, aImage );

                            pNewEntry->SetUserData( _pGroupInfo );

                            pFunctionListBox->aArr.push_back( _pGroupInfo );

                        }
                    }
                }
            }
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            break;
        }

        default:
        {
            return;
        }
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( 0, 0 ) );

    pFunctionListBox->SetUpdateMode(sal_True);
}

sal_Bool SvxConfigGroupListBox_Impl::Expand( SvTreeListEntry* pParent )
{
    sal_Bool bRet = SvTreeListBox::Expand( pParent );
    if ( bRet )
    {
        sal_uLong nEntries = GetOutputSizePixel().Height() / GetEntryHeight();

        sal_uLong nChildCount = GetVisibleChildCount( pParent );

        if ( nChildCount+1 > nEntries )
        {
            MakeVisible( pParent, sal_True );
        }
        else
        {
            SvTreeListEntry *pEntry = GetFirstEntryInView();
            sal_uLong nParentPos = 0;
            while ( pEntry && pEntry != pParent )
            {
                ++nParentPos;
                pEntry = GetNextEntryInView( pEntry );
            }

            if ( nParentPos + nChildCount + 1 > nEntries )
                ScrollOutputArea( (short)( nEntries - ( nParentPos + nChildCount + 1 ) ) );
        }
    }

    return bRet;
}

void SvxConfigGroupListBox_Impl::RequestingChildren( SvTreeListEntry *pEntry )
{
    SvxGroupInfo_Impl *pInfo = (SvxGroupInfo_Impl*) pEntry->GetUserData();
    pInfo->bWasOpened = sal_True;
    switch ( pInfo->nKind )
    {
        case SVX_CFGGROUP_SCRIPTCONTAINER:
        {
            if ( !GetChildCount( pEntry ) )
            {
                Reference< browse::XBrowseNode > rootNode( pInfo->xBrowseNode ) ;
                fillScriptList( rootNode, pEntry, true /* i30923 */ );
            }
            break;
        }

        default:
            OSL_FAIL( "Falscher Gruppentyp!" );
            break;
    }
}

/*
 * Implementation of SvxScriptSelectorDialog
 *
 * This dialog is used for selecting Slot API commands
 * and Scripting Framework Scripts.
 */

SvxScriptSelectorDialog::SvxScriptSelectorDialog(
  Window* pParent, sal_Bool bShowSlots, const Reference< frame::XFrame >& xFrame )
    :
    ModelessDialog( pParent, CUI_RES( RID_DLG_SCRIPTSELECTOR ) ),
    aDialogDescription( this, CUI_RES( TXT_SELECTOR_DIALOG_DESCRIPTION ) ),
    aGroupText( this, CUI_RES( TXT_SELECTOR_CATEGORIES ) ),
    aCategories( this, CUI_RES( BOX_SELECTOR_CATEGORIES ), bShowSlots, xFrame ),
    aFunctionText( this, CUI_RES( TXT_SELECTOR_COMMANDS ) ),
    aCommands( this, CUI_RES( BOX_SELECTOR_COMMANDS ) ),
    aOKButton( this, CUI_RES( BTN_SELECTOR_OK ) ),
    aCancelButton( this, CUI_RES( BTN_SELECTOR_CANCEL ) ),
    aHelpButton( this, CUI_RES( BTN_SELECTOR_HELP ) ),
    aDescription( this, CUI_RES( GRP_SELECTOR_DESCRIPTION ) ),
    aDescriptionText( this, CUI_RES( TXT_SELECTOR_DESCRIPTION ) ),
    m_bShowSlots( bShowSlots )
{

    ResMgr& rMgr = CUI_MGR();

    // If we are showing Slot API commands update labels in the UI, and
    // enable drag'n'drop
    if ( m_bShowSlots )
    {
        aGroupText.SetText( String( ResId( STR_SELECTOR_CATEGORIES, rMgr ) ) );
        aOKButton.SetText( String( ResId( STR_SELECTOR_ADD, rMgr ) ) );
        aCancelButton.SetText( String( ResId( STR_SELECTOR_CLOSE, rMgr ) ) );
        aFunctionText.SetText( String( ResId( STR_SELECTOR_COMMANDS, rMgr ) ) );
        SetDialogDescription(
            String( ResId( STR_SELECTOR_ADD_COMMANDS_DESCRIPTION, rMgr ) ) );
        SetText( String( ResId( STR_SELECTOR_ADD_COMMANDS, rMgr ) ) );

        aCommands.SetDragDropMode( SV_DRAGDROP_APP_COPY );
    }

    ResizeControls();

    aCategories.SetFunctionListBox( &aCommands );
    aCategories.Init();

    aCategories.SetSelectHdl(
            LINK( this, SvxScriptSelectorDialog, SelectHdl ) );
    aCommands.SetSelectHdl( LINK( this, SvxScriptSelectorDialog, SelectHdl ) );
    aCommands.SetDoubleClickHdl( LINK( this, SvxScriptSelectorDialog, FunctionDoubleClickHdl ) );

    aOKButton.SetClickHdl( LINK( this, SvxScriptSelectorDialog, ClickHdl ) );
    aCancelButton.SetClickHdl( LINK( this, SvxScriptSelectorDialog, ClickHdl ) );

    UpdateUI();
    FreeResource();
}

void SvxScriptSelectorDialog::ResizeControls()
{
    Point p, newp;
    Size s, news;
    long gap;

    sal_uInt16 style = TEXT_DRAW_MULTILINE | TEXT_DRAW_TOP |
                   TEXT_DRAW_LEFT | TEXT_DRAW_WORDBREAK;

    // get dimensions of dialog instructions control
    p = aDialogDescription.GetPosPixel();
    s = aDialogDescription.GetSizePixel();

    // get dimensions occupied by text in the control
    Rectangle rect =
        GetTextRect( Rectangle( p, s ), aDialogDescription.GetText(), style );
    news = rect.GetSize();

    // the gap is the difference between the control height and its text height
    gap = s.Height() - news.Height();

    // resize the dialog instructions control
    news = Size( s.Width(), s.Height() - gap );
    aDialogDescription.SetSizePixel( news );

    // resize other controls to fill the gap
    p = aGroupText.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aGroupText.SetPosPixel( newp );

    p = aCategories.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aCategories.SetPosPixel( newp );
    s = aCategories.GetSizePixel();
    news = Size( s.Width(), s.Height() + gap );
    aCategories.SetSizePixel( news );

    p = aFunctionText.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aFunctionText.SetPosPixel( newp );

    p = aCommands.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aCommands.SetPosPixel( newp );
    s = aCommands.GetSizePixel();
    news = Size( s.Width(), s.Height() + gap );
    aCommands.SetSizePixel( news );

    p = aOKButton.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aOKButton.SetPosPixel( newp );

    p = aCancelButton.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aCancelButton.SetPosPixel( newp );

    p = aHelpButton.GetPosPixel();
    newp = Point( p.X(), p.Y() - gap );
    aHelpButton.SetPosPixel( newp );
}

SvxScriptSelectorDialog::~SvxScriptSelectorDialog()
{
}

IMPL_LINK( SvxScriptSelectorDialog, SelectHdl, Control*, pCtrl )
{
    if ( pCtrl == &aCategories )
    {
        aCategories.GroupSelected();
    }
    else if ( pCtrl == &aCommands )
    {
        aCommands.FunctionSelected();
    }
    UpdateUI();
    return 0;
}

IMPL_LINK( SvxScriptSelectorDialog, FunctionDoubleClickHdl, Control*, pCtrl )
{
    (void)pCtrl;
    if ( aOKButton.IsEnabled() )
        return ClickHdl( &aOKButton );
    return 0;
}

// Check if command is selected and enable the OK button accordingly
// Grab the help text for this id if available and update the description field
void
SvxScriptSelectorDialog::UpdateUI()
{
    OUString url = GetScriptURL();
    if ( url != NULL && !url.isEmpty() )
    {
        String rMessage =
            aCommands.GetHelpText( aCommands.FirstSelected() );
        aDescriptionText.SetText( rMessage );

        aOKButton.Enable( sal_True );
    }
    else
    {
        aDescriptionText.SetText( String() );
        aOKButton.Enable( sal_False );
    }
}

IMPL_LINK( SvxScriptSelectorDialog, ClickHdl, Button *, pButton )
{
    if ( pButton == &aCancelButton )
    {
        // If we are displaying Slot API commands then the dialog is being
        // run from Tools/Configure and we should not close it, just hide it
        if ( m_bShowSlots == sal_False )
        {
            EndDialog( RET_CANCEL );
        }
        else
        {
            Hide();
        }
    }
    else if ( pButton == &aOKButton )
    {
        GetAddHdl().Call( this );

        // If we are displaying Slot API commands then this the dialog is being
        // run from Tools/Configure and we should not close it
        if ( m_bShowSlots == sal_False )
        {
            EndDialog( RET_OK );
        }
        else
        {
            // Select the next entry in the list if possible
            SvTreeListEntry* current = aCommands.FirstSelected();
            SvTreeListEntry* next = aCommands.NextSibling( current );

            if ( next != NULL )
            {
                aCommands.Select( next );
            }
        }
    }

    return 0;
}

void
SvxScriptSelectorDialog::SetRunLabel()
{
    aOKButton.SetText( String( CUI_RES( STR_SELECTOR_RUN ) ) );
}

void
SvxScriptSelectorDialog::SetDialogDescription( const String& rDescription )
{
    aDialogDescription.SetText( rDescription );
}

String
SvxScriptSelectorDialog::GetScriptURL() const
{
    OUString result;

    SvTreeListEntry *pEntry = const_cast< SvxScriptSelectorDialog* >( this )->aCommands.GetLastSelectedEntry();
    if ( pEntry )
    {
        SvxGroupInfo_Impl *pData = (SvxGroupInfo_Impl*) pEntry->GetUserData();
        if  (   ( pData->nKind == SVX_CFGFUNCTION_SLOT )
            ||  ( pData->nKind == SVX_CFGFUNCTION_SCRIPT )
            )
        {
            result = pData->sURL;
        }
    }

    return result;
}

String
SvxScriptSelectorDialog::GetSelectedDisplayName()
{
    return aCommands.GetEntryText( aCommands.GetLastSelectedEntry() );
}

String
SvxScriptSelectorDialog::GetSelectedHelpText()
{
    return aCommands.GetHelpText( aCommands.GetLastSelectedEntry() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
