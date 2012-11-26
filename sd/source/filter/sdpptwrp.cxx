/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <osl/module.hxx>
#include <filter/msfilter/msoleexp.hxx>
#include <filter/msfilter/svxmsbas.hxx>
#include <svx/svxerr.hxx>
#include <unotools/fltrcfg.hxx>

#include "sdpptwrp.hxx"
#include "ppt/pptin.hxx"
#include "drawdoc.hxx"
#include <tools/urlobj.hxx>
#include <filter/msfilter/msfiltertracer.hxx>

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef bool ( __LOADONCALLAPI *ExportPPT )( const std::vector< com::sun::star::beans::PropertyValue >&, SvStorageRef&,
                                             Reference< XModel > &,
                                             Reference< XStatusIndicator > &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

typedef sal_Bool ( SAL_CALL *ImportPPT )( const ::rtl::OUString&, Sequence< PropertyValue >*,
                                          SdDrawDocument*, SvStream&, SvStorage&, SfxMedium& );

typedef bool ( __LOADONCALLAPI *SaveVBA )( SfxObjectShell&, SvMemoryStream*& );

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
    SotStorageRef pStorage = new SotStorage( mrMedium.GetInStream(), false );
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

            if ( pStorage->IsStream( String( RTL_CONSTASCII_USTRINGPARAM("EncryptedSummary") ) ) )
                mrMedium.SetError( ERRCODE_SVX_READ_FILTER_PPOINT, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
            else
            {
                ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
                if ( pLibrary )
                {
                    ImportPPT PPTImport = reinterpret_cast< ImportPPT >( pLibrary->getFunctionSymbol( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImportPPT" ) ) ) );
                    if ( PPTImport )
                        bRet = PPTImport( aTraceConfigPath, &aConfigData, &mrDocument, *pDocStream, *pStorage, mrMedium );

                    if ( !bRet )
                        mrMedium.SetError( SVSTREAM_WRONGVERSION, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
                }
            }

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
            SotStorageRef    xStorRef = new SotStorage( mrMedium.GetOutStream(), false );
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
                    if ( pFilterOptions->IsEnablePPTPreview() )
                        nCnvrtFlags |= 0x8000;
                }

                mrDocument.SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_TEMP );

                if( mbShowProgress )
                    CreateStatusIndicator();

                rtl::OUString sBaseURI( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
                std::vector< PropertyValue > aProperties;
                PropertyValue aProperty;
                aProperty.Name = sBaseURI;
                aProperty.Value = makeAny( mrMedium.GetBaseURL( true ) );
                aProperties.push_back( aProperty );

                bRet = PPTExport( aProperties, xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags );
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
        ::osl::Module* pLibrary = OpenLibrary( mrMedium.GetFilter()->GetUserData() );
        if( pLibrary )
        {
            SaveVBA pSaveVBA= reinterpret_cast<SaveVBA>(pLibrary->getFunctionSymbol( ::rtl::OUString::createFromAscii("SaveVBA") ));
            if( pSaveVBA )
            {
                pSaveVBA( (SfxObjectShell&) mrDocShell, pBas );
            }
        }
    }
}
