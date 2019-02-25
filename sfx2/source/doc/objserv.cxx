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

#include <config_features.h>

#include <sot/storage.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XControlAccess.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XCmisDocument.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <tools/urlobj.hxx>
#include <svl/whiter.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/visitem.hxx>
#include <vcl/weld.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <sal/log.hxx>

#include <comphelper/string.hxx>
#include <basic/sbx.hxx>
#include <basic/sberrors.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/saveopt.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/DocumentToGraphicRenderer.hxx>
#include <vcl/gdimtf.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/documentconstants.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <tools/link.hxx>

#include <sfx2/asyncfunc.hxx>
#include <sfx2/app.hxx>
#include <sfx2/signaturestate.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/event.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <objshimp.hxx>
#include <sfxtypes.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include <versdlg.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/docfac.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/sfxhelp.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/checkin.hxx>
#include <sfx2/infobar.hxx>
#include <SfxRedactionHelper.hxx>

#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

#include <guisaveas.hxx>
#include <saveastemplatedlg.hxx>
#include <memory>
#include <cppuhelper/implbase.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <svx/unoshape.hxx>
#include <com/sun/star/util/Color.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::security;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::graphic;

#define ShellClass_SfxObjectShell
#include <sfxslots.hxx>

SFX_IMPL_SUPERCLASS_INTERFACE(SfxObjectShell, SfxShell)

void SfxObjectShell::InitInterface_Impl()
{
}

class SfxClosePreventer_Impl : public ::cppu::WeakImplHelper< css::util::XCloseListener >
{
    bool m_bGotOwnership;
    bool m_bPreventClose;

public:
    SfxClosePreventer_Impl();

    bool HasOwnership() { return m_bGotOwnership; }

    void SetPreventClose( bool bPrevent ) { m_bPreventClose = bPrevent; }

    virtual void SAL_CALL queryClosing( const lang::EventObject& aEvent, sal_Bool bDeliverOwnership ) override;

    virtual void SAL_CALL notifyClosing( const lang::EventObject& aEvent ) override ;

    virtual void SAL_CALL disposing( const lang::EventObject& aEvent ) override ;

} ;

SfxClosePreventer_Impl::SfxClosePreventer_Impl()
: m_bGotOwnership( false )
, m_bPreventClose( true )
{
}

void SAL_CALL SfxClosePreventer_Impl::queryClosing( const lang::EventObject&, sal_Bool bDeliverOwnership )
{
    if ( m_bPreventClose )
    {
        if ( !m_bGotOwnership )
            m_bGotOwnership = bDeliverOwnership;

        throw util::CloseVetoException();
    }
}

void SAL_CALL SfxClosePreventer_Impl::notifyClosing( const lang::EventObject& )
{}

void SAL_CALL SfxClosePreventer_Impl::disposing( const lang::EventObject& )
{}


class SfxInstanceCloseGuard_Impl
{
    rtl::Reference<SfxClosePreventer_Impl> m_xPreventer;
    uno::Reference< util::XCloseable > m_xCloseable;

public:
    SfxInstanceCloseGuard_Impl() {}

    ~SfxInstanceCloseGuard_Impl();

    bool Init_Impl( const uno::Reference< util::XCloseable >& xCloseable );
};

bool SfxInstanceCloseGuard_Impl::Init_Impl( const uno::Reference< util::XCloseable >& xCloseable )
{
    bool bResult = false;

    // do not allow reinit after the successful init
    if ( xCloseable.is() && !m_xCloseable.is() )
    {
        try
        {
            m_xPreventer = new SfxClosePreventer_Impl();
            xCloseable->addCloseListener( m_xPreventer.get() );
            m_xCloseable = xCloseable;
            bResult = true;
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Could not register close listener!" );
        }
    }

    return bResult;
}

SfxInstanceCloseGuard_Impl::~SfxInstanceCloseGuard_Impl()
{
    if ( !m_xCloseable.is() || !m_xPreventer.is() )
        return;

    try
    {
        m_xCloseable->removeCloseListener( m_xPreventer.get() );
    }
    catch( uno::Exception& )
    {
    }

    try
    {
        if ( m_xPreventer.is() )
        {
            m_xPreventer->SetPreventClose( false );

            if ( m_xPreventer->HasOwnership() )
                m_xCloseable->close( true ); // TODO: do it asynchronously
        }
    }
    catch( uno::Exception& )
    {
    }
}


void SfxObjectShell::PrintExec_Impl(SfxRequest &rReq)
{
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this);
    if ( pFrame )
    {
        rReq.SetSlot( SID_PRINTDOC );
        pFrame->GetViewShell()->ExecuteSlot(rReq);
    }
}


void SfxObjectShell::PrintState_Impl(SfxItemSet &rSet)
{
    bool bPrinting = false;
    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame )
    {
        SfxPrinter *pPrinter = pFrame->GetViewShell()->GetPrinter();
        bPrinting = pPrinter && pPrinter->IsPrinting();
    }
    rSet.Put( SfxBoolItem( SID_PRINTOUT, bPrinting ) );
}

bool SfxObjectShell::APISaveAs_Impl(const OUString& aFileName, SfxItemSet& rItemSet)
{
    bool bOk = false;


    if ( GetMedium() )
    {
        OUString aFilterName;
        const SfxStringItem* pFilterNameItem = rItemSet.GetItem<SfxStringItem>(SID_FILTER_NAME, false);
        if( pFilterNameItem )
        {
            aFilterName = pFilterNameItem->GetValue();
        }
        else
        {
            const SfxStringItem* pContentTypeItem = rItemSet.GetItem<SfxStringItem>(SID_CONTENTTYPE, false);
            if ( pContentTypeItem )
            {
                std::shared_ptr<const SfxFilter> pFilter = SfxFilterMatcher( GetFactory().GetFactoryName() ).GetFilter4Mime( pContentTypeItem->GetValue(), SfxFilterFlags::EXPORT );
                if ( pFilter )
                    aFilterName = pFilter->GetName();
            }
        }

        // in case no filter defined use default one
        if( aFilterName.isEmpty() )
        {
            std::shared_ptr<const SfxFilter> pFilt = SfxFilter::GetDefaultFilterFromFactory(GetFactory().GetFactoryName());

            DBG_ASSERT( pFilt, "No default filter!\n" );
            if( pFilt )
                aFilterName = pFilt->GetFilterName();

            rItemSet.Put(SfxStringItem(SID_FILTER_NAME, aFilterName));
        }


        {
            SfxObjectShellRef xLock( this ); // ???

            // use the title that is provided in the media descriptor
            const SfxStringItem* pDocTitleItem = rItemSet.GetItem<SfxStringItem>(SID_DOCINFO_TITLE, false);
            if ( pDocTitleItem )
                getDocProperties()->setTitle( pDocTitleItem->GetValue() );

            bOk = CommonSaveAs_Impl(INetURLObject(aFileName), aFilterName, rItemSet);
        }
    }

    return bOk;
}

void SfxObjectShell::CheckOut( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        xCmisDoc->checkOut( );

        // Remove the info bar
        SfxViewFrame* pViewFrame = GetFrame();
        pViewFrame->RemoveInfoBar( "checkout" );
    }
    catch ( const uno::RuntimeException& e )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrame()->GetWindow().GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, e.Message));
        xBox->run();
    }
}

void SfxObjectShell::CancelCheckOut( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        xCmisDoc->cancelCheckOut( );

        uno::Reference< util::XModifiable > xModifiable( GetModel( ), uno::UNO_QUERY );
        if ( xModifiable.is( ) )
            xModifiable->setModified( false );
    }
    catch ( const uno::RuntimeException& e )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrame()->GetWindow().GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, e.Message));
        xBox->run();
    }
}

void SfxObjectShell::CheckIn( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        // Pop up dialog to ask for comment and major
        SfxCheckinDialog checkinDlg(GetFrame()->GetWindow().GetFrameWeld());
        if (checkinDlg.run() == RET_OK)
        {
            xCmisDoc->checkIn(checkinDlg.IsMajor(), checkinDlg.GetComment());
            uno::Reference< util::XModifiable > xModifiable( GetModel( ), uno::UNO_QUERY );
            if ( xModifiable.is( ) )
                xModifiable->setModified( false );
        }
    }
    catch ( const uno::RuntimeException& e )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrame()->GetWindow().GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, e.Message));
        xBox->run();
    }
}

