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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XInsertObjectDialog.hpp>
#include <com/sun/star/embed/MSOLEObjectSystemCreator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <comphelper/processfactory.hxx>

#include "insdlg.hxx"
#include <plfilter.hxx>
#include <dialmgr.hxx>
#include <svtools/sores.hxx>

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <svl/urihelper.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/classids.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/ownlist.hxx>
#include <comphelper/seqstream.hxx>

#include "cuires.hrc"

#include <osl/file.hxx>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/settings.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;


static OUString impl_getSvtResString( sal_uInt32 nId )
{
    OUString aRet;
    ResMgr* pMgr = ResMgr::CreateResMgr( "svt", Application::GetSettings().GetUILanguageTag() );
    if( pMgr )
    {
        aRet = ResId( nId, *pMgr );
        delete pMgr;
    }
    return aRet;
}

bool InsertObjectDialog_Impl::IsCreateNew() const
{
    return false;
}

uno::Reference< io::XInputStream > InsertObjectDialog_Impl::GetIconIfIconified( OUString* /*pGraphicMediaType*/ )
{
    return uno::Reference< io::XInputStream >();
}

InsertObjectDialog_Impl::InsertObjectDialog_Impl(vcl::Window * pParent, const OUString& rID,
    const OUString& rUIXMLDescription,
    const css::uno::Reference < css::embed::XStorage >& xStorage)
    : ModalDialog(pParent, rID, rUIXMLDescription)
    , m_xStorage( xStorage )
    , aCnt( m_xStorage )
{
}


IMPL_LINK_NOARG_TYPED(SvInsertOleDlg, DoubleClickHdl, ListBox&, void)
{
    EndDialog( RET_OK );
}

IMPL_LINK_NOARG_TYPED(SvInsertOleDlg, BrowseHdl, Button*, void)
{
    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    Reference< XFilePicker3 > xFilePicker = FilePicker::createWithMode(xContext, TemplateDescription::FILEOPEN_SIMPLE);

    // add filter
    try
    {
        xFilePicker->appendFilter(
             OUString(),
             "*.*"
             );
    }
    catch( const IllegalArgumentException& )
    {
        SAL_WARN( "cui", "caught IllegalArgumentException when registering filter\n" );
    }

    if( xFilePicker->execute() == ExecutableDialogResults::OK )
    {
        Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
        INetURLObject aObj( aPathSeq[0] );
        m_pEdFilepath->SetText( aObj.PathToFileName() );
    }
}


IMPL_LINK_NOARG_TYPED(SvInsertOleDlg, RadioHdl, Button*, void)
{
    if ( m_pRbNewObject->IsChecked() )
    {
        m_pObjectTypeFrame->Show();
        m_pFileFrame->Hide();
    }
    else
    {
        m_pFileFrame->Show();
        m_pObjectTypeFrame->Hide();
    }
}


void SvInsertOleDlg::SelectDefault()
{
    m_pLbObjecttype->SelectEntryPos(0);
}


SvInsertOleDlg::SvInsertOleDlg
(
    vcl::Window* pParent,
    const Reference < embed::XStorage >& xStorage,
    const SvObjectServerList* pServers
)
    : InsertObjectDialog_Impl( pParent, "InsertOLEObjectDialog", "cui/ui/insertoleobject.ui", xStorage ),
    m_pServers( pServers )
{
    get(m_pRbNewObject, "createnew");
    get(m_pRbObjectFromfile, "createfromfile");
    get(m_pObjectTypeFrame, "objecttypeframe");
    get(m_pLbObjecttype, "types");
    get(m_pFileFrame, "fileframe");
    get(m_pEdFilepath, "urled");
    get(m_pBtnFilepath, "urlbtn");
    get(m_pCbFilelink, "linktofile");
    m_pLbObjecttype->SetDoubleClickHdl( LINK( this, SvInsertOleDlg, DoubleClickHdl ) );
    m_pBtnFilepath->SetClickHdl( LINK( this, SvInsertOleDlg, BrowseHdl ) );
    Link<Button*,void> aLink( LINK( this, SvInsertOleDlg, RadioHdl ) );
    m_pRbNewObject->SetClickHdl( aLink );
    m_pRbObjectFromfile->SetClickHdl( aLink );
    m_pRbNewObject->Check();
    RadioHdl( nullptr );
}

SvInsertOleDlg::~SvInsertOleDlg()
{
    disposeOnce();
}

