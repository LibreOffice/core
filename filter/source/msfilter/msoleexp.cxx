/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <sot/clsids.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sot/formats.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/embedhlp.hxx>
#include <filter/msfilter/msdffimp.hxx> // extern sichtbare Header-Datei

#include "filter/msfilter/msoleexp.hxx"

#define CREATE_CONST_ASC(s) String::CreateFromAscii( \
    RTL_CONSTASCII_STRINGPARAM(s))

using namespace ::com::sun::star;

SvGlobalName GetEmbeddedVersion( const SvGlobalName& aAppName )
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

String GetStorageType( const SvGlobalName& aEmbName )
{
    if ( aEmbName == SvGlobalName( SO3_SM_OLE_EMBED_CLASSID_8 ) )
            return String::CreateFromAscii( "opendocument.MathDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SW_OLE_EMBED_CLASSID_8 ) )
            return String::CreateFromAscii( "opendocument.WriterDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SC_OLE_EMBED_CLASSID_8 ) )
            return String::CreateFromAscii( "opendocument.CalcDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SDRAW_OLE_EMBED_CLASSID_8 ) )
            return String::CreateFromAscii( "opendocument.DrawDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SIMPRESS_OLE_EMBED_CLASSID_8 ) )
            return String::CreateFromAscii( "opendocument.ImpressDocument.1" );
    else if ( aEmbName == SvGlobalName( SO3_SCH_OLE_EMBED_CLASSID_8 ) )
            return String::CreateFromAscii( "opendocument.ChartDocument.1" );

    return String();
}

sal_Bool UseOldMSExport()
{
    uno::Reference< lang::XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();

    if ( xFactory.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xProvider( xFactory->createInstance(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider"))),
                    uno::UNO_QUERY);
        if ( xProvider.is() )
        {
            try {
                uno::Sequence< uno::Any > aArg( 1 );
                aArg[0] <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common/InternalMSExport") );
                uno::Reference< container::XNameAccess > xNameAccess(
                    xProvider->createInstanceWithArguments(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) ),
                        aArg ),
                    uno::UNO_QUERY );
                if ( xNameAccess.is() )
                {
                    uno::Any aResult = xNameAccess->getByName(
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseOldExport" ) ) );

                    sal_Bool bResult = sal_Bool();
                    if ( aResult >>= bResult )
                        return bResult;
                }
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    OSL_ENSURE( sal_False, "Could not get access to configuration entry!\n" );
    return sal_False;
}

void SvxMSExportOLEObjects::ExportOLEObject( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject>& rObj, SotStorage& rDestStg )
{
    svt::EmbeddedObjectRef aObj( rObj, embed::Aspects::MSOLE_CONTENT );
    ExportOLEObject( aObj, rDestStg );
}

void SvxMSExportOLEObjects::ExportOLEObject( svt::EmbeddedObjectRef& rObj, SvStorage& rDestStg )
{
    SvGlobalName aOwnGlobalName;
    SvGlobalName aObjName( rObj->getClassID() );
    const SfxFilter* pExpFilter = NULL;
    {
        static struct _ObjExpType {
            UINT32 nFlag;
            const char* pFilterNm;
            // GlobalNameId
            struct _GlobalNameIds {
                UINT32 n1;
                USHORT n2, n3;
                BYTE b8, b9, b10, b11, b12, b13, b14, b15;
            }
            aGlNmIds[4];
        } aArr[] = {
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
                {{SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50},	// SJ: !!!! SO3_SDRAW_CLASSID is only available up from
                 {SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50 }}}, // ver 5.0, it is purpose to have double entrys here.

            { 0xffff,0,
                {{SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50},
                {SO3_SDRAW_CLASSID_60}, {SO3_SDRAW_CLASSID_50}}}
        };

        for( const _ObjExpType* pArr = aArr; !pExpFilter && ( pArr->nFlag != 0xffff ); ++pArr )
        {
            for ( int n = 0; n < 4; ++n )
            {
                const _ObjExpType::_GlobalNameIds& rId = pArr->aGlNmIds[ n ];
                SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                            rId.b8, rId.b9, rId.b10, rId.b11,
                            rId.b12, rId.b13, rId.b14, rId.b15 );
                if( aObjName == aGlbNm )
                {
                    aOwnGlobalName = aGlbNm;

                    // flags for checking if conversion is wanted at all (SaveOptions?!)
                    if( GetFlags() & pArr->nFlag )
                    {
                        pExpFilter = SfxFilterMatcher().GetFilter4FilterName(String::CreateFromAscii(pArr->pFilterNm));
                        break;
                    }
                }
            }
        }
    }

    if( pExpFilter )				        // use this filter for the export
    {
        try
        {
            if ( rObj->getCurrentState() == embed::EmbedStates::LOADED )
                rObj->changeState( embed::EmbedStates::RUNNING );
            //TODO/LATER: is stream instead of outputstream a better choice?!
            //TODO/LATER: a "StoreTo" method at embedded object would be nice
            uno::Sequence < beans::PropertyValue > aSeq(2);
            SvStream* pStream = new SvMemoryStream;
            aSeq[0].Name = ::rtl::OUString::createFromAscii( "OutputStream" );
            ::uno::Reference < io::XOutputStream > xOut = new ::utl::OOutputStreamWrapper( *pStream );
            aSeq[0].Value <<= xOut;
            aSeq[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ) );
            aSeq[1].Value <<= ::rtl::OUString( pExpFilter->GetName() );
            uno::Reference < frame::XStorable > xStor( rObj->getComponent(), uno::UNO_QUERY );
            xStor->storeToURL( ::rtl::OUString::createFromAscii( "private:stream" ), aSeq );
            SotStorageRef xOLEStor = new SotStorage( pStream, TRUE );
            xOLEStor->CopyTo( &rDestStg );
            rDestStg.Commit();
        }
        catch( uno::Exception& )
        {
            // TODO/LATER: Error handling
            DBG_ERROR( "The object could not be exported!" );
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
                                SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE,
                                GetStorageType( aEmbName ) );
            SotStorageStreamRef xExtStm = rDestStg.OpenSotStream(
                                            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "properties_stream" ) ),
                                            STREAM_STD_READWRITE );

            sal_Bool bExtentSuccess = sal_False;
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
                    catch( embed::NoVisualAreaSizeException& )
                    {
                        OSL_ENSURE( sal_False, "Could not get visual area size!\n" );
                        aSize.Width = 5000;
                        aSize.Height = 5000;
                    }
                    catch( uno::Exception& )
                    {
                        OSL_ENSURE( sal_False, "Unexpected exception while getting visual area size!\n" );
                        aSize.Width = 5000;
                        aSize.Height = 5000;
                    }
                    
                    //Rectangle aVisArea = xSfxIPObj->GetVisArea( ASPECT_CONTENT );
                    sal_Int32 pRect[4];
                    //pRect[0] = aVisArea.Left();
                    //pRect[1] = aVisArea.Right();
                    //pRect[2] = aVisArea.Top();
                    //pRect[3] = aVisArea.Bottom();
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
                            aWriteSet[ind*4+nByte] = (sal_Int8) nVal % 0x100;
                            nVal /= 0x100;
                        }
                    }

                    bExtentSuccess = ( xExtStm->Write( aWriteSet, 16 ) == 16 );
                }
            }

            if ( bExtentSuccess )
            {
                SotStorageStreamRef xEmbStm = rDestStg.OpenSotStream(
                                                String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "package_stream" ) ),
                                                STREAM_STD_READWRITE );
                if( !xEmbStm->GetError() )
                {
                    try
                    {
                        if ( rObj->getCurrentState() == embed::EmbedStates::LOADED )
                            rObj->changeState( embed::EmbedStates::RUNNING );
                        //TODO/LATER: is stream instead of outputstream a better choice?!
                        //TODO/LATER: a "StoreTo" method at embedded object would be nice
                        uno::Sequence < beans::PropertyValue > aSeq(1);
                        aSeq[0].Name = ::rtl::OUString::createFromAscii( "OutputStream" );
                        ::uno::Reference < io::XOutputStream > xOut = new ::utl::OOutputStreamWrapper( *xEmbStm );
                        aSeq[0].Value <<= xOut;
                        uno::Reference < frame::XStorable > xStor( rObj->getComponent(), uno::UNO_QUERY );
                        xStor->storeToURL( ::rtl::OUString::createFromAscii( "private:stream" ), aSeq );
                    }
                    catch( uno::Exception& )
                    {
                        // TODO/LATER: Error handling
                        DBG_ERROR( "The object could not be exported!" );
                    }
                }
            }
        }
        else
        {
            DBG_ERROR("Own binary format inside own container document!");
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
            ::rtl::OUString aTempName(::rtl::OUString::createFromAscii("bla"));
            try
            {
                xPers->storeToEntry( xStor, aTempName, aEmptySeq, aEmptySeq );
            }
            catch ( uno::Exception& )
            {}

            SotStorageRef xOLEStor = SotStorage::OpenOLEStorage( xStor, aTempName, STREAM_STD_READ );
            xOLEStor->CopyTo( &rDestStg );
            rDestStg.Commit();
        }
    }

    //We never need this stream: See #99809# and #i2179#
    rDestStg.Remove(CREATE_CONST_ASC(SVEXT_PERSIST_STREAM));
}