uno::Sequence< document::CmisVersion > SfxObjectShell::GetCmisVersions( )
{
    try
    {
        uno::Reference< document::XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY_THROW );
        return xCmisDoc->getAllVersions( );
    }
    catch ( const uno::RuntimeException& e )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrame()->GetWindow().GetFrameWeld(),
                                                  VclMessageType::Warning, VclButtonsType::Ok, e.Message));
        xBox->run();
    }
    return uno::Sequence< document::CmisVersion > ( );
}

void SfxObjectShell::ExecFile_Impl(SfxRequest &rReq)
{
    weld::Window* pDialogParent = rReq.GetFrameWeld();
    if (!pDialogParent)
    {
        SfxViewFrame* pFrame = GetFrame();
        if (!pFrame)
            pFrame = SfxViewFrame::GetFirst(this);
        if (pFrame)
            pDialogParent = pFrame->GetWindow().GetFrameWeld();
    }

    sal_uInt16 nId = rReq.GetSlot();

    if( SID_SIGNATURE == nId || SID_MACRO_SIGNATURE == nId )
    {
        if ( QueryHiddenInformation( HiddenWarningFact::WhenSigning, nullptr ) == RET_YES )
            ( SID_SIGNATURE == nId ) ? SignDocumentContent(pDialogParent) : SignScriptingContent(pDialogParent);
        return;
    }

    if ( !GetMedium() && nId != SID_CLOSEDOC )
    {
        rReq.Ignore();
        return;
    }

    // this guard is created here to have it destruction at the end of the method
    SfxInstanceCloseGuard_Impl aModelGuard;

    bool bIsPDFExport = false;
    switch(nId)
    {
        case SID_VERSION:
        {
            SfxViewFrame* pFrame = GetFrame();
            if ( !pFrame )
                pFrame = SfxViewFrame::GetFirst( this );
            if ( !pFrame )
                return;

            if ( !IsOwnStorageFormat( *GetMedium() ) )
                return;

            SfxVersionDialog aDlg(pDialogParent, pFrame, IsSaveVersionOnClose());
            aDlg.run();
            SetSaveVersionOnClose(aDlg.IsSaveVersionOnClose());
            rReq.Done();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCINFO:
        {
            const SfxDocumentInfoItem* pDocInfItem = rReq.GetArg<SfxDocumentInfoItem>(SID_DOCINFO);
            if ( pDocInfItem )
            {
                // parameter, e.g. from replayed macro
                pDocInfItem->UpdateDocumentInfo(getDocProperties(), true);
                SetUseUserData( pDocInfItem->IsUseUserData() );
                SetUseThumbnailSave( pDocInfItem->IsUseThumbnailSave() );
            }
            else
            {
                // no argument containing DocInfo; check optional arguments
                bool bReadOnly = IsReadOnly();
                const SfxBoolItem* pROItem = rReq.GetArg<SfxBoolItem>(SID_DOC_READONLY);
                if ( pROItem )
                    // override readonly attribute of document
                    // e.g. if a readonly document is saved elsewhere and user asks for editing DocInfo before
                    bReadOnly = pROItem->GetValue();

                // URL for dialog
                const OUString aURL( HasName() ? GetMedium()->GetName() : GetFactory().GetFactoryURL() );

                Reference< XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY );
                uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties();

                SfxDocumentInfoItem aDocInfoItem( aURL, getDocProperties(), aCmisProperties,
                    IsUseUserData(), IsUseThumbnailSave() );
                if ( !GetSlotState( SID_DOCTEMPLATE ) )
                    // templates not supported
                    aDocInfoItem.SetTemplate(false);

                SfxItemSet aSet(GetPool(), svl::Items<SID_DOCINFO, SID_DOCINFO, SID_DOC_READONLY, SID_DOC_READONLY,
                                SID_EXPLORER_PROPS_START, SID_EXPLORER_PROPS_START, SID_BASEURL, SID_BASEURL>{} );
                aSet.Put( aDocInfoItem );
                aSet.Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                aSet.Put( SfxStringItem( SID_EXPLORER_PROPS_START, GetTitle() ) );
                aSet.Put( SfxStringItem( SID_BASEURL, GetMedium()->GetBaseURL() ) );

                // creating dialog is done via virtual method; application will
                // add its own statistics page
                VclAbstractDialog::AsyncContext aCtx;
                std::shared_ptr<SfxRequest> pReq = std::make_shared<SfxRequest>(rReq);
                VclPtr<SfxDocumentInfoDialog> pDlg(CreateDocumentInfoDialog(aSet));

                aCtx.mxOwner = pDlg;
                aCtx.maEndDialogFn = [this, pDlg, xCmisDoc, pReq](sal_Int32 nResult)
                {
                    if (RET_OK == nResult)
                    {
                        const SfxDocumentInfoItem* pDocInfoItem = SfxItemSet::GetItem<SfxDocumentInfoItem>(pDlg->GetOutputItemSet(), SID_DOCINFO, false);
                        if ( pDocInfoItem )
                        {
                            // user has done some changes to DocumentInfo
                            pDocInfoItem->UpdateDocumentInfo(getDocProperties());
                            const uno::Sequence< document::CmisProperty >& aNewCmisProperties =
                                pDocInfoItem->GetCmisProperties( );
                            if ( aNewCmisProperties.getLength( ) > 0 )
                                xCmisDoc->updateCmisProperties( aNewCmisProperties );
                            SetUseUserData( pDocInfoItem->IsUseUserData() );
                            SetUseThumbnailSave( pDocInfoItem-> IsUseThumbnailSave() );
                            // add data from dialog for possible recording purpose
                            pReq->AppendItem( SfxDocumentInfoItem( GetTitle(),
                                getDocProperties(), aNewCmisProperties, IsUseUserData(), IsUseThumbnailSave() ) );
                        }

                        css::uno::Reference< css::uno::XInterface > xInterface;
                        const SfxUnoAnyItem* pUnoAny = pReq->GetArg<SfxUnoAnyItem>(FN_PARAM_2);
                        AsyncFunc* pAsyncFunc = pUnoAny && (pUnoAny->GetValue() >>= xInterface ) ?
                            AsyncFunc::getImplementation(xInterface) : nullptr;
                        if (pAsyncFunc)
                            pAsyncFunc->Execute();

                        pReq->Done();
                    }
                    else
                        // nothing done; no recording
                        pReq->Ignore();
                };

                pDlg->StartExecuteAsync(aCtx);
                rReq.Ignore();
            }

            return;
        }

        case SID_REDACTDOC:
        {
            css::uno::Reference<css::frame::XModel> xModel = GetModel();
            if(!xModel.is())
                return;

            uno::Reference< lang::XComponent > xSourceDoc( xModel );

            DocumentToGraphicRenderer aRenderer(xSourceDoc, false);

            bool bIsWriter = aRenderer.isWriter();
            bool bIsCalc = aRenderer.isCalc();

            if (!bIsWriter && !bIsCalc)
            {
                SAL_WARN( "sfx.doc", "Redaction is supported only for Writer and Calc! (for now...)");
                return;
            }

            sal_Int32 nPages = aRenderer.getPageCount();
            std::vector< GDIMetaFile > aMetaFiles;

            // Convert the pages of the document to gdimetafiles
            SfxRedactionHelper::getPageMetaFilesFromDoc(aMetaFiles, nPages, aRenderer, bIsWriter, bIsCalc);

            // Create an empty Draw component.
            uno::Reference<frame::XDesktop2> xDesktop = css::frame::Desktop::create(comphelper::getProcessComponentContext());
            uno::Reference<frame::XComponentLoader> xComponentLoader(xDesktop, uno::UNO_QUERY);
            uno::Reference<lang::XComponent> xComponent = xComponentLoader->loadComponentFromURL("private:factory/sdraw", "_default", 0, {});

            // Add the doc pages to the new draw document
            SfxRedactionHelper::addPagesToDraw(xComponent, nPages, aMetaFiles, bIsCalc);

            // Show the Redaction toolbar
            SfxViewFrame* pViewFrame = SfxViewFrame::Current();
            if (!pViewFrame)
                return;
            SfxRedactionHelper::showRedactionToolbar(pViewFrame);

            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DIRECTEXPORTDOCASPDF:
        {
            uno::Reference< lang::XComponent > xComponent( GetCurrentComponent(), uno::UNO_QUERY );
            if (!xComponent.is())
                return;

            uno::Reference< lang::XServiceInfo > xServiceInfo( xComponent, uno::UNO_QUERY);

            // Redaction finalization takes place in Draw
            if ( xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument")
                 && SfxRedactionHelper::isRedactMode(rReq) )
            {
                OUString sRedactionStyle(SfxRedactionHelper::getStringParam(rReq, SID_REDACTION_STYLE));

                // Access the draw pages
                uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
                uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();

                sal_Int32 nPageCount = xDrawPages->getCount();
                for (sal_Int32 nPageNum = 0; nPageNum < nPageCount; ++nPageNum)
                {
                    // Get the page
                    uno::Reference< drawing::XDrawPage > xPage( xDrawPages->getByIndex( nPageNum ), uno::UNO_QUERY );

                    if (!xPage.is())
                        continue;

                    // Go through all shapes
                    sal_Int32 nShapeCount = xPage->getCount();
                    for (sal_Int32 nShapeNum = 0; nShapeNum < nShapeCount; ++nShapeNum)
                    {
                        uno::Reference< drawing::XShape > xCurrShape(xPage->getByIndex(nShapeNum), uno::UNO_QUERY);
                        if (!xCurrShape.is())
                            continue;

                        uno::Reference< beans::XPropertySet > xPropSet(xCurrShape, uno::UNO_QUERY);
                        if (!xPropSet.is())
                            continue;

                        uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
                        if (!xInfo.is())
                            continue;

                        OUString sShapeName;
                        if (xInfo->hasPropertyByName("Name"))
                        {
                            uno::Any aAnyShapeName = xPropSet->getPropertyValue("Name");
                            aAnyShapeName >>= sShapeName;
                        }
                        else
                            continue;

                        // Rectangle redaction
                        if (sShapeName == "RectangleRedactionShape"
                                && xInfo->hasPropertyByName("FillTransparence") && xInfo->hasPropertyByName("FillColor"))
                        {
                            xPropSet->setPropertyValue("FillTransparence", css::uno::makeAny(static_cast<sal_Int16>(0)));
                            if (sRedactionStyle == "White")
                            {
                                xPropSet->setPropertyValue("FillColor", css::uno::makeAny(COL_WHITE));
                                xPropSet->setPropertyValue("LineStyle", css::uno::makeAny(css::drawing::LineStyle::LineStyle_SOLID));
                                xPropSet->setPropertyValue("LineColor", css::uno::makeAny(COL_BLACK));
                            }
                            else
                            {
                                xPropSet->setPropertyValue("FillColor", css::uno::makeAny(COL_BLACK));
                                xPropSet->setPropertyValue("LineStyle", css::uno::makeAny(css::drawing::LineStyle::LineStyle_NONE));
                            }
                        }
                        // Freeform redaction
                        else if (sShapeName == "FreeformRedactionShape"
                                 && xInfo->hasPropertyByName("LineTransparence") && xInfo->hasPropertyByName("LineColor"))
                        {
                            xPropSet->setPropertyValue("LineTransparence", css::uno::makeAny(static_cast<sal_Int16>(0)));

                            if (sRedactionStyle == "White")
                            {
                                xPropSet->setPropertyValue("LineColor", css::uno::makeAny(COL_WHITE));
                            }
                            else
                            {
                                xPropSet->setPropertyValue("LineColor", css::uno::makeAny(COL_BLACK));
                            }
                        }
                    }
                }
            }
        }
            [[fallthrough]];
        case SID_EXPORTDOCASPDF:
            bIsPDFExport = true;
            [[fallthrough]];
        case SID_EXPORTDOCASEPUB:
        case SID_DIRECTEXPORTDOCASEPUB:
        case SID_EXPORTDOC:
        case SID_SAVEASDOC:
        case SID_SAVEASREMOTE:
        case SID_SAVEDOC:
        {
            // derived class may decide to abort this
            if( !QuerySlotExecutable( nId ) )
            {
                rReq.SetReturnValue( SfxBoolItem( 0, false ) );
                return;
            }

            //!! detailed analysis of an error code
            SfxObjectShellRef xLock( this );

            // the model can not be closed till the end of this method
            // if somebody tries to close it during this time the model will be closed
            // at the end of the method
            aModelGuard.Init_Impl( uno::Reference< util::XCloseable >( GetModel(), uno::UNO_QUERY ) );

            ErrCode nErrorCode = ERRCODE_NONE;

            // by default versions should be preserved always except in case of an explicit
            // SaveAs via GUI, so the flag must be set accordingly
            pImpl->bPreserveVersions = (nId == SID_SAVEDOC);
            try
            {
                SfxErrorContext aEc( ERRCTX_SFX_SAVEASDOC, GetTitle() ); // ???

                if ( nId == SID_SAVEASDOC || nId == SID_SAVEASREMOTE )
                {
                    // in case of plugin mode the SaveAs operation means SaveTo
                    const SfxBoolItem* pViewOnlyItem = SfxItemSet::GetItem<SfxBoolItem>(GetMedium()->GetItemSet(), SID_VIEWONLY, false);
                    if ( pViewOnlyItem && pViewOnlyItem->GetValue() )
                        rReq.AppendItem( SfxBoolItem( SID_SAVETO, true ) );
                }

                // TODO/LATER: do the following GUI related actions in standalone method

                // Introduce a status indicator for GUI operation
                const SfxUnoAnyItem* pStatusIndicatorItem = rReq.GetArg<SfxUnoAnyItem>(SID_PROGRESS_STATUSBAR_CONTROL);
                if ( !pStatusIndicatorItem )
                {
                    // get statusindicator
                    uno::Reference< task::XStatusIndicator > xStatusIndicator;
                    uno::Reference < frame::XController > xCtrl( GetModel()->getCurrentController() );
                    if ( xCtrl.is() )
                    {
                        uno::Reference< task::XStatusIndicatorFactory > xStatFactory( xCtrl->getFrame(), uno::UNO_QUERY );
                        if( xStatFactory.is() )
                            xStatusIndicator = xStatFactory->createStatusIndicator();
                    }

                    OSL_ENSURE( xStatusIndicator.is(), "Can not retrieve default status indicator!" );

                    if ( xStatusIndicator.is() )
                    {
                        SfxUnoAnyItem aStatIndItem( SID_PROGRESS_STATUSBAR_CONTROL, uno::makeAny( xStatusIndicator ) );

                        if ( nId == SID_SAVEDOC )
                        {
                            // in case of saving it is not possible to transport the parameters from here
                            // but it is not clear here whether the saving will be done or saveAs operation
                            GetMedium()->GetItemSet()->Put( aStatIndItem );
                        }

                        rReq.AppendItem( aStatIndItem );
                    }
                }
                else if ( nId == SID_SAVEDOC )
                {
                    // in case of saving it is not possible to transport the parameters from here
                    // but it is not clear here whether the saving will be done or saveAs operation
                    GetMedium()->GetItemSet()->Put( *pStatusIndicatorItem );
                }

                // Introduce an interaction handler for GUI operation
                const SfxUnoAnyItem* pInteractionHandlerItem = rReq.GetArg<SfxUnoAnyItem>(SID_INTERACTIONHANDLER);
                if ( !pInteractionHandlerItem )
                {
                    uno::Reference<css::awt::XWindow> xParentWindow;
                    uno::Reference<frame::XController> xCtrl(GetModel()->getCurrentController());
                    if (xCtrl.is())
                        xParentWindow = xCtrl->getFrame()->getContainerWindow();

                    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

                    uno::Reference< task::XInteractionHandler2 > xInteract(
                        task::InteractionHandler::createWithParent(xContext, xParentWindow) );

                    SfxUnoAnyItem aInteractionItem( SID_INTERACTIONHANDLER, uno::makeAny( xInteract ) );
                    if ( nId == SID_SAVEDOC )
                    {
                        // in case of saving it is not possible to transport the parameters from here
                        // but it is not clear here whether the saving will be done or saveAs operation
                        GetMedium()->GetItemSet()->Put( aInteractionItem );
                    }

                    rReq.AppendItem( aInteractionItem );
                }
                else if ( nId == SID_SAVEDOC )
                {
                    // in case of saving it is not possible to transport the parameters from here
                    // but it is not clear here whether the saving will be done or saveAs operation
                    GetMedium()->GetItemSet()->Put( *pInteractionHandlerItem );
                }


                bool bPreselectPassword = false;
                const SfxUnoAnyItem* pOldEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(GetMedium()->GetItemSet(), SID_ENCRYPTIONDATA, false);
                const SfxStringItem* pOldPasswordItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_PASSWORD, false);
                if ( pOldEncryptionDataItem || pOldPasswordItem )
                    bPreselectPassword = true;

                uno::Sequence< beans::PropertyValue > aDispatchArgs;
                if ( rReq.GetArgs() )
                    TransformItems( nId,
                                    *rReq.GetArgs(),
                                     aDispatchArgs );

                bool bForceSaveAs = nId == SID_SAVEDOC && IsReadOnlyMedium();
                const SfxSlot* pSlot = GetModule()->GetSlotPool()->GetSlot( bForceSaveAs ? SID_SAVEASDOC : nId );
                if ( !pSlot )
                    throw uno::Exception("no slot", nullptr);

                SfxStoringHelper aHelper;

                if ( QueryHiddenInformation( bIsPDFExport ? HiddenWarningFact::WhenCreatingPDF : HiddenWarningFact::WhenSaving, nullptr ) != RET_YES )
                {
                    // the user has decided not to store the document
                    throw task::ErrorCodeIOException(
                        "SfxObjectShell::ExecFile_Impl: ERRCODE_IO_ABORT",
                        uno::Reference< uno::XInterface >(), sal_uInt32(ERRCODE_IO_ABORT));
                }

                aHelper.GUIStoreModel( GetModel(),
                                       OUString::createFromAscii( pSlot->GetUnoName() ),
                                       aDispatchArgs,
                                       bPreselectPassword,
                                       GetDocumentSignatureState() );


                // merge aDispatchArgs to the request
                SfxAllItemSet aResultParams( GetPool() );
                TransformParameters( nId,
                                     aDispatchArgs,
                                     aResultParams );
                rReq.SetArgs( aResultParams );

                // the StoreAsURL/StoreToURL method have called this method with false
                // so it has to be restored to true here since it is a call from GUI
                GetMedium()->SetUpdatePickList( true );

                // TODO: in future it must be done in following way
                // if document is opened from GUI, it immediately appears in the picklist
                // if the document is a new one then it appears in the picklist immediately
                // after SaveAs operation triggered from GUI
            }
            catch( const task::ErrorCodeIOException& aErrorEx )
            {
                nErrorCode = ErrCode(aErrorEx.ErrCode);
                SAL_WARN( "sfx.doc", "Fatal IO error during save " << aErrorEx );
            }
            catch( Exception& )
            {
                nErrorCode = ERRCODE_IO_GENERAL;
            }

            // by default versions should be preserved always except in case of an explicit
            // SaveAs via GUI, so the flag must be reset to guarantee this
            pImpl->bPreserveVersions = true;
            ErrCode lErr=GetErrorCode();

            if ( !lErr && nErrorCode )
                lErr = nErrorCode;

            if ( lErr && nErrorCode == ERRCODE_NONE )
            {
                const SfxBoolItem* pWarnItem = rReq.GetArg<SfxBoolItem>(SID_FAIL_ON_WARNING);
                if ( pWarnItem && pWarnItem->GetValue() )
                    nErrorCode = lErr;
            }

            // may be nErrorCode should be shown in future
            if ( lErr != ERRCODE_IO_ABORT )
            {
                SfxErrorContext aEc(ERRCTX_SFX_SAVEASDOC,GetTitle());
                ErrorHandler::HandleError(lErr, pDialogParent);
            }

            if (nId == SID_DIRECTEXPORTDOCASPDF &&
                    SfxRedactionHelper::isRedactMode(rReq))
            {
                // Return the finalized redaction shapes back to normal (gray & transparent)
                uno::Reference< lang::XComponent > xComponent( GetCurrentComponent(), uno::UNO_QUERY );
                if (!xComponent.is())
                    return;

                uno::Reference< lang::XServiceInfo > xServiceInfo( xComponent, uno::UNO_QUERY);

                // Redaction finalization takes place in Draw
                if ( xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.drawing.DrawingDocument") )
                {
                    // Access the draw pages
                    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
                    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();

                    sal_Int32 nPageCount = xDrawPages->getCount();
                    for (sal_Int32 nPageNum = 0; nPageNum < nPageCount; ++nPageNum)
                    {
                        // Get the page
                        uno::Reference< drawing::XDrawPage > xPage( xDrawPages->getByIndex( nPageNum ), uno::UNO_QUERY );

                        if (!xPage.is())
                            continue;

                        // Go through all shapes
                        sal_Int32 nShapeCount = xPage->getCount();
                        for (sal_Int32 nShapeNum = 0; nShapeNum < nShapeCount; ++nShapeNum)
                        {
                            uno::Reference< drawing::XShape > xCurrShape(xPage->getByIndex(nShapeNum), uno::UNO_QUERY);
                            if (!xCurrShape.is())
                                continue;

                            uno::Reference< beans::XPropertySet > xPropSet(xCurrShape, uno::UNO_QUERY);
                            if (!xPropSet.is())
                                continue;

                            uno::Reference< beans::XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
                            if (!xInfo.is())
                                continue;

                            // Not a shape we converted?
                            if (!xInfo->hasPropertyByName("Name"))
                                continue;

                            OUString sShapeName;
                            if (xInfo->hasPropertyByName("Name"))
                            {
                                uno::Any aAnyShapeName = xPropSet->getPropertyValue("Name");
                                aAnyShapeName >>= sShapeName;
                            }
                            else
                                continue;

                            // Rectangle redaction
                            if (sShapeName == "RectangleRedactionShape"
                                    && xInfo->hasPropertyByName("FillTransparence") && xInfo->hasPropertyByName("FillColor"))
                            {
                                xPropSet->setPropertyValue("FillTransparence", css::uno::makeAny(static_cast<sal_Int16>(50)));
                                xPropSet->setPropertyValue("FillColor", css::uno::makeAny(COL_GRAY7));
                                xPropSet->setPropertyValue("LineStyle", css::uno::makeAny(css::drawing::LineStyle::LineStyle_NONE));

                            }
                            // Freeform redaction
                            else if (sShapeName == "FreeformRedactionShape")
                            {
                                xPropSet->setPropertyValue("LineTransparence", css::uno::makeAny(static_cast<sal_Int16>(50)));
                                xPropSet->setPropertyValue("LineColor", css::uno::makeAny(COL_GRAY7));
                            }
                        }
                    }


                }
            }

            if ( nId == SID_EXPORTDOCASPDF )
            {
                // This function is used by the SendMail function that needs information if a export
                // file was written or not. This could be due to cancellation of the export
                // or due to an error. So IO abort must be handled like an error!
                nErrorCode = ( lErr != ERRCODE_IO_ABORT ) && ( nErrorCode == ERRCODE_NONE ) ? nErrorCode : lErr;
            }

            if ( ( nId == SID_SAVEASDOC || nId == SID_SAVEASREMOTE ) && nErrorCode == ERRCODE_NONE )
            {
                const SfxBoolItem* saveTo = rReq.GetArg<SfxBoolItem>(SID_SAVETO);
                if (saveTo == nullptr || !saveTo->GetValue())
                {
                    SfxViewFrame *pFrame = GetFrame();
                    if (pFrame)
                        pFrame->RemoveInfoBar("readonly");
                    SetReadOnlyUI(false);
                }
            }

            rReq.SetReturnValue( SfxBoolItem(0, nErrorCode == ERRCODE_NONE ) );

            ResetError();

            Invalidate();
            break;
        }

        case SID_SAVEACOPY:
        {
            SfxAllItemSet aArgs( GetPool() );
            aArgs.Put( SfxBoolItem( SID_SAVEACOPYITEM, true ) );
            SfxRequest aSaveACopyReq( SID_EXPORTDOC, SfxCallMode::API, aArgs );
            ExecFile_Impl( aSaveACopyReq );
            if ( !aSaveACopyReq.IsDone() )
            {
                rReq.Ignore();
                return;
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_CLOSEDOC:
        {
            // Evaluate Parameter
            const SfxBoolItem* pSaveItem = rReq.GetArg<SfxBoolItem>(SID_CLOSEDOC_SAVE);
            const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(SID_CLOSEDOC_FILENAME);
            if ( pSaveItem )
            {
                if ( pSaveItem->GetValue() )
                {
                    if ( !pNameItem )
                    {
#if HAVE_FEATURE_SCRIPTING
                        SbxBase::SetError( ERRCODE_BASIC_WRONG_ARGS );
#endif
                        rReq.Ignore();
                        return;
                    }
                    SfxAllItemSet aArgs( GetPool() );
                    SfxStringItem aTmpItem( SID_FILE_NAME, pNameItem->GetValue() );
                    aArgs.Put( aTmpItem, aTmpItem.Which() );
                    SfxRequest aSaveAsReq( SID_SAVEASDOC, SfxCallMode::API, aArgs );
                    ExecFile_Impl( aSaveAsReq );
                    if ( !aSaveAsReq.IsDone() )
                    {
                        rReq.Ignore();
                        return;
                    }
                }
                else
                    SetModified(false);
            }

            // Cancelled by the user?
            if (!PrepareClose())
            {
                rReq.SetReturnValue( SfxBoolItem(0, false) );
                rReq.Done();
                return;
            }

            SetModified( false );
            ErrCode lErr = GetErrorCode();
            ErrorHandler::HandleError(lErr, pDialogParent);

            rReq.SetReturnValue( SfxBoolItem(0, true) );
            rReq.Done();
            rReq.ReleaseArgs(); // because the pool is destroyed in Close
            DoClose();
            return;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_DOCTEMPLATE:
        {
            // save as document templates
            SfxSaveAsTemplateDialog aDlg(pDialogParent, GetModel());
            (void)aDlg.run();
            break;
        }

        case SID_CHECKOUT:
        {
            CheckOut( );
            break;
        }
        case SID_CANCELCHECKOUT:
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                                      VclMessageType::Question, VclButtonsType::YesNo, SfxResId(STR_QUERY_CANCELCHECKOUT)));
            if (xBox->run() == RET_YES)
            {
                CancelCheckOut( );

                // Reload the document as we may still have local changes
                SfxViewFrame *pFrame = GetFrame();
                if ( pFrame )
                    pFrame->GetDispatcher()->Execute(SID_RELOAD);
            }
            break;
        }
        case SID_CHECKIN:
        {
            CheckIn( );
            break;
        }
    }

    // Prevent entry in the Pick-lists
    if ( rReq.IsAPI() )
        GetMedium()->SetUpdatePickList( false );

    // Ignore()-branches have already returned
    rReq.Done();
}


void SfxObjectShell::GetState_Impl(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );

    for ( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich; nWhich = aIter.NextWhich() )
    {
        switch ( nWhich )
        {
            case SID_DOCTEMPLATE :
            {
                if ( !GetFactory().GetTemplateFilter() )
                    rSet.DisableItem( nWhich );
                break;
            }

            case SID_CHECKOUT:
                {
                    bool bShow = false;
                    Reference< XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY );
                    uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties();

                    if ( xCmisDoc->isVersionable( ) && aCmisProperties.hasElements( ) )
                    {
                        // Loop over the CMIS Properties to find cmis:isVersionSeriesCheckedOut
                        bool bIsGoogleFile = false;
                        bool bCheckedOut = false;
                        for ( sal_Int32 i = 0; i < aCmisProperties.getLength(); ++i )
                        {
                            if ( aCmisProperties[i].Id == "cmis:isVersionSeriesCheckedOut" )
                            {
                                uno::Sequence< sal_Bool > bTmp;
                                aCmisProperties[i].Value >>= bTmp;
                                bCheckedOut = bTmp[0];
                            }
                            // using title to know if it's a Google Drive file
                            // maybe there's a safer way.
                            if ( aCmisProperties[i].Name == "title" )
                                bIsGoogleFile = true;
                        }
                        bShow = !bCheckedOut && !bIsGoogleFile;
                    }

                    if ( !bShow )
                    {
                        rSet.DisableItem( nWhich );
                        rSet.Put( SfxVisibilityItem( nWhich, false ) );
                    }
                }
                break;

            case SID_CANCELCHECKOUT:
            case SID_CHECKIN:
                {
                    bool bShow = false;
                    Reference< XCmisDocument > xCmisDoc( GetModel(), uno::UNO_QUERY );
                    uno::Sequence< document::CmisProperty> aCmisProperties = xCmisDoc->getCmisProperties( );

                    if ( xCmisDoc->isVersionable( ) && aCmisProperties.hasElements( ) )
                    {
                        // Loop over the CMIS Properties to find cmis:isVersionSeriesCheckedOut
                        bool bFoundCheckedout = false;
                        bool bCheckedOut = false;
                        for ( sal_Int32 i = 0; i < aCmisProperties.getLength() && !bFoundCheckedout; ++i )
                        {
                            if ( aCmisProperties[i].Id == "cmis:isVersionSeriesCheckedOut" )
                            {
                                bFoundCheckedout = true;
                                uno::Sequence< sal_Bool > bTmp;
                                aCmisProperties[i].Value >>= bTmp;
                                bCheckedOut = bTmp[0];
                            }
                        }
                        bShow = bCheckedOut;
                    }

                    if ( !bShow )
                    {
                        rSet.DisableItem( nWhich );
                        rSet.Put( SfxVisibilityItem( nWhich, false ) );
                    }
                }
                break;

            case SID_VERSION:
                {
                    SfxObjectShell *pDoc = this;
                    SfxViewFrame* pFrame = GetFrame();
                    if ( !pFrame )
                        pFrame = SfxViewFrame::GetFirst( this );

                    if ( !pFrame || !pDoc->HasName() ||
                        !IsOwnStorageFormat( *pDoc->GetMedium() ) )
                        rSet.DisableItem( nWhich );
                    break;
                }
            case SID_SAVEDOC:
                {
                    if ( !IsReadOnly() )
                        rSet.Put(SfxStringItem(
                            nWhich, SfxResId(STR_SAVEDOC)));
                    else
                        rSet.DisableItem(nWhich);
                }
                break;

            case SID_DOCINFO:
                break;

            case SID_CLOSEDOC:
            {
                rSet.Put(SfxStringItem(nWhich, SfxResId(STR_CLOSEDOC)));
                break;
            }

            case SID_SAVEASDOC:
            {
                if( !( pImpl->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT ) )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }
                if ( /*!pCombinedFilters ||*/ !GetMedium() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxStringItem( nWhich, SfxResId(STR_SAVEASDOC) ) );
                break;
            }

            case SID_SAVEACOPY:
            {
                if( !( pImpl->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT ) )
                {
                    rSet.DisableItem( nWhich );
                    break;
                }
                if ( /*!pCombinedFilters ||*/ !GetMedium() )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxStringItem( nWhich, SfxResId(STR_SAVEACOPY) ) );
                break;
            }

            case SID_EXPORTDOCASPDF:
            case SID_DIRECTEXPORTDOCASPDF:
            case SID_EXPORTDOCASEPUB:
            case SID_DIRECTEXPORTDOCASEPUB:
            case SID_REDACTDOC:
            {
                break;
            }

            case SID_DOC_MODIFIED:
            {
                rSet.Put( SfxBoolItem( SID_DOC_MODIFIED, IsModified() ) );
                break;
            }

            case SID_MODIFIED:
            {
                rSet.Put( SfxBoolItem( SID_MODIFIED, IsModified() ) );
                break;
            }

            case SID_DOCINFO_TITLE:
            {
                rSet.Put( SfxStringItem(
                    SID_DOCINFO_TITLE, getDocProperties()->getTitle() ) );
                break;
            }
            case SID_FILE_NAME:
            {
                if( GetMedium() && HasName() )
                    rSet.Put( SfxStringItem(
                        SID_FILE_NAME, GetMedium()->GetName() ) );
                break;
            }
            case SID_SIGNATURE:
            {
                SfxViewFrame *pFrame = SfxViewFrame::GetFirst(this);
                if ( pFrame )
                {
                    SignatureState eState = GetDocumentSignatureState();
                    InfoBarType aInfoBarType(InfoBarType::Info);
                    OUString sMessage("");

                    switch (eState)
                    {
                    case SignatureState::BROKEN:
                        sMessage = SfxResId(STR_SIGNATURE_BROKEN);
                        aInfoBarType = InfoBarType::Danger;
                        break;
                    case SignatureState::INVALID:
                        sMessage = SfxResId(STR_SIGNATURE_INVALID);
                        // Warning only, I've tried Danger and it looked too scary
                        aInfoBarType = InfoBarType::Warning;
                        break;
                    case SignatureState::NOTVALIDATED:
                        sMessage = SfxResId(STR_SIGNATURE_NOTVALIDATED);
                        aInfoBarType = InfoBarType::Warning;
                        break;
                    case SignatureState::PARTIAL_OK:
                        sMessage = SfxResId(STR_SIGNATURE_PARTIAL_OK);
                        aInfoBarType = InfoBarType::Warning;
                        break;
                    case SignatureState::OK:
                        sMessage = SfxResId(STR_SIGNATURE_OK);
                        aInfoBarType = InfoBarType::Info;
                        break;
                    case SignatureState::NOTVALIDATED_PARTIAL_OK:
                        sMessage = SfxResId(STR_SIGNATURE_NOTVALIDATED_PARTIAL_OK);
                        aInfoBarType = InfoBarType::Warning;
                        break;
                    //FIXME SignatureState::Unknown, own message?
                    default:
                        break;
                    }

                    // new info bar
                    if ( !pFrame->HasInfoBarWithID("signature") )
                    {
                        if ( !sMessage.isEmpty() )
                        {
                            auto pInfoBar = pFrame->AppendInfoBar("signature", sMessage, aInfoBarType);
                            if (pInfoBar == nullptr || pInfoBar->IsDisposed())
                                return;
                            VclPtrInstance<PushButton> xBtn(&(pFrame->GetWindow()));
                            xBtn->SetText(SfxResId(STR_SIGNATURE_SHOW));
                            xBtn->SetSizePixel(xBtn->GetOptimalSize());
                            xBtn->SetClickHdl(LINK(this, SfxObjectShell, SignDocumentHandler));
                            pInfoBar->addButton(xBtn);
                        }
                    }
                    else // info bar exists already
                    {
                        if ( eState == SignatureState::NOSIGNATURES )
                            pFrame->RemoveInfoBar("signature");
                        else
                            pFrame->UpdateInfoBar("signature", sMessage, aInfoBarType);
                    }
                }

                rSet.Put( SfxUInt16Item( SID_SIGNATURE, static_cast<sal_uInt16>(GetDocumentSignatureState()) ) );
                break;
            }
            case SID_MACRO_SIGNATURE:
            {
                // the slot makes sense only if there is a macro in the document
                if ( pImpl->documentStorageHasMacros() || pImpl->aMacroMode.hasMacroLibrary() )
                    rSet.Put( SfxUInt16Item( SID_MACRO_SIGNATURE, static_cast<sal_uInt16>(GetScriptingSignatureState()) ) );
                else
                    rSet.DisableItem( nWhich );
                break;
            }
            case SID_DOC_REPAIR:
            {
                SfxUndoManager* pIUndoMgr = GetUndoManager();
                if (pIUndoMgr)
                    rSet.Put( SfxBoolItem(nWhich, pIUndoMgr->IsEmptyActions()) );
                else
                    rSet.DisableItem( nWhich );
                break;
            }
        }
    }
}

