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
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <insdlg.hxx>
#include <dialmgr.hxx>
#include <osl/diagnose.h>
#include <svtools/imagemgr.hxx>
#include <svtools/strings.hrc>
#include <svtools/svtresid.hxx>

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <utility>
#include <vcl/image.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/classids.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/viewsh.hxx>
#include <comphelper/seqstream.hxx>
#include <sfx2/viewfrm.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;

bool InsertObjectDialog_Impl::IsCreateNew() const
{
    return false;
}

uno::Reference< io::XInputStream > InsertObjectDialog_Impl::GetIconIfIconified( OUString* /*pGraphicMediaType*/ )
{
    return uno::Reference< io::XInputStream >();
}

InsertObjectDialog_Impl::InsertObjectDialog_Impl(weld::Window* pParent,
    const OUString& rUIXMLDescription, const OUString& rID,
    css::uno::Reference < css::embed::XStorage > xStorage)
    : GenericDialogController(pParent, rUIXMLDescription, rID)
    , m_xStorage(std::move( xStorage ))
    , aCnt( m_xStorage )
{
}

IMPL_LINK_NOARG(SvInsertOleDlg, DoubleClickHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG(SvInsertOleDlg, BrowseHdl, weld::Button&, void)
{
    sfx2::FileDialogHelper aHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, m_xDialog.get());
    aHelper.SetContext(sfx2::FileDialogHelper::InsertOLE);
    const Reference< XFilePicker3 >& xFilePicker = aHelper.GetFilePicker();

    // add filter
    try
    {
        xFilePicker->appendFilter(CuiResId(RID_CUISTR_FILTER_ALL), "*.*");
    }
    catch( const IllegalArgumentException& )
    {
        TOOLS_WARN_EXCEPTION("cui.dialogs", "caught IllegalArgumentException when registering filter" );
    }

    if( xFilePicker->execute() == ExecutableDialogResults::OK )
    {
        Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
        INetURLObject aObj( aPathSeq[0] );
        m_xEdFilepath->set_text(aObj.PathToFileName());
    }
}

IMPL_LINK(SvInsertOleDlg, RadioHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    if (m_xRbNewObject->get_active())
    {
        m_xObjectTypeFrame->show();
        m_xFileFrame->hide();
    }
    else
    {
        m_xFileFrame->show();
        m_xObjectTypeFrame->hide();
    }
}

SvInsertOleDlg::SvInsertOleDlg(weld::Window* pParent, const Reference<embed::XStorage>& xStorage,
        const SvObjectServerList* pServers)
    : InsertObjectDialog_Impl( pParent, "cui/ui/insertoleobject.ui", "InsertOLEObjectDialog", xStorage)
    , m_pServers( pServers )
    , m_xRbNewObject(m_xBuilder->weld_radio_button("createnew"))
    , m_xRbObjectFromfile(m_xBuilder->weld_radio_button("createfromfile"))
    , m_xObjectTypeFrame(m_xBuilder->weld_frame("objecttypeframe"))
    , m_xLbObjecttype(m_xBuilder->weld_tree_view("types"))
    , m_xFileFrame(m_xBuilder->weld_frame("fileframe"))
    , m_xEdFilepath(m_xBuilder->weld_entry("urled"))
    , m_xBtnFilepath(m_xBuilder->weld_button("urlbtn"))
    , m_xCbFilelink(m_xBuilder->weld_check_button("linktofile"))
    , m_xCbAsIcon(m_xBuilder->weld_check_button("asicon"))
{
    m_xLbObjecttype->set_size_request(m_xLbObjecttype->get_approximate_digit_width() * 32,
                                      m_xLbObjecttype->get_height_rows(6));
    m_xLbObjecttype->connect_row_activated(LINK(this, SvInsertOleDlg, DoubleClickHdl));
    m_xBtnFilepath->connect_clicked(LINK( this, SvInsertOleDlg, BrowseHdl));
    Link<weld::Toggleable&,void> aLink( LINK( this, SvInsertOleDlg, RadioHdl ) );
    m_xRbNewObject->connect_toggled( aLink );
    m_xRbObjectFromfile->connect_toggled( aLink );
    m_xRbNewObject->set_active(true);
}

