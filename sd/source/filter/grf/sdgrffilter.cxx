/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdgrffilter.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 18:23:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef WNT
#pragma warning (disable:4190)
#endif

#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_GRAPHIC_GRAPHICTYPE_HPP_
#include <com/sun/star/graphic/GraphicType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS2_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#endif

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
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _XOUTBMP_HXX
#include <svx/xoutbmp.hxx>
#endif

// --

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdgrffilter.hxx"
#include "../../ui/inc/ViewShellBase.hxx"

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWVIEW_HPP_
#include <com/sun/star/drawing/XDrawView.hpp>
#endif
#ifndef SD_DRAW_CONTROLLER_HXX
#include "../../ui/inc/DrawController.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#include <com/sun/star/drawing/XShape.hpp>
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_GRAPHICFILTERREQUEST_HPP_
#include <com/sun/star/drawing/GraphicFilterRequest.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::ucb;
using namespace com::sun::star::ui::dialogs;
using rtl::OUString;
using namespace ::sfx2;


// -----------------------------------------------------------------------------

class SdGRFFilter_ImplInteractionHdl : public ::cppu::WeakImplHelper1< com::sun::star::task::XInteractionHandler >
{
    com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > m_xInter;
    USHORT nFilterError;

    public:

    SdGRFFilter_ImplInteractionHdl( com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > xInteraction ) :
        m_xInter( xInteraction ),
        nFilterError( GRFILTER_OK )
        {}

    ~SdGRFFilter_ImplInteractionHdl();

    USHORT GetErrorCode() const { return nFilterError; };

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
        nFilterError = (USHORT)aErr.ErrCode;
    else
        m_xInter->handle( xRequest );
}


// ---------------
// - SdPPTFilter -
// ---------------

SdGRFFilter::SdGRFFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell, sal_True )
{
    mbHideSpell = mrDocument.GetHideSpell();
    mrDocument.SetHideSpell(TRUE);
}

// -----------------------------------------------------------------------------

SdGRFFilter::~SdGRFFilter()
{
    mrDocument.SetHideSpell(mbHideSpell);
}

// -----------------------------------------------------------------------------

