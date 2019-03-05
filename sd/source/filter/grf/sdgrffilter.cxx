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

#include <com/sun/star/drawing/GraphicExportFilter.hpp>

#include <vcl/errinf.hxx>
#include <vcl/weld.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/svdograf.hxx>

#include <strings.hrc>
#include <DrawViewShell.hxx>
#include <DrawDocShell.hxx>

#include <comphelper/processfactory.hxx>
#include <vcl/graphicfilter.hxx>

#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <sdgrffilter.hxx>
#include <ViewShellBase.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/drawing/GraphicFilterRequest.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace com::sun::star::ui::dialogs;
using namespace ::sfx2;

class SdGRFFilter_ImplInteractionHdl : public ::cppu::WeakImplHelper< css::task::XInteractionHandler >
{
    css::uno::Reference< css::task::XInteractionHandler > m_xInter;
    ErrCode nFilterError;

    public:

    explicit SdGRFFilter_ImplInteractionHdl( css::uno::Reference< css::task::XInteractionHandler > const & xInteraction ) :
        m_xInter( xInteraction ),
        nFilterError( ERRCODE_NONE )
        {}

    ErrCode const & GetErrorCode() const { return nFilterError; };

    virtual void SAL_CALL   handle( const css::uno::Reference< css::task::XInteractionRequest >& ) override;
};

void SdGRFFilter_ImplInteractionHdl::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest )
{
    if( !m_xInter.is() )
        return;

    css::drawing::GraphicFilterRequest aErr;
    if ( xRequest->getRequest() >>= aErr )
        nFilterError = ErrCode(aErr.ErrCode);
    else
        m_xInter->handle( xRequest );
}


SdGRFFilter::SdGRFFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell )
{
}

SdGRFFilter::~SdGRFFilter()
{
}

void SdGRFFilter::HandleGraphicFilterError( ErrCode nFilterError, ErrCode nStreamError )
{
    if (ERRCODE_NONE != nStreamError)
    {
        ErrorHandler::HandleError(nStreamError);
        return;
    }

    const char* pId;

    if( nFilterError == ERRCODE_GRFILTER_OPENERROR )
        pId = STR_IMPORT_GRFILTER_OPENERROR;
    else if( nFilterError == ERRCODE_GRFILTER_IOERROR )
        pId = STR_IMPORT_GRFILTER_IOERROR;
    else if( nFilterError == ERRCODE_GRFILTER_FORMATERROR )
        pId = STR_IMPORT_GRFILTER_FORMATERROR;
    else if( nFilterError == ERRCODE_GRFILTER_VERSIONERROR )
        pId = STR_IMPORT_GRFILTER_VERSIONERROR;
    else if( nFilterError == ERRCODE_GRFILTER_TOOBIG )
        pId = STR_IMPORT_GRFILTER_TOOBIG;
    else if( nFilterError == ERRCODE_NONE )
        pId = nullptr;
    else
        pId = STR_IMPORT_GRFILTER_FILTERERROR;

    if (pId && strcmp(pId, STR_IMPORT_GRFILTER_IOERROR) == 0)
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
    else
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(nullptr,
                                                       VclMessageType::Warning, VclButtonsType::Ok, pId ? SdResId(pId) : OUString()));
        xErrorBox->run();
    }
}

