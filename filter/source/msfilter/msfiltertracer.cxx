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
#include <filter/msfilter/msfiltertracer.hxx>
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/logging/LogLevel.hpp>
#include <com/sun/star/util/SearchAlgorithms.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <svtools/FilterConfigItem.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>


// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star;

MSFilterTracer::MSFilterTracer( const ::rtl::OUString& rConfigPath, uno::Sequence< beans::PropertyValue >* pConfigData ) :
    mpCfgItem( new FilterConfigItem( rConfigPath, pConfigData ) ),
    mpAttributeList( new SvXMLAttributeList() ),
    mpStream( NULL ),
    mbEnabled( sal_False )	// will be set to true in StartTracing()
{
    if ( mpCfgItem->ReadBool( rtl::OUString::createFromAscii( "On" ), sal_False ) )
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        if ( xMgr.is() )
        {
            /* the following methods try to read a property, if it is not available it will put the second
            parameter as default into the property sequence of the FilterConfigItem. It means we ensure that
            the property is available by trying to read it (the return value of the method is ignored) */
            ::rtl::OUString aEmptyString;
            mpCfgItem->ReadInt32( rtl::OUString::createFromAscii( "LogLevel" ), util::logging::LogLevel::ALL );
            mpCfgItem->ReadString( rtl::OUString::createFromAscii( "ClassFilter" ), aEmptyString );
            mpCfgItem->ReadString( rtl::OUString::createFromAscii( "MethodFilter" ), aEmptyString );
            mpCfgItem->ReadString( rtl::OUString::createFromAscii( "MessageFilter" ), aEmptyString );
            util::SearchAlgorithms eSearchAlgorithm = (util::SearchAlgorithms)
                mpCfgItem->ReadInt32( rtl::OUString::createFromAscii( "SearchAlgorithm" ), util::SearchAlgorithms_ABSOLUTE );

            // creating the name of the log file
            rtl::OUString aPath( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "Path" ), aEmptyString ) );
            rtl::OUString aName( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "Name" ), aEmptyString ) );
            rtl::OUString aDocumentURL( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "DocumentURL" ), aEmptyString ) );
            INetURLObject aLogFile( aDocumentURL );
            if ( aLogFile.GetMainURL( INetURLObject::NO_DECODE ).getLength() )
            {
                if ( aPath.getLength() )
                {
                    String aOldName( aLogFile.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::NO_DECODE ) );
                    aLogFile = INetURLObject( aPath );
                    aLogFile.insertName( aOldName );
                }
                if ( aName.getLength() )
                    aLogFile.setName( aName );
            }
            else
            {
                if ( aPath.getLength() )
                    aLogFile = INetURLObject( aPath );
                else
                {
                    String aURLStr;
                    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aURLStr ) )
                    {
                        aLogFile = INetURLObject(aURLStr);
                        aLogFile .removeSegment();
                        aLogFile .removeFinalSlash();
                    }
                }
                if ( !aName.getLength() )
                    aName = rtl::OUString::createFromAscii( "tracer" );
                aLogFile.insertName( aName );
            }
            aLogFile.setExtension( rtl::OUString::createFromAscii( "log" ) );

            // creating the file stream
            mpStream = ::utl::UcbStreamHelper::CreateStream( aLogFile.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYNONE );
            if ( mpStream && !mpStream->GetError() )
            {
                // creating a wrapper for our stream
                utl::OOutputStreamWrapper* pHelper = new ::utl::OOutputStreamWrapper( *mpStream );
                uno::Reference< io::XOutputStream > xOutputStream( pHelper );

                // instanciating the DocumentHandler, then setting the OutputStream
                mxHandler = uno::Reference< xml::sax::XDocumentHandler >( xMgr->createInstance( rtl::OUString::createFromAscii( "com.sun.star.xml.sax.Writer" ) ), uno::UNO_QUERY );
                uno::Reference< io::XActiveDataSource > xDocSrc( mxHandler, uno::UNO_QUERY );
                xDocSrc->setOutputStream( xOutputStream );
                mxHandler->startDocument();
                mxHandler->ignorableWhitespace ( rtl::OUString::createFromAscii( " " ) );

                // writing the "DocumentHandler" property, so the FilterTracer component
                // will use it for the output
                uno::Any aAny;
                aAny <<= xDocSrc;
                mpCfgItem->WriteAny( rtl::OUString::createFromAscii( "DocumentHandler" ), aAny );

                SvXMLAttributeList* pAttrList = new SvXMLAttributeList;
                pAttrList->AddAttribute( rtl::OUString::createFromAscii( "DocumentURL" ), aDocumentURL );
                uno::Reference < xml::sax::XAttributeList > xAttributeList(pAttrList);
                mxHandler->startElement( rtl::OUString::createFromAscii( "Document" ), xAttributeList );
            }

            uno::Sequence< uno::Any > aArgument( 1 );
            uno::Sequence< beans::PropertyValue > aPropValues( mpCfgItem->GetFilterData() );
            aArgument[ 0 ] <<= aPropValues;
            mxFilterTracer = xMgr->createInstanceWithArguments( rtl::OUString::createFromAscii( "com.sun.star.util.FilterTracer" ), aArgument );
            if ( mxFilterTracer.is() )
            {				
                mxTextSearch = uno::Reference< util::XTextSearch >( mxFilterTracer, uno::UNO_QUERY );
                mxLogger = uno::Reference< util::logging::XLogger >( mxFilterTracer, uno::UNO_QUERY );
                if ( mxTextSearch.is() )
                {
                    maSearchOptions.algorithmType = eSearchAlgorithm;
                    mxTextSearch->setOptions( maSearchOptions );
                }
            }
        }
    }
}