short SvInsertOleDlg::run()
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
    m_xLbObjecttype->freeze();
    for ( size_t i = 0; i < m_pServers->Count(); i++ )
        m_xLbObjecttype->append_text((*m_pServers)[i].GetHumanName());
    m_xLbObjecttype->thaw();
    m_xLbObjecttype->select(0);

    DBG_ASSERT( m_xStorage.is(), "No storage!");
    if ( m_xStorage.is() && ( nRet = InsertObjectDialog_Impl::run() ) == RET_OK )
    {
        OUString aFileName;
        OUString aName;
        bool bCreateNew = IsCreateNew();
        if ( bCreateNew )
        {
            // create and insert new embedded object
            OUString aServerName = m_xLbObjecttype->get_selected_text();
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

                            uno::Reference<task::XStatusIndicator> xProgress;
                            OUString aProgressText;
                            if (SfxViewFrame* pFrame = SfxViewFrame::Current())
                            {
                                // Have a current frame, create a matching progressbar, but don't start it yet.
                                uno::Reference<frame::XFrame> xFrame
                                    = pFrame->GetFrame().GetFrameInterface();
                                uno::Reference<task::XStatusIndicatorFactory> xProgressFactory(
                                    xFrame, uno::UNO_QUERY);
                                if (xProgressFactory.is())
                                {
                                    xProgress = xProgressFactory->createStatusIndicator();
                                    if (xProgress)
                                    {
                                        aProgressText = CuiResId(RID_CUISTR_OLE_INSERT);
                                    }
                                }
                            }

                            const embed::InsertedObjectInfo aNewInf = xDialogCreator->createInstanceByDialog(
                                                                    m_xStorage,
                                                                    aName,
                                                                    {comphelper::makePropertyValue("StatusIndicator", xProgress),
                                                                     comphelper::makePropertyValue("StatusIndicatorText", aProgressText)} );

                            OSL_ENSURE( aNewInf.Object.is(), "The object must be created or an exception must be thrown!" );
                            m_xObj = aNewInf.Object;
                            for ( const auto& opt : aNewInf.Options )
                                if ( opt.Name == "Icon" )
                                {
                                    opt.Value >>= m_aIconMetaFile;
                                }
                                else if ( opt.Name == "IconFormat" )
                                {
                                    datatransfer::DataFlavor aFlavor;
                                    if ( opt.Value >>= aFlavor )
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
                        OUString aErr(SvtResId(STR_ERROR_OBJNOCREATE_FROM_FILE));
                        aErr = aErr.replaceFirst( "%", aFileName );

                        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                  VclMessageType::Warning, VclButtonsType::Ok, aErr));
                        xBox->run();
                    }
                    else
                    {
                        // object couldn't be created
                        // global Resource from svtools (former so3 resource)
                        OUString aErr(SvtResId(STR_ERROR_OBJNOCREATE));
                        aErr = aErr.replaceFirst( "%", aServerName );

                        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                                  VclMessageType::Warning, VclButtonsType::Ok, aErr));
                        xBox->run();
                    }
                }
            }
        }
        else
        {
            aFileName = m_xEdFilepath->get_text();
            INetURLObject aURL;
            aURL.SetSmartProtocol( INetProtocol::File );
            aURL.SetSmartURL( aFileName );
            aFileName = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            bool bLink = m_xCbFilelink->get_active();

            if ( !aFileName.isEmpty() )
            {
                uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                uno::Reference< task::XInteractionHandler2 > xInteraction(
                    task::InteractionHandler::createWithParent(xContext, nullptr) );

                // create MediaDescriptor for file to create object from
                uno::Sequence < beans::PropertyValue > aMedium{
                    comphelper::makePropertyValue("URL", aFileName),
                    comphelper::makePropertyValue("InteractionHandler", xInteraction)
                };

                // create object from media descriptor

                uno::Reference<task::XStatusIndicator> xProgress;
                if (SfxViewFrame* pFrame = SfxViewFrame::Current())
                {
                    // Have a current frame, create visual indication that insert is in progress.
                    uno::Reference<frame::XFrame> xFrame = pFrame->GetFrame().GetFrameInterface();
                    uno::Reference<task::XStatusIndicatorFactory> xProgressFactory(xFrame, uno::UNO_QUERY);
                    if (xProgressFactory.is())
                    {
                        xProgress = xProgressFactory->createStatusIndicator();
                        if (xProgress)
                        {
                            OUString aOleInsert(CuiResId(RID_CUISTR_OLE_INSERT));
                            xProgress->start(aOleInsert, 100);
                        }
                    }
                }

                if ( bLink )
                    m_xObj = aCnt.InsertEmbeddedLink( aMedium, aName );
                else
                    m_xObj = aCnt.InsertEmbeddedObject( aMedium, aName );

                if (xProgress.is())
                {
                    xProgress->end();
                }
            }

            if ( !m_xObj.is() )
            {
                // object couldn't be created from file
                // global Resource from svtools (former so3 resource)
                OUString aErr(SvtResId(STR_ERROR_OBJNOCREATE_FROM_FILE));
                aErr = aErr.replaceFirst( "%", aFileName );

                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok, aErr));
                xBox->run();
            }
            else
            {
                if (m_xCbAsIcon->get_active())
                {
                    //something nice here I guess would be to write the filename into
                    //the image with this icon above it
                    Image aImage = SvFileInformationManager::GetImage(aURL, true);
                    SvMemoryStream aTemp;
                    WriteDIBBitmapEx(aImage.GetBitmapEx(), aTemp);
                    m_aIconMetaFile = Sequence<sal_Int8>(static_cast<const sal_Int8*>(aTemp.GetData()), aTemp.TellEnd());
                    m_aIconMediaType = "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"";
                }
            }
        }
    }

    m_pServers = nullptr;
    return nRet;
}