IMPL_LINK_NOARG(SfxObjectShell, SignDocumentHandler, Button*, void)
{
    GetDispatcher()->Execute(SID_SIGNATURE);
}

void SfxObjectShell::ExecProps_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_MODIFIED:
        {
            SetModified( rReq.GetArgs()->Get(SID_MODIFIED).GetValue() );
            rReq.Done();
            break;
        }

        case SID_DOCTITLE:
            SetTitle( rReq.GetArgs()->Get(SID_DOCTITLE).GetValue() );
            rReq.Done();
            break;

        case SID_DOCINFO_AUTHOR :
            getDocProperties()->setAuthor( static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(rReq.GetSlot())).GetValue() );
            break;

        case SID_DOCINFO_COMMENTS :
            getDocProperties()->setDescription( static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(rReq.GetSlot())).GetValue() );
            break;

        case SID_DOCINFO_KEYWORDS :
        {
            const OUString aStr = static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(rReq.GetSlot())).GetValue();
            getDocProperties()->setKeywords(
                ::comphelper::string::convertCommaSeparated(aStr) );
            break;
        }
    }
}


void SfxObjectShell::StateProps_Impl(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_DOCINFO_AUTHOR :
            {
                rSet.Put( SfxStringItem( nSID,
                            getDocProperties()->getAuthor() ) );
                break;
            }

            case SID_DOCINFO_COMMENTS :
            {
                rSet.Put( SfxStringItem( nSID,
                            getDocProperties()->getDescription()) );
                break;
            }

            case SID_DOCINFO_KEYWORDS :
            {
                rSet.Put( SfxStringItem( nSID, ::comphelper::string::
                    convertCommaSeparated(getDocProperties()->getKeywords())) );
                break;
            }

            case SID_DOCPATH:
            {
                OSL_FAIL( "Not supported anymore!" );
                break;
            }

            case SID_DOCFULLNAME:
            {
                rSet.Put( SfxStringItem( SID_DOCFULLNAME, GetTitle(SFX_TITLE_FULLNAME) ) );
                break;
            }

            case SID_DOCTITLE:
            {
                rSet.Put( SfxStringItem( SID_DOCTITLE, GetTitle() ) );
                break;
            }

            case SID_DOC_READONLY:
            {
                rSet.Put( SfxBoolItem( SID_DOC_READONLY, IsReadOnly() ) );
                break;
            }

            case SID_DOC_SAVED:
            {
                rSet.Put( SfxBoolItem( SID_DOC_SAVED, !IsModified() ) );
                break;
            }

            case SID_CLOSING:
            {
                rSet.Put( SfxBoolItem( SID_CLOSING, false ) );
                break;
            }

            case SID_DOC_LOADING:
                rSet.Put( SfxBoolItem( nSID, ! ( pImpl->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT ) ) );
                break;

            case SID_IMG_LOADING:
                rSet.Put( SfxBoolItem( nSID, ! ( pImpl->nLoadedFlags & SfxLoadedFlags::IMAGES ) ) );
                break;
        }
    }
}