void SvInsertOleDlg::dispose()
{
    m_pRbNewObject.clear();
    m_pRbObjectFromfile.clear();
    m_pObjectTypeFrame.clear();
    m_pLbObjecttype.clear();
    m_pFileFrame.clear();
    m_pEdFilepath.clear();
    m_pBtnFilepath.clear();
    m_pCbFilelink.clear();
    InsertObjectDialog_Impl::dispose();
}

short SvInsertOleDlg::Execute()
{
    short nRet = RET_OK;
    SvObjectServerList  aObjS;
    if ( !m_pServers )
    {
        // if no list was provided, take the complete one
        aObjS.FillInsertObjects();
        m_pServers = &aObjS;
    }

    // fill listbox and select default
    ListBox& rBox = GetObjectTypes();
    rBox.SetUpdateMode( false );
    for ( sal_uLong i = 0; i < m_pServers->Count(); i++ )
        rBox.InsertEntry( (*m_pServers)[i].GetHumanName() );
    rBox.SetUpdateMode( true );
    SelectDefault();
    OUString aName;

    DBG_ASSERT( m_xStorage.is(), "No storage!");
    if ( m_xStorage.is() && ( nRet = Dialog::Execute() ) == RET_OK )
    {
        OUString aFileName;
        bool bCreateNew = IsCreateNew();
        if ( bCreateNew )
        {
            // create and insert new embedded object
            OUString aServerName = rBox.GetSelectEntry();
            const SvObjectServer* pS = m_pServers->Get( aServerName );
            if ( pS )
            {
                if( pS->GetClassName() == SvGlobalName( SO3_OUT_CLASSID ) )
                {
                    try
                    {
                        uno::Reference < embed::XInsertObjectDialog > xDialogCreator(
                            embed::MSOLEObjectSystemCreator::create( ::comphelper::getProcessComponentContext() ),
                            uno::UNO_QUERY );

                        if ( xDialogCreator.is() )
                        {
                            aName = aCnt.CreateUniqueObjectName();
                            embed::InsertedObjectInfo aNewInf = xDialogCreator->createInstanceByDialog(
                                                                    m_xStorage,
                                                                    aName,
                                                                    uno::Sequence < beans::PropertyValue >() );

                            OSL_ENSURE( aNewInf.Object.is(), "The object must be created or an exception must be thrown!" );
                            m_xObj = aNewInf.Object;
                            for ( sal_Int32 nInd = 0; nInd < aNewInf.Options.getLength(); nInd++ )
                                if ( aNewInf.Options[nInd].Name == "Icon" )
                                {
                                    aNewInf.Options[nInd].Value >>= m_aIconMetaFile;
                                }
                                else if ( aNewInf.Options[nInd].Name == "IconFormat" )
                                {
                                    datatransfer::DataFlavor aFlavor;
                                    if ( aNewInf.Options[nInd].Value >>= aFlavor )
                                        m_aIconMediaType = aFlavor.MimeType;
                                }

                        }
                    }
                    catch( ucb::CommandAbortedException& )
                    {
                        // the user has pressed cancel
                    }
                    catch( uno::Exception& )
                    {
                        // TODO: Error handling
                    }
                }
                else
                {
                    // create object with desired ClassId
                    m_xObj = aCnt.CreateEmbeddedObject( pS->GetClassName().GetByteSequence(), aName );
                }

                if ( !m_xObj.is() )
                {
                    if( !aFileName.isEmpty() )  // from OLE Dialog
                    {
                        // object couldn't be created from file
                        // global Resource from svtools (former so3 resource)
                        OUString aErr( impl_getSvtResString( STR_ERROR_OBJNOCREATE_FROM_FILE ) );
                        aErr = aErr.replaceFirst( "%", aFileName );
                        ScopedVclPtrInstance<MessageDialog>::Create(this, aErr)->Execute();
                    }
                    else
                    {
                        // object couldn't be created
                        // global Resource from svtools (former so3 resource)
                        OUString aErr( impl_getSvtResString( STR_ERROR_OBJNOCREATE ) );
                        aErr = aErr.replaceFirst( "%", aServerName );
                        ScopedVclPtrInstance<MessageDialog>::Create(this, aErr)->Execute();
                    }
                }
            }
        }
        else
        {
            aFileName = GetFilePath();
            INetURLObject aURL;
            aURL.SetSmartProtocol( INetProtocol::File );
            aURL.SetSmartURL( aFileName );
            aFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );
            bool bLink = IsLinked();

            if ( !aFileName.isEmpty() )
            {
                // create MediaDescriptor for file to create object from
                uno::Sequence < beans::PropertyValue > aMedium( 2 );
                aMedium[0].Name = "URL";
                aMedium[0].Value <<= OUString( aFileName );

                uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                uno::Reference< task::XInteractionHandler2 > xInteraction(
                    task::InteractionHandler::createWithParent(xContext, nullptr) );

               aMedium[1].Name = "InteractionHandler";
               aMedium[1].Value <<= xInteraction;

                // create object from media descriptor
                if ( bLink )
                    m_xObj = aCnt.InsertEmbeddedLink( aMedium, aName );
                else
                    m_xObj = aCnt.InsertEmbeddedObject( aMedium, aName );
            }

            if ( !m_xObj.is() )
            {
                // object couldn't be created from file
                // global Resource from svtools (former so3 resource)
                OUString aErr( impl_getSvtResString( STR_ERROR_OBJNOCREATE_FROM_FILE ) );
                aErr = aErr.replaceFirst( "%", aFileName );
                ScopedVclPtrInstance<MessageDialog>::Create(this, aErr)->Execute();
            }
        }
    }

    m_pServers = nullptr;
    return nRet;
}

