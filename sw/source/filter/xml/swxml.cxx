/*************************************************************************
 *
 *  $RCSfile: swxml.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: mtg $ $Date: 2001-02-28 12:41:01 $
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


#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATACONTROL_HPP_
#include <com/sun/star/io/XActiveDataControl.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif

#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <sfx2/docfile.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif
#ifndef _FLTINI_HXX //autogen wg. XMLReader
#include <fltini.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX //autogen wg. SwDoc
#include <docsh.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <svx/xmlgrhlp.hxx>
#endif
#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

XMLReader::XMLReader()
{
}

int XMLReader::GetReaderType()
{
    return SW_STORAGE_READER | SW_STREAM_READER;
}

sal_uInt32 XMLReader::Read( SwDoc &rDoc, SwPaM &rPaM, const String & rName )
{
    // Get service factory
    Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_READ_ERROR;

    Reference< io::XActiveDataSource > xSource;
    Reference< XInterface > xPipe;
    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;
    SvStorageStreamRef xDocStream;

     xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;

    SvStorage *pStorage = 0;
    if( pMedium )
        pStorage = pMedium->GetStorage();
    else
        pStorage = pStg;
    if( pStorage )
    {
        pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage,
                                                     GRAPHICHELPER_MODE_READ,
                                                     sal_False );
        xGraphicResolver = pGraphicHelper;

        SvPersist *pPersist = rDoc.GetPersist();
        if( pPersist )
        {
            pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                           *pStorage, *pPersist,
                                        EMBEDDEDOBJECTHELPER_MODE_READ,
                                        sal_False );
            xObjectResolver = pObjectHelper;
        }

        OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
        xDocStream = pStorage->OpenStream( sDocName,
                                  STREAM_READ | STREAM_NOCREATE );
        xDocStream->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xDocStream );
    }
    else if( pMedium )
    {
        // if there is a medium and if this medium has a load environment,
        // we get an active data source from the medium.
        pMedium->GetInStream()->Seek( 0 );
        xSource = pMedium->GetDataSource();
        ASSERT( xSource.is(), "XMLReader:: got no data source from medium" );
        if( !xSource.is() )
            return ERR_SWG_READ_ERROR;

        // get a pipe for connecting the data source to the parser
        xPipe = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.io.Pipe") );
        ASSERT( xPipe.is(),
                "XMLReader::Read: com.sun.star.io.Pipe service missing" );
        if( !xPipe.is() )
            return ERR_SWG_READ_ERROR;

        // connect pipe's output stream to the data source
        Reference< io::XOutputStream > xPipeOutput( xPipe, UNO_QUERY );
        xSource->setOutputStream( xPipeOutput );

        aParserInput.aInputStream = Reference< io::XInputStream >( xPipe,
                                                                   UNO_QUERY );
    }
    else
    {
        pStrm->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *pStrm );
    }

    // get parser
    Reference< xml::sax::XParser > xParser(
            xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
            UNO_QUERY );
    ASSERT( xParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
    if( !xParser.is() )
        return ERR_SWG_READ_ERROR;

    // get filter
    Sequence < Any > aArgs( 2 );
    Any *pArgs = aArgs.getArray();
    *pArgs++ <<= xGraphicResolver;
    *pArgs++ <<= xObjectResolver;
    Reference< xml::sax::XDocumentHandler > xFilter(
            xServiceFactory->createInstanceWithArguments(
                OUString::createFromAscii("com.sun.star.office.sax.importer.Writer"),
                aArgs ),
            UNO_QUERY );
    ASSERT( xFilter.is(),
            "XMLReader::Read: com.sun.star.xml.sax.importer.Writer service missing" );
    if( !xFilter.is() )
        return ERR_SWG_READ_ERROR;

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    // Get model
    SwDocShell *pDocSh = rDoc.GetDocShell();
    ASSERT( pDocSh, "XMLReader::Read: got no doc shell" );
    if( !pDocSh )
        return ERR_SWG_READ_ERROR;

    Reference< lang::XComponent > xModelComp( pDocSh->GetModel(), UNO_QUERY );
    ASSERT( xModelComp.is(),
            "XMLReader::Read: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_READ_ERROR;

    // connect model and filter
    Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComp );

    Reference<XUnoTunnel> xFilterTunnel;
    SwXMLImport *pFilter = 0;
    if( aOpt.IsFmtsOnly() || bInsertMode || IsBlockMode() )
    {
        xFilterTunnel = Reference<XUnoTunnel>( xFilter, UNO_QUERY );
        pFilter = (SwXMLImport *)xFilterTunnel->getSomething(
                                            SwXMLImport::getUnoTunnelId() );
    }
    if( aOpt.IsFmtsOnly() )
    {
        sal_uInt16 nStyleFamilyMask = 0U;
        if( aOpt.IsFrmFmts() )
            nStyleFamilyMask |= SFX_STYLE_FAMILY_FRAME;
        if( aOpt.IsPageDescs() )
            nStyleFamilyMask |= SFX_STYLE_FAMILY_PAGE;
        if( aOpt.IsTxtFmts() )
            nStyleFamilyMask |= (SFX_STYLE_FAMILY_CHAR|SFX_STYLE_FAMILY_PARA);
        if( aOpt.IsNumRules() )
            nStyleFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;

        ASSERT( pFilter, "There is the filter?" );
        pFilter->setStyleInsertMode( nStyleFamilyMask, !aOpt.IsMerge() );
    }
    else if( bInsertMode )
    {
        Reference < XTextRange > xTextRange =
            SwXTextRange::CreateTextRangeFromPosition( &rDoc, *rPaM.GetPoint(),
                                                        0 );
        ASSERT( pFilter, "There is the filter?" );
        pFilter->setTextInsertMode( xTextRange );
    }
    else if( IsBlockMode() )
    {
        ASSERT( pFilter, "There is the filter?" );
        pFilter->setBlockMode();
    }

    aOpt.ResetAllFmtsOnly();

    rDoc.AddLink(); // prevent deletion
    sal_uInt32 nRet = 0;

    // parse
    if( xSource.is() )
    {
        Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
        xSourceControl->start();
    }
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& r )
    {
        String sErr( String::CreateFromInt32( r.LineNumber ));
        sErr += ',';
        sErr += String::CreateFromInt32( r.ColumnNumber );

        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }
    catch( xml::sax::SAXException& r )
    {
        nRet = ERR_SWG_READ_ERROR;
    }
    catch( io::IOException& r )
    {
        nRet = ERR_SWG_READ_ERROR;
    }

    // import autotext events
    if ((NULL != pStorage) && IsBlockMode())
    {
        OUString sStreamName(RTL_CONSTASCII_USTRINGPARAM(
            "AutoTextEvents.xml"));
        OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.office.sax.importer.AutoTextEventReader"));

        // prepare ParserInputSrouce
        xml::sax::InputSource aEventsParserInput;
        aEventsParserInput.sSystemId = rName;

        // open stream (and set parser input)
        SvStorageStreamRef xEventsStream;
        xEventsStream = pStorage->OpenStream( sStreamName,
                                             STREAM_READ | STREAM_NOCREATE );
        xEventsStream->SetBufferSize( 16*1024 );
        aEventsParserInput.aInputStream =
            new utl::OInputStreamWrapper( *xEventsStream );

        // get parser
        Reference< xml::sax::XParser > xEventsParser(
            xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
            UNO_QUERY );
        ASSERT( xEventsParser.is(),
                "Can't create parser" );
        if( !xEventsParser.is() )
            return ERR_SWG_READ_ERROR;

        // get filter
        Sequence < Any > aArgs( 0 );
        Reference< xml::sax::XDocumentHandler > xEventsFilter(
            xServiceFactory->createInstance(sServiceName), UNO_QUERY );
        ASSERT( xEventsFilter.is(),
                "Can't instantiate auto text events reader." );
        if( !xEventsFilter.is() )
            return ERR_SWG_READ_ERROR;

        // connect parser and filter
        xEventsParser->setDocumentHandler( xEventsFilter );

        // connect model and filter
        Reference < XImporter > xEventsImporter( xEventsFilter, UNO_QUERY );
        xEventsImporter->setTargetDocument( xModelComp );

        try
        {
            xEventsParser->parseStream( aEventsParserInput );
        }
        catch( xml::sax::SAXParseException& r )
        {
            nRet = ERR_SWG_READ_ERROR;
        }
        catch( xml::sax::SAXException& r )
        {
            nRet = ERR_SWG_READ_ERROR;
        }
        catch( io::IOException& r )
        {
            nRet = ERR_SWG_READ_ERROR;
        }
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;
    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;
    rDoc.RemoveLink();

    return nRet;
}