void SdGRFFilter::HandleGraphicFilterError( USHORT nFilterError, ULONG nStreamError )
{
    USHORT nId;

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
        ErrorBox aErrorBox( NULL, WB_OK, String( SdResId( nId ) ) );
        aErrorBox.Execute();
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Import()
{
    Graphic         aGraphic;
    const String    aFileName( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
    GraphicFilter*  pGraphicFilter = GetGrfFilter();
    const USHORT    nFilter = pGraphicFilter->GetImportFormatNumberForTypeName( mrMedium.GetFilter()->GetTypeName() );
    sal_Bool        bRet = sal_False;

    // ggf. Filterdialog ausfuehren
    if ( !pGraphicFilter->HasImportDialog( nFilter ) || pGraphicFilter->DoImportDialog( NULL, nFilter ) )
    {
        SvStream*       pIStm = mrMedium.GetInStream();
        USHORT          nReturn = pIStm ? pGraphicFilter->ImportGraphic( aGraphic, aFileName, *pIStm, nFilter ) : 1;

        if( nReturn )
            HandleGraphicFilterError( nReturn, pGraphicFilter->GetLastError().nStreamError );
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
            if ( ( aGrfSize.Height() > aPagSize.Height() ) || (aGrfSize.Width() > aPagSize.Width() ) &&
                 aGrfSize.Height() && aPagSize.Height() )
            {
                double fGrfWH = (double) aGrfSize.Width() / aGrfSize.Height();
                double fWinWH = (double) aPagSize.Width() / aPagSize.Height();

                // Grafik an Pagesize anpassen (skaliert)
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

            // Ausgaberechteck fuer Grafik setzen
            aPos.X() = ( ( aPagSize.Width() - aGrfSize.Width() ) >> 1 ) + pPage->GetLftBorder();
            aPos.Y() = ( ( aPagSize.Height() - aGrfSize.Height() ) >> 1 )  + pPage->GetUppBorder();

            pPage->InsertObject( new SdrGrafObj( aGraphic, Rectangle( aPos, aGrfSize ) ) );
            bRet = sal_True;
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdGRFFilter::Export()
{
    // SJ: todo: error handling, the GraphicExportFilter does not support proper errorhandling

    sal_Bool bRet = sal_False;

     uno::Reference< lang::XMultiServiceFactory >
        xSMgr( ::comphelper::getProcessServiceFactory() );
    uno::Reference< uno::XInterface > xComponent
        ( xSMgr->createInstance( rtl::OUString::createFromAscii( "com.sun.star.drawing.GraphicExportFilter" ) ),
            uno::UNO_QUERY );
    if ( xComponent.is() )
    {
        uno::Reference< document::XExporter > xExporter
            ( xComponent, uno::UNO_QUERY );
        uno::Reference< document::XFilter > xFilter
            ( xComponent, uno::UNO_QUERY );
        if ( xExporter.is() && xFilter.is() )
        {
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
                    GraphicFilter* pGraphicFilter = GetGrfFilter();
                    if ( pSet && pGraphicFilter && xSource.is() )
                    {
                        const String aTypeName( mrMedium.GetFilter()->GetTypeName() );
                        const USHORT nFilter = pGraphicFilter->GetExportFormatNumberForTypeName( aTypeName );
                        if ( nFilter != GRFILTER_FORMAT_NOTFOUND )
                        {
                            uno::Reference< task::XInteractionHandler > mXInteractionHandler;

                            beans::PropertyValues aArgs;
                            TransformItems( SID_SAVEASDOC, *pSet, aArgs );

                            rtl::OUString sInteractionHandler( RTL_CONSTASCII_USTRINGPARAM( "InteractionHandler" ) );
                            rtl::OUString sFilterName( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
                            rtl::OUString sShortName( pGraphicFilter->GetExportFormatShortName( nFilter ) );

                            sal_Bool    bFilterNameFound = sal_False;
                            sal_Int32   i, nCount;
                            for ( i = 0, nCount = aArgs.getLength(); i < nCount; i++ )
                            {
                                rtl::OUString& rStr = aArgs[ i ].Name;
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
                            bRet = xFilter->filter( aArgs );
                            if ( !bRet && mXInteractionHandler.is() )
                                SdGRFFilter::HandleGraphicFilterError(
                                    static_cast< SdGRFFilter_ImplInteractionHdl* >( mXInteractionHandler.get() )->GetErrorCode(),
                                                    pGraphicFilter->GetLastError().nStreamError );
                        }
                     }
                }
            }
        }
    }
    return bRet;
}

void SdGRFFilter::SaveGraphic( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape )
{
    try
    {
        Reference< XMultiServiceFactory > xSM( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );

        Reference< XGraphicProvider > xProvider( xSM->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.graphic.GraphicProvider" ) ) ), UNO_QUERY_THROW );
        Reference< XPropertySet > xShapeSet( xShape, UNO_QUERY_THROW );

        // detect mime type of graphic
        OUString aMimeType;

        // first try to detect from graphic object
        Reference< XPropertySet > xGraphicSet( xShapeSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic" ) ) ), UNO_QUERY_THROW );
        xGraphicSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MimeType" ) ) ) >>= aMimeType;

        if( aMimeType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "image/x-vclgraphic" ) ) || !aMimeType.getLength() )
        {
            // this failed, try to detect it from graphic stream and URL
            OUString aURL;
            xShapeSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicURL" ) ) ) >>= aURL;

            if( aURL.getLength() == 0 )
                xShapeSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicStreamURL" ) ) ) >>= aURL;

            {
                Reference< XInputStream > xGraphStream( xShapeSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicStream" ) ) ), UNO_QUERY );
                PropertyValues aDesc(2);
                aDesc[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                aDesc[0].Value <<= aURL;
                aDesc[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ) );
                aDesc[1].Value <<= xGraphStream;

                Reference< XPropertySet > xDescSet( xProvider->queryGraphicDescriptor( aDesc ), UNO_QUERY_THROW );

                xDescSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MimeType" ) ) ) >>= aMimeType;
            }
        }

        if( aMimeType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "image/x-vclgraphic" ) ) || !aMimeType.getLength() )
        {
            // this also failed, now set a mimetype that fits graphic best

            // gif for animated pixel
            // png for non animated pixel
            // svm for vector format
            sal_Int8 nGraphicType = 0;
            xGraphicSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicType" ) ) ) >>= nGraphicType;
            switch( nGraphicType )
            {
            case ::com::sun::star::graphic::GraphicType::VECTOR:
                aMimeType = OUString::createFromAscii( "image/x-svm" );
                break;

            case ::com::sun::star::graphic::GraphicType::PIXEL:
                {
                    sal_Bool bAnimated = sal_False;
                    xGraphicSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Animated" ) ) ) >>= bAnimated;

                    if( bAnimated )
                    {
                        aMimeType = OUString::createFromAscii( "image/gif" );
                        break;
                    }
                }
                // Fallthrough!
//          case ::com::sun::star::graphic::GraphicType::EMPTY:
            default:
                aMimeType = OUString::createFromAscii( "image/png" );
                break;
            }
        }

        // init dialog
        SvtPathOptions aPathOpt;
        String sGrfPath( aPathOpt.GetGraphicPath() );

        FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_AUTOEXTENSION, 0 );
        Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

        String aTitle( SdResId( STR_TITLE_SAVE_AS_PICTURE ) );
        aDlgHelper.SetTitle( aTitle );

        INetURLObject aPath;
        aPath.SetSmartURL( sGrfPath);
        xFP->setDisplayDirectory( aPath.GetMainURL(INetURLObject::DECODE_TO_IURI) );

        // populate filter dialog filter list and select default filter to match graphic mime type

        GraphicFilter& rGF = *GetGrfFilter();
        Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);
        OUString aDefaultFormatName;
        USHORT nCount = rGF.GetExportFormatCount();

        std::map< OUString, OUString > aMimeTypeMap;

        for ( USHORT i = 0; i < nCount; i++ )
        {
            const OUString aExportFormatName( rGF.GetExportFormatName( i ) );
            const OUString aFilterMimeType( rGF.GetExportFormatMediaType( i ) );
            xFltMgr->appendFilter( aExportFormatName, rGF.GetExportWildcard( i ) );
            aMimeTypeMap[ aExportFormatName ] = aFilterMimeType;
            if( aMimeType == aFilterMimeType )
                aDefaultFormatName = aExportFormatName;
        }

        if( aDefaultFormatName.getLength() == 0 )
        {
            nCount = rGF.GetImportFormatCount();
            for( USHORT i = 0; i < nCount; i++ )
            {
                const OUString aFilterMimeType( rGF.GetImportFormatMediaType( i ) );
                if( aMimeType == aFilterMimeType )
                {
                    aDefaultFormatName = rGF.GetImportFormatName( i );
                    xFltMgr->appendFilter( aDefaultFormatName,  rGF.GetImportWildcard( i ) );
                    aMimeTypeMap[ aDefaultFormatName ] = aFilterMimeType;
                    break;
                }
            }
        }

        if( aDefaultFormatName.getLength() == 0 )
            aDefaultFormatName = OUString( RTL_CONSTASCII_USTRINGPARAM( "PNG - Portable Network Graphic" ) );

        xFltMgr->setCurrentFilter( aDefaultFormatName );

        // execute dialog

        if( aDlgHelper.Execute() == ERRCODE_NONE )
        {
            OUString sPath( xFP->getFiles().getConstArray()[0] );
            aPath.SetSmartURL( sPath);
            sGrfPath = aPath.GetPath();

            OUString aExportMimeType( aMimeTypeMap[xFltMgr->getCurrentFilter()] );

            Reference< XInputStream > xGraphStream;
            if( aMimeType == aExportMimeType )
                xShapeSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "GraphicStream" ) ) ) >>= xGraphStream;

            if( xGraphStream.is() )
            {
                Reference< XSimpleFileAccess2 > xFileAccess( xSM->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" ) ) ), UNO_QUERY_THROW );
                xFileAccess->writeFile( sPath, xGraphStream );
            }
            else
            {
                PropertyValues aDesc(2);
                aDesc[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                aDesc[0].Value <<= sPath;
                aDesc[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "MimeType" ) );
                aDesc[1].Value <<= aExportMimeType;
                Reference< XGraphic > xGraphic( xShapeSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic" ) ) ), UNO_QUERY_THROW );
                xProvider->storeGraphic( xGraphic, aDesc );
            }
        }
    }
    catch( Exception& )
    {
        DBG_ERROR(
            (rtl::OString("SdGRFFilter::SaveGraphic(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}
