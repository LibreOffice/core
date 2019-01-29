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

#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <osl/diagnose.h>
#include <sot/exchange.hxx>
#include <svtools/embedtransfer.hxx>
#include <tools/mapunit.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gdimtf.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/propertysequence.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <svtools/embedhlp.hxx>

using namespace ::com::sun::star;

SvEmbedTransferHelper::SvEmbedTransferHelper( const uno::Reference< embed::XEmbeddedObject >& xObj,
                                              const Graphic* pGraphic,
                                                sal_Int64 nAspect )
: m_xObj( xObj )
, m_pGraphic( pGraphic ? new Graphic( *pGraphic ) : nullptr )
, m_nAspect( nAspect )
{
    if( xObj.is() )
    {
        TransferableObjectDescriptor aObjDesc;

        FillTransferableObjectDescriptor( aObjDesc, m_xObj, nullptr, m_nAspect );
        PrepareOLE( aObjDesc );
    }
}


SvEmbedTransferHelper::~SvEmbedTransferHelper()
{
}

void SvEmbedTransferHelper::SetParentShellID( const OUString& rShellID )
{
    maParentShellID = rShellID;
}


void SvEmbedTransferHelper::AddSupportedFormats()
{
    AddFormat( SotClipboardFormatId::EMBED_SOURCE );
    AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
    AddFormat( SotClipboardFormatId::GDIMETAFILE );
    AddFormat( SotClipboardFormatId::BITMAP );
}


