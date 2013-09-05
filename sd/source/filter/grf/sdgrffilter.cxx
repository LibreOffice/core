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
#include <tools/errinf.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frame.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>

#include "../../ui/inc/strings.hrc"
#include "../../ui/inc/DrawViewShell.hxx"
#include "../../ui/inc/DrawDocShell.hxx"
#include "../../ui/inc/ClientView.hxx"
#include "../../ui/inc/FrameView.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

// --
#include <comphelper/processfactory.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>

// --

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdgrffilter.hxx"
#include "../../ui/inc/ViewShellBase.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include "../../ui/inc/DrawController.hxx"
#include <cppuhelper/implbase2.hxx>
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


// -----------------------------------------------------------------------------

class SdGRFFilter_ImplInteractionHdl : public ::cppu::WeakImplHelper1< com::sun::star::task::XInteractionHandler >
{
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInter;
    sal_uInt16 nFilterError;

    public:

    SdGRFFilter_ImplInteractionHdl( com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteraction ) :
        m_xInter( xInteraction ),
        nFilterError( GRFILTER_OK )
        {}

    ~SdGRFFilter_ImplInteractionHdl();

    sal_uInt16 GetErrorCode() const { return nFilterError; };

    virtual void SAL_CALL   handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& )
                                throw( com::sun::star::uno::RuntimeException );
};

SdGRFFilter_ImplInteractionHdl::~SdGRFFilter_ImplInteractionHdl()
{
}

void SdGRFFilter_ImplInteractionHdl::handle( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionRequest >& xRequest )
        throw( com::sun::star::uno::RuntimeException )
{
    if( !m_xInter.is() )
        return;

    com::sun::star::drawing::GraphicFilterRequest aErr;
    if ( xRequest->getRequest() >>= aErr )
        nFilterError = (sal_uInt16)aErr.ErrCode;
    else
        m_xInter->handle( xRequest );
}


// ---------------
// - SdPPTFilter -
// ---------------

SdGRFFilter::SdGRFFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell, sal_True )
{
}

// -----------------------------------------------------------------------------

SdGRFFilter::~SdGRFFilter()
{
}

// -----------------------------------------------------------------------------

void SdGRFFilter::HandleGraphicFilterError( sal_uInt16 nFilterError, sal_uLong nStreamError )
{
    sal_uInt16 nId;

    switch( nFilterError )
    {
        case GRFILTER_OPENERROR:
            nId = STR_IMPORT_GRFILTER_OPENERROR;
            break;
        case GRFILTER_IOERROR:
            nId = STR_IMPORT_GRFILTER_IOERROR;
            break;
        case GRFILTER_FORMATERROR:
            nId = STR_IMPORT_GRFILTER_FORMATERROR;
            break;
        case GRFILTER_VERSIONERROR:
            nId = STR_IMPORT_GRFILTER_VERSIONERROR;
            break;
        case GRFILTER_TOOBIG:
            nId = STR_IMPORT_GRFILTER_TOOBIG;
            break;
        case 0 :
            nId = 0;
            break;

        default:
        case GRFILTER_FILTERERROR:
            nId = STR_IMPORT_GRFILTER_FILTERERROR;
            break;
    }

    if( ERRCODE_NONE != nStreamError )
        ErrorHandler::HandleError( nStreamError );
    else if( STR_IMPORT_GRFILTER_IOERROR == nId )
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
    else
    {
        ErrorBox aErrorBox(NULL, WB_OK, SD_RESSTR(nId));
        aErrorBox.Execute();
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Import()
{
    Graphic         aGraphic;
    const OUString  aFileName( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nFilter = rGraphicFilter.GetImportFormatNumberForTypeName( mrMedium.GetFilter()->GetTypeName() );
    sal_Bool        bRet = sal_False;

        SvStream*       pIStm = mrMedium.GetInStream();
        sal_uInt16          nReturn = pIStm ? rGraphicFilter.ImportGraphic( aGraphic, aFileName, *pIStm, nFilter ) : 1;

        if( nReturn )
            HandleGraphicFilterError( nReturn, rGraphicFilter.GetLastError().nStreamError );
        else
        {
            if( mrDocument.GetPageCount() == 0L )
                mrDocument.CreateFirstPages();

            SdPage*     pPage = mrDocument.GetSdPage( 0, PK_STANDARD );
            Point       aPos;
            Size        aPagSize( pPage->GetSize() );
            Size        aGrfSize( OutputDevice::LogicToLogic( aGraphic.GetPrefSize(),
                                  aGraphic.GetPrefMapMode(), MAP_100TH_MM ) );

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

            pPage->InsertObject( new SdrGrafObj( aGraphic, Rectangle( aPos, aGrfSize ) ) );
            bRet = sal_True;
        }
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Export()
{
    // SJ: todo: error handling, the GraphicExportFilter does not support proper errorhandling

    sal_Bool bRet = sal_False;

     uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    uno::Reference< drawing::XGraphicExportFilter > xExporter = drawing::GraphicExportFilter::create( xContext );

    SdPage* pPage = NULL;
    sd::DrawViewShell*  pDrawViewShell = static_cast< ::sd::DrawViewShell* >
        ( ( ( mrDocShell.GetViewShell() && mrDocShell.GetViewShell()->ISA(::sd::DrawViewShell ) ) ? mrDocShell.GetViewShell() : NULL ) );

    PageKind ePageKind = PK_STANDARD;
    if( pDrawViewShell )
    {
        ePageKind = pDrawViewShell->GetPageKind();
        if( PK_HANDOUT == ePageKind )
            pPage = mrDocument.GetSdPage( 0, PK_HANDOUT );
        else
            pPage = pDrawViewShell->GetActualPage();
    }
    else
        pPage = mrDocument.GetSdPage( 0, PK_STANDARD );

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
                    uno::Reference< task::XInteractionHandler > mXInteractionHandler;

                    beans::PropertyValues aArgs;
                    TransformItems( SID_SAVEASDOC, *pSet, aArgs );

                    OUString sInteractionHandler( "InteractionHandler" );
                    OUString sFilterName( "FilterName" );
                    OUString sShortName( rGraphicFilter.GetExportFormatShortName( nFilter ) );

                    sal_Bool    bFilterNameFound = sal_False;
                    sal_Int32   i, nCount;
                    for ( i = 0, nCount = aArgs.getLength(); i < nCount; i++ )
                    {
                        OUString& rStr = aArgs[ i ].Name;
                        if ( rStr == sFilterName )
                        {
                            bFilterNameFound = sal_True;
                            aArgs[ i ].Name = sFilterName;
                            aArgs[ i ].Value <<= sShortName;
                        }
                        else if ( rStr == sInteractionHandler )
                        {
                            uno::Reference< task::XInteractionHandler > xHdl;
                            if ( aArgs[ i ].Value >>= xHdl )
                            {
                                mXInteractionHandler = new SdGRFFilter_ImplInteractionHdl( xHdl );
                                aArgs[ i ].Value <<= mXInteractionHandler;
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
                    if( ( SFX_ITEM_SET == pSet->GetItemState( SID_SELECTION ) )
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
                    if ( !bRet && mXInteractionHandler.is() )
                        SdGRFFilter::HandleGraphicFilterError(
                            static_cast< SdGRFFilter_ImplInteractionHdl* >( mXInteractionHandler.get() )->GetErrorCode(),
                                            rGraphicFilter.GetLastError().nStreamError );
                }
             }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