void SfxObjectShell::ExecView_Impl(SfxRequest &rReq)
{
    switch ( rReq.GetSlot() )
    {
        case SID_ACTIVATE:
        {
            SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
            if ( pFrame )
                pFrame->GetFrame().Appear();
            rReq.SetReturnValue( SfxObjectItem( 0, pFrame ) );
            rReq.Done();
            break;
        }
    }
}


void SfxObjectShell::StateView_Impl(SfxItemSet& /*rSet*/)
{
}

SignatureState SfxObjectShell::ImplCheckSignaturesInformation( const uno::Sequence< security::DocumentSignatureInformation >& aInfos )
{
    bool bCertValid = true;
    SignatureState nResult = SignatureState::NOSIGNATURES;
    int nInfos = aInfos.getLength();
    bool bCompleteSignature = true;
    if( nInfos )
    {
        nResult = SignatureState::OK;
        for ( int n = 0; n < nInfos; n++ )
        {
            if ( bCertValid )
            {
                sal_Int32 nCertStat = aInfos[n].CertificateStatus;
                bCertValid = nCertStat == security::CertificateValidity::VALID;
            }

            if ( !aInfos[n].SignatureIsValid )
            {
                nResult = SignatureState::BROKEN;
                break; // we know enough
            }
            bCompleteSignature &= !aInfos[n].PartialDocumentSignature;
        }
    }

    if (nResult == SignatureState::OK && !bCertValid && !bCompleteSignature)
        nResult = SignatureState::NOTVALIDATED_PARTIAL_OK;
    else if (nResult == SignatureState::OK && !bCertValid)
        nResult = SignatureState::NOTVALIDATED;
    else if ( nResult == SignatureState::OK && bCertValid && !bCompleteSignature)
        nResult = SignatureState::PARTIAL_OK;

    // this code must not check whether the document is modified
    // it should only check the provided info

    return nResult;
}