bool SdGRFFilter::Import()
{
    Graphic         aGraphic;
    const OUString  aFileName( mrMedium.GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nFilter = rGraphicFilter.GetImportFormatNumberForTypeName( mrMedium.GetFilter()->GetTypeName() );
    bool        bRet = false;

    SvStream*       pIStm = mrMedium.GetInStream();
    ErrCode         nReturn = pIStm ? rGraphicFilter.ImportGraphic( aGraphic, aFileName, *pIStm, nFilter ) : ErrCode(1);

    if( nReturn )
        HandleGraphicFilterError( nReturn, rGraphicFilter.GetLastError().nStreamError );
    else
    {
        if( mrDocument.GetPageCount() == 0 )
            mrDocument.CreateFirstPages();

        SdPage*     pPage = mrDocument.GetSdPage( 0, PageKind::Standard );
        Point       aPos;
        Size        aPagSize( pPage->GetSize() );
        Size        aGrfSize( OutputDevice::LogicToLogic( aGraphic.GetPrefSize(),
                                aGraphic.GetPrefMapMode(), MapMode(MapUnit::Map100thMM)));

        aPagSize.AdjustWidth( -(pPage->GetLeftBorder() + pPage->GetRightBorder()) );
        aPagSize.AdjustHeight( -(pPage->GetUpperBorder() + pPage->GetLowerBorder()) );

        // scale to fit page
        if ( ( ( aGrfSize.Height() > aPagSize.Height() ) || ( aGrfSize.Width() > aPagSize.Width() ) ) &&
                aGrfSize.Height() && aPagSize.Height() )
        {
            double fGrfWH = static_cast<double>(aGrfSize.Width()) / aGrfSize.Height();
            double fWinWH = static_cast<double>(aPagSize.Width()) / aPagSize.Height();

            // adjust graphic to page size (scales)
            if( fGrfWH < fWinWH )
            {
                aGrfSize.setWidth( static_cast<long>( aPagSize.Height() * fGrfWH ) );
                aGrfSize.setHeight( aPagSize.Height() );
            }
            else if( fGrfWH > 0.F )
            {
                aGrfSize.setWidth( aPagSize.Width() );
                aGrfSize.setHeight( static_cast<long>( aPagSize.Width() / fGrfWH ) );
            }
        }

        // set output rectangle for graphic
        aPos.setX( ( ( aPagSize.Width() - aGrfSize.Width() ) >> 1 ) + pPage->GetLeftBorder() );
        aPos.setY( ( ( aPagSize.Height() - aGrfSize.Height() ) >> 1 )  + pPage->GetUpperBorder() );

        pPage->InsertObject(
            new SdrGrafObj(
                pPage->getSdrModelFromSdrPage(),
                aGraphic,
                ::tools::Rectangle(aPos, aGrfSize)));
        bRet = true;
    }

    return bRet;
}

bool SdGRFFilter::Export()
{
    // SJ: todo: error handling, the GraphicExportFilter does not support proper errorhandling
    bool bRet = false;

    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference< drawing::XGraphicExportFilter > xExporter = drawing::GraphicExportFilter::create( xContext );

    SdPage* pPage = nullptr;
    sd::DrawViewShell*  pDrawViewShell = dynamic_cast<::sd::DrawViewShell* >(mrDocShell.GetViewShell() );

    PageKind ePageKind = PageKind::Standard;
    if( pDrawViewShell )
    {
        ePageKind = pDrawViewShell->GetPageKind();
        if( PageKind::Handout == ePageKind )
            pPage = mrDocument.GetSdPage( 0, PageKind::Handout );
        else
            pPage = pDrawViewShell->GetActualPage();
    }
    else
        pPage = mrDocument.GetSdPage( 0, PageKind::Standard );

    if ( pPage )
    {
        // taking the 'correct' page number, seems that there might exist a better method to archive this
        pPage = mrDocument.GetSdPage( pPage->GetPageNum() ? ( pPage->GetPageNum() - 1 ) >> 1 : 0, ePageKind );
        if ( pPage )
        {
            uno::Reference< lang::XComponent > xSource( pPage->getUnoPage(), uno::UNO_QUERY );
            SfxItemSet* pSet = mrMedium.GetItemSet();
            if ( pSet && xSource.is() )
            {
                const OUString aTypeName( mrMedium.GetFilter()->GetTypeName() );
                GraphicFilter &rGraphicFilter = GraphicFilter::GetGraphicFilter();
                const sal_uInt16 nFilter = rGraphicFilter.GetExportFormatNumberForTypeName( aTypeName );
                if ( nFilter != GRFILTER_FORMAT_NOTFOUND )
                {
                    uno::Reference< task::XInteractionHandler > xInteractionHandler;

                    beans::PropertyValues aArgs;
                    TransformItems( SID_SAVEASDOC, *pSet, aArgs );

                    const OUString sFilterName( "FilterName" );
                    OUString sShortName( rGraphicFilter.GetExportFormatShortName( nFilter ) );

                    bool    bFilterNameFound = false;
                    sal_Int32   i, nCount;
                    for ( i = 0, nCount = aArgs.getLength(); i < nCount; i++ )
                    {
                        OUString& rStr = aArgs[ i ].Name;
                        if ( rStr == sFilterName )
                        {
                            bFilterNameFound = true;
                            aArgs[ i ].Value <<= sShortName;
                        }
                        else if ( rStr == "InteractionHandler" )
                        {
                            uno::Reference< task::XInteractionHandler > xHdl;
                            if ( aArgs[ i ].Value >>= xHdl )
                            {
                                xInteractionHandler = new SdGRFFilter_ImplInteractionHdl( xHdl );
                                aArgs[ i ].Value <<= xInteractionHandler;
                            }
                        }
                    }
                    if ( !bFilterNameFound )
                    {
                        aArgs.realloc( ++nCount );
                        aArgs[ i ].Name = sFilterName;
                        aArgs[ i ].Value <<= sShortName;
                    }

                    // take selection if needed
                    if( ( SfxItemState::SET == pSet->GetItemState( SID_SELECTION ) )
                        && pSet->Get( SID_SELECTION ).GetValue()
                        && pDrawViewShell )
                    {
                        uno::Reference< view::XSelectionSupplier > xSelectionSupplier(
                            pDrawViewShell->GetViewShellBase().GetController(), uno::UNO_QUERY );
                        if ( xSelectionSupplier.is() )
                        {
                            uno::Any aSelection( xSelectionSupplier->getSelection() );
                            uno::Reference< lang::XComponent > xSelection;
                            if ( aSelection >>= xSelection )
                                xSource = xSelection;
                        }
                    }
                    xExporter->setSourceDocument( xSource );
                    bRet = xExporter->filter( aArgs );
                    if ( !bRet && xInteractionHandler.is() )
                        SdGRFFilter::HandleGraphicFilterError(
                            static_cast< SdGRFFilter_ImplInteractionHdl* >( xInteractionHandler.get() )->GetErrorCode(),
                                            rGraphicFilter.GetLastError().nStreamError );
                }
             }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
