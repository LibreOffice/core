/*************************************************************************
 *
 *  $RCSfile: sdxmlwrp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mib $ $Date: 2000-12-03 09:12:03 $
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

#pragma hdrstop

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SDXMLIMP_HXX
#include <xmloff/sdxmlimp.hxx>
#endif
#ifndef _SDXMLEXP_HXX
#include <xmloff/sdxmlexp.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <svx/xmlgrhlp.hxx>
#endif

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "sdxmlwrp.hxx"
#include "pkgurl.hxx"

using namespace com::sun::star;
using namespace rtl;

//////////////////////////////////////////////////////////////////////////////

char __READONLY_DATA sXML_drawing[] = "drawing";
char __READONLY_DATA sXML_impress[] = "presentation";
char __READONLY_DATA sXML_content[] = "Content";

// ----------------
// - SdXMLWrapper -
// ----------------

SdXMLWrapper::SdXMLWrapper( uno::Reference<frame::XModel>& xRef,
                            SfxMedium& rMedium, BOOL bIsDraw, BOOL bShowProg ) :
    mxLocalModel    ( xRef ),
    mrMedium        ( rMedium ),
    mbIsDraw        ( bIsDraw ),
    mbShowProgress  ( bShowProg )
{
}

// -----------------------------------------------------------------------------

BOOL SdXMLWrapper::Import()
{
    if( !mxLocalModel.is() )
    {
        DBG_ERROR("Got NO Model in XMLImport");
        return FALSE;
    }

    uno::Reference<lang::XServiceInfo> xServiceInfo(mxLocalModel, uno::UNO_QUERY);

    if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) ) )
    {
        DBG_ERROR( "Model is no DrawingDocument in XMLImport" );
        return FALSE;
    }

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

    if( !xServiceFactory.is() )
    {
        DBG_ERROR( "XMLReader::Read: got no service manager" );
        return FALSE;
    }

    uno::Reference< uno::XInterface> xXMLParser( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ) ) ) );

    if( !xXMLParser.is() )
    {
        DBG_ERROR( "com.sun.star.xml.sax.Parser service missing" );
        return FALSE;
    }

    UINT16  nStyleFamilyMask = 0;
    BOOL    bLoadDoc = TRUE, bRet = FALSE;

// this is stuff for loading only styles or add-load documents, needed later
//  USHORT nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
//  BOOL bInsert;
//  if( aOpt.IsFmtsOnly() )
//  {
//      bLoadDoc = FALSE;
//      bInsert = aOpt.IsMerge();
//      nStyleFamilyMask = 0U;
//      if( aOpt.IsFrmFmts() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_FRAME;
//      if( aOpt.IsPageDescs() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_PAGE;
//      if( aOpt.IsTxtFmts() )
//          nStyleFamilyMask |= (SFX_STYLE_FAMILY_CHAR|SFX_STYLE_FAMILY_PARA);
//      if( aOpt.IsNumRules() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;
//  }
//  else
//  {
//      bLoadDoc = TRUE;
//      bInsert = bInsertMode;
//      nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
//  }
//  aOpt.ResetAllFmtsOnly();

    try
    {
        xml::sax::InputSource                   aParserInput;
        SvStorageStreamRef                      xIStm;
        SvStorage*                              pStorage = mrMedium.GetStorage();
        uno::Reference< io::XActiveDataSource > xSource;

        aParserInput.sSystemId = mrMedium.GetName();

        if( pStorage )
        {

            static const String aContentStmName( RTL_CONSTASCII_USTRINGPARAM( sXML_content ) );

            xIStm = pStorage->OpenStream( aContentStmName, STREAM_READ | STREAM_NOCREATE );

            if( xIStm.Is() )
            {
                xIStm->SetBufferSize( 16 * 1024 );
                aParserInput.aInputStream = new utl::OInputStreamWrapper( *xIStm );
            }
            else
            {
                DBG_ERROR( "could not open Content stream" );
                return FALSE;
            }
        }
        else
        {
            uno::Reference< uno::XInterface > xPipe;

            mrMedium.GetInStream()->Seek( 0 );

            xSource = mrMedium.GetDataSource();
            DBG_ASSERT( xSource.is(), "got no data source from medium" );

            if( !xSource.is() )
                return sal_False;

            xPipe = xServiceFactory->createInstance( OUString::createFromAscii( "com.sun.star.io.Pipe" ) );
            DBG_ASSERT( xPipe.is(), "com.sun.star.io.Pipe service missing" );

            if( !xPipe.is() )
                return sal_False;

            xSource->setOutputStream( uno::Reference< io::XOutputStream >( xPipe, uno::UNO_QUERY ) );
            aParserInput.aInputStream = uno::Reference< io::XInputStream >( xPipe, uno::UNO_QUERY );
        }

        if( aParserInput.aInputStream.is() )
        {
            SvXMLGraphicHelper*                             pGraphicHelper;
            uno::Reference< container::XIndexContainer >    xGrfContainer;
            uno::Reference< xml::sax::XParser >             xParser( xXMLParser, uno::UNO_QUERY );

            if( pStorage )
            {
                pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_READ );
                xGrfContainer = pGraphicHelper;
            }

            xParser->setDocumentHandler( new SdXMLImport( mxLocalModel, xGrfContainer,
                                                          bLoadDoc, nStyleFamilyMask,
                                                          mbShowProgress, IsDraw() ) );

            if( !pStorage )
                uno::Reference< io::XActiveDataControl >( xSource, uno::UNO_QUERY )->start();

            xParser->parseStream( aParserInput );

            if( pStorage )
                SvXMLGraphicHelper::Destroy( pGraphicHelper );

            bRet = TRUE;
        }
    }
    catch( xml::sax::SAXParseException )
    {
    }
    catch( xml::sax::SAXException )
    {
    }
    catch( io::IOException )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL SdXMLWrapper::Export()
{
    BOOL bRet = FALSE;

    if( !mxLocalModel.is() )
    {
        DBG_ERROR("Got NO Model in XMLExport");
        return FALSE;
    }

    uno::Reference< lang::XServiceInfo > xServiceInfo( mxLocalModel, uno::UNO_QUERY );

    if( !xServiceInfo.is() || !xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.drawing.DrawingDocument" ) ) ) )
    {
        DBG_ERROR( "Model is no DrawingDocument in XMLExport" );
        return FALSE;
    }

    uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );

    if( !xServiceFactory.is() )
    {
        DBG_ERROR( "got no service manager" );
        return FALSE;
    }

    uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );

    if( !xWriter.is() )
    {
        DBG_ERROR( "com.sun.star.xml.sax.Writer service missing" );
        return FALSE;
    }

    SvStorage* pStorage = mrMedium.GetOutputStorage( sal_True );

    if( pStorage )
    {
        SvStorageStreamRef  xOStm( pStorage->OpenStream( String( RTL_CONSTASCII_USTRINGPARAM( "Content" ) ),
                                                         STREAM_READ | STREAM_WRITE | STREAM_TRUNC ) );

        if( !xOStm.Is() || xOStm->GetError() )
        {
            DBG_ERROR( "Could not create output stream" );
            return FALSE;
        }

        uno::Reference< io::XActiveDataSource >     xSrc( xWriter, uno::UNO_QUERY );
        uno::Reference<xml::sax::XDocumentHandler>  xHandler( xWriter, uno::UNO_QUERY );

        xSrc->setOutputStream( new ::utl::OOutputStreamWrapper( *xOStm ) );

        try
        {
            SvXMLGraphicHelper* pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_WRITE, FALSE );
            SdXMLExport         aExp( mxLocalModel, mrMedium.GetName(), xHandler, pGraphicHelper, mbShowProgress, IsDraw() );

            bRet = ( 0 == aExp.exportDoc( IsDraw() ? sXML_drawing : sXML_impress ) );

            SvXMLGraphicHelper::Destroy( pGraphicHelper );
        }
        catch( xml::sax::SAXParseException )
        {
        }
        catch( xml::sax::SAXException )
        {
        }
        catch( io::IOException )
        {
        }
    }

    return bRet;
}