/// Does this ZIP storage have a signature stream?
static bool HasSignatureStream(const uno::Reference<embed::XStorage>& xStorage)
{
    uno::Reference<container::XNameAccess> xNameAccess(xStorage, uno::UNO_QUERY);
    if (!xNameAccess.is())
        return false;

    if (xNameAccess->hasByName("META-INF"))
    {
        // ODF case.
        try
        {
            uno::Reference<embed::XStorage> xMetaInf
                = xStorage->openStorageElement("META-INF", embed::ElementModes::READ);
            uno::Reference<container::XNameAccess> xMetaInfNames(xMetaInf, uno::UNO_QUERY);
            if (xMetaInfNames.is())
            {
                return xMetaInfNames->hasByName("documentsignatures.xml")
                       || xMetaInfNames->hasByName("macrosignatures.xml")
                       || xMetaInfNames->hasByName("packagesignatures.xml");
            }
        }
        catch (const css::io::IOException& rException)
        {
            SAL_WARN("sfx.doc", "HasSignatureStream: failed to open META-INF: " << rException.Message);
        }
    }

    // OOXML case.
    return xNameAccess->hasByName("_xmlsignatures");
}

uno::Sequence< security::DocumentSignatureInformation > SfxObjectShell::GetDocumentSignatureInformation( bool bScriptingContent, const uno::Reference< security::XDocumentDigitalSignatures >& xSigner )
{
    uno::Sequence< security::DocumentSignatureInformation > aResult;
    uno::Reference< security::XDocumentDigitalSignatures > xLocSigner = xSigner;

    bool bSupportsSigning = GetMedium() && GetMedium()->GetFilter() && GetMedium()->GetFilter()->GetSupportsSigning();
    if (GetMedium() && !GetMedium()->GetName().isEmpty() && ((IsOwnStorageFormat(*GetMedium()) && GetMedium()->GetStorage().is()) || bSupportsSigning))
    {
        try
        {
            if ( !xLocSigner.is() )
            {
                OUString aVersion;
                try
                {
                    uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
                    xPropSet->getPropertyValue("Version") >>= aVersion;
                }
                catch( uno::Exception& )
                {
                }

                xLocSigner.set( security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), aVersion) );

            }

            if ( bScriptingContent )
                aResult = xLocSigner->verifyScriptingContentSignatures( GetMedium()->GetZipStorageToSign_Impl(),
                                                                uno::Reference< io::XInputStream >() );
            else
            {
                if (GetMedium()->GetStorage().is())
                {
                    // Something ZIP-based.
                    // Only call into xmlsecurity if we see a signature stream,
                    // as libxmlsec init is expensive.
                    if (HasSignatureStream(GetMedium()->GetZipStorageToSign_Impl()))
                        aResult = xLocSigner->verifyDocumentContentSignatures( GetMedium()->GetZipStorageToSign_Impl(),
                                                                        uno::Reference< io::XInputStream >() );
                }
                else
                {
                    // Not ZIP-based, e.g. PDF.
                    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(GetMedium()->GetName(), StreamMode::READ));
                    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
                    uno::Reference<io::XInputStream> xInputStream(xStream, uno::UNO_QUERY);
                    aResult = xLocSigner->verifyDocumentContentSignatures(uno::Reference<embed::XStorage>(), xInputStream);
                }
            }
        }
        catch( css::uno::Exception& )
        {
        }
    }

    return aResult;
}