uno::Reference< io::XInputStream > SvInsertOleDlg::GetIconIfIconified( OUString* pGraphicMediaType )
{
    if ( m_aIconMetaFile.getLength() )
    {
        if ( pGraphicMediaType )
            *pGraphicMediaType = m_aIconMediaType;

        return uno::Reference< io::XInputStream >( new ::comphelper::SequenceInputStream( m_aIconMetaFile ) );
    }

    return uno::Reference< io::XInputStream >();
}

IMPL_LINK_NOARG_TYPED(SvInsertPlugInDialog, BrowseHdl, Button*, void)
{
    Sequence< OUString > aFilterNames, aFilterTypes;
    fillNetscapePluginFilters( aFilterNames, aFilterTypes );

    Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    Reference< XFilePicker3 > xFilePicker = ui::dialogs::FilePicker::createWithMode( xContext, ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );

    // add the filters
    try
    {
        const OUString* pNames = aFilterNames.getConstArray();
        const OUString* pTypes = aFilterTypes.getConstArray();
        for( int i = 0; i < aFilterNames.getLength(); i++ )
            xFilePicker->appendFilter( pNames[i], pTypes[i] );
    }
    catch( IllegalArgumentException& )
    {
        DBG_ASSERT( false, "caught IllegalArgumentException when registering filter\n" );
    }

    if( xFilePicker->execute() == ExecutableDialogResults::OK )
    {
        Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
        INetURLObject aObj( aPathSeq[0] );
        m_pEdFileurl->SetText(aObj.PathToFileName());
    }
}



SvInsertPlugInDialog::SvInsertPlugInDialog(vcl::Window* pParent,
    const uno::Reference < embed::XStorage >& xStorage)
    : InsertObjectDialog_Impl(pParent, "InsertPluginDialog", "cui/ui/insertplugin.ui", xStorage)
    , m_pURL(nullptr)
{
    get(m_pEdFileurl, "urled");
    get(m_pBtnFileurl, "urlbtn");
    get(m_pEdPluginsOptions, "pluginoptions");
    m_pBtnFileurl->SetClickHdl(LINK(this, SvInsertPlugInDialog, BrowseHdl));
}

SvInsertPlugInDialog::~SvInsertPlugInDialog()
{
    disposeOnce();
}

void SvInsertPlugInDialog::dispose()
{
    delete m_pURL;
    m_pURL = nullptr;
    m_pEdFileurl.clear();
    m_pBtnFileurl.clear();
    m_pEdPluginsOptions.clear();
    InsertObjectDialog_Impl::dispose();
}

static void Plugin_ImplFillCommandSequence( const OUString& aCommands, uno::Sequence< beans::PropertyValue >& aCommandSequence )
{
    sal_Int32 nEaten;
    SvCommandList aLst;
    aLst.AppendCommands( aCommands, &nEaten );

    const size_t nCount = aLst.size();
    aCommandSequence.realloc( nCount );
    for( size_t nIndex = 0; nIndex < nCount; nIndex++ )
    {
        aCommandSequence[nIndex].Name = aLst[ nIndex ].GetCommand();
        aCommandSequence[nIndex].Handle = -1;
        aCommandSequence[nIndex].Value = makeAny( aLst[ nIndex ].GetArgument() );
        aCommandSequence[nIndex].State = beans::PropertyState_DIRECT_VALUE;
    }
}