uno::Reference< io::XInputStream > SvInsertOleDlg::GetIconIfIconified( OUString* pGraphicMediaType )
{
    if ( m_aIconMetaFile.hasElements() )
    {
        if ( pGraphicMediaType )
            *pGraphicMediaType = m_aIconMediaType;

        return uno::Reference< io::XInputStream >( new ::comphelper::SequenceInputStream( m_aIconMetaFile ) );
    }

    return uno::Reference< io::XInputStream >();
}


SfxInsertFloatingFrameDialog::SfxInsertFloatingFrameDialog(weld::Window *pParent,
                            const css::uno::Reference < css::embed::XStorage >& xStorage)
    : InsertObjectDialog_Impl(pParent, "cui/ui/insertfloatingframe.ui", "InsertFloatingFrameDialog",
                              xStorage)
{
    Init();
}

SfxInsertFloatingFrameDialog::SfxInsertFloatingFrameDialog(weld::Window *pParent,
                            const uno::Reference < embed::XEmbeddedObject >& xObj)
    : InsertObjectDialog_Impl(pParent, "cui/ui/insertfloatingframe.ui", "InsertFloatingFrameDialog",
                              uno::Reference<embed::XStorage>())
{
    m_xObj = xObj;

    Init();
}

void SfxInsertFloatingFrameDialog::Init()
{
    m_xEDName = m_xBuilder->weld_entry("edname");
    m_xEDURL = m_xBuilder->weld_entry("edurl");
    m_xBTOpen = m_xBuilder->weld_button("buttonbrowse");
    m_xRBScrollingOn = m_xBuilder->weld_radio_button("scrollbaron");
    m_xRBScrollingOff = m_xBuilder->weld_radio_button("scrollbaroff");
    m_xRBScrollingAuto = m_xBuilder->weld_radio_button("scrollbarauto");
    m_xRBFrameBorderOn = m_xBuilder->weld_radio_button("borderon");
    m_xRBFrameBorderOff = m_xBuilder->weld_radio_button("borderoff");
    m_xFTMarginWidth = m_xBuilder->weld_label("widthlabel");
    m_xNMMarginWidth = m_xBuilder->weld_spin_button("width");
    m_xCBMarginWidthDefault = m_xBuilder->weld_check_button("defaultwidth");
    m_xFTMarginHeight = m_xBuilder->weld_label("heightlabel");
    m_xNMMarginHeight = m_xBuilder->weld_spin_button("height");
    m_xCBMarginHeightDefault = m_xBuilder->weld_check_button("defaultheight");

    Link<weld::Toggleable&, void> aLink(LINK(this, SfxInsertFloatingFrameDialog, CheckHdl));
    m_xCBMarginWidthDefault->connect_toggled(aLink);
    m_xCBMarginHeightDefault->connect_toggled(aLink);

    m_xCBMarginWidthDefault->set_active(true);
    m_xCBMarginHeightDefault->set_active(true);
    m_xRBScrollingAuto->set_active(true);
    m_xRBFrameBorderOn->set_active(true);

    m_xBTOpen->connect_clicked(LINK(this, SfxInsertFloatingFrameDialog, OpenHdl));
}

