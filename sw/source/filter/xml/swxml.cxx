/*************************************************************************
 *
 *  $RCSfile: swxml.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: mib $ $Date: 2001-01-17 10:55:18 $
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

    // Get data source ...
    DBG_ASSERT( pMedium, "There is the medium" );
    if( !pMedium )
        return ERR_SWG_READ_ERROR;

    Reference< io::XActiveDataSource > xSource;
    Reference< XInterface > xPipe;
    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    SvStorageStreamRef xDocStream;

     xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;

    SvStorage *pStorage = pMedium->GetStorage();
    if( pStorage )
    {
        pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage,
                                                     GRAPHICHELPER_MODE_READ,
                                                     sal_False );
        xGraphicResolver = pGraphicHelper;

        OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
        xDocStream = pStorage->OpenStream( sDocName,
                                  STREAM_READ | STREAM_NOCREATE );
        xDocStream->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xDocStream );
    }
    else
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
    Sequence < Any > aArgs( 1 );
    Any *pArgs = aArgs.getArray();
    *pArgs++ <<= xGraphicResolver;
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

        Reference<XUnoTunnel> xFilterTunnel( xFilter, UNO_QUERY );
        SwXMLImport *pFilter = (SwXMLImport *)xFilterTunnel->getSomething(
                                            SwXMLImport::getUnoTunnelId() );
        pFilter->setStyleInsertMode( nStyleFamilyMask, !aOpt.IsMerge() );
    }
    else if( bInsertMode )
    {
        Reference < XTextRange > xTextRange =
            SwXTextRange::CreateTextRangeFromPosition( &rDoc, *rPaM.GetPoint(),
                                                        0 );
        Reference<XUnoTunnel> xFilterTunnel( xFilter, UNO_QUERY );
        SwXMLImport *pFilter = (SwXMLImport *)xFilterTunnel->getSomething(
                                            SwXMLImport::getUnoTunnelId() );
        pFilter->setTextInsertMode( xTextRange );
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

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;
    rDoc.RemoveLink();

    return nRet;
}