short SvInsertPlugInDialog::Execute()
{
    short nRet = RET_OK;
    m_aCommands.clear();
    DBG_ASSERT( m_xStorage.is(), "No storage!");
    if ( m_xStorage.is() && ( nRet = Dialog::Execute() ) == RET_OK )
    {
        if ( !m_pURL )
            m_pURL = new INetURLObject();
        else
            *m_pURL = INetURLObject();

        m_aCommands = GetPlugInOptions();
        OUString aURL = GetPlugInFile();

        // URL can be a valid and absolute URL or a system file name
        m_pURL->SetSmartProtocol( INetProtocol::File );
        if ( aURL.isEmpty() || m_pURL->SetSmartURL( aURL ) )
        {
            // create a plugin object
            OUString aName;
            SvGlobalName aClassId( SO3_PLUGIN_CLASSID );
            m_xObj = aCnt.CreateEmbeddedObject( aClassId.GetByteSequence(), aName );
        }

        if ( m_xObj.is() )
        {
            // set properties from dialog
            if ( m_xObj->getCurrentState() == embed::EmbedStates::LOADED )
                m_xObj->changeState( embed::EmbedStates::RUNNING );

            uno::Reference < beans::XPropertySet > xSet( m_xObj->getComponent(), uno::UNO_QUERY );
            if ( xSet.is() )
            {
                xSet->setPropertyValue( "PluginURL",
                        makeAny( OUString( m_pURL->GetMainURL( INetURLObject::NO_DECODE ) ) ) );
                uno::Sequence< beans::PropertyValue > aCommandSequence;
                Plugin_ImplFillCommandSequence( m_aCommands, aCommandSequence );
                xSet->setPropertyValue( "PluginCommands", makeAny( aCommandSequence ) );
            }
        }
        else
        {
            // PlugIn couldn't be created
            // global Resource from svtools (former so3 resource)
            OUString aErr( impl_getSvtResString( STR_ERROR_OBJNOCREATE_PLUGIN ) );
            aErr = aErr.replaceFirst( "%", aURL );
            ScopedVclPtrInstance<MessageDialog>::Create(this, aErr)->Execute();
        }
    }

    return nRet;
}

SfxInsertFloatingFrameDialog::SfxInsertFloatingFrameDialog( vcl::Window *pParent,
                            const css::uno::Reference < css::embed::XStorage >& xStorage )
    : InsertObjectDialog_Impl( pParent, "InsertFloatingFrameDialog", "cui/ui/insertfloatingframe.ui",
                              xStorage )
{
    Init();
}

SfxInsertFloatingFrameDialog::SfxInsertFloatingFrameDialog( vcl::Window *pParent,
                            const uno::Reference < embed::XEmbeddedObject >& xObj )
    : InsertObjectDialog_Impl( pParent, "InsertFloatingFrameDialog", "cui/ui/insertfloatingframe.ui",
                              uno::Reference < embed::XStorage >() )
{
    m_xObj = xObj;

    Init();
}

SfxInsertFloatingFrameDialog::~SfxInsertFloatingFrameDialog()
{
    disposeOnce();
}

void SfxInsertFloatingFrameDialog::dispose()
{
    m_pEDName.clear();
    m_pEDURL.clear();
    m_pBTOpen.clear();
    m_pRBScrollingOn.clear();
    m_pRBScrollingOff.clear();
    m_pRBScrollingAuto.clear();
    m_pRBFrameBorderOn.clear();
    m_pRBFrameBorderOff.clear();
    m_pFTMarginWidth.clear();
    m_pNMMarginWidth.clear();
    m_pCBMarginWidthDefault.clear();
    m_pFTMarginHeight.clear();
    m_pNMMarginHeight.clear();
    m_pCBMarginHeightDefault.clear();
    InsertObjectDialog_Impl::dispose();
}