SignatureState SfxObjectShell::ImplGetSignatureState( bool bScriptingContent )
{
    SignatureState* pState = bScriptingContent ? &pImpl->nScriptingSignatureState : &pImpl->nDocumentSignatureState;

    if ( *pState == SignatureState::UNKNOWN )
    {
        *pState = SignatureState::NOSIGNATURES;

        uno::Sequence< security::DocumentSignatureInformation > aInfos = GetDocumentSignatureInformation( bScriptingContent );
        *pState = ImplCheckSignaturesInformation( aInfos );
    }

    if ( *pState == SignatureState::OK || *pState == SignatureState::NOTVALIDATED
        || *pState == SignatureState::PARTIAL_OK)
    {
        if ( IsModified() )
            *pState = SignatureState::INVALID;
    }

    return *pState;
}

bool SfxObjectShell::PrepareForSigning(weld::Window* pDialogParent)
{
    // check whether the document is signed
    ImplGetSignatureState(); // document signature
    if (GetMedium() && GetMedium()->GetFilter() && GetMedium()->GetFilter()->IsOwnFormat())
        ImplGetSignatureState( true ); // script signature
    bool bHasSign = ( pImpl->nScriptingSignatureState != SignatureState::NOSIGNATURES || pImpl->nDocumentSignatureState != SignatureState::NOSIGNATURES );

    // the target ODF version on saving (only valid when signing ODF of course)
    SvtSaveOptions aSaveOpt;
    SvtSaveOptions::ODFDefaultVersion nVersion = aSaveOpt.GetODFDefaultVersion();

    // the document is not new and is not modified
    OUString aODFVersion(comphelper::OStorageHelper::GetODFVersionFromStorage(GetStorage()));

    if ( IsModified() || !GetMedium() || GetMedium()->GetName().isEmpty()
      || (GetMedium()->GetFilter()->IsOwnFormat() && aODFVersion != ODFVER_012_TEXT && !bHasSign) )
    {
        // the document might need saving ( new, modified or in ODF1.1 format without signature )

        if ( nVersion >= SvtSaveOptions::ODFVER_012 )
        {
            OUString sQuestion(bHasSign ? SfxResId(STR_XMLSEC_QUERY_SAVESIGNEDBEFORESIGN) : SfxResId(RID_SVXSTR_XMLSEC_QUERY_SAVEBEFORESIGN));
            std::unique_ptr<weld::MessageDialog> xQuestion(Application::CreateMessageDialog(pDialogParent,
                                                           VclMessageType::Question, VclButtonsType::YesNo, sQuestion));


            if (xQuestion->run() == RET_YES)
            {
                sal_uInt16 nId = SID_SAVEDOC;
                if ( !GetMedium() || GetMedium()->GetName().isEmpty() )
                    nId = SID_SAVEASDOC;
                SfxRequest aSaveRequest( nId, SfxCallMode::SLOT, GetPool() );
                //ToDo: Review. We needed to call SetModified, otherwise the document would not be saved.
                SetModified();
                ExecFile_Impl( aSaveRequest );

                // Check if it is stored a format which supports signing
                if (GetMedium() && GetMedium()->GetFilter() && !GetMedium()->GetName().isEmpty()
                    && ((!GetMedium()->GetFilter()->IsOwnFormat()
                         && !GetMedium()->GetFilter()->GetSupportsSigning())
                        || (GetMedium()->GetFilter()->IsOwnFormat()
                            && !GetMedium()->HasStorage_Impl())))
                {
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                        pDialogParent, VclMessageType::Info, VclButtonsType::Ok,
                        SfxResId(STR_INFO_WRONGDOCFORMAT)));

                    xBox->run();
                    return false;
                }
            }
            else
            {
                // When the document is modified then we must not show the
                // digital signatures dialog
                // If we have come here then the user denied to save.
                if (!bHasSign)
                    return false;
            }
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pDialogParent,
                                                      VclMessageType::Warning, VclButtonsType::Ok, SfxResId(STR_XMLSEC_ODF12_EXPECTED)));
            xBox->run();
            return false;
        }

        if ( IsModified() || !GetMedium() || GetMedium()->GetName().isEmpty() )
            return false;
    }

    // the document is not modified currently, so it can not become modified after signing
    pImpl->m_bAllowModifiedBackAfterSigning = false;
    if ( IsEnableSetModified() )
    {
        EnableSetModified( false );
        pImpl->m_bAllowModifiedBackAfterSigning = true;
    }

    // we have to store to the original document, the original medium should be closed for this time
    if ( ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
    {
        GetMedium()->CloseAndRelease();
        return true;
    }
    return false;
}

