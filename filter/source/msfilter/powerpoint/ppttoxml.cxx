/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ppttoxml.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:41:48 $
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
#include "precompiled_filter.hxx"

#include "ppttoxml.hxx"

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

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
