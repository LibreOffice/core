/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdpptwrp.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:37:26 $
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

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <osl/module.hxx>
#include <svx/msoleexp.hxx>
#include <svx/svxmsbas.hxx>
#include <svx/svxerr.hxx>
#include <svtools/fltrcfg.hxx>

#include "sdpptwrp.hxx"
#include "pptin.hxx"
#include "drawdoc.hxx"

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _MS_FILTERTRACER_HXX
#include <svx/msfiltertracer.hxx>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;


typedef BOOL ( __LOADONCALLAPI *ExportPPT )( SvStorageRef&,
                                             Reference< XModel > &,
                                             Reference< XStatusIndicator > &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

// ---------------
// - SdPPTFilter -
// ---------------

SdPPTFilter::SdPPTFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress ),
    pBas    ( NULL )
{
}

// -----------------------------------------------------------------------------

SdPPTFilter::~SdPPTFilter()
{
    delete pBas;    // deleting the compressed basic storage
}

// -----------------------------------------------------------------------------

sal_Bool SdPPTFilter::Import()
{
    sal_Bool    bRet = sal_False;
    SotStorageRef pStorage = new SotStorage( mrMedium.GetInStream(), FALSE );
    if( !pStorage->GetError() )
    {
        /* check if there is a dualstorage, then the
        document is propably a PPT95 containing PPT97 */
        SvStorageRef xDualStorage;
        String sDualStorage( RTL_CONSTASCII_USTRINGPARAM( "PP97_DUALSTORAGE" ) );
        if ( pStorage->IsContained( sDualStorage ) )
        {
            xDualStorage = pStorage->OpenSotStorage( sDualStorage, STREAM_STD_READ );
            pStorage = xDualStorage;
        }
        SvStream* pDocStream = pStorage->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM("PowerPoint Document") ), STREAM_STD_READ );
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetKey( pStorage->GetKey() );

            String aTraceConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Tracing/Import/PowerPoint" ) );
            Sequence< PropertyValue > aConfigData( 1 );
            PropertyValue aPropValue;
            aPropValue.Value <<= rtl::OUString( mrMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
            aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentURL" ) );
            aConfigData[ 0 ] = aPropValue;

            MSFilterTracer aTracer( aTraceConfigPath, &aConfigData );
            aTracer.StartTracing();
            SdPPTImport* pImport = new SdPPTImport( &mrDocument, *pDocStream, *pStorage, mrMedium, &aTracer );
            if ( ( bRet = pImport->Import() ) == sal_False )
            {
                if ( pStorage->IsStream( String( RTL_CONSTASCII_USTRINGPARAM("EncryptedSummary") ) ) )
                    mrMedium.SetError( ERRCODE_SVX_READ_FILTER_PPOINT );
                else
                    mrMedium.SetError( SVSTREAM_WRONGVERSION );
            }
            aTracer.EndTracing();
            delete pImport;
            delete pDocStream;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SdPPTFilter::Export()
{
    ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
    sal_Bool        bRet = sal_False;

    if( pLibrary )
    {
        if( mxModel.is() )
        {
            SotStorageRef    xStorRef = new SotStorage( mrMedium.GetOutStream(), FALSE );
            ExportPPT       PPTExport = reinterpret_cast<ExportPPT>(pLibrary->getFunctionSymbol( ::rtl::OUString::createFromAscii("ExportPPT") ));

            /* !!!
            if ( pViewShell && pViewShell->GetView() )
                pViewShell->GetView()->SdrEndTextEdit();
            */
            if( PPTExport && xStorRef.Is() )
            {
                sal_uInt32          nCnvrtFlags = 0;
                SvtFilterOptions* pFilterOptions = SvtFilterOptions::Get();
                if ( pFilterOptions )
                {
                    if ( pFilterOptions->IsMath2MathType() )
                        nCnvrtFlags |= OLE_STARMATH_2_MATHTYPE;
                    if ( pFilterOptions->IsWriter2WinWord() )
                        nCnvrtFlags |= OLE_STARWRITER_2_WINWORD;
                    if ( pFilterOptions->IsCalc2Excel() )
                        nCnvrtFlags |= OLE_STARCALC_2_EXCEL;
                    if ( pFilterOptions->IsImpress2PowerPoint() )
                        nCnvrtFlags |= OLE_STARIMPRESS_2_POWERPOINT;
                }

                mrDocument.SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

                if( mbShowProgress )
                    CreateStatusIndicator();

                bRet = PPTExport( xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags );
                xStorRef->Commit();
            }
        }
        delete pLibrary;
    }
    return bRet;
}

void SdPPTFilter::PreSaveBasic()
{
    SvtFilterOptions* pFilterOptions = SvtFilterOptions::Get();
    if( pFilterOptions && pFilterOptions->IsLoadPPointBasicStorage() )
    {
        SvStorageRef xDest( new SvStorage( new SvMemoryStream(), TRUE ) );
        SvxImportMSVBasic aMSVBas( (SfxObjectShell&) mrDocShell, *xDest, FALSE, FALSE );
        aMSVBas.SaveOrDelMSVBAStorage( TRUE, String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead") ) );

        SvStorageRef xOverhead = xDest->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead") ) );
        if ( xOverhead.Is() && ( xOverhead->GetError() == SVSTREAM_OK ) )
        {
            SvStorageRef xOverhead2 = xOverhead->OpenSotStorage( String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead") ) );
            if ( xOverhead2.Is() && ( xOverhead2->GetError() == SVSTREAM_OK ) )
            {
                SvStorageStreamRef xTemp = xOverhead2->OpenSotStream( String( RTL_CONSTASCII_USTRINGPARAM("_MS_VBA_Overhead2") ) );
                if ( xTemp.Is() && ( xTemp->GetError() == SVSTREAM_OK ) )
                {
                    UINT32 nLen = xTemp->GetSize();
                    if ( nLen )
                    {
                        char* pTemp = new char[ nLen ];
                        if ( pTemp )
                        {
                            xTemp->Seek( STREAM_SEEK_TO_BEGIN );
                            xTemp->Read( pTemp, nLen );
                            pBas = new SvMemoryStream( pTemp, nLen, STREAM_READ );
                            pBas->ObjectOwnsMemory( TRUE );
                        }
                    }
                }
            }
        }
    }
}
