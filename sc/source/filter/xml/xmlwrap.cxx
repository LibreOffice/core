/*************************************************************************
 *
 *  $RCSfile: xmlwrap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-16 18:03:05 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <rsc/rscsfx.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <vos/xception.hxx>
#include <comphelper/processfactory.hxx>
#include <xmloff/xmlkywd.hxx>

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "document.hxx"
#include "xmlwrap.hxx"
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

ScXMLImportWrapper::ScXMLImportWrapper(ScDocument& rD, SfxMedium& rM) :
    rDoc(rD),
    rMedium(rM)
{
}

sal_Bool ScXMLImportWrapper::Import()
{
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( !xServiceFactory.is() )
        return sal_False;

    // Get data source ...
    uno::Reference<io::XActiveDataSource> xSource = rMedium.GetDataSource();
    DBG_ASSERT( xSource.is(), "got no data source from medium" );

    if( !xSource.is() )
    {
//      // If we didn't get a data source from a medium, we have to create
//      // one.
//      uno::Reference<uno::XInterface> xFactory =
//          xServiceFactory->createInstance(
//                          L"com.sun.star.frame.DataSourceFactory" );
//      if( xFactory.is() )
//      {
//          XMultiServiceFactoryRef xMFactory( xFactory, uno::UNO_QUERY );
//          if( xMFactory.is() )
//          {
//              UString sURL( mrMedium.GetName() );
//              Sequence<UsrAny> aArgs(1);
//              UsrAny* pArgs = aArgs.getArray();
//              pArgs->setString( sURL );
//
//              uno::Reference<uno::XInterface> xSrc = xMFactory->createInstanceWithArguments(
//                                      sURL, aArgs );
//              if( xSrc.is() )
//                  xSrc->queryInterface( XActiveDataSource::getSmartUik(),
//                                        xSource );
//          }
//      }
    }

    // get data source
    DBG_ASSERT( xSource.is(), "data source missing" );
    if( !xSource.is() )
        return sal_False;

    // get parser
    uno::Reference<uno::XInterface> xXMLParser =
        xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" )) );
    DBG_ASSERT( xXMLParser.is(), "com.sun.star.xml.sax.Parser service missing" );
    if( !xXMLParser.is() )
        return sal_False;

    // get a pipe for connecting the data source to the parser
    uno::Reference<uno::XInterface> xPipe =
        xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.Pipe" )) );
    DBG_ASSERT( xPipe.is(), "com.sun.star.io.Pipe service missing" );
    if( !xPipe.is() )
        return sal_False;

    sal_uInt16 nStyleFamilyMask(0);
//  USHORT nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
    sal_Bool bLoadDoc(sal_True);
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

    // get filter
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    if ( pObjSh )
    {
        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();

        uno::Reference<xml::sax::XDocumentHandler> xFilter = new ScXMLImport(
            xModel, bLoadDoc, nStyleFamilyMask);

        // connect pipe's output stream to the data source
        uno::Reference<io::XOutputStream> xPipeOutput( xPipe, uno::UNO_QUERY );
        xSource->setOutputStream( xPipeOutput );

        // connect pipe's input stream to the parser
        xml::sax::InputSource aParserInput;
        uno::Reference<io::XInputStream> xPipeInput( xPipe, uno::UNO_QUERY );
        aParserInput.aInputStream = xPipeInput;
        aParserInput.sSystemId = OUString(rMedium.GetName());

        // connect parser and filter
        uno::Reference<xml::sax::XParser> xParser( xXMLParser, uno::UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

        // parse
        uno::Reference<io::XActiveDataControl> xSourceControl( xSource, uno::UNO_QUERY );
        xSourceControl->start();
        sal_Bool bRetval(sal_True);

        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException e )
        {
            bRetval = sal_False;
        }
        catch( xml::sax::SAXException e )
        {
            bRetval = sal_False;
        }
        catch( io::IOException e )
        {
            bRetval = sal_False;
        }

        return bRetval;
    }
    return sal_False;
}

sal_Bool ScXMLImportWrapper::Export()
{
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( !xServiceFactory.is() )
        return sal_False;

    uno::Reference<uno::XInterface> xWriter =
        xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" )) );
    DBG_ASSERT( xWriter.is(), "com.sun.star.xml.sax.Writer service missing" );
    if(!xWriter.is())
        return sal_False;

    uno::Reference<io::XOutputStream> xOut = rMedium.GetDataSink();
    uno::Reference<io::XActiveDataSource> xSrc( xWriter, uno::UNO_QUERY );
    xSrc->setOutputStream( xOut );

    uno::Reference<xml::sax::XDocumentHandler> xHandler( xWriter, uno::UNO_QUERY );

    OUString sFileName = rMedium.GetName();
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    if ( pObjSh )
    {
        pObjSh->UpdateDocInfoForSave();     // update information

        LanguageType eDocLang = rDoc.GetLanguage();
        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();

        ScXMLExport *pExp = new ScXMLExport( xModel, sFileName, xHandler, sal_False );

        sal_Bool bRet = (0 == pExp->exportDoc( sXML_spreadsheet ));

        delete pExp;

        return bRet;
    }

    // later: give string descriptor as parameter for doc type

    return sal_False;
}