MSFilterTracer::~MSFilterTracer()
{
    mxLogger = NULL;
    mxFilterTracer = NULL;
    if ( mxHandler.is() )
    {
        mxHandler->ignorableWhitespace ( rtl::OUString::createFromAscii( " " ) );
        mxHandler->endElement( rtl::OUString::createFromAscii( "Document" ) );
        mxHandler->ignorableWhitespace ( rtl::OUString::createFromAscii( " " ) );
        mxHandler->endDocument();
        mxHandler = NULL;
    }
    delete mpAttributeList;
    delete mpCfgItem;
    delete mpStream;
}

void MSFilterTracer::StartTracing()
{
    mbEnabled = mpCfgItem->ReadBool( rtl::OUString::createFromAscii( "On" ), sal_False );
}

void MSFilterTracer::EndTracing()
{
    mbEnabled = sal_False;
}

void MSFilterTracer::StartElement( const rtl::OUString& rName, uno::Reference< xml::sax::XAttributeList > xAttribs )
{
    if ( mxHandler.is() )
        mxHandler->startElement( rName, xAttribs );
}

void MSFilterTracer::EndElement( const rtl::OUString& rName )
{
    if ( mxHandler.is() )
        mxHandler->endElement( rName );
}

void MSFilterTracer::Trace( const rtl::OUString& rElement, const rtl::OUString& rMessage )
{
    if ( mbEnabled && mxLogger.is() )
    {
        sal_Bool bFilter = sal_False;
        if ( rMessage.getLength() && mxTextSearch.is() )
        {
            maSearchOptions.searchString = rMessage;
            mxTextSearch->setOptions(  maSearchOptions );
            util::SearchResult aSearchResult = mxTextSearch->searchForward( rMessage, 0, rMessage.getLength() );
            bFilter = aSearchResult.subRegExpressions != 0;
        }
        if ( !bFilter )
        {
            uno::Reference < xml::sax::XAttributeList > xAttrList( new SvXMLAttributeList( *mpAttributeList ) );
            if ( mxHandler.is() )
                mxHandler->startElement( rElement, xAttrList );
            if ( rMessage.getLength() )
            {
                rtl::OUString aEmpty;
                mxLogger->logp( 0, aEmpty, aEmpty, rMessage );
            }
            if ( mxHandler.is() )
                mxHandler->endElement( rElement );
        }
    }
}

void MSFilterTracer::AddAttribute( const ::rtl::OUString& sName , const ::rtl::OUString& sValue )
{
    if ( mbEnabled )
        mpAttributeList->AddAttribute( sName, sValue );
}
void MSFilterTracer::ClearAttributes()
{
    if ( mbEnabled )
        mpAttributeList->Clear();
}

void MSFilterTracer::RemoveAttribute( const ::rtl::OUString& sName )
{
    if ( mbEnabled )
        mpAttributeList->RemoveAttribute( sName );
}

uno::Any MSFilterTracer::GetProperty( const rtl::OUString& rPropName, const uno::Any* pDefault ) const
{
    uno::Any aDefault;
    if ( pDefault )
        aDefault = *pDefault;
    return mpCfgItem->ReadAny( rPropName, aDefault );
}

void MSFilterTracer::SetProperty( const ::rtl::OUString& rPropName, const uno::Any& rProperty )
{
    mpCfgItem->WriteAny( rPropName, rProperty );
}