void SfxInsertFloatingFrameDialog::Init()
{
    get(m_pEDName, "edname");
    get(m_pEDURL, "edurl");
    get(m_pBTOpen, "buttonbrowse");
    get(m_pRBScrollingOn, "scrollbaron");
    get(m_pRBScrollingOff, "scrollbaroff");
    get(m_pRBScrollingAuto, "scrollbarauto");
    get(m_pRBFrameBorderOn, "borderon");
    get(m_pRBFrameBorderOff, "borderoff");
    get(m_pFTMarginWidth, "widthlabel");
    get(m_pNMMarginWidth, "width");
    get(m_pCBMarginWidthDefault, "defaultwidth");
    get(m_pFTMarginHeight, "heightlabel");
    get(m_pNMMarginHeight, "height");
    get(m_pCBMarginHeightDefault, "defaultheight");

    Link<Button*, void> aLink( LINK( this, SfxInsertFloatingFrameDialog, CheckHdl ) );
    m_pCBMarginWidthDefault->SetClickHdl( aLink );
    m_pCBMarginHeightDefault->SetClickHdl( aLink );

    m_pCBMarginWidthDefault->Check();
    m_pCBMarginHeightDefault->Check();
    m_pRBScrollingAuto->Check();
    m_pRBFrameBorderOn->Check();

    m_pBTOpen->SetClickHdl( LINK( this, SfxInsertFloatingFrameDialog, OpenHdl ) );
}

short SfxInsertFloatingFrameDialog::Execute()
{
    short nRet = RET_OK;
    bool bOK = false;
    uno::Reference < beans::XPropertySet > xSet;
    if ( m_xObj.is() )
    {
        try
        {
            if ( m_xObj->getCurrentState() == embed::EmbedStates::LOADED )
                m_xObj->changeState( embed::EmbedStates::RUNNING );
            xSet.set( m_xObj->getComponent(), uno::UNO_QUERY );
            OUString aStr;
            uno::Any aAny = xSet->getPropertyValue( "FrameURL" );
            if ( aAny >>= aStr )
                m_pEDURL->SetText( aStr );
            aAny = xSet->getPropertyValue( "FrameName" );
            if ( aAny >>= aStr )
                m_pEDName->SetText( aStr );

            sal_Int32 nSize = SIZE_NOT_SET;
            aAny = xSet->getPropertyValue( "FrameMarginWidth" );
            aAny >>= nSize;

            if ( nSize == SIZE_NOT_SET )
            {
                m_pCBMarginWidthDefault->Check();
                m_pNMMarginWidth->SetText( OUString::number(DEFAULT_MARGIN_WIDTH) );
                m_pFTMarginWidth->Enable( false );
                m_pNMMarginWidth->Enable( false );
            }
            else
                m_pNMMarginWidth->SetText( OUString::number( nSize ) );

            aAny = xSet->getPropertyValue( "FrameMarginHeight" );
            aAny >>= nSize;

            if ( nSize == SIZE_NOT_SET )
            {
                m_pCBMarginHeightDefault->Check();
                m_pNMMarginHeight->SetText( OUString::number(DEFAULT_MARGIN_HEIGHT) );
                m_pFTMarginHeight->Enable( false );
                m_pNMMarginHeight->Enable( false );
            }
            else
                m_pNMMarginHeight->SetText( OUString::number( nSize ) );

            bool bScrollOn = false;
            bool bScrollOff = false;
            bool bScrollAuto = false;

            bool bSet = false;
            aAny = xSet->getPropertyValue( "FrameIsAutoScroll" );
            aAny >>= bSet;
            if ( !bSet )
            {
                aAny = xSet->getPropertyValue( "FrameIsScrollingMode" );
                aAny >>= bSet;
                bScrollOn = bSet;
                bScrollOff = !bSet;
            }
            else
                bScrollAuto = true;

            m_pRBScrollingOn->Check( bScrollOn );
            m_pRBScrollingOff->Check( bScrollOff );
            m_pRBScrollingAuto->Check( bScrollAuto );

            bSet = false;
            aAny = xSet->getPropertyValue( "FrameIsAutoBorder" );
            aAny >>= bSet;
            if ( !bSet )
            {
                aAny = xSet->getPropertyValue( "FrameIsBorder" );
                aAny >>= bSet;
                m_pRBFrameBorderOn->Check( bSet );
                m_pRBFrameBorderOff->Check( !bSet );
            }

            SetUpdateMode( true );
            bOK = true;
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "No IFrame!" );
        }
    }
    else
    {
        DBG_ASSERT( m_xStorage.is(), "No storage!");
        bOK = m_xStorage.is();
    }

    if ( bOK && ( nRet = Dialog::Execute() ) == RET_OK )
    {
        OUString aURL;
        if ( !m_pEDURL->GetText().isEmpty() )
        {
            // URL can be a valid and absolute URL or a system file name
            INetURLObject aObj;
            aObj.SetSmartProtocol( INetProtocol::File );
            if ( aObj.SetSmartURL( m_pEDURL->GetText() ) )
                aURL = aObj.GetMainURL( INetURLObject::NO_DECODE );
        }

        if ( !m_xObj.is() && !aURL.isEmpty() )
        {
            // create the object
            OUString aName;
            SvGlobalName aClassId( SO3_IFRAME_CLASSID );
            m_xObj = aCnt.CreateEmbeddedObject( aClassId.GetByteSequence(), aName );
            if ( m_xObj->getCurrentState() == embed::EmbedStates::LOADED )
                m_xObj->changeState( embed::EmbedStates::RUNNING );
            xSet.set( m_xObj->getComponent(), uno::UNO_QUERY );
        }

        if ( m_xObj.is() )
        {
            try
            {
                bool bIPActive = m_xObj->getCurrentState() == embed::EmbedStates::INPLACE_ACTIVE;
                if ( bIPActive )
                    m_xObj->changeState( embed::EmbedStates::RUNNING );

                OUString aName = m_pEDName->GetText();
                ScrollingMode eScroll = ScrollingNo;
                if ( m_pRBScrollingOn->IsChecked() )
                    eScroll = ScrollingYes;
                if ( m_pRBScrollingOff->IsChecked() )
                    eScroll = ScrollingNo;
                if ( m_pRBScrollingAuto->IsChecked() )
                    eScroll = ScrollingAuto;

                bool bHasBorder = m_pRBFrameBorderOn->IsChecked();

                long lMarginWidth;
                if ( !m_pCBMarginWidthDefault->IsChecked() )
                    lMarginWidth = (long) m_pNMMarginWidth->GetText().toInt32();
                else
                    lMarginWidth = SIZE_NOT_SET;

                long lMarginHeight;
                if ( !m_pCBMarginHeightDefault->IsChecked() )
                    lMarginHeight = (long) m_pNMMarginHeight->GetText().toInt32();
                else
                    lMarginHeight = SIZE_NOT_SET;

                xSet->setPropertyValue( "FrameURL", makeAny( aURL ) );
                xSet->setPropertyValue( "FrameName", makeAny( aName ) );

                if ( eScroll == ScrollingAuto )
                    xSet->setPropertyValue( "FrameIsAutoScroll", makeAny( true ) );
                else
                    xSet->setPropertyValue( "FrameIsScrollingMode", makeAny( eScroll == ScrollingYes ) );

                xSet->setPropertyValue( "FrameIsBorder", makeAny( bHasBorder ) );
                xSet->setPropertyValue( "FrameMarginWidth", makeAny( sal_Int32( lMarginWidth ) ) );
                xSet->setPropertyValue( "FrameMarginHeight", makeAny( sal_Int32( lMarginHeight ) ) );

                if ( bIPActive )
                    m_xObj->changeState( embed::EmbedStates::INPLACE_ACTIVE );
            }
            catch ( uno::Exception& )
            {
                OSL_FAIL( "No IFrame!" );
            }
        }
    }

    return nRet;
}


