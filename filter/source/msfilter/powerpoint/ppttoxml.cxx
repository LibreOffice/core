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
#include "precompiled_filter.hxx"

#include "ppttoxml.hxx"
#include <sot/storage.hxx>

PptToXml::PptToXml()
{
}

PptToXml::~PptToXml()
{
}

sal_Bool PptToXml::filter( const SEQ( NMSP_BEANS::PropertyValue )& aDescriptor,
                            REF( NMSP_SAX::XDocumentHandler ) xHandler )
{
    xHdl = xHandler;

    sal_Bool bStatus = sal_False;
    sal_Int32 i;
    for( i = 0; i < aDescriptor.getLength(); i++ )
    {
        NMSP_RTL::OUString strTemp;
        aDescriptor[ i ].Value >>= strTemp;

/* we will open the document by url, so the stream is not needed
        if( aDescriptor[i].Name == B2UCONST( "InputStream" ) )
        {
            REF( NMSP_IO::XInputStream ) rInputStream;
            aDescriptor[ i].Value >>= rInputStream;
        }
        else
*/
        if ( aDescriptor[ i ].Name == B2UCONST( "URL" ) )
        {
            NMSP_RTL::OUString sURL;
            aDescriptor[ i ].Value >>= sURL;
            SotStorageRef xStg( new SotStorage( sURL, STREAM_STD_READ, 0 ) );
            if ( xStg.Is() )
            {
                SotStorageStreamRef xDocStream( xStg->OpenSotStream( B2UCONST( "PowerPoint Document" ), STREAM_STD_READ ) );
                if( xDocStream.Is() )
                {
                    xDocStream->SetVersion( xStg->GetVersion() );
                    xDocStream->SetKey( xStg->GetKey() );

//                  xHdl->unknown( PPT_DTD_STRING );
                    xHdl->startDocument();


                    xHdl->endDocument();
                    bStatus = sal_True;
                }
            }
        }
    }
    return bStatus;
}

void PptToXml::cancel()
{

}
