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

#ifdef _MSC_VER
#pragma warning (disable:4190)
#endif
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>

#include <unotools/localfilehelper.hxx>
#include <vcl/errinf.hxx>
#include <vcl/layout.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frame.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include "strings.hrc"
#include "DrawViewShell.hxx"
#include "DrawDocShell.hxx"
#include "ClientView.hxx"
#include "FrameView.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdgrffilter.hxx"
#include "ViewShellBase.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include "DrawController.hxx"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/drawing/XShape.hpp>
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

    ErrCode GetErrorCode() const { return nFilterError; };

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
        ScopedVclPtrInstance< MessageDialog > aErrorBox(nullptr, SdResId(pId));
        aErrorBox->Execute();
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
                                  aGraphic.GetPrefMapMode(), MapUnit::Map100thMM ) );

            aPagSize.Width() -= pPage->GetLftBorder() + pPage->GetRgtBorder();
            aPagSize.Height() -= pPage->GetUppBorder() + pPage->GetLwrBorder();

            // scale to fit page
            if ( ( ( aGrfSize.Height() > aPagSize.Height() ) || ( aGrfSize.Width() > aPagSize.Width() ) ) &&
                 aGrfSize.Height() && aPagSize.Height() )
            {
                double fGrfWH = (double) aGrfSize.Width() / aGrfSize.Height();
                double fWinWH = (double) aPagSize.Width() / aPagSize.Height();

                // adjust graphic to page size (scales)
                if( fGrfWH < fWinWH )
                {
                    aGrfSize.Width() = (long) ( aPagSize.Height() * fGrfWH );
                    aGrfSize.Height() = aPagSize.Height();
                }
                else if( fGrfWH > 0.F )
                {
                    aGrfSize.Width() = aPagSize.Width();
                    aGrfSize.Height()= (long) ( aPagSize.Width() / fGrfWH );
                }
            }

            // set output rectangle for graphic
            aPos.X() = ( ( aPagSize.Width() - aGrfSize.Width() ) >> 1 ) + pPage->GetLftBorder();
            aPos.Y() = ( ( aPagSize.Height() - aGrfSize.Height() ) >> 1 )  + pPage->GetUppBorder();

            pPage->InsertObject( new SdrGrafObj( aGraphic, ::tools::Rectangle( aPos, aGrfSize ) ) );
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

                    OUString sFilterName( "FilterName" );
                    OUString sShortName( rGraphicFilter.GetExportFormatShortName( nFilter ) );

                    bool    bFilterNameFound = false;
                    sal_Int32   i, nCount;
                    for ( i = 0, nCount = aArgs.getLength(); i < nCount; i++ )
                    {
                        OUString& rStr = aArgs[ i ].Name;
                        if ( rStr == sFilterName )
                        {
                            bFilterNameFound = true;
                            aArgs[ i ].Name = sFilterName;
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
                        && static_cast< const SfxBoolItem& >( pSet->Get( SID_SELECTION ) ).GetValue()
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