IMPL_LINK_TYPED( SfxInsertFloatingFrameDialog, CheckHdl, Button*, pButton, void )
{
    CheckBox* pCB = static_cast<CheckBox*>(pButton);
    if ( pCB == m_pCBMarginWidthDefault )
    {
        if ( pCB->IsChecked() )
            m_pNMMarginWidth->SetText( OUString::number(DEFAULT_MARGIN_WIDTH) );
        m_pFTMarginWidth->Enable( !pCB->IsChecked() );
        m_pNMMarginWidth->Enable( !pCB->IsChecked() );
    }

    if ( pCB == m_pCBMarginHeightDefault )
    {
        if ( pCB->IsChecked() )
            m_pNMMarginHeight->SetText( OUString::number(DEFAULT_MARGIN_HEIGHT) );
        m_pFTMarginHeight->Enable( !pCB->IsChecked() );
        m_pNMMarginHeight->Enable( !pCB->IsChecked() );
    }
}


IMPL_LINK_NOARG_TYPED( SfxInsertFloatingFrameDialog, OpenHdl, Button*, void)
{
    vcl::Window* pOldParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );

    // create the file dialog
    sfx2::FileDialogHelper aFileDlg(
            ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0, OUString() );

    // set the title
    aFileDlg.SetTitle(CUI_RESSTR(RID_SVXSTR_SELECT_FILE_IFRAME));

    // show the dialog
    if ( aFileDlg.Execute() == ERRCODE_NONE )
        m_pEDURL->SetText(
            INetURLObject( aFileDlg.GetPath() ).GetMainURL( INetURLObject::DECODE_WITH_CHARSET ) );

    Application::SetDefDialogParent( pOldParent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
