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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <osl/diagnose.h>
#include <comphelper/classids.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/embedhlp.hxx>
#include <filter/msfilter/msdffimp.hxx>

#include <filter/msfilter/msoleexp.hxx>

using namespace ::com::sun::star;

static SvGlobalName GetEmbeddedVersion( const SvGlobalName& aAppName )
{
    if ( aAppName == SvGlobalName( SO3_SM_CLASSID_60 ) )
            return SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_8 );
    else if ( aAppName == SvGlobalName( SO3_SW_CLASSID_60 ) )
            return SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_8 );
    else if ( aAppName == SvGlobalName( SO3_SC_CLASSID_60 ) )
            return SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_8 );
    else if ( aAppName == SvGlobalName( SO3_SDRAW_CLASSID_60 ) )
            return SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_8 );
    else if ( aAppName == SvGlobalName( SO3_SIMPRESS_CLASSID_60 ) )
            return SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_8 );
    else if ( aAppName == SvGlobalName( SO3_SCH_CLASSID_60 ) )
            return SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_8 );

    return SvGlobalName();
}

static OUString GetStorageType( const SvGlobalName& aEmbName )
{
    if ( aEmbName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_8 ) )
        return OUString( "LibreOffice.MathDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_8 ) )
        return OUString( "LibreOffice.WriterDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_8 ) )
        return OUString( "LibreOffice.CalcDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_8 ) )
        return OUString( "LibreOffice.DrawDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_8 ) )
        return OUString( "LibreOffice.ImpressDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_8 ) )
        return OUString("LibreOffice.ChartDocument.1");
    return OUString();
}

static bool UseOldMSExport()
{
    uno::Reference< lang::XMultiServiceFactory > xProvider(
        configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext()));
    try {
        uno::Sequence< uno::Any > aArg( 1 );
        aArg[0] <<= OUString( "/org.openoffice.Office.Common/InternalMSExport" );
        uno::Reference< container::XNameAccess > xNameAccess(
            xProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationUpdateAccess",
                aArg ),
            uno::UNO_QUERY );
        if ( xNameAccess.is() )
        {
            uno::Any aResult = xNameAccess->getByName( "UseOldExport" );

            bool bResult;
            if ( aResult >>= bResult )
                return bResult;
        }
    }
    catch( const uno::Exception& )
    {
    }

    OSL_FAIL( "Could not get access to configuration entry!" );
    return false;
}

void SvxMSExportOLEObjects::ExportOLEObject( const css::uno::Reference < css::embed::XEmbeddedObject>& rObj, SotStorage& rDestStg )
{
    svt::EmbeddedObjectRef aObj( rObj, embed::Aspects::MSOLE_CONTENT );
    ExportOLEObject( aObj, rDestStg );
}

void SvxMSExportOLEObjects::ExportOLEObject( svt::EmbeddedObjectRef const & rObj, SotStorage& rDestStg )
{
    SvGlobalName aOwnGlobalName;
    SvGlobalName aObjName( rObj->getClassID() );
    std::shared_ptr<const SfxFilter> pExpFilter;
    {
        static struct ObjExpType {
            sal_uInt32 nFlag;
            const char* pFilterNm;
            // GlobalNameId
            struct GlobalNameIds {
                sal_uInt32 n1;
                sal_uInt16 n2, n3;
                sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
            }
            aGlNmIds[4];
        } const aArr[] = {
            { OLE_STARMATH_2_MATHTYPE, "MathType 3.x",
                {{SO3_SM_CLASSID_60}, {SO3_SM_CLASSID_50},
                 {SO3_SM_CLASSID_40}, {SO3_SM_CLASSID_30 }}},
            { OLE_STARWRITER_2_WINWORD, "MS Word 97",
                {{SO3_SW_CLASSID_60}, {SO3_SW_CLASSID_50},
                 {SO3_SW_CLASSID_40}, {SO3_SW_CLASSID_30 }}},
            { OLE_STARCALC_2_EXCEL, "MS Excel 97",
                {{SO3_SC_CLASSID_60}, {SO3_SC_CLASSID_50},
                 {SO3_SC_CLASSID_40}, {SO3_SC_CLASSID_30 }}},
            { OLE_STARIMPRESS_2_POWERPOINT, "MS PowerPoint 97",
                {{SO3_SIMPRESS_CLASSID_60}, {SO3_SIMPRESS_CLASSID_50},
                 {SO3_SIMPRESS_CLASSID_40}, {SO3_SIMPRESS_CLASSID_30 }}},
            { 0, "",
                {{SO3_SCH_CLASSID_60}, {SO3_SCH_CLASSID_50},
                 {SO3_SCH_CLASSID_40}, {SO3_SCH_CLASSID_30 }}},
            { 0, "",
                {{SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50},    // SJ: !!!! SO3_SDRAW_CLASSID is only available up from
                 {SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50 }}}, // ver 5.0, it is purpose to have double entries here.

            { 0xffff,nullptr,
                {{SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50},
                {SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50}}}
        };

        for( const ObjExpType* pArr = aArr; !pExpFilter && ( pArr->nFlag != 0xffff ); ++pArr )
        {
            for (const ObjExpType::GlobalNameIds& rId : pArr->aGlNmIds)
            {
                SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                            rId.b8, rId.b9, rId.b10, rId.b11,
                            rId.b12, rId.b13, rId.b14, rId.b15 );
                if( aObjName == aGlbNm )
                {
                    aOwnGlobalName = aGlbNm;

                    // flags for checking if conversion is wanted at all (SaveOptions?!)
                    if( nConvertFlags & pArr->nFlag )
                    {
                        pExpFilter = SfxFilterMatcher().GetFilter4FilterName(OUString::createFromAscii(pArr->pFilterNm));
                        break;
                    }
                }
            }
        }
    }

    if( pExpFilter )                        // use this filter for the export
    {
        try
        {
            if ( rObj->getCurrentState() == embed::EmbedStates::LOADED )
                rObj->changeState( embed::EmbedStates::RUNNING );
            //TODO/LATER: is stream instead of outputstream a better choice?!
            //TODO/LATER: a "StoreTo" method at embedded object would be nice
            uno::Sequence < beans::PropertyValue > aSeq(2);
            SvStream* pStream = new SvMemoryStream;
            aSeq[0].Name = "OutputStream";
            ::uno::Reference < io::XOutputStream > xOut = new ::utl::OOutputStreamWrapper( *pStream );
            aSeq[0].Value <<= xOut;
            aSeq[1].Name = "FilterName";
            aSeq[1].Value <<= pExpFilter->GetName();
            uno::Reference < frame::XStorable > xStor( rObj->getComponent(), uno::UNO_QUERY );
            try
            {
                xStor->storeToURL( "private:stream", aSeq );
            }
            catch( const uno::Exception& ) {} // #TODO really handle exceptions - interactionalhandler etc. ?

            tools::SvRef<SotStorage> xOLEStor = new SotStorage( pStream, true );
            xOLEStor->CopyTo( &rDestStg );
            rDestStg.Commit();
        }
        catch( const uno::Exception& )
        {
            // TODO/LATER: Error handling
            OSL_FAIL( "The object could not be exported!" );
        }
    }
    else if( aOwnGlobalName != SvGlobalName() )
    {
        // own format, maybe SO6 format or lower
        SvGlobalName aEmbName = GetEmbeddedVersion( aOwnGlobalName );
        if ( aEmbName != SvGlobalName() && !UseOldMSExport() )
        {
            // this is a SO6 embedded object, save in old binary format
            rDestStg.SetVersion( SOFFICE_FILEFORMAT_31 );
            rDestStg.SetClass( aEmbName,
                                SotClipboardFormatId::EMBEDDED_OBJ_OLE,
                                GetStorageType( aEmbName ) );
            tools::SvRef<SotStorageStream> xExtStm = rDestStg.OpenSotStream(
                                            "properties_stream");

            bool bExtentSuccess = false;
            if( !xExtStm->GetError() )
            {
                // write extent
                //TODO/MBA: check if writing a size is enough
                if( rObj.GetObject().is() )
                {
                    // MSOLE objects don't need to be in running state for VisualArea access
                    awt::Size aSize;
                    try
                    {
                        // this is an own object, the content size must be stored in the
                        // extension stream
                        aSize = rObj->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
                    }
                    catch( const embed::NoVisualAreaSizeException& )
                    {
                        OSL_FAIL( "Could not get visual area size!" );
                        aSize.Width = 5000;
                        aSize.Height = 5000;
                    }
                    catch( const uno::Exception& )
                    {
                        OSL_FAIL( "Unexpected exception while getting visual area size!" );
                        aSize.Width = 5000;
                        aSize.Height = 5000;
                    }

                    sal_Int32 pRect[4];
                    pRect[0] = 0;
                    pRect[1] = aSize.Width;
                    pRect[2] = 0;
                    pRect[3] = aSize.Height;

                    sal_Int8 aWriteSet[16];
                    for ( int ind = 0; ind < 4; ind++ )
                    {
                        sal_Int32 nVal = pRect[ind];
                        for ( int nByte = 0; nByte < 4; nByte++ )
                        {
                            aWriteSet[ind*4+nByte] = static_cast<sal_Int8>(nVal) % 0x100;
                            nVal /= 0x100;
                        }
                    }

                    bExtentSuccess = (xExtStm->WriteBytes(aWriteSet, 16) == 16);
                }
            }

            if ( bExtentSuccess )
            {
                tools::SvRef<SotStorageStream> xEmbStm = rDestStg.OpenSotStream(
                                                "package_stream");
                if( !xEmbStm->GetError() )
                {
                    try
                    {
                        if ( rObj->getCurrentState() == embed::EmbedStates::LOADED )
                            rObj->changeState( embed::EmbedStates::RUNNING );
                        //TODO/LATER: is stream instead of outputstream a better choice?!
                        //TODO/LATER: a "StoreTo" method at embedded object would be nice
                        uno::Sequence < beans::PropertyValue > aSeq(1);
                        aSeq[0].Name = "OutputStream";
                        ::uno::Reference < io::XOutputStream > xOut = new ::utl::OOutputStreamWrapper( *xEmbStm );
                        aSeq[0].Value <<= xOut;
                        uno::Reference < frame::XStorable > xStor( rObj->getComponent(), uno::UNO_QUERY );
                        xStor->storeToURL( "private:stream", aSeq );
                    }
                    catch( const uno::Exception& )
                    {
                        // TODO/LATER: Error handling
                        OSL_FAIL( "The object could not be exported!" );
                    }
                }
            }
        }
        else
        {
            OSL_FAIL("Own binary format inside own container document!");
        }
    }
    else
    {
        // alien objects
        //TODO/LATER: a "StoreTo" method at embedded object would be nice
        rDestStg.SetVersion( SOFFICE_FILEFORMAT_31 );
        uno::Reference < embed::XStorage > xStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
        uno::Reference < embed::XEmbedPersist > xPers( rObj.GetObject(), uno::UNO_QUERY );
        if ( xPers.is() )
        {
            uno::Sequence < beans::PropertyValue > aEmptySeq;
            OUString aTempName( "bla" );
            try
            {
                xPers->storeToEntry( xStor, aTempName, aEmptySeq, aEmptySeq );
            }
            catch ( const uno::Exception& )
            {}

            tools::SvRef<SotStorage> xOLEStor = SotStorage::OpenOLEStorage( xStor, aTempName, StreamMode::STD_READ );
            xOLEStor->CopyTo( &rDestStg );
            rDestStg.Commit();
        }
    }

    //We never need this stream: See #99809# and #i2179#
    rDestStg.Remove( SVEXT_PERSIST_STREAM );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