void SfxObjectShell::RecheckSignature(bool bAlsoRecheckScriptingSignature)
{
    if (bAlsoRecheckScriptingSignature)
        pImpl->nScriptingSignatureState = SignatureState::UNKNOWN; // Re-Check

    pImpl->nDocumentSignatureState = SignatureState::UNKNOWN; // Re-Check

    Invalidate(SID_SIGNATURE);
    Invalidate(SID_MACRO_SIGNATURE);
    Broadcast(SfxHint(SfxHintId::TitleChanged));
}

void SfxObjectShell::AfterSigning(bool bSignSuccess, bool bSignScriptingContent)
{
    pImpl->m_bSavingForSigning = true;
    DoSaveCompleted( GetMedium() );
    pImpl->m_bSavingForSigning = false;

    if ( bSignSuccess )
        RecheckSignature(bSignScriptingContent);

    if ( pImpl->m_bAllowModifiedBackAfterSigning )
        EnableSetModified();
}

bool SfxObjectShell::CheckIsReadonly(bool bSignScriptingContent)
{
    if (GetMedium()->IsOriginallyReadOnly())
    {
        // If the file is physically read-only, we just show the existing signatures
        try
        {
            OUString aODFVersion(
                comphelper::OStorageHelper::GetODFVersionFromStorage(GetStorage()));
            uno::Reference<security::XDocumentDigitalSignatures> xSigner(
                security::DocumentDigitalSignatures::createWithVersionAndValidSignature(
                    comphelper::getProcessComponentContext(), aODFVersion, HasValidSignatures()));
            if (bSignScriptingContent)
                xSigner->showScriptingContentSignatures(GetMedium()->GetZipStorageToSign_Impl(),
                                                        uno::Reference<io::XInputStream>());
            else
            {
                uno::Reference<embed::XStorage> xStorage = GetMedium()->GetZipStorageToSign_Impl();
                if (xStorage.is())
                    xSigner->showDocumentContentSignatures(xStorage,
                                                           uno::Reference<io::XInputStream>());
                else
                {
                    std::unique_ptr<SvStream> pStream(
                        utl::UcbStreamHelper::CreateStream(GetName(), StreamMode::READ));
                    uno::Reference<io::XInputStream> xStream(new utl::OStreamWrapper(*pStream));
                    xSigner->showDocumentContentSignatures(uno::Reference<embed::XStorage>(),
                                                           xStream);
                }
            }
        }
        catch (const uno::Exception&)
        {
            SAL_WARN("sfx.doc", "Couldn't use signing functionality!");
        }
        return true;
    }
    return false;
}

