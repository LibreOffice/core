/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: embedtransfer.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:09:57 $
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
#include "precompiled_svtools.hxx"

#ifndef _COM_SUN_STAR_EMBED_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_NOVISUALAREASIZEEXCEPTION_HPP_
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#include <embedtransfer.hxx>
#include <vcl/mapunit.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include "embedhlp.hxx"

using namespace ::com::sun::star;

SvEmbedTransferHelper::SvEmbedTransferHelper( const uno::Reference< embed::XEmbeddedObject >& xObj )
: m_xObj( xObj )
{
}

// -----------------------------------------------------------------------------

SvEmbedTransferHelper::~SvEmbedTransferHelper()
{
}

// -----------------------------------------------------------------------------

void SvEmbedTransferHelper::AddSupportedFormats()
{
    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
    AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
    AddFormat( FORMAT_GDIMETAFILE );
}

// -----------------------------------------------------------------------------

sal_Bool SvEmbedTransferHelper::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_Bool bRet = sal_False;

    if( m_xObj.is() )
    {
        try
        {
            sal_uInt32 nFormat = SotExchange::GetFormat( rFlavor );
            if( HasFormat( nFormat ) )
            {
                if( nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
                {
                    TransferableObjectDescriptor aDesc;
                    FillTransferableObjectDescriptor( aDesc, m_xObj );
                    bRet = SetTransferableObjectDescriptor( aDesc, rFlavor );
                }
                else if( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
                {
                    try
                    {
                        utl::TempFile aTmp;
                        aTmp.EnableKillingFile( TRUE );
                        uno::Reference < embed::XEmbedPersist > xPers( m_xObj, uno::UNO_QUERY );
                        if ( xPers.is() )
                        {
                            uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetTemporaryStorage();
                            ::rtl::OUString aName = ::rtl::OUString::createFromAscii("Dummy");
                            SvStream* pStream = NULL;
                            BOOL bDeleteStream = FALSE;
                            uno::Sequence < beans::PropertyValue > aEmpty;
                            xPers->storeToEntry( xStg, aName, aEmpty, aEmpty );
                            if ( xStg->isStreamElement( aName ) )
                            {
                                uno::Reference < io::XStream > xStm = xStg->cloneStreamElement( aName );
                                pStream = utl::UcbStreamHelper::CreateStream( xStm );
                                bDeleteStream = TRUE;
                            }
                            else
                            {
                                pStream = aTmp.GetStream( STREAM_STD_READWRITE );
                                uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetStorageFromStream( new utl::OStreamWrapper( *pStream ) );
                                xStg->openStorageElement( aName, embed::ElementModes::READ )->copyToStorage( xStor );
                            }

                            ::com::sun::star::uno::Any                  aAny;
                            const sal_uInt32                            nLen = pStream->Seek( STREAM_SEEK_TO_END );
                            ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( nLen );

                            pStream->Seek( STREAM_SEEK_TO_BEGIN );
                            pStream->Read( aSeq.getArray(),  nLen );
                            if ( bDeleteStream )
                                delete pStream;

                            if( ( bRet = ( aSeq.getLength() > 0 ) ) == sal_True )
                            {
                                aAny <<= aSeq;
                                SetAny( aAny, rFlavor );
                            }
                        }
                        else
                        {
                            //TODO/LATER: how to handle objects without persistance?!
                        }
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }
                else if ( m_xObj.is() && :: svt::EmbeddedObjectRef::TryRunningState( m_xObj ) )
                {
                    uno::Reference< datatransfer::XTransferable > xTransferable( m_xObj->getComponent(), uno::UNO_QUERY );
                    if ( xTransferable.is() )
                    {
                        uno::Any aAny = xTransferable->getTransferData( rFlavor );
                        SetAny( aAny, rFlavor );
                        bRet = sal_True;
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
            // Error handling?
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SvEmbedTransferHelper::ObjectReleased()
{
    m_xObj = uno::Reference< embed::XEmbeddedObject >();
}

void SvEmbedTransferHelper::FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc,
    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >& xObj )
{
    //TODO/LATER: need TypeName to fill it into the Descriptor (will be shown in listbox)
    ::com::sun::star::datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aFlavor );

    rDesc.maClassName = SvGlobalName( xObj->getClassID() );
    rDesc.maTypeName = aFlavor.HumanPresentableName;

    //TODO/LATER: the object does *not* know its aspect! Per default we assume CONTENT
    rDesc.mnViewAspect = sal::static_int_cast<sal_uInt16>(embed::Aspects::MSOLE_CONTENT);

    //TODO/LATER: status needs to become sal_Int64
    rDesc.mnOle2Misc = sal::static_int_cast<sal_Int32>(xObj->getStatus( rDesc.mnViewAspect ));

    awt::Size aSz;
    try
    {
        aSz = xObj->getVisualAreaSize( rDesc.mnViewAspect );
    }
    catch( embed::NoVisualAreaSizeException& )
    {
        OSL_ENSURE( sal_False, "Can not get visual area size!\n" );
        aSz.Width = 5000;
        aSz.Height = 5000;
    }

    // TODO/LEAN: getMapUnit can switch object to running state
    MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( rDesc.mnViewAspect ) );
    rDesc.maSize = OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ), aMapUnit, MAP_100TH_MM );
    rDesc.maDragStartPos = Point();
    rDesc.maDisplayName = String();
    rDesc.mbCanLink = FALSE;
}