bool SvEmbedTransferHelper::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc )
{
    bool bRet = false;

    if( m_xObj.is() )
    {
        try
        {
            SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );
            if( HasFormat( nFormat ) )
            {
                if( nFormat == SotClipboardFormatId::OBJECTDESCRIPTOR )
                {
                    TransferableObjectDescriptor aDesc;
                    FillTransferableObjectDescriptor( aDesc, m_xObj, m_pGraphic.get(), m_nAspect );
                    bRet = SetTransferableObjectDescriptor( aDesc );
                }
                else if( nFormat == SotClipboardFormatId::EMBED_SOURCE )
                {
                    try
                    {
                        // TODO/LATER: Probably the graphic should be copied here as well
                        // currently it is handled by the applications
                        utl::TempFile aTmp;
                        aTmp.EnableKillingFile();
                        uno::Reference < embed::XEmbedPersist > xPers( m_xObj, uno::UNO_QUERY );
                        if ( xPers.is() )
                        {
                            uno::Reference < embed::XStorage > xStg = comphelper::OStorageHelper::GetTemporaryStorage();
                            OUString aName( "Dummy" );
                            SvStream* pStream = nullptr;
                            bool bDeleteStream = false;
                            uno::Sequence < beans::PropertyValue > aEmpty;
                            uno::Sequence<beans::PropertyValue> aObjArgs( comphelper::InitPropertySequence({
                                    { "SourceShellID", uno::Any(maParentShellID) },
                                    { "DestinationShellID", uno::Any(rDestDoc) }
                                }));
                            xPers->storeToEntry(xStg, aName, aEmpty, aObjArgs);
                            if ( xStg->isStreamElement( aName ) )
                            {
                                uno::Reference < io::XStream > xStm = xStg->cloneStreamElement( aName );
                                pStream = utl::UcbStreamHelper::CreateStream( xStm ).release();
                                bDeleteStream = true;
                            }
                            else
                            {
                                pStream = aTmp.GetStream( StreamMode::STD_READWRITE );
                                uno::Reference < embed::XStorage > xStor = comphelper::OStorageHelper::GetStorageFromStream( new utl::OStreamWrapper( *pStream ) );
                                xStg->openStorageElement( aName, embed::ElementModes::READ )->copyToStorage( xStor );
                            }

                            const sal_uInt32               nLen = pStream->TellEnd();
                            css::uno::Sequence< sal_Int8 > aSeq( nLen );

                            pStream->Seek( STREAM_SEEK_TO_BEGIN );
                            pStream->ReadBytes(aSeq.getArray(), nLen);
                            if ( bDeleteStream )
                                delete pStream;

                            bRet = ( aSeq.getLength() > 0 );
                            if( bRet )
                            {
                                SetAny( uno::Any(aSeq) );
                            }
                        }
                        else
                        {
                            //TODO/LATER: how to handle objects without persistence?!
                        }
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }
                else if ( nFormat == SotClipboardFormatId::GDIMETAFILE && m_pGraphic )
                {
                    SvMemoryStream aMemStm( 65535, 65535 );
                    aMemStm.SetVersion( SOFFICE_FILEFORMAT_CURRENT );

                    const GDIMetaFile& aMetaFile = m_pGraphic->GetGDIMetaFile();
                    const_cast<GDIMetaFile*>(&aMetaFile)->Write( aMemStm );
                    uno::Any aAny;
                    aAny <<= uno::Sequence< sal_Int8 >( static_cast< const sal_Int8* >( aMemStm.GetData() ),
                                                    aMemStm.TellEnd() );
                    SetAny( aAny );
                    bRet = true;
                }
                else if ( ( nFormat == SotClipboardFormatId::BITMAP || nFormat == SotClipboardFormatId::PNG ) && m_pGraphic )
                {
                    bRet = SetBitmapEx( m_pGraphic->GetBitmapEx(), rFlavor );
                }
                else if ( m_xObj.is() && ::svt::EmbeddedObjectRef::TryRunningState( m_xObj ) )
                {
                    uno::Reference< datatransfer::XTransferable > xTransferable( m_xObj->getComponent(), uno::UNO_QUERY );
                    if ( xTransferable.is() )
                    {
                        uno::Any aAny = xTransferable->getTransferData( rFlavor );
                        SetAny( aAny );
                        bRet = true;
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


void SvEmbedTransferHelper::ObjectReleased()
{
    m_xObj.clear();
}

void SvEmbedTransferHelper::FillTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc,
    const css::uno::Reference< css::embed::XEmbeddedObject >& xObj,
    const Graphic* pGraphic,
    sal_Int64 nAspect )
{
    //TODO/LATER: need TypeName to fill it into the Descriptor (will be shown in listbox)
    css::datatransfer::DataFlavor aFlavor;
    SotExchange::GetFormatDataFlavor( SotClipboardFormatId::OBJECTDESCRIPTOR, aFlavor );

    rDesc.maClassName = SvGlobalName( xObj->getClassID() );
    rDesc.maTypeName = aFlavor.HumanPresentableName;

    //TODO/LATER: the aspect size in the descriptor is wrong, unfortunately the stream
    // representation of the descriptor allows only 4 bytes for the aspect
    // so for internal transport something different should be found
    rDesc.mnViewAspect = sal::static_int_cast<sal_uInt16>( nAspect );

    Size aSize;
    MapMode aMapMode( MapUnit::Map100thMM );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
    {
        if ( pGraphic )
        {
            aMapMode = pGraphic->GetPrefMapMode();
            aSize = pGraphic->GetPrefSize();
        }
        else
            aSize = Size( 2500, 2500 );
    }
    else
    {
        try
        {
            awt::Size aSz;
            aSz = xObj->getVisualAreaSize( rDesc.mnViewAspect );
            aSize = Size( aSz.Width, aSz.Height );
        }
        catch( embed::NoVisualAreaSizeException& )
        {
            OSL_FAIL( "Can not get visual area size!" );
            aSize = Size( 5000, 5000 );
        }

        // TODO/LEAN: getMapUnit can switch object to running state
        aMapMode = MapMode( VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( rDesc.mnViewAspect ) ) );
    }

    rDesc.maSize = OutputDevice::LogicToLogic( aSize, aMapMode, MapMode( MapUnit::Map100thMM ) );
    rDesc.maDragStartPos = Point();
    rDesc.maDisplayName.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