bool SfxObjectShell::HasValidSignatures()
{
    return pImpl->nDocumentSignatureState == SignatureState::OK
           || pImpl->nDocumentSignatureState == SignatureState::NOTVALIDATED
           || pImpl->nDocumentSignatureState == SignatureState::PARTIAL_OK;
}

SignatureState SfxObjectShell::GetDocumentSignatureState()
{
    return ImplGetSignatureState();
}

void SfxObjectShell::SignDocumentContent(weld::Window* pDialogParent)
{
    if (!PrepareForSigning(pDialogParent))
        return;

    if (CheckIsReadonly(false))
        return;

    bool bSignSuccess = GetMedium()->SignContents_Impl(pDialogParent, false, HasValidSignatures());

    AfterSigning(bSignSuccess, false);
}

bool SfxObjectShell::SignDocumentContentUsingCertificate(const Reference<XCertificate>& xCertificate)
{
    // 1. PrepareForSigning

    // check whether the document is signed
    ImplGetSignatureState(false); // document signature
    if (GetMedium() && GetMedium()->GetFilter() && GetMedium()->GetFilter()->IsOwnFormat())
        ImplGetSignatureState( true ); // script signature
    bool bHasSign = ( pImpl->nScriptingSignatureState != SignatureState::NOSIGNATURES || pImpl->nDocumentSignatureState != SignatureState::NOSIGNATURES );

    // the target ODF version on saving (only valid when signing ODF of course)
    SvtSaveOptions aSaveOpt;
    SvtSaveOptions::ODFDefaultVersion nVersion = aSaveOpt.GetODFDefaultVersion();

    // the document is not new and is not modified
    OUString aODFVersion(comphelper::OStorageHelper::GetODFVersionFromStorage(GetStorage()));

    if (IsModified() || !GetMedium() || GetMedium()->GetName().isEmpty()
      || (GetMedium()->GetFilter()->IsOwnFormat() && aODFVersion != ODFVER_012_TEXT && !bHasSign))
    {
        if ( nVersion >= SvtSaveOptions::ODFVER_012 )
        {
            sal_uInt16 nId = SID_SAVEDOC;
            if ( !GetMedium() || GetMedium()->GetName().isEmpty() )
                nId = SID_SAVEASDOC;
            SfxRequest aSaveRequest( nId, SfxCallMode::SLOT, GetPool() );
            //ToDo: Review. We needed to call SetModified, otherwise the document would not be saved.
            SetModified();
            ExecFile_Impl( aSaveRequest );

            // Check if it is stored a format which supports signing
            if (GetMedium() && GetMedium()->GetFilter() && !GetMedium()->GetName().isEmpty()
                && ((!GetMedium()->GetFilter()->IsOwnFormat()
                     && !GetMedium()->GetFilter()->GetSupportsSigning())
                    || (GetMedium()->GetFilter()->IsOwnFormat()
                        && !GetMedium()->HasStorage_Impl())))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

        if ( IsModified() || !GetMedium() || GetMedium()->GetName().isEmpty() )
            return false;
    }

    // the document is not modified currently, so it can not become modified after signing
    pImpl->m_bAllowModifiedBackAfterSigning = false;
    if ( IsEnableSetModified() )
    {
        EnableSetModified( false );
        pImpl->m_bAllowModifiedBackAfterSigning = true;
    }

    // we have to store to the original document, the original medium should be closed for this time
    bool bResult = ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium);

    if (!bResult)
        return false;

    GetMedium()->CloseAndRelease();

    // 2. Check Read-Only
    if (GetMedium()->IsOriginallyReadOnly())
        return false;

    // 3. Sign
    bool bSignSuccess = GetMedium()->SignDocumentContentUsingCertificate(HasValidSignatures(), xCertificate);

    // 4. AfterSigning
    AfterSigning(bSignSuccess, false);

    return true;
}

void SfxObjectShell::SignSignatureLine(weld::Window* pDialogParent,
                                       const OUString& aSignatureLineId,
                                       const Reference<XCertificate>& xCert,
                                       const Reference<XGraphic>& xValidGraphic,
                                       const Reference<XGraphic>& xInvalidGraphic,
                                       const OUString& aComment)
{
    if (!PrepareForSigning(pDialogParent))
        return;

    if (CheckIsReadonly(false))
        return;

    bool bSignSuccess = GetMedium()->SignContents_Impl(pDialogParent,
        false, HasValidSignatures(), aSignatureLineId, xCert, xValidGraphic, xInvalidGraphic, aComment);

    AfterSigning(bSignSuccess, false);

    // Reload the document to get the updated graphic
    // FIXME: Update just the signature line graphic instead of reloading the document
    SfxViewFrame *pFrame = GetFrame();
    if (pFrame)
        pFrame->GetDispatcher()->Execute(SID_RELOAD);
}

SignatureState SfxObjectShell::GetScriptingSignatureState()
{
    return ImplGetSignatureState( true );
}

void SfxObjectShell::SignScriptingContent(weld::Window* pDialogParent)
{
    if (!PrepareForSigning(pDialogParent))
        return;

    if (CheckIsReadonly(true))
        return;

    bool bSignSuccess = GetMedium()->SignContents_Impl(pDialogParent, true, HasValidSignatures());

    AfterSigning(bSignSuccess, true);
}

namespace
{
    class theSfxObjectShellUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSfxObjectShellUnoTunnelId > {};
}

const uno::Sequence<sal_Int8>& SfxObjectShell::getUnoTunnelId()
{
    return theSfxObjectShellUnoTunnelId::get().getSeq();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
