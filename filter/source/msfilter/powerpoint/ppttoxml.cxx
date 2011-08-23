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
