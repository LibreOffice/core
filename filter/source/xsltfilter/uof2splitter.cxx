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
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
//This file is about the conversion of the UOF v2.0 and ODF document format
#include "precompiled_filter.hxx"


#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>

#include "uof2splitter.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::xml::sax;
using namespace osl;

namespace XSLT {

UOF2Splitter::UOF2Splitter( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rMultiFactory,
                         const ::rtl::OUString & rURL )
: m_rMultiFactory( rMultiFactory )
,aURL( rURL )
,m_aThread( 0 )
{
}

UOF2Splitter::~UOF2Splitter()
{
    if( m_aThread )
    {
        osl_joinWithThread( m_aThread );
        osl_destroyThread( m_aThread );
    }
}

// XActiveDataControl
void UOF2Splitter::addListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener> & rxListener ) throw( ::com::sun::star::uno::RuntimeException)
{
    m_StreamListeners.insert( rxListener );
}

void UOF2Splitter::removeListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener> & rxListener ) throw( ::com::sun::star::uno::RuntimeException)
{
    m_StreamListeners.erase( rxListener );
}

void UOF2Splitter::start (void) throw( ::com::sun::star::uno::RuntimeException)
{
    if ( !m_xStream.is() )
        return;

    try {
        typedef  ::std::set< ::com::sun::star::uno::Reference< ::com::sun::star::io::XStreamListener > > set_Listeners;
        set_Listeners::const_iterator start_it = m_StreamListeners.begin();
        for( ; start_it != m_StreamListeners.end(); start_it++ )
        {
            Reference< XStreamListener > xListener = *start_it;
            xListener->started();
        }

        try{
            InputSource aInput;
            aInput.sSystemId = aURL;
            aInput.sPublicId = aURL;
            aInput.aInputStream = m_xInputStream;
            Reference< XParser > xParser( m_rMultiFactory->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser") ), UNO_QUERY );
            xParser->setDocumentHandler( new UOF2SplitHandler( m_rMultiFactory, m_xStream ) );
            xParser->parseStream( aInput );
        }
        catch( Exception& e)
        {
            OSL_ENSURE(0, ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        }
        set_Listeners::const_iterator close_it = m_StreamListeners.begin();
        for( ; close_it != m_StreamListeners.end(); close_it++ )
        {
            Reference< XStreamListener > xListener = *close_it;
            xListener->closed();
        }
    }
    catch( Exception& e)
    {
        OSL_ENSURE(0, ::rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
    }
}

void UOF2Splitter::terminate (void) throw( ::com::sun::star::uno::RuntimeException)
{
}

// XActiveDataSink
void UOF2Splitter::setInputStream ( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> &rxInputStream) throw( ::com::sun::star::uno::RuntimeException)
{
    m_xInputStream = rxInputStream;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream> UOF2Splitter::getInputStream(void) throw( ::com::sun::star::uno::RuntimeException)
{
    return m_xInputStream;
}

// XActiveDataStreamer
void UOF2Splitter::setStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream> &rxStream) throw( ::com::sun::star::uno::RuntimeException)
{
    m_xStream = rxStream;
}

::com::sun::star::uno::Reference< ::com::sun::star::io::XStream> UOF2Splitter::getStream (void) throw( ::com::sun::star::uno::RuntimeException)
{
    return m_xStream;
}

}
