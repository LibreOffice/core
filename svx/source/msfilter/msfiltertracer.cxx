/*************************************************************************
 *
 *  $RCSfile: msfiltertracer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-07 15:25:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MS_FILTERTRACER_HXX
#include "msfiltertracer.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/util/logging/LogLevel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHALGORITHMS_HPP_
#include <com/sun/star/util/SearchAlgorithms.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _FILTER_CONFIG_ITEM_HXX_
#include <svtools/FilterConfigItem.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif


// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star;

MSFilterTracer::MSFilterTracer( const ::rtl::OUString& rConfigPath, ::uno::Sequence< ::beans::PropertyValue >* pConfigData ) :
    mpCfgItem( new FilterConfigItem( rConfigPath, pConfigData ) ),
    mpAttributeList( new SvXMLAttributeList() ),
    mpStream( NULL ),
    mbEnabled( sal_False )  // will be set to true in StartTracing()
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
            String aPath( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "Path" ), aEmptyString ) );
            String aName( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "Name" ), aEmptyString ) );
            String aDocumentURL( mpCfgItem->ReadString( rtl::OUString::createFromAscii( "DocumentURL" ), aEmptyString ) );
            INetURLObject aLogFile( aDocumentURL );
            if ( aLogFile.GetMainURL( INetURLObject::NO_DECODE ).Len() )
            {
                if ( aPath.Len() )
                {
                    String aOldName( aLogFile.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::NO_DECODE ) );
                    aLogFile = INetURLObject( aPath );
                    aLogFile.insertName( aOldName );
                }
                if ( aName.Len() )
                    aLogFile.setName( aName );
            }
            else
            {
                if ( aPath.Len() )
                    aLogFile = INetURLObject( aPath );
                else
                {
                    String aURLStr;
                    if( ::utl::LocalFileHelper::ConvertPhysicalNameToURL( Application::GetAppFileName(), aURLStr ) )
                    {
                        aLogFile = aURLStr;
                        aLogFile .removeSegment();
                        aLogFile .removeFinalSlash();
                    }
                }
                if ( !aName.Len() )
                    aName = rtl::OUString::createFromAscii( "tracer" );
                aLogFile.insertName( aName );
            }
            aLogFile.setExtension( rtl::OUString::createFromAscii( "log" ) );

            // creating the file stream
            mpStream = ::utl::UcbStreamHelper::CreateStream( aLogFile.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC | STREAM_SHARE_DENYNONE );
            if ( mpStream )
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
    mxHandler->startElement( rName, xAttribs );
}

void MSFilterTracer::EndElement( const rtl::OUString& rName )
{
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
            mxHandler->startElement( rElement, xAttrList );
            if ( rMessage.getLength() )
            {
                rtl::OUString aEmpty;
                mxLogger->logp( 0, aEmpty, aEmpty, rMessage );
            }
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