short SfxInsertFloatingFrameDialog::run()
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
                m_xEDURL->set_text( aStr );
            aAny = xSet->getPropertyValue( "FrameName" );
            if ( aAny >>= aStr )
                m_xEDName->set_text(aStr);

            sal_Int32 nSize = SIZE_NOT_SET;
            aAny = xSet->getPropertyValue( "FrameMarginWidth" );
            aAny >>= nSize;

            if ( nSize == SIZE_NOT_SET )
            {
                m_xCBMarginWidthDefault->set_active(true);
                m_xNMMarginWidth->set_text(OUString::number(DEFAULT_MARGIN_WIDTH));
                m_xFTMarginWidth->set_sensitive(false);
                m_xNMMarginWidth->set_sensitive(false);
            }
            else
                m_xNMMarginWidth->set_text(OUString::number(nSize));

            aAny = xSet->getPropertyValue( "FrameMarginHeight" );
            aAny >>= nSize;

            if ( nSize == SIZE_NOT_SET )
            {
                m_xCBMarginHeightDefault->set_active(true);
                m_xNMMarginHeight->set_text(OUString::number(DEFAULT_MARGIN_HEIGHT));
                m_xFTMarginHeight->set_sensitive(false);
                m_xNMMarginHeight->set_sensitive(false);
            }
            else
                m_xNMMarginHeight->set_text(OUString::number(nSize));

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

            m_xRBScrollingOn->set_sensitive(bScrollOn);
            m_xRBScrollingOff->set_sensitive(bScrollOff);
            m_xRBScrollingAuto->set_sensitive(bScrollAuto);

            bSet = false;
            aAny = xSet->getPropertyValue( "FrameIsAutoBorder" );
            aAny >>= bSet;
            if ( !bSet )
            {
                aAny = xSet->getPropertyValue( "FrameIsBorder" );
                aAny >>= bSet;
                m_xRBFrameBorderOn->set_active(bSet);
                m_xRBFrameBorderOff->set_active(!bSet);
            }

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

    if (!bOK)
        return RET_OK;

    nRet = InsertObjectDialog_Impl::run();
    if ( nRet == RET_OK )
    {
        OUString aURL;
        if (!m_xEDURL->get_text().isEmpty())
        {
            // URL can be a valid and absolute URL or a system file name
            INetURLObject aObj;
            aObj.SetSmartProtocol( INetProtocol::File );
            if ( aObj.SetSmartURL( m_xEDURL->get_text() ) )
                aURL = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
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

                OUString aName = m_xEDName->get_text();
                ScrollingMode eScroll = ScrollingMode::No;
                if (m_xRBScrollingOn->get_active())
                    eScroll = ScrollingMode::Yes;
                if (m_xRBScrollingOff->get_active())
                    eScroll = ScrollingMode::No;
                if (m_xRBScrollingAuto->get_active())
                    eScroll = ScrollingMode::Auto;

                bool bHasBorder = m_xRBFrameBorderOn->get_active();

                tools::Long lMarginWidth;
                if (!m_xCBMarginWidthDefault->get_active())
                    lMarginWidth = static_cast<tools::Long>(m_xNMMarginWidth->get_text().toInt32());
                else
                    lMarginWidth = SIZE_NOT_SET;

                tools::Long lMarginHeight;
                if (!m_xCBMarginHeightDefault->get_active())
                    lMarginHeight = static_cast<tools::Long>(m_xNMMarginHeight->get_text().toInt32());
                else
                    lMarginHeight = SIZE_NOT_SET;

                xSet->setPropertyValue( "FrameURL", Any( aURL ) );
                xSet->setPropertyValue( "FrameName", Any( aName ) );

                if ( eScroll == ScrollingMode::Auto )
                    xSet->setPropertyValue( "FrameIsAutoScroll", Any( true ) );
                else
                    xSet->setPropertyValue( "FrameIsScrollingMode", Any( eScroll == ScrollingMode::Yes ) );

                xSet->setPropertyValue( "FrameIsBorder", Any( bHasBorder ) );
                xSet->setPropertyValue( "FrameMarginWidth", Any( sal_Int32( lMarginWidth ) ) );
                xSet->setPropertyValue( "FrameMarginHeight", Any( sal_Int32( lMarginHeight ) ) );

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

IMPL_LINK(SfxInsertFloatingFrameDialog, CheckHdl, weld::Toggleable&, rButton, void)
{
    weld::CheckButton& rCB = dynamic_cast<weld::CheckButton&>(rButton);
    if (&rCB == m_xCBMarginWidthDefault.get())
    {
        if (rCB.get_active())
            m_xNMMarginWidth->set_text(OUString::number(DEFAULT_MARGIN_WIDTH));
        m_xFTMarginWidth->set_sensitive(!rCB.get_active());
        m_xNMMarginWidth->set_sensitive(!rCB.get_active());
    }

    if (&rCB == m_xCBMarginHeightDefault.get())
    {
        if (rCB.get_active())
            m_xNMMarginHeight->set_text(OUString::number(DEFAULT_MARGIN_HEIGHT));
        m_xFTMarginHeight->set_sensitive(!rCB.get_active());
        m_xNMMarginHeight->set_sensitive(!rCB.get_active());
    }
}

IMPL_LINK_NOARG( SfxInsertFloatingFrameDialog, OpenHdl, weld::Button&, void)
{
    // create the file dialog
    sfx2::FileDialogHelper aFileDlg(
            ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, OUString(),
            SfxFilterFlags::NONE, SfxFilterFlags::NONE, m_xDialog.get());

    // set the title
    aFileDlg.SetTitle(CuiResId(RID_CUISTR_SELECT_FILE_IFRAME));

    // show the dialog
    if ( aFileDlg.Execute() == ERRCODE_NONE )
        m_xEDURL->set_text(INetURLObject(aFileDlg.GetPath()).GetMainURL(INetURLObject::DecodeMechanism::WithCharset));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
